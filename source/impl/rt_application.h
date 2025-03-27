#pragma once
#include "comm/datalayer/datalayer.h"
#include "common/scheduler/i_scheduler3.h"
#include <map> 
#include "../User/EtherCATUpdates.h"

namespace Example{
  class RTApplication:public common::scheduler::ICallable
  {
    public:
      common::scheduler::SchedEventResponse execute(const common::scheduler::SchedEventType& eventType,
                                                    const common::scheduler::SchedEventPhase& eventPhase,
                                                    comm::datalayer::Variant& param); 
      void setDatalyer(comm::datalayer::IDataLayerFactory3* datalayerFactory);
      void resetDataLayer();
        
    private: 
      comm::datalayer::IDataLayerFactory3* m_datalayer;
      comm::datalayer::IClient3* m_client; 
      std::shared_ptr<comm::datalayer::IMemoryUser> m_inputs_ECAT; 
      std::shared_ptr<comm::datalayer::IMemoryUser> m_outputs_ECAT;  
      uint32_t m_inputRev_ECAT; 
      uint32_t m_outputRev_ECAT; 
      std::map<std::string,uint32_t> m_inMap_ECAT; 
      std::map<std::string,uint32_t> m_outMap_ECAT; 
      void createClient(); 
      void openMemory(std::shared_ptr<comm::datalayer::IMemoryUser> mem, std::map<std::string,uint32_t>* mem_Map, uint32_t* m_Rev, std::string m_DatalayerPath);
      void closeMemory(std::shared_ptr<comm::datalayer::IMemoryUser> mem); 
      void destroyClient(); 
  };
}