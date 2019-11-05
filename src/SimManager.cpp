
#include "SimulationManager.h"

#include "AMM/BaseLogger.h"

using namespace std;
using namespace std::chrono;

bool closed = false;

static void ShowUsage(const std::string &name) {
    cerr << "Usage: " << name << " <option(s)>"
         << "\nOptions:\n"
         << "\t-r,--rate <sample_rate>\tSpecify the sample rate to run at "
            "(samples per second)\n"
         << "\t-a\t\t\tAuto-start ticks\n"
         << "\t-d\t\t\tDaemonize\n"
         << "\t-h,--help\t\t\tShow this help message\n"
         << endl;
}

void ShowMenu(AMM::SimulationManager *simManager) {
    string action;

    // std::endl is an automatic flush and should be avoided unless required.
    cout << "\n"
            " [1]Status\n"
            " [2]Run/Resume\n"
            " [3]Reset/Halt\n"
            " [4]Halt/Pause\n"
            " [5]Save State\n"
            " [6]Shutdown\n"
            " [7]Publish OpDesc\n"
            // " [7]Command console\n"
            " >> ";

    getline(cin, action);
    transform(action.begin(), action.end(), action.begin(), ::toupper);

    /// Tell Sim Manager to write a new Simulation Control Topic to the network.
    bool doWriteTopic = true;

    /// Get state
    if (action == "1") {
        if (simManager->IsRunning()) {
            cout << " == Running!  At tick count: ";
        } else {
            cout << " == Not currently running, paused at tick count: ";
        }

        cout << simManager->GetTickCount() << endl;
        cout << "  = Operating at " << simManager->GetSampleRate()
             << " frames per second." << endl;

        /// Start sim
    } else if (action == "2") {
        if (!simManager->IsRunning()) {
            cout << " == Running simulation..." << endl;
            simManager->RunSimulation(doWriteTopic);
        } else {
            cout << " == Simulation already running" << endl;
        }

        /// Reset sim
    } else if (action == "3") {
        cout << " == Resetting simulation ..." << endl;
        simManager->ResetSimulation(doWriteTopic);

        /// Halt sim
    } else if (action == "4") {
        if (simManager->IsRunning()) {
            cout << " == Halting simulation..." << endl;
            simManager->HaltSimulation(doWriteTopic);
        } else {
            cout << " == Simulation not running" << endl;
        }

        /// Save sim state
    } else if (action == "5") {
        cout << " == Saving simulation ..." << endl;
        cout << " ==   Not implemented" << endl;
        simManager->SaveSimulation(doWriteTopic);

        /// Shutdown sim
    } else if (action == "6") {
       if (!simManager->IsRunning()) {
          cout << " == Simulation not running, but shutting down anyway" << endl;
       } else {
          cout << " == Stopping simulation and sending shutdown notice..." << endl;
       }

       simManager->HaltSimulation(doWriteTopic);
       cout << " == Exited after " << simManager->GetTickCount() << " ticks."
            << endl;

       cout << "=== [SimManager] Shutting down Simulation Manager." << endl;
       closed = true;
       simManager->Shutdown();
    } else if (action == "7") {
       simManager->PublishOperationalDescription();
    } else {
        // unknown menu action
    }
}

int main(int argc, char *argv[]) {
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);

    int sampleRate = 50;
    int daemonize = 0;
    int autostart = 0;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            ShowUsage(argv[0]);
            return 0;
        }

        if (arg == "-d") {
            daemonize = 1;
        }

        if (arg == "-a") {
            autostart = 1;
        }

    }

   LOG_INFO << "Simulation Manager starting";
    AMM::SimulationManager simManager;
    simManager.SetSampleRate(sampleRate);

   std::this_thread::sleep_for(std::chrono::milliseconds(250));

    simManager.PublishOperationalDescription();
    simManager.PublishConfiguration();

    if (autostart == 1) {
        LOG_INFO << "Auto-starting simulation";
        simManager.RunSimulation(true);
    }

    while (!closed) {
        if (daemonize != 1 && autostart != 1) {
            ShowMenu(&simManager);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    LOG_INFO << "Exiting.";

    return 0;
}
