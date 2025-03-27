#include "rt_application.h"
#include "Logger.h"

namespace Example{
common::scheduler::SchedEventResponse RTApplication::execute(const common::scheduler::SchedEventType& eventType,
                                                            const common::scheduler::SchedEventPhase& eventPhase,
                                                            comm::datalayer::Variant& param)
{
  switch (eventType)
  {
    //if(eventType == common::scheduler::SchedEventType::SCHED_EVENT_TICK)
    case common::scheduler::SchedEventType::SCHED_EVENT_TICK:
    {
      //begin ECAT Input Access
      u_int8_t* inData; 
      u_int8_t* outData; 

      auto result = m_inputs_ECAT->beginAccess(inData, m_inputRev_ECAT); 
      if(result == DL_OK)
      {
        RTUpdate::AT(inData, m_inMap_ECAT);
      } 
      else
      {
        LOG_WARNING("Failed to open the input data!")
      } 
      //End ECAT Input Access
      m_inputs_ECAT->endAccess(); 

      //begin PLC Output Access
      result = m_outputs_PLC->beginAccess(outData, m_outputRev_PLC);
      if(result == comm::datalayer::DlResult::DL_OK)
        { 
          RTUpdate::PLC_OUT(outData, m_outMap_PLC);
        }
      else
      {
        LOG_WARNING("Failed to open the output data!")
      }  
      //End ECAT Input Access
      m_outputs_PLC->endAccess(); 
      
      
      //begin ECAT Output Access
      result = m_outputs_ECAT->beginAccess(outData, m_outputRev_ECAT);
      if(result == comm::datalayer::DlResult::DL_OK)
        { 
          RTUpdate::MDT(outData, m_outMap_ECAT);
        }
      else
      {
        LOG_WARNING("Failed to open the output data!")
      }  
      //End ECAT Output Access
      m_outputs_ECAT->endAccess(); 
      
      
      //begin PLC Input Access
      result = m_inputs_PLC->beginAccess(inData, m_inputRev_PLC); 
      if(result == DL_OK)
      {
        RTUpdate::PLC_IN(inData, m_inMap_PLC);
      } 
      else
      {
        LOG_WARNING("Failed to open the input data!")
      }
      //End PLC Input Access
      m_inputs_PLC->endAccess(); 
      
      

      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
    }

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_SERVICE:
    {
      m_outputs_ECAT->endAccess();
      m_inputs_ECAT->endAccess(); 
      //m_outputs_PLC->endAccess();
      //m_inputs_PLC->endAccess(); 
      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
    }
  }
}
void RTApplication::setDatalyer(comm::datalayer::IDataLayerFactory3* datalayerFactory){
  m_datalayer = datalayerFactory; 
  createClient(); 
  std::string datalayerPath = "fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/input";
  openMemory(&m_inputs_ECAT, &m_inMap_ECAT, &m_inputRev_ECAT, datalayerPath); 
  datalayerPath = "fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/output";
  openMemory(&m_outputs_ECAT, &m_outMap_ECAT, &m_outputRev_ECAT, datalayerPath); 


  //Open PLC memory
  openMemory(&m_inputs_PLC, &m_inMap_PLC, &m_inputRev_PLC, m_datalayer_PLC_In); 
  openMemory(&m_outputs_PLC, &m_outMap_PLC, &m_outputRev_PLC, m_datalayer_PLC_Out); 
  
}

void RTApplication::resetDataLayer(){
  //Close the etherCAT memory
  closeMemory(&m_inputs_ECAT); 
  closeMemory(&m_outputs_ECAT); 
  //Close the PLC memory
  closeMemory(&m_inputs_PLC); 
  closeMemory(&m_outputs_PLC); 
  destroyClient(); 
  m_datalayer = nullptr; 
}

void RTApplication::createClient(){
  m_client = m_datalayer->createClient3(DL_IPC_AUTO);
}

void RTApplication::openMemory(std::shared_ptr<comm::datalayer::IMemoryUser>* mem, std::map<std::string,uint32_t>* mem_Map,
    uint32_t* m_Rev, std::string m_DatalayerPath){
  if(m_client){
    comm::datalayer::Variant dlMap; 
    auto result = m_client->readSync(m_DatalayerPath + "/map", &dlMap); 
    auto varMap = comm::datalayer::GetMemoryMap(dlMap.getData());
    *m_Rev = varMap->revision(); 
    for(auto variables = varMap->variables()->begin(); variables!= varMap->variables()->end(); variables++){
      (*mem_Map)[variables->name()->str()] = variables->bitoffset(); 
    }
    result = m_datalayer->openMemory(*mem, m_DatalayerPath); 
  }
}

void RTApplication::closeMemory(std::shared_ptr<comm::datalayer::IMemoryUser>* mem){
  if(*mem){
    m_datalayer->closeMemory(*mem); 
    *mem = nullptr; 
  }
}

void RTApplication::destroyClient(){
  if(m_client)
    delete m_client; 
}

}