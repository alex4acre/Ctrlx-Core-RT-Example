#pragma once
#include <iostream>
#include "common/scheduler/i_scheduler3.h"
#include "comm/datalayer/datalayer.h"
//#include "common/log/trace/i_registration.h"
#include "../common.log.trace/trace_itf_wrapper.h"
#include "Trace.h"
#include "../impl/rt_applicationFactory.h"
class ExampleComponent
{
  public:
    void init();
    void start(); 
    void stop();
    void deInit();
    void setSchedularService(common::scheduler::IScheduler3* schedular);
    void setDatalayerService(comm::datalayer::IDataLayerFactory3* dataLyaer);
    //void setRTTraceService(common::log::trace::IRegistrationRealTime3* log);
    void traceServiceStateChanged(common::log::trace::IRegistrationRealTime3 *service);
    void traceServiceAdded(common::log::trace::IRegistrationRealTime3 *service);
    void traceServiceRemoved(common::log::trace::IRegistrationRealTime3 *service);
     
  private: 
    comm::datalayer::IDataLayerFactory3* m_dataLayer; 
    common::scheduler::IScheduler3* m_schedular; 
    //common::log::trace::IRegistrationRealTime3* m_log; 
    std::shared_ptr<Example::RTApplicationFactory> m_appFactory;// = std::make_shared<Example::RTApplicationFactory>();
};