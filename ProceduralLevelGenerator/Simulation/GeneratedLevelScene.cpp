#include "GeneratedLevelScene.h"

GeneratedLevelScene::GeneratedLevelScene() : pointLight(D3DXVECTOR4(1.0f, 1.0f, 0.9f, 1.0f), D3DXVECTOR4(0.5f, 0.5f, 0.4f, 1.0f)) {
    rootSceneObject.reset(new SceneObject());

    auto world = rootSceneObject->attachChild(std::make_unique<SceneObject>(), "world");

    const auto cubeModel = modelLoader.getModel(ModelLoader::ModelId::CUBE);
    auto background = world->attachChild(std::make_unique<SceneObject>(cubeModel), "background");
    // cubeModel->addInstance({ {-10.0f, 5.0f, 0.0f} });
    // cubeModel->addInstance({ {10.0f, 0.0f, 0.0f} });
    background->scale(100.0f, 1.0f, 100.f);
    background->translate(0.0f, -4.0f, 0.0f);

    snowflakeModel = modelLoader.getModel(ModelLoader::ModelId::HEXAGON);
    
    auto hexLattice = world->attachChild(std::make_unique<SceneObject>(snowflakeModel), "snowflake");
    // hexLattice->rotateZ(1.5708f);
    hexLattice->scale(0.105f, 0.105f, 0.105f);
    // hexLattice->scale(3.0f, 3.0f, 3.0f);
    
    // auto cellGrid = world->attachChild(std::make_unique<SceneObject>(), "cells");
    
    for (int y = 0; y < HEXAGON_LATTICE_HEIGHT; y++) {
        int t = y % 2;
        for (int x = 0; x < HEXAGON_LATTICE_WIDTH; x++) {
            // hexagonCell->translate(x * 2 + t, y * 1.75, 0.0f);
            cells[x][y] = { x, y, BETA };
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
    cells[seedX][seedY].waterVaporValue = ICE;
    cells[seedX][seedY].nonReceptiveValue = ICE;
    snowflakeModel->addInstance({ calculateCellInstancePosition(seedX, seedY) });
    
    iceCells.push_back(&cells[seedX][seedY]);
    for (auto iceCell : iceCells) {
        for (auto neighbour : iceCell->neighbours) {
            if (neighbour->waterVaporValue < ICE) {
                boundaryCells.push_back(neighbour);
            }
        }
    }

    // auto pointLightIndicator = world->attachChild(std::make_unique<SceneObject>(cubeModel));
    // pointLightIndicator->translate(0.0f, 50.0f, -20.0f);
    pointLight.translate(0.0f, 50.0f, -20.0f);
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
    // rootSceneObject->getChild("world")->getChild("snowflake")->rotateY(0.1f * deltaTime);
    
    // if (updateCount % 120 == 0) {
        // std::string log = "Automaton step...\n";
        // OutputDebugStringW(std::wstring(log.begin(), log.end()).c_str());
    
        for (auto iceCell : iceCells) {
            iceCell->receptiveValue = iceCell->waterVaporValue + GAMMA;
            iceCell->nonReceptiveValue = 0.0f;
        }
        for (auto boundaryCell : boundaryCells) {
            boundaryCell->receptiveValue = boundaryCell->waterVaporValue + GAMMA;
            boundaryCell->nonReceptiveValue = 0.0f;
        }
    
        iceCells.clear();
        boundaryCells.clear();
        snowflakeModel->clearInstances();
    
        const auto diffusionWeight = ALPHA / 12.0f;
        for (int y = 0; y < HEXAGON_LATTICE_HEIGHT; y++) {
            for (int x = 0; x < HEXAGON_LATTICE_WIDTH; x++) {
                // Diffusion
                cells[x][y].waterVaporValue = cells[x][y].nonReceptiveValue * (1.0f - diffusionWeight * 6.0f); // cells[x][y].neighbours.size()
                for (auto neighbour : cells[x][y].neighbours) {
                    cells[x][y].waterVaporValue += neighbour->nonReceptiveValue * diffusionWeight;
                }
    
                // Growth
                cells[x][y].waterVaporValue += cells[x][y].receptiveValue;
            }
        }
    
        // Update state
        for (int y = 0; y < HEXAGON_LATTICE_HEIGHT; y++) {
            for (int x = 0; x < HEXAGON_LATTICE_WIDTH; x++) {
                cells[x][y].receptiveValue = 0.0f;
                cells[x][y].nonReceptiveValue = cells[x][y].waterVaporValue;
                if (cells[x][y].waterVaporValue >= ICE) {
                    iceCells.push_back(&cells[x][y]);
                    snowflakeModel->addInstance({ calculateCellInstancePosition(x, y) });
                }
            }
        }
        for (auto iceCell : iceCells) {
            for (auto neighbour : iceCell->neighbours) {
                if (neighbour->waterVaporValue < ICE) {
                    boundaryCells.push_back(neighbour);
                }
            }
        }
    // }
}

D3DXVECTOR3 GeneratedLevelScene::calculateCellInstancePosition(int x, int y) {
    // return D3DXVECTOR3(x * 20 + (y % 2 * 10), y * 20.75f, 0.0f);
    return D3DXVECTOR3(x * 2 + (y % 2), y * 1.75f, 0.0f);
}
