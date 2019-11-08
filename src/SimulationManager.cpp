#include "SimulationManager.h"

using namespace std;
using namespace std::chrono;

namespace AMM {
SimulationManager::SimulationManager() {

   m_mgr->InitializeTick();
   m_mgr->InitializeSimulationControl();
   m_mgr->InitializeOperationalDescription();
   m_mgr->InitializeStatus();
   m_mgr->InitializeModuleConfiguration();

   m_mgr->CreateTickPublisher();
   m_mgr->CreateSimulationControlPublisher();
   m_mgr->CreateOperationalDescriptionPublisher();
   m_mgr->CreateModuleConfigurationPublisher();
   m_mgr->CreateStatusPublisher();

   m_mgr->CreateSimulationControlSubscriber(this, &SimulationManager::onNewSimulationControl);

   m_uuid.id(m_mgr->GenerateUuidString());

   m_runThread = false;
}


void SimulationManager::PublishOperationalDescription() {
   AMM::OperationalDescription od;
   od.name(moduleName);
   od.model("Simulation Manager");
   od.manufacturer("Vcom3D");
   od.serial_number("1.0.0");
   od.module_id(m_uuid);
   od.module_version("1.0.0");
   const std::string capabilities = Utility::read_file_to_string("config/sim_manager_capabilities.xml");
   od.capabilities_schema(capabilities);
   od.description(
      "The Simulation Manager is a core software module that drives the simulation by publishing "
      "simulation ticks at a frequency of 50 Hz.  It operates as a simulation state engine, "
      "allowing for control of time-related functions and loading scenarios for execution.");
      m_mgr->WriteOperationalDescription(od);
   }

   void SimulationManager::PublishConfiguration() {
      AMM::ModuleConfiguration mc;
      auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
      mc.timestamp(ms);
      mc.module_id(m_uuid);
      mc.name(moduleName);
      const std::string configuration = Utility::read_file_to_string("config/sim_manager_configuration.xml");
      mc.capabilities_configuration(configuration);
      m_mgr->WriteModuleConfiguration(mc);
   }

   void SimulationManager::RunSimulation(bool doWriteTopic) {

      // TODO:
      // Remove doWriteTopic

      if (!m_runThread) {
         m_runThread = true;
         m_thread = std::thread(&SimulationManager::TickLoop, this);

         if (doWriteTopic) {
            AMM::SimulationControl simControl;
            auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            simControl.timestamp(ms);
            simControl.type(AMM::ControlType::RUN);
            m_mgr->WriteSimulationControl(simControl);
         }

         LOG_INFO << "Simulation running.";
      }
   }

   void SimulationManager::HaltSimulation(bool doWriteTopic) {
      if (m_runThread) {
         m_mutex.lock();
         m_runThread = false;
         std::this_thread::sleep_for(std::chrono::milliseconds(200));
         m_mutex.unlock();
         m_thread.detach();

         if (doWriteTopic) {
            AMM::SimulationControl simControl;
            auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            simControl.timestamp(ms);
            simControl.type(AMM::ControlType::HALT);
            m_mgr->WriteSimulationControl(simControl);
         }

         LOG_INFO << "Simulation halted.";
      }
   }

   void SimulationManager::ResetSimulation(bool doWriteTopic) {
      /// Implies both Halt then Reset.

      LOG_INFO << "Sim reset in progress.";

      this->HaltSimulation(doWriteTopic);

      m_tickCount = 0;

      if (doWriteTopic) {
         AMM::SimulationControl simControl;
         auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
         simControl.timestamp(ms);
         simControl.type(AMM::ControlType::RESET);
         m_mgr->WriteSimulationControl(simControl);
      }

      LOG_INFO << "Sim reset complete. Ready to start.";
   }

   void SimulationManager::SaveSimulation(bool doWriteTopic) {
      LOG_INFO << "Saving sim state.";

      if (doWriteTopic) {
         AMM::SimulationControl simControl;
         auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
         simControl.timestamp(ms);
         simControl.type(AMM::ControlType::SAVE);
         m_mgr->WriteSimulationControl(simControl);
      }

      LOG_INFO << "Sim state saved.";
   }

   void SimulationManager::onNewSimulationControl(AMM::SimulationControl &simControl, SampleInfo_t *info) {
      bool doWriteTopic = false;

      switch (simControl.type()) {
         case AMM::ControlType::RUN: {
            LOG_INFO << "Message recieved; Run sim.";
            RunSimulation(doWriteTopic);
            break;
         }

         case AMM::ControlType::HALT: {
            LOG_INFO << "Message recieved; Halt sim";
            HaltSimulation(doWriteTopic);
            break;
         }

         case AMM::ControlType::RESET: {
            LOG_INFO << "Message recieved; Reset sim";
            ResetSimulation(doWriteTopic);
            break;
         }

         case AMM::ControlType::SAVE: {
            LOG_INFO << "Message recieved; Save sim";
            SaveSimulation(doWriteTopic);
            break;
         }
      }
   }

   int SimulationManager::GetTickCount() { return m_tickCount; }

   bool SimulationManager::IsRunning() { return m_runThread; }

   void SimulationManager::SetSampleRate(int rate) { m_sampleRate = rate; }

   int SimulationManager::GetSampleRate() { return m_sampleRate; }

   void SimulationManager::TickLoop() {
      using frames = duration<int64_t, ratio<1, 50>>;
      auto nextFrame = system_clock::now();
      auto lastFrame = nextFrame - frames{1};

      while (m_runThread) {
         this_thread::sleep_until(nextFrame);
         m_mutex.lock();
         AMM::Tick tickInstance;
         tickInstance.frame(m_tickCount++);
         m_mgr->WriteTick(tickInstance);
         lastFrame = nextFrame;
         nextFrame += frames{1};
         m_mutex.unlock();
      }
   }

   void SimulationManager::Shutdown() {
      if (m_runThread) {
         m_runThread = false;
         std::this_thread::sleep_for(std::chrono::milliseconds(200));
         m_thread.join();
      }
   }

} // namespace AMM
