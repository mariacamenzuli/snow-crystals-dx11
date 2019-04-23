#include "ApplicationConfig.h"

#include <regex>
#include <fstream>
#include <filesystem>

ApplicationConfig::ApplicationConfig() {
    const std::regex regexConfigLine(R"((.+): (.+))");
    const std::regex regexEnumName("([_A-Z]+)");

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

            if (key == "convolutions") {
                std::string convolutionToParse = value.str();
                while (std::regex_search(convolutionToParse, regexMatches, regexEnumName)) {
                    std::string convolutionName = regexMatches[0].str();
                    if (convolutionName == "EDGE_DETECTION") {
                        convolutions.push_back(D3D11Renderer::Convolution::EDGE_DETECTION);
                    } else if (convolutionName == "GAUSSIAN_BLUR") {
                        convolutions.push_back(D3D11Renderer::Convolution::GAUSSIAN_BLUR);
                    } else if (convolutionName == "EMBOSS") {
                        convolutions.push_back(D3D11Renderer::Convolution::EMBOSS);
                    } else if (convolutionName == "SHARPEN") {
                        convolutions.push_back(D3D11Renderer::Convolution::SHARPEN);
                    }
                    convolutionToParse = regexMatches.suffix();
                }
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

float ApplicationConfig::getAlpha() {
    return alpha;
}

float ApplicationConfig::getBeta() {
    return beta;
}

float ApplicationConfig::getGamma() {
    return gamma;
}

std::vector<D3D11Renderer::Convolution> ApplicationConfig::getConvolutions() {
    return convolutions;
}

bool ApplicationConfig::readBoolean(const std::string& property) {
    return property == "true";
}
