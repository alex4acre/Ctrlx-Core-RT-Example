#include "rt_application.h"
#include "Logger.h"
#include <thread>
#include "common/scheduler/i_scheduler3.h"

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
        if (!startFlag)
        {
          LOG_INFO("Cyclic Access Succesfully started!");
          startFlag = true;
        }
        u_int8_t* inData; 
        u_int8_t* outData; 

        //Begin ECAT Input Access
        auto result = mMemManager_ECAT_IN._beginAccess(inData); 
        if(result == DL_OK)
        {
          RTUpdate::AT(inData, mMemManager_ECAT_IN.m_Map);
        } 
        //End ECAT Input Access
        mMemManager_ECAT_IN._EndAccess(); 
        
        //Begin ECAT Output Access
        result = mMemManager_ECAT_OUT._beginAccess(outData); 
        if(result == DL_OK)
        {
          RTUpdate::MDT(outData, mMemManager_ECAT_OUT.m_Map);
        } 
        //End ECAT Output Access
        mMemManager_ECAT_OUT._EndAccess(); 

        //Begin PLC Output Access
        result = mMemManager_PLC_OUT._beginAccess(outData); 
        if(result == DL_OK)
        {
          RTUpdate::PLC_OUT(outData, mMemManager_PLC_OUT.m_Map);
        } 
        //End PLC Output Access
        mMemManager_PLC_OUT._EndAccess(); 
        
        //Begin PLC Input Access
        result = mMemManager_PLC_IN._beginAccess(inData); 
        if(result == DL_OK)
        {
          RTUpdate::PLC_IN(inData, mMemManager_PLC_IN.m_Map);
        }  
        //End PLC Input Access
        mMemManager_PLC_IN._EndAccess(); 
      

      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
    }

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_SETUP:
    {
      LOG_INFO("Switching to Setup");
      /*close all of the memory*/
      if (eventPhase == common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_END)
      {
        m_fbsCurrentState = common::scheduler::fbs2::CurrentState::CurrentState_SETUP;
        m_finalStateMachineState = FinalStateMachineState::SCHED_SETUP_PREPARE;
      }
      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
    }
    
    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_CONFIG:
    {
      LOG_INFO("Switching to Config");
      /*close all of the memory*/
      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
    }

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_EXIT:
    {
      LOG_INFO("Switching to Exit");
      /*close all of the memory*/
      if (eventPhase == common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_END)
      {
        m_finalStateMachineState = FinalStateMachineState::SCHED_EXIT;
        m_fbsCurrentState = common::scheduler::fbs2::CurrentState::CurrentState_EXIT;
      }
      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
    }

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_SERVICE:
    {
      LOG_INFO("Switching to Service");
      /*close all of the memory*/
      mMemManager_ECAT_OUT._closeMemory();
      mMemManager_ECAT_IN._closeMemory();
      mMemManager_PLC_OUT._closeMemory();
      mMemManager_PLC_IN._closeMemory();
      if (eventPhase == common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_END)
      {
        m_finalStateMachineState = FinalStateMachineState::SCHED_SERVICE;
        m_fbsCurrentState = common::scheduler::fbs2::CurrentState::CurrentState_SERVICE;
      }
      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
    }

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_OPERATING:
    {
      startFlag = false;
      switch (eventPhase)
      {
      case common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_NONE:
        m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PHASE_NONE;
        break;
      case common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_BEGIN:
        m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PHASE_BEGIN;
        break;
  
      case common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_EXECUTE:
        m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATION_PHASE_EXECUTE;
        break;
  
      case common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_END:
        m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE;
        m_fbsCurrentState = common::scheduler::fbs2::CurrentState::CurrentState_OPERATING;
        break;
  
      default:
        LOG_WARNING("#%d UNHANDLED eventPhase=%s ", m_instanceID, common::scheduler::getSchedEventPhaseAsString(eventPhase).c_str());
        break;
      }
      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
    }
  }
}
  
void RTApplication::setDatalayer(comm::datalayer::IDataLayerFactory3* datalayerFactory)
{
  calltocreateclient++;
  LOG_INFO("call to create client %i", calltocreateclient);
  if (!m_client)
  {
    LOG_INFO("Creating Client and Opening Memory");
    m_datalayer = datalayerFactory; 
    createClient(); 
  }
  if (m_client)
  {
    LOG_INFO("Succesfully created client");
  }
  else
    LOG_ERROR("Failed to create client");
}

void RTApplication::resetDataLayer(){
  //Close the etherCAT memory
  //closeMemory(&m_inputs_ECAT); 
  //closeMemory(&m_outputs_ECAT); 
  //Close the PLC memory
  //closeMemory(&m_inputs_PLC); 
  //closeMemory(&m_outputs_PLC);
  //mMemManager_ECAT_IN._closeMemory(m_datalayer); 
  mMemManager_ECAT_IN._closeMemory();
  mMemManager_ECAT_OUT._closeMemory();
  mMemManager_PLC_IN._closeMemory();
  mMemManager_PLC_OUT._closeMemory();
  destroyClient(); 
  m_datalayer = nullptr; 
}

void Example::RTApplication::createClient(){
  if (!m_client)
  {
    m_client = m_datalayer->createClient3(DL_IPC_AUTO);
  }
}

//! Starts the final state machine in a separate thread.
void RTApplication::finalStateMachineStart()
{
  LOG_INFO("#%d", m_instanceID);

  m_finalStateMachineState = FinalStateMachineState::IDLE;
  m_thread = std::thread(&RTApplication::finalStateMachine, this);
}

// Stops the final state machine thread.
void RTApplication::finalStateMachineStop()
{
  LOG_INFO("#%d", m_instanceID);

  RTApplication::m_finalStateMachineState = FinalStateMachineState::SCHED_EXIT;
  if (m_thread.joinable())
  {
    m_thread.join();
  }
  LOG_INFO("#%d stopped.", m_instanceID);
}


//! Implementation of the final state machine, handles none realtime actions (outside the scheduler context).
void RTApplication::finalStateMachine()
{
  char counter = 0;
  while (true)
  {
    counter = (++counter) & 7;
    //LOG_INFO("%d #%d state=%s schedEventType=%s", counter, m_instanceID, s_states[m_finalStateMachineState].c_str(), common::scheduler::getSchedEventTypeAsString(m_schedEventType).c_str());

    switch (m_finalStateMachineState)
    {
        // Known states, but nothing to do
      case FinalStateMachineState::IDLE:
      case FinalStateMachineState::ERROR:
      case FinalStateMachineState::UNKNOWN:
      case FinalStateMachineState::SCHED_SERVICE:
      case FinalStateMachineState::SCHED_OPERATING_PHASE_NONE:
      case FinalStateMachineState::SCHED_OPERATING_PHASE_BEGIN:
      case FinalStateMachineState::SCHED_OPERATION_PHASE_EXECUTE:
      case FinalStateMachineState::TASK_PROPERTIES_CHANGE:
        break;
      case FinalStateMachineState::SCHED_EXIT:
      {
        LOG_INFO("Terminating thread #%d", m_instanceID);
        return; // Terminate thread
      }
      case FinalStateMachineState::SCHED_SETUP_PREPARE:
      case FinalStateMachineState::SCHED_OPERATING_PREPARE:
        {
          if(!mMemManager_ECAT_IN.m_MemOpen)
            {
              mMemManager_ECAT_IN._openMemory(m_datalayer_ECAT_In, m_datalayer);
            }
            
          /*Open the ECAT OUT memory*/  
          else if (!mMemManager_ECAT_OUT.m_MemOpen)
            {
              mMemManager_ECAT_OUT._openMemory(m_datalayer_ECAT_Out, m_datalayer);
            }  
          
          /*Open the PLC IN memory*/
          else if (!mMemManager_PLC_IN.m_MemOpen)
            {
              mMemManager_PLC_IN._openMemory(m_datalayer_PLC_In, m_datalayer);
            }
          
          /*Open the PLC OUT memory*/  
          else if (!mMemManager_PLC_OUT.m_MemOpen)
            {
              mMemManager_PLC_OUT._openMemory(m_datalayer_PLC_Out, m_datalayer);
            }

          else if (!mMemManager_ECAT_IN.m_MapRetrieved)
            {
              mMemManager_ECAT_IN._getMap();
            }

          else if (!mMemManager_ECAT_IN.m_MapValid)
            {
              mMemManager_ECAT_IN._verifyMap();
            }

          else if (!mMemManager_ECAT_IN.m_ReadMap)
            {
              mMemManager_ECAT_IN._readMap(m_client);
            } 

          /*set up the ECAT OUT memory*/  
          else if (!mMemManager_ECAT_OUT.m_MapRetrieved)
            {
              mMemManager_ECAT_OUT._getMap();
            }

          else if (!mMemManager_ECAT_OUT.m_MapValid)
            {
              mMemManager_ECAT_OUT._verifyMap();
            }
          else if (!mMemManager_ECAT_OUT.m_ReadMap)
            {
              mMemManager_ECAT_OUT._readMap(m_client);
            }
            
          /*set up the PLC IN memory*/
          else if (!mMemManager_PLC_IN.m_MapRetrieved)
            {
              mMemManager_PLC_IN._getMap();
            }
          else if (!mMemManager_PLC_IN.m_MapValid)
            {
              mMemManager_PLC_IN._verifyMap();
            }
          else if (!mMemManager_PLC_IN.m_ReadMap)
            {
              mMemManager_PLC_IN._readMap(m_client);
            } 

          /*set up the PLC OUT memory*/  
          else if (!mMemManager_PLC_OUT.m_MapRetrieved)
            {
              mMemManager_PLC_OUT._getMap();
            }
          else if (!mMemManager_PLC_OUT.m_MapValid)
            {
              mMemManager_PLC_OUT._verifyMap();
            }
          else if (!mMemManager_PLC_OUT.m_ReadMap)
            {
              mMemManager_PLC_OUT._readMap(m_client);
            } 
          
          m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING;
          break;
          }
      case FinalStateMachineState::SCHED_SETUP:
      case FinalStateMachineState::SCHED_OPERATING:
        // execute() will call executeSchedulerEventTickReadDiWriteDo()
        break;

      default:
        LOG_WARNING("Unhandled FinalStateMachineState %d", m_finalStateMachineState);
        break;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
/*
bool RTApplication::openMemory_V2(std::shared_ptr<comm::datalayer::IMemoryUser> &mem, std::map<std::string,uint32_t>* mem_Map,
  uint32_t* m_Rev, std::string &m_DatalayerPath){
if(m_client){
  comm::datalayer::Variant dlMap; 

  auto result = m_datalayer->openMemory(mem, m_DatalayerPath); 
  if (comm::datalayer::STATUS_FAILED(result))
  {
    LOG_ERROR("open Memory failed: %s", result.toString());
    return false;
  }
  else
  LOG_INFO("openMemory succeeded: %s", result.toString());
  int tries = 10;
  do
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    result = mem->getMemoryMap(dlMap);
    if (comm::datalayer::STATUS_FAILED(result))
    {
      tries--;
      if (tries <= 0)
        break;
    }
  } while (comm::datalayer::STATUS_FAILED(result));
  if (comm::datalayer::STATUS_FAILED(result))
  {
    LOG_ERROR("getMemoryMap failed: %s", m_DatalayerPath.c_str(),
              result.toString());
              m_datalayer->closeMemory(mem);
    return false;
  }
  else
  LOG_INFO("getMemoryMap succeeded: %s", result.toString());

  result = dlMap.verifyFlatbuffers(comm::datalayer::VerifyMemoryMapBuffer);
  if (comm::datalayer::STATUS_FAILED(result))
  {
    LOG_ERROR("Verifing map %s failed: %s",m_DatalayerPath.c_str(), result.toString());
    m_datalayer->closeMemory(mem);
    return false;
  }
  else
    LOG_INFO("Verifing map %s Succeeded: %s",m_DatalayerPath.c_str(), result.toString());
  
  if (STATUS_FAILED(result = m_client->readSync(m_DatalayerPath + "/map", &dlMap)))
    {LOG_ERROR("Failed to read node failed: %s", result);
    return false;}
  auto varMap = comm::datalayer::GetMemoryMap(dlMap.getData());
  *m_Rev = varMap->revision(); 
  for(auto variables = varMap->variables()->begin(); variables!= varMap->variables()->end(); variables++){
    (*mem_Map)[variables->name()->str()] = variables->bitoffset(); 
  }
  return true;
  }
  else
    LOG_WARNING("Client was not created!");
}
*/
/*
void RTApplication::closeMemory(std::shared_ptr<comm::datalayer::IMemoryUser>* mem){
  if(*mem){
    m_datalayer->closeMemory(*mem); 
    *mem = nullptr; 
  }
    
}*/

void RTApplication::destroyClient()
{
  if(m_client)
    delete m_client; 
}
}