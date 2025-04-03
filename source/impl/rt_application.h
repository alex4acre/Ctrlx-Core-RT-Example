#pragma once
#include "comm/datalayer/datalayer.h"
#include "common/scheduler/i_scheduler3.h"
#include <map> 
#include "../User/EtherCATUpdates.h"
#include "Logger.h"

    
class RTMemObject
      {
        public:
          uint32_t m_Rev;
          std::map<std::string,uint32_t> m_Map;
          std::string m_DatalayerPath;
          std::shared_ptr<comm::datalayer::IMemoryUser> m_Mem; 
          comm::datalayer::IDataLayerFactory3* m_localdatalayer;
          comm::datalayer::Variant _dataLayerMap; 
          bool m_MemOpen = false;
          bool m_MapRetrieved = false;
          bool m_MapValid = false;
          bool m_ReadMap = false;

          void _setData(std::string &path, comm::datalayer::IDataLayerFactory3* m_datalayer)
          {
            m_DatalayerPath = path;
            m_localdatalayer = m_datalayer;
          }
          void _closeMemory()
          {
            if(m_Mem){
              LOG_INFO("Closing Memory");
              m_localdatalayer->closeMemory(m_Mem); 
              m_MemOpen = false;
              m_MapRetrieved = false;
              m_MapValid = false;
              m_ReadMap = false;
              m_Mem = nullptr; 
            }
          }
          bool _openMemory(std::string &path, comm::datalayer::IDataLayerFactory3* m_datalayer)
          {
            m_localdatalayer = m_datalayer;
            m_DatalayerPath = path;
            LOG_INFO("Opening memory at %s.", m_DatalayerPath.c_str());
            auto result = m_localdatalayer->openMemory(m_Mem, path); 
            if (comm::datalayer::STATUS_FAILED(result))
            {
              LOG_ERROR("open Memory failed: %s at %s", result.toString(), m_DatalayerPath.c_str());
              return false;
            }
            else
              LOG_INFO("openMemory succeeded: %s at %s", result.toString(), m_DatalayerPath.c_str());
              m_MemOpen = true;
              return true;
          }
          bool _getMap()
          {
            LOG_INFO("Getting Map at %s.", m_DatalayerPath.c_str());
            auto result = m_Mem->getMemoryMap(_dataLayerMap);
            if (comm::datalayer::STATUS_FAILED(result))
              {
                LOG_ERROR("getMemoryMap failed: %s", m_DatalayerPath.c_str(), result.toString());
                return false;
              }
            else
              {
                LOG_INFO("getMemoryMap succeeded: %s", result.toString());
                m_MapRetrieved = true;
                return true;    
              }
            }
          bool _verifyMap()
          {
            LOG_INFO("Verifying Map at %s.", m_DatalayerPath.c_str());
            auto result = _dataLayerMap.verifyFlatbuffers(comm::datalayer::VerifyMemoryMapBuffer);
            if (comm::datalayer::STATUS_FAILED(result))
            {
              LOG_ERROR("Verifing map failed: %s", result.toString());
              return false;
            }
            else
            { 
              LOG_INFO("Verifing map Succeeded: %s", result.toString());
              m_MapValid = true;
              return true;
            }  
          }
            
          bool _readMap(comm::datalayer::IClient3* &m_client)
          {
            LOG_INFO("Reading Map at %s.", m_DatalayerPath.c_str());
            auto result = m_client->readSync(m_DatalayerPath + "/map", &_dataLayerMap);
            if (STATUS_FAILED(result))
              {
                LOG_ERROR("Failed to read node failed: %s", result);
                return false;
              }
            else
            {
              auto varMap = comm::datalayer::GetMemoryMap(_dataLayerMap.getData());
              m_Rev = varMap->revision();
              for(auto variables = varMap->variables()->begin(); variables!= varMap->variables()->end(); variables++){
                m_Map[variables->name()->str()] = variables->bitoffset(); 
              }
              LOG_INFO("Succesfully mapped %s", m_DatalayerPath.c_str());
              m_ReadMap = true;
              return true;
            }
          }  
          comm::datalayer::DlResult _beginAccess(uint8_t*& data)
          {
            if (m_MapRetrieved & m_MapValid & m_MemOpen & m_ReadMap)
            {
              return m_Mem->beginAccess(data, m_Rev);
            }
            else
            {
              LOG_WARNING("RT Data not yet available.")
              return comm::datalayer::DlResult::DL_FAILED;
            }
          }
          comm::datalayer::DlResult _EndAccess()
          { 
            if (m_Mem)
            {
              return m_Mem->endAccess();
            }
            else  
              return comm::datalayer::DlResult::DL_FAILED;
          }
      };

namespace Example{

  class RTApplication:public common::scheduler::ICallable
  {
    public:
      common::scheduler::SchedEventResponse execute(const common::scheduler::SchedEventType& eventType,
                                                    const common::scheduler::SchedEventPhase& eventPhase,
                                                    comm::datalayer::Variant& param); 
      void setDatalayer(comm::datalayer::IDataLayerFactory3* datalayerFactory);
      void resetDataLayer();

    private: 

      bool startFlag = false;
      comm::datalayer::IDataLayerFactory3* m_datalayer;
      comm::datalayer::IClient3* m_client; 
      std::shared_ptr<comm::datalayer::IMemoryUser> m_inputs_ECAT; 
      std::shared_ptr<comm::datalayer::IMemoryUser> m_outputs_ECAT;  
      std::shared_ptr<comm::datalayer::IMemoryUser> m_inputs_PLC; 
      std::shared_ptr<comm::datalayer::IMemoryUser> m_outputs_PLC;  
      uint32_t m_inputRev_ECAT; 
      uint32_t m_outputRev_ECAT; 
      uint32_t m_inputRev_PLC; 
      uint32_t m_outputRev_PLC; 
      std::map<std::string,uint32_t> m_inMap_ECAT; 
      std::map<std::string,uint32_t> m_outMap_ECAT; 
      std::map<std::string,uint32_t> m_inMap_PLC; 
      std::string m_datalayer_PLC_In = "plc/app/Application/realtime_data/PRG_RTDatalayer_GVL_Input";
      std::string m_datalayer_PLC_Out = "plc/app/Application/realtime_data/PRG_RTDatalayer_GVL_Output";
      std::string m_datalayer_ECAT_In = "fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/input";
      std::string m_datalayer_ECAT_Out = "fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/output";
      RTMemObject mMemManager_ECAT_IN;
      RTMemObject mMemManager_ECAT_OUT;
      RTMemObject mMemManager_PLC_IN;
      RTMemObject mMemManager_PLC_OUT;
      std::map<std::string,uint32_t> m_outMap_PLC; 
      void createClient(); 
      //void openMemory(std::shared_ptr<comm::datalayer::IMemoryUser>* mem, std::map<std::string,uint32_t>* mem_Map, uint32_t* m_Rev, std::string m_DatalayerPath);
      //void openMemory(std::shared_ptr<comm::datalayer::IMemoryUser> &mem, std::map<std::string,uint32_t>* mem_Map, uint32_t* m_Rev, std::string &m_DatalayerPath);
      bool openMemory_V2(std::shared_ptr<comm::datalayer::IMemoryUser> &mem, std::map<std::string,uint32_t>* mem_Map, uint32_t* m_Rev, std::string &m_DatalayerPath);
      void closeMemory(std::shared_ptr<comm::datalayer::IMemoryUser>* mem); 
      void destroyClient(); 
      uint calltocreateclient = 0;
  };

 
}