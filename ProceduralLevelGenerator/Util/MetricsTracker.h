#pragma once

#include <chrono>

class MetricsTracker {
public:
    MetricsTracker();
    ~MetricsTracker();

    unsigned int framesPerSecond;
    unsigned int simulationUpdatesPerSecond;
    float timeSinceLastSimulationUpdate;

    void newFrameRendered();
    void newSimulationUpdate();
    void incrementTimeSinceLastSimulationUpdate();

private:
    unsigned int frameCounter;
    unsigned int simulationUpdateCounter;
    std::chrono::time_point<std::chrono::system_clock> fpsCountStartTime;
    std::chrono::time_point<std::chrono::system_clock> simulationUpdateCountStartTime;
    std::chrono::time_point<std::chrono::system_clock> simulationSynchTimer;

    static float toSeconds(long milliseconds);
};
