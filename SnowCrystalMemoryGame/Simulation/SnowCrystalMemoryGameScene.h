#pragma once

#include "../3D Components/Scene.h"
#include "ModelLoader.h"
#include "../Util/UserInputReader.h"
#include <queue>

class SnowCrystalMemoryGameScene : public Scene {
public:
    SnowCrystalMemoryGameScene(const int hexagonLatticeWidth,
                             const int hexagonLatticeHeight,
                             const float alpha,
                             const float beta,
                             const float gamma);
    ~SnowCrystalMemoryGameScene();

    SceneObject* getRootSceneObject() override;
    D3DXVECTOR4 getAmbientLight() override;
    PointLight* getPointLight() override;
    void update(float deltaTime);

    void incrementAlpha(float increment);
    void incrementGamma(float increment);
    void startGame();
    void inputRightTurn();
    void inputLeftTurn();
    bool isShowingSequenceToMemorize();

private:
    struct SnowflakeAutomatonCell {
        float waterVaporValue;
        float receptiveValue;
        float nonReceptiveValue;
        int x;
        int y;
        std::vector<SnowflakeAutomatonCell*> neighbours;

        SnowflakeAutomatonCell() = default;

        SnowflakeAutomatonCell(int x, int y, float value) : waterVaporValue(value),
                                                            receptiveValue(0.0f),
                                                            nonReceptiveValue(value),
                                                            x(x),
                                                            y(y) {
        }
    };

    struct SnowflakeTurnDisplay {
        boolean isLeftTurn;
        boolean isPlayerInput;
    };

    struct Game {
        int sequenceLength = 2;
        std::queue<SnowflakeTurnDisplay> sequenceToDisplay;
        std::queue<boolean> sequenceInputExpected;
    };

    enum class SnowflakeState {
        GROWING,
        IDLE,
        TURNING_RIGHT,
        TURNING_LEFT
    };

    enum class GameState {
        INIT,
        SHOWING_SEQUENCE,
        ACCEPTING_SEQUENCE,
        SHUTTING_DOWN
    };

    constexpr const static float TURN_SPEED = 1.25f;
    constexpr const static float ICE = 1.0f; // cell is frozen if >= ICE
    constexpr const static float SIXTY_DEGREES_IN_RADIANS = 1.0472f;

    SnowflakeState snowflakeState = SnowflakeState::GROWING;
    GameState gameState = GameState::INIT;

    Game game;

    int hexagonLatticeWidth;
    int hexagonLatticeHeight;

    float alpha;
    float beta;
    float gamma;

    ModelLoader modelLoader;
    std::unique_ptr<SceneObject> rootSceneObject;
    PointLight pointLight;

    Model* snowflakeModel;
    std::vector<SnowflakeAutomatonCell*> iceCells;
    std::vector<SnowflakeAutomatonCell*> boundaryCells;

    int updateCount = 0;

    float angleTurned = 0;
    SnowflakeTurnDisplay currentTurnDisplay;

    bool progressSimulation = false;

    D3DXVECTOR3 calculateCellInstancePosition(int x, int y);
    std::vector<Model::Instance> progressSnowflakeGrowingAutomaton(SnowflakeAutomatonCell** cells);
    void turnSnowflakeRight();
    void turnSnowflakeLeft();
    void generateTurnSequence();

    void snowflakeGeneratorThread();
};
