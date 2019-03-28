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

    int getHexagonLatticeWidth();
    int getHexagonLatticeHeight();
    int getAutomatonStepEveryNthFrame();
    float getAlpha();
    float getBeta();
    float getGamma();

private:
    bool fullscreenEnabled = false;
    bool vsyncEnabled = true;
    float screenDepth = 1000.0f;
    float screenNear = 0.1f;
    int shadowMapSize = 1080;

    int hexagonLatticeWidth = 100;
    int hexagonLatticeHeight = 100;
    int automatonStepEveryNthFrame = 1;
    float alpha = 1.0f;
    float beta = 0.4f;
    float gamma = 0.000005f;

    bool readBoolean(const std::string& property);
};

