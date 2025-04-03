#include "rt_applicationFactory.h"
#include "common/scheduler/callable_configurations_generated.h"
#include "Trace.h"

namespace Example{
std::shared_ptr<common::scheduler::ICallable> RTApplicationFactory::createCallable(const comm::datalayer::Variant& param){
    m_application->setDatalayer(m_dataLayer); 
    m_application->finalStateMachineStart();
    return m_application;
}

common::scheduler::SchedStatus RTApplicationFactory::destroyCallable(const std::shared_ptr<common::scheduler::ICallable>& callable){
    m_application->resetDataLayer(); 
    m_application->finalStateMachineStop();
    return common::scheduler::SchedStatus::SCHED_S_OK;
}

common::scheduler::SchedStatus RTApplicationFactory::getCallableArguments(std::vector<std::string>& arguments){
    return common::scheduler::SchedStatus::SCHED_S_OK;
}

void RTApplicationFactory::getFactoryDescription(std::vector<std::string>& description) const{
  description.push_back("MyRTExample"); 
}

void RTApplicationFactory::getCallableConfigurations(comm::datalayer::Variant& configurations) const{
    /*/  flatbuffers::FlatBufferBuilder builder;
     // the callable shell be created in a task with the name 'myTask', a low priority and a cycletime of two milliseconds
      auto fbsTaskSpec = common::scheduler::fbs::CreateTaskSpecsDirect(builder,
                                                                          "myTask",
                                                                          common::scheduler::TASK_PRIORITY_RANGE_LOW,
                                                                          "cyclic/ms/2");
      
      // define synchronization points
      auto afterList = builder.CreateVectorOfStrings({ "myCallableEntry" });
      auto beforeList = builder.CreateVectorOfStrings({ "myCallableExit" });
      auto fbsSyncPoints = common::scheduler::fbs::CreateSyncPoints(builder, afterList, beforeList);
      
         // arguments to create callable
      auto args = builder.CreateVectorOfStrings({ "RTExample"});
      
         // give the callable an alias
      auto alias = builder.CreateString("MyRTApp");
      std::vector<flatbuffers::Offset<common::scheduler::fbs::CallableConfiguration>> fbsCallableConfig = 
      { common::scheduler::fbs::CreateCallableConfiguration(builder, alias, fbsSyncPoints, args, common::scheduler::fbs::CallableWdgConfig_WDG_DEFAULT, fbsTaskSpec) };
       
      auto finished = common::scheduler::fbs::CreateCallableConfigurationsDirect(builder, &fbsCallableConfig);
       
      builder.Finish(finished);
      configurations.copyFlatbuffers(builder);*/
      return;
}

void RTApplicationFactory::setDataLayer(comm::datalayer::IDataLayerFactory3* dataLayer){
  m_dataLayer = dataLayer; 
}

void RTApplicationFactory::resetDataLayer(){
  m_application->resetDataLayer(); 
  m_dataLayer = nullptr; 
}

}
