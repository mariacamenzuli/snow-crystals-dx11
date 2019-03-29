#include "SnowflakeMemoryGameScene.h"

SnowflakeMemoryGameScene::SnowflakeMemoryGameScene(const int hexagonLatticeWidth,
                                                   const int hexagonLatticeHeight,
                                                   const int automatonStepEveryNthFrame,
                                                   const float alpha,
                                                   const float beta,
                                                   const float gamma) : hexagonLatticeWidth(hexagonLatticeWidth),
                                                                        hexagonLatticeHeight(hexagonLatticeHeight),
                                                                        automatonStepEveryNthFrame(automatonStepEveryNthFrame),
                                                                        alpha(alpha),
                                                                        beta(beta),
                                                                        gamma(gamma),
                                                                        pointLight(D3DXVECTOR4(1.0f, 1.0f, 0.9f, 1.0f), D3DXVECTOR4(0.5f, 0.5f, 0.4f, 1.0f)) {
    cells = new Cell*[hexagonLatticeWidth];
    for (int i = 0; i < hexagonLatticeWidth; ++i) {
        cells[i] = new Cell[hexagonLatticeHeight];
    }

    rootSceneObject.reset(new SceneObject());

    auto world = rootSceneObject->attachChild(std::make_unique<SceneObject>(), "world");

    // const auto cubeModel = modelLoader.getModel(ModelLoader::ModelId::CUBE);
    // auto background = world->attachChild(std::make_unique<SceneObject>(cubeModel), "background");
    // background->scale(150.0f, 1.0f, 150.f);
    // background->rotateX(1.5708f);
    // background->translate(0, 0, 10);

    snowflakeModel = modelLoader.getModel(ModelLoader::ModelId::HEXAGON);

    auto hexLattice = world->attachChild(std::make_unique<SceneObject>(snowflakeModel), "snowflake");
    hexLattice->translate(- (hexagonLatticeHeight / 2) * 16, - (hexagonLatticeWidth / 2) * 18, 0.0f);
    hexLattice->scale(0.105f, 0.105f, 0.805f);
    hexLattice->rotateZ(1.5708f);
    hexLattice->rotateY(3.14159f);

    for (int y = 0; y < hexagonLatticeHeight; y++) {
        int t = y % 2;
        for (int x = 0; x < hexagonLatticeWidth; x++) {
            cells[x][y] = {x, y, beta};
        }
    }

    for (int y = 0; y < hexagonLatticeHeight; y++) {
        int t = y % 2;
        for (int x = 0; x < hexagonLatticeWidth; x++) {
            if (x > 0) {
                cells[x][y].neighbours.push_back(&cells[x - 1][y]);
            }

            if (x < hexagonLatticeWidth - 1) {
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
                    if (x < hexagonLatticeWidth - 1) {
                        cells[x][y].neighbours.push_back(&cells[x + 1][y - 1]);
                    }
                }
            }

            if (y < hexagonLatticeHeight - 1) {
                if (t == 0) {
                    if (x > 0) {
                        cells[x][y].neighbours.push_back(&cells[x - 1][y + 1]);
                    }
                    cells[x][y].neighbours.push_back(&cells[x][y + 1]);
                } else {
                    cells[x][y].neighbours.push_back(&cells[x][y + 1]);
                    if (x < hexagonLatticeWidth - 1) {
                        cells[x][y].neighbours.push_back(&cells[x + 1][y + 1]);
                    }
                }
            }
        }
    }

    // seed ice
    const int seedX = hexagonLatticeWidth / 2;
    const int seedY = hexagonLatticeHeight / 2;
    cells[seedX][seedY].waterVaporValue = ICE;
    cells[seedX][seedY].nonReceptiveValue = ICE;
    snowflakeModel->addInstance({calculateCellInstancePosition(seedX, seedY)});

    iceCells.push_back(&cells[seedX][seedY]);
    for (auto iceCell : iceCells) {
        for (auto neighbour : iceCell->neighbours) {
            if (neighbour->waterVaporValue < ICE) {
                boundaryCells.push_back(neighbour);
            }
        }
    }

    // auto pointLightIndicator = world->attachChild(std::make_unique<SceneObject>(cubeModel));
    // pointLightIndicator->translate(0.0f, 0.0f, -20.0f);
    // pointLightIndicator->translate(0.0f, 50.0f, -20.0f);
    pointLight.translate(0.0f, -12.5f, -50.0f);
}

SnowflakeMemoryGameScene::~SnowflakeMemoryGameScene() {
    for (int i = 0; i < hexagonLatticeWidth; ++i) {
        delete[] cells[i];
    }
    delete[] cells;
};

SceneObject* SnowflakeMemoryGameScene::getRootSceneObject() {
    return rootSceneObject.get();
}

D3DXVECTOR4 SnowflakeMemoryGameScene::getAmbientLight() {
    return D3DXVECTOR4(0.3f, 0.3f, 0.3f, 1.0f);
}

PointLight* SnowflakeMemoryGameScene::getPointLight() {
    return &pointLight;
}

void SnowflakeMemoryGameScene::update(float deltaTime) {
    updateCount++;
    // rootSceneObject->getChild("world")->getChild("snowflake")->rotateZ(0.6f * deltaTime);

    if (automatonRunning && updateCount % automatonStepEveryNthFrame == 0) {
        // std::string log = "Automaton step...\n";
        // OutputDebugStringW(std::wstring(log.begin(), log.end()).c_str());

        for (auto iceCell : iceCells) {
            iceCell->receptiveValue = iceCell->waterVaporValue + gamma;
            iceCell->nonReceptiveValue = 0.0f;
        }
        for (auto boundaryCell : boundaryCells) {
            boundaryCell->receptiveValue = boundaryCell->waterVaporValue + gamma;
            boundaryCell->nonReceptiveValue = 0.0f;
        }

        iceCells.clear();
        boundaryCells.clear();
        snowflakeModel->clearInstances();

        const auto diffusionWeight = alpha / 12.0f;
        for (int y = 0; y < hexagonLatticeHeight; y++) {
            for (int x = 0; x < hexagonLatticeWidth; x++) {
                // Diffusion
                cells[x][y].waterVaporValue = cells[x][y].nonReceptiveValue * (1.0f - diffusionWeight * 6.0f);
                for (auto neighbour : cells[x][y].neighbours) {
                    cells[x][y].waterVaporValue += neighbour->nonReceptiveValue * diffusionWeight;
                }

                // Growth
                cells[x][y].waterVaporValue += cells[x][y].receptiveValue;
            }
        }

        // Update state
        for (int y = 0; y < hexagonLatticeHeight; y++) {
            for (int x = 0; x < hexagonLatticeWidth; x++) {
                cells[x][y].receptiveValue = 0.0f;
                cells[x][y].nonReceptiveValue = cells[x][y].waterVaporValue;
                if (cells[x][y].waterVaporValue >= ICE) {
                    iceCells.push_back(&cells[x][y]);
                    snowflakeModel->addInstance({calculateCellInstancePosition(x, y)});

                    for (auto neighbour : cells[x][y].neighbours) {
                        if (neighbour->waterVaporValue < ICE) {
                            boundaryCells.push_back(neighbour);
                        }
                    }
                }
            }
        }

        if (iceCells.size() == hexagonLatticeHeight * hexagonLatticeHeight) {
            automatonRunning = false;
        }
    }
}

void SnowflakeMemoryGameScene::startGame() {
    automatonRunning = false;
}

D3DXVECTOR3 SnowflakeMemoryGameScene::calculateCellInstancePosition(int x, int y) {
    // note that this is in local space
    // also, this is before the hexagons get a 90 degree rotation, so x and y get swapped
    return D3DXVECTOR3(y * 16, x * 18 + (y % 2) * 8.5f, 0.0f);
    // return D3DXVECTOR3(x * 2 + (y % 2), y * 1.75f, 0.0f);
}
