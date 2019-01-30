#include "ApplicationConfig.h"

#include <regex>
#include <fstream>
#include <filesystem>

ApplicationConfig::ApplicationConfig() {
    const std::regex regexConfigLine(R"((.+): (.+))");

    std::ifstream inputStream;

    inputStream.open("config.yml");

    if (inputStream.fail()) {
        // Do not throw exception, just use default values;
        return;
    }

    while (!inputStream.eof()) {
        std::string line;
        std::getline(inputStream, line);

        std::smatch regexMatches;

        if (std::regex_search(line, regexMatches, regexConfigLine)) {
            auto key = regexMatches[1];
            auto value = regexMatches[2];

            if (key == "fullscreen_enabled") {
                fullscreenEnabled = readBoolean(value);
                continue;
            }

            if (key == "vsync_enabled") {
                vsyncEnabled = readBoolean(value);
                continue;
            }

            if (key == "screen_depth") {
                screenDepth = std::stof(value);
                continue;
            }

            if (key == "screen_near") {
                screenNear = std::stof(value);
                continue;
            }

            if (key == "shadowmap_size") {
                shadowMapSize = std::stoi(value);
                continue;
            }
        }
    }
}

ApplicationConfig::~ApplicationConfig() = default;

bool ApplicationConfig::isFullscreenEnabled() {
    return fullscreenEnabled;
}

bool ApplicationConfig::isVsyncEnabled() {
    return vsyncEnabled;
}

float ApplicationConfig::getScreenDepth() {
    return screenDepth;
}

float ApplicationConfig::getScreenNear() {
    return screenNear;
}

int ApplicationConfig::getShadowMapSize() {
    return shadowMapSize;
}

bool ApplicationConfig::readBoolean(const std::string& property) {
    return property == "true";
}
