#pragma once

#include "amm_std.h"

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

#include <thread>
#include <chrono>

#include "amm/Utility.h"


namespace AMM {
    class SimulationManager {

    public:
        SimulationManager();

        // TODO:
        // Remove doWriteTopic.

        void PublishOperationalDescription();

        void PublishConfiguration();

        void RunSimulation(bool doWriteTopic);

        void HaltSimulation(bool doWriteTopic);

        void ResetSimulation(bool doWriteTopic);

        void SaveSimulation(bool doWriteTopic);

        void Shutdown();

        // Sim properties
        void SetSampleRate(int rate);

        bool IsRunning();

        int GetSampleRate();

        int GetTickCount();

        void TickLoop();

        void onNewSimulationControl(AMM::SimulationControl &simControl, SampleInfo_t *info);

    protected:
        std::thread m_thread;
        std::mutex m_mutex;
        bool m_runThread = false;

        AMM::UUID m_uuid;

        std::string moduleName = "AMM_SimulationManager";
        std::string configFile = "config/sim_manager_amm.xml";
        DDSManager<SimulationManager> *m_mgr = new DDSManager<SimulationManager>(configFile);

        int m_tickCount = 0;
        int m_sampleRate = 50;

    };

} // namespace AMM
