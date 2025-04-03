#include "cmp.h"
#include "common/log/trace/defs.h"
#include "common/log/trace/log_buffered3.h"
#include "../impl/Logger.h"
void ExampleComponent::init(){
  m_appFactory = std::make_shared<Example::RTApplicationFactory>();
}
void ExampleComponent::start(){
  m_appFactory->setDataLayer(m_dataLayer); 
  m_schedular->registerCallableFactory(m_appFactory, "Example RT App"); 
}

void ExampleComponent::stop() {
  LOG_INFO("Stopping the celix bundle.");
  m_appFactory->resetDataLayer(); 
  m_schedular->unregisterCallableFactory(m_appFactory, true); 
}

void ExampleComponent::deInit(){
  m_appFactory = nullptr;
  std::cout<<"deInit" << std::endl; 
}


//call back for scheduler
void ExampleComponent::setSchedularService(common::scheduler::IScheduler3* schedular){
  m_schedular = schedular; 
}

//call back for datalayer
void ExampleComponent::setDatalayerService(comm::datalayer::IDataLayerFactory3* dataLyaer){
  m_dataLayer = dataLyaer; 
}


//! This method is called when the service was added.
//!
//! @param[in]  service  A pointer to the service.
void ExampleComponent::traceServiceAdded(common::log::trace::IRegistrationRealTime3 *service)
{
    LOG_INFO("");

    common::log::trace::TraceResult traceResult;
    ATTACH_TRACE(traceResult, service);
    if (statusFailed(traceResult))
    {
        LOG_ERROR("ATTACH_TRACE failed, deactivate buffered traces to ensure real time behaviour.");
        DISABLE_ALL_BUFFERED_TRACES();
    }
}

//! This method is called when the service was removed.
//!
//! @param[in]  service  A pointer to the service.
void ExampleComponent::traceServiceRemoved(common::log::trace::IRegistrationRealTime3 *service)
{
    LOG_INFO("");

    common::log::trace::TraceResult traceResult;
    DETACH_TRACE(traceResult);
    if (statusFailed(traceResult))
    {
        LOG_ERROR("DETACH_TRACE failed.")
    }
}
