#pragma once
#include <string>

class ApplicationConfig {
public:
    ApplicationConfig();
    ~ApplicationConfig();

    bool isFullscreenEnabled();
    bool isVsyncEnabled();
    float getScreenDepth();
    float getScreenNear();
    int getShadowMapSize();

private:
    bool fullscreenEnabled = false;
    bool vsyncEnabled = true;
    float screenDepth = 1000.0f;
    float screenNear = 0.1f;
    int shadowMapSize = 1080;

    bool readBoolean(const std::string& property);
};

