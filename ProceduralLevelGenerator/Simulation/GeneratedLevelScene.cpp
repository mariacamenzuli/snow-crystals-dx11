#include "GeneratedLevelScene.h"

GeneratedLevelScene::GeneratedLevelScene() : pointLight(D3DXVECTOR4(1.0f, 1.0f, 0.9f, 1.0f), D3DXVECTOR4(0.5f, 0.5f, 0.4f, 1.0f)) {
    rootSceneObject.reset(new SceneObject());

    auto world = rootSceneObject->attachChild(std::make_unique<SceneObject>(), "world");

    const auto cubeModel = modelLoader.getModel(ModelLoader::ModelId::CUBE);
    auto background = world->attachChild(std::make_unique<SceneObject>(cubeModel), "background");
    // cubeModel->addInstance({ {12.0f, 12.0f, 12.0f} });
    // cubeModel->addInstance({ {12.0f, 12.0f, 12.0f} });
    background->scale(100.0f, 1.0f, 100.f);
    background->translate(0.0f, -4.0f, 0.0f);

    const auto hexagonModel = modelLoader.getModel(ModelLoader::ModelId::HEXAGON);

    // auto hexLattice = world->attachChild(std::make_unique<SceneObject>(hexagonModel), "snowflake");
    // hexLattice->scale(3.0f, 3.0f, 3.0f);

    auto cellGrid = world->attachChild(std::make_unique<SceneObject>(), "cells");

    for (int y = 0; y < HEXAGON_LATTICE_HEIGHT; y++) {
        int t = y % 2;
        for (int x = 0; x < HEXAGON_LATTICE_WIDTH; x++) {
            auto hexagonCell = cellGrid->attachChild(std::make_unique<SceneObject>(hexagonModel));
            // hexagonCell->scale(12.0f, 12.0f, 12.0f);
            hexagonCell->scale(0.105f, 0.105f, 0.105f);
            // hexagonCell->rotateY(-0.23f);
            hexagonCell->rotateZ(1.5708f);
            hexagonCell->translate(x * 2 + t, y * 1.75, 0.0f);
    
            cells[x][y] = { x, y, CellState::AIR, hexagonCell, BETA };
        }
    }
    
    for (int y = 0; y < HEXAGON_LATTICE_HEIGHT; y++) {
        int t = y % 2;
        for (int x = 0; x < HEXAGON_LATTICE_WIDTH; x++) {
            if (x > 0) {
                cells[x][y].neighbours.push_back(&cells[x - 1][y]);
            }
    
            if (x < HEXAGON_LATTICE_WIDTH - 1) {
                cells[x][y].neighbours.push_back(&cells[x + 1][y]);
            }
    
            if (y > 0) {
                if (t == 0) {
                    if (x > 0) {
                        cells[x][y].neighbours.push_back(&cells[x - 1][y - 1]);
                    }
                    cells[x][y].neighbours.push_back(&cells[x][y - 1]);
                } else {
                    cells[x][y].neighbours.push_back(&cells[x][y - 1]);
                    if (x < HEXAGON_LATTICE_WIDTH - 1) {
                        cells[x][y].neighbours.push_back(&cells[x + 1][y - 1]);
                    }
                }
            }
    
            if (y < HEXAGON_LATTICE_HEIGHT - 1) {
                if (t == 0) {
                    if (x > 0) {
                        cells[x][y].neighbours.push_back(&cells[x - 1][y + 1]);
                    }
                    cells[x][y].neighbours.push_back(&cells[x][y + 1]);
                } else {
                    cells[x][y].neighbours.push_back(&cells[x][y + 1]);
                    if (x < HEXAGON_LATTICE_WIDTH - 1) {
                        cells[x][y].neighbours.push_back(&cells[x + 1][y + 1]);
                    }
                }
            }
        }
    }
    
    // seed ice
    const int seedX = HEXAGON_LATTICE_WIDTH / 2;
    const int seedY = HEXAGON_LATTICE_HEIGHT / 2;
    cells[seedX][seedY].value = ICE;
    cells[seedX][seedY].nonReceptiveValue = ICE;
    cells[seedX][seedY].freeze();
    
    ice.push_back(&cells[seedX][seedY]);
    for (auto iceCell : ice) {
        for (auto neighbour : iceCell->neighbours) {
            if (neighbour->value < ICE) {
                boundary.push_back(neighbour);
            }
        }
    }

    pointLight.translate(0.0f, 50.0f, -10.0f);
}

GeneratedLevelScene::~GeneratedLevelScene() = default;

SceneObject* GeneratedLevelScene::getRootSceneObject() {
    return rootSceneObject.get();
}

D3DXVECTOR4 GeneratedLevelScene::getAmbientLight() {
    return D3DXVECTOR4(0.3f, 0.3f, 0.3f, 1.0f);
}

PointLight* GeneratedLevelScene::getPointLight() {
    return &pointLight;
}

void GeneratedLevelScene::update(float deltaTime) {
    updateCount++;
    // rootSceneObject->getChild("world")->getChild("cells")->rotateY(0.1f * deltaTime);
    
    // if (updateCount % 120 == 0) {
        // std::string log = "Automaton step...\n";
        // OutputDebugStringW(std::wstring(log.begin(), log.end()).c_str());
    
        for (auto iceCell : ice) {
            iceCell->receptiveValue = iceCell->value + GAMMA;
            iceCell->nonReceptiveValue = 0.0f;
        }
        for (auto boundaryCell : boundary) {
            boundaryCell->receptiveValue = boundaryCell->value + GAMMA;
            boundaryCell->nonReceptiveValue = 0.0f;
        }
    
        ice.clear();
        boundary.clear();
    
        const auto diffusionWeight = ALPHA / 12.0f;
        for (int y = 0; y < HEXAGON_LATTICE_HEIGHT; y++) {
            for (int x = 0; x < HEXAGON_LATTICE_WIDTH; x++) {
                // Diffusion
                cells[x][y].value = cells[x][y].nonReceptiveValue * (1.0f - diffusionWeight * 6.0f); // cells[x][y].neighbours.size()
                for (auto neighbour : cells[x][y].neighbours) {
                    cells[x][y].value += neighbour->nonReceptiveValue * diffusionWeight;
                }
    
                // Growth
                cells[x][y].value += cells[x][y].receptiveValue;
            }
        }
    
        // Update state
        for (int y = 0; y < HEXAGON_LATTICE_HEIGHT; y++) {
            for (int x = 0; x < HEXAGON_LATTICE_WIDTH; x++) {
                cells[x][y].receptiveValue = 0.0f;
                cells[x][y].nonReceptiveValue = cells[x][y].value;
                if (cells[x][y].value >= ICE) {
                    ice.push_back(&cells[x][y]);
                    cells[x][y].freeze();
                } else {
                    cells[x][y].thaw();
                }
            }
        }
        for (auto iceCell : ice) {
            for (auto neighbour : iceCell->neighbours) {
                if (neighbour->value < ICE) {
                    boundary.push_back(neighbour);
                }
            }
        }
    // }
}
