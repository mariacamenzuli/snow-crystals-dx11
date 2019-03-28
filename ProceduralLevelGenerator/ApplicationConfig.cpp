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

            if (key == "hexagon_lattice_width") {
                hexagonLatticeWidth = std::stoi(value);
                continue;
            }

            if (key == "hexagon_lattice_height") {
                hexagonLatticeHeight = std::stoi(value);
                continue;
            }

            if (key == "automaton_step_every_nth_frame") {
                automatonStepEveryNthFrame = std::stoi(value);
                continue;
            }

            if (key == "alpha") {
                alpha = std::stof(value);
                continue;
            }

            if (key == "beta") {
                beta = std::stof(value);
                continue;
            }

            if (key == "gamma") {
                gamma = std::stof(value);
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

int ApplicationConfig::getHexagonLatticeWidth() {
    return hexagonLatticeWidth;
}

int ApplicationConfig::getHexagonLatticeHeight() {
    return hexagonLatticeHeight;
}

int ApplicationConfig::getAutomatonStepEveryNthFrame() {
    return automatonStepEveryNthFrame;
}

float ApplicationConfig::getAlpha() {
    return alpha;
}

float ApplicationConfig::getBeta() {
    return beta;
}

float ApplicationConfig::getGamma() {
    return gamma;
}

bool ApplicationConfig::readBoolean(const std::string& property) {
    return property == "true";
}
