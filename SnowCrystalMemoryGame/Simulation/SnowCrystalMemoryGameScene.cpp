#include "SnowCrystalMemoryGameScene.h"
#include <ctime>
#include <thread>

SnowCrystalMemoryGameScene::SnowCrystalMemoryGameScene(const int hexagonLatticeWidth,
                                                   const int hexagonLatticeHeight,
                                                   const float alpha,
                                                   const float beta,
                                                   const float gamma) : hexagonLatticeWidth(hexagonLatticeWidth),
                                                                        hexagonLatticeHeight(hexagonLatticeHeight),
                                                                        alpha(alpha),
                                                                        beta(beta),
                                                                        gamma(gamma),
                                                                        pointLight(D3DXVECTOR4(1.0f, 1.0f, 0.9f, 1.0f), D3DXVECTOR4(0.5f, 0.5f, 0.4f, 1.0f)) {
    rootSceneObject.reset(new SceneObject());

    auto world = rootSceneObject->attachChild(std::make_unique<SceneObject>(), "world");

    snowflakeModel = modelLoader.getModel(ModelLoader::ModelId::HEXAGON);

    auto hexLattice = world->attachChild(std::make_unique<SceneObject>(snowflakeModel), "snowflake");
    hexLattice->translate(- (hexagonLatticeHeight / 2) * 16, - (hexagonLatticeWidth / 2) * 18, 0.0f);
    hexLattice->scale(0.105f, 0.105f, 0.805f);
    hexLattice->rotateZ(1.5708f);
    hexLattice->rotateY(3.14159f);

    pointLight.translate(0.0f, -12.5f, -50.0f);

    std::thread thread(&SnowCrystalMemoryGameScene::snowflakeGeneratorThread, this);
    thread.detach();

    srand(time(0));
}

SnowCrystalMemoryGameScene::~SnowCrystalMemoryGameScene() {
    gameState = GameState::SHUTTING_DOWN;

    while(snowflakeState == SnowflakeState::GROWING) {
        // no-op, waiting for thread to stop
    }
};

SceneObject* SnowCrystalMemoryGameScene::getRootSceneObject() {
    return rootSceneObject.get();
}

D3DXVECTOR4 SnowCrystalMemoryGameScene::getAmbientLight() {
    return D3DXVECTOR4(0.3f, 0.3f, 0.3f, 1.0f);
}

PointLight* SnowCrystalMemoryGameScene::getPointLight() {
    return &pointLight;
}

void SnowCrystalMemoryGameScene::update(float deltaTime) {
    switch (snowflakeState) {
    case SnowflakeState::GROWING:
        progressSimulation = true;
        break;
    case SnowflakeState::IDLE:
        if (gameState == GameState::SHOWING_SEQUENCE && game.sequenceToDisplay.empty()) {
            OutputDebugString(L"Accepting sequence input...\n");
            gameState = GameState::ACCEPTING_SEQUENCE;
        } else if (!game.sequenceToDisplay.empty()) {
            auto snowflakeTurnDisplay = game.sequenceToDisplay.front();
            game.sequenceToDisplay.pop();
            currentTurnDisplay = snowflakeTurnDisplay;
            if (currentTurnDisplay.isLeftTurn) {
                turnSnowflakeLeft();
            } else {
                turnSnowflakeRight();
            }
        }
        break;
    case SnowflakeState::TURNING_RIGHT:
        if (angleTurned > - SIXTY_DEGREES_IN_RADIANS) {
            const auto angleRemaining = - SIXTY_DEGREES_IN_RADIANS - angleTurned;
            const auto angleIncrement = - TURN_SPEED * deltaTime;
            const auto clampedAngleIncrement = fmax(angleRemaining, angleIncrement);
            rootSceneObject->getChild("world")->getChild("snowflake")->rotateZ(clampedAngleIncrement);
            angleTurned += clampedAngleIncrement;
        } else {
            snowflakeState = SnowflakeState::IDLE;
        }
        break;
    case SnowflakeState::TURNING_LEFT:
        if (angleTurned < SIXTY_DEGREES_IN_RADIANS) {
            const auto angleRemaining = SIXTY_DEGREES_IN_RADIANS - angleTurned;
            const auto angleIncrement = TURN_SPEED * deltaTime;
            const auto clampedAngleIncrement = fmin(angleRemaining, angleIncrement);
            rootSceneObject->getChild("world")->getChild("snowflake")->rotateZ(clampedAngleIncrement);
            angleTurned += clampedAngleIncrement;
        } else {
            snowflakeState = SnowflakeState::IDLE;
        }
        break;
    default: ;
    }
}

void SnowCrystalMemoryGameScene::incrementAlpha(float increment) {
    alpha = fmax(0, alpha + increment);
}

void SnowCrystalMemoryGameScene::incrementGamma(float increment) {
    gamma = fmax(0, gamma + increment);
}

void SnowCrystalMemoryGameScene::startGame() {
    if (gameState == GameState::INIT) {
        OutputDebugString(L"Starting game...\n");
        snowflakeState = SnowflakeState::IDLE;
        generateTurnSequence();
    }
}

void SnowCrystalMemoryGameScene::inputRightTurn() {
    if (gameState == GameState::ACCEPTING_SEQUENCE && snowflakeState == SnowflakeState::IDLE && !game.sequenceInputExpected.empty()) {
        auto expectingTurnRight = !game.sequenceInputExpected.front();
        game.sequenceInputExpected.pop();

        int sequenceLengthIncrement = 0;
        if (expectingTurnRight) {
            game.sequenceToDisplay.push({ false, true });
            sequenceLengthIncrement = 1;
        } else {
            while (!game.sequenceInputExpected.empty()) {
                game.sequenceInputExpected.pop();
            }
        }

        if (game.sequenceInputExpected.empty()) {
            game.sequenceLength += sequenceLengthIncrement;
            generateTurnSequence();
        }
    }
}

void SnowCrystalMemoryGameScene::inputLeftTurn() {
    if (gameState == GameState::ACCEPTING_SEQUENCE && snowflakeState == SnowflakeState::IDLE && !game.sequenceInputExpected.empty()) {
        auto expectingTurnLeft = game.sequenceInputExpected.front();
        game.sequenceInputExpected.pop();

        int sequenceLengthIncrement = 0;
        if (expectingTurnLeft) {
            game.sequenceToDisplay.push({ true, true });
            sequenceLengthIncrement = 1;
        } else {
            while (!game.sequenceInputExpected.empty()) {
                game.sequenceInputExpected.pop();
            }
        }

        if (game.sequenceInputExpected.empty()) {
            game.sequenceLength += sequenceLengthIncrement;
            generateTurnSequence();
        }
    }
}

bool SnowCrystalMemoryGameScene::isShowingSequenceToMemorize() {
    return gameState == GameState::SHOWING_SEQUENCE && !currentTurnDisplay.isPlayerInput;
}

D3DXVECTOR3 SnowCrystalMemoryGameScene::calculateCellInstancePosition(int x, int y) {
    // note that this is in local space
    // also, this is before the hexagons get a 90 degree rotation, so x and y get swapped
    return D3DXVECTOR3(y * 16, x * 18 + (y % 2) * 8.5f, 0.0f);
}

std::vector<Model::Instance> SnowCrystalMemoryGameScene::progressSnowflakeGrowingAutomaton(SnowflakeAutomatonCell** cells) {
    std::vector<Model::Instance> instances;

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
                instances.push_back({ calculateCellInstancePosition(x, y) });

                for (auto neighbour : cells[x][y].neighbours) {
                    if (neighbour->waterVaporValue < ICE) {
                        boundaryCells.push_back(neighbour);
                    }
                }
            }
        }
    }

    if (iceCells.size() == hexagonLatticeHeight * hexagonLatticeHeight) {
        startGame();
    }

    return instances;
}

void SnowCrystalMemoryGameScene::turnSnowflakeRight() {
    if (snowflakeState == SnowflakeState::IDLE) {
        snowflakeState = SnowflakeState::TURNING_RIGHT;
        angleTurned = 0;
    }
}

void SnowCrystalMemoryGameScene::turnSnowflakeLeft() {
    if (snowflakeState == SnowflakeState::IDLE) {
        snowflakeState = SnowflakeState::TURNING_LEFT;
        angleTurned = 0;
    }
}

void SnowCrystalMemoryGameScene::generateTurnSequence() {
    OutputDebugString(L"Generating new turn sequence...\n");
    for (int i = 0; i < game.sequenceLength; i++) {
        const auto turnLeft = rand() % 2 == 0;
        game.sequenceToDisplay.push({ turnLeft, false });
        game.sequenceInputExpected.push(turnLeft);
    }
    gameState = GameState::SHOWING_SEQUENCE;
}

void SnowCrystalMemoryGameScene::snowflakeGeneratorThread() {
    SnowflakeAutomatonCell** cells = new SnowflakeAutomatonCell*[hexagonLatticeWidth];
    for (int i = 0; i < hexagonLatticeWidth; ++i) {
        cells[i] = new SnowflakeAutomatonCell[hexagonLatticeHeight];
    }

    for (int y = 0; y < hexagonLatticeHeight; y++) {
        int t = y % 2;
        for (int x = 0; x < hexagonLatticeWidth; x++) {
            cells[x][y] = { x, y, beta };
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
    snowflakeModel->addInstance({ calculateCellInstancePosition(seedX, seedY) });

    iceCells.push_back(&cells[seedX][seedY]);
    for (auto iceCell : iceCells) {
        for (auto neighbour : iceCell->neighbours) {
            if (neighbour->waterVaporValue < ICE) {
                boundaryCells.push_back(neighbour);
            }
        }
    }

    while (snowflakeState == SnowflakeState::GROWING) {
        if (gameState == GameState::SHUTTING_DOWN) {
            snowflakeState = SnowflakeState::IDLE;
            continue;
        }

        if (progressSimulation) {
            auto updatedInstances = progressSnowflakeGrowingAutomaton(cells);
            snowflakeModel->overwriteInstances(std::move(updatedInstances));
            progressSimulation = false;
        }
    }

    for (int i = 0; i < hexagonLatticeWidth; ++i) {
        delete[] cells[i];
    }
    delete[] cells;
}
