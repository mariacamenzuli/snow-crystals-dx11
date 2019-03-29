#pragma once

#include "../3D Components/Scene.h"
#include "ModelLoader.h"
#include "../Util/UserInputReader.h"

class SnowflakeMemoryGameScene : public Scene {
public:
    SnowflakeMemoryGameScene(const int hexagonLatticeWidth,
                             const int hexagonLatticeHeight,
                             const int automatonStepEveryNthFrame,
                             const float alpha,
                             const float beta,
                             const float gamma);
    ~SnowflakeMemoryGameScene();

    SceneObject* getRootSceneObject() override;
    D3DXVECTOR4 getAmbientLight() override;
    PointLight* getPointLight() override;
    void update(float deltaTime);

    void incrementAutomatonStepEveryNthFrame(int increment);
    void incrementAlpha(float increment);
    void incrementGamma(float increment);
    void stopAutomaton();
    void turnRight();
    void turnLeft();

private:
    struct Cell {
        float waterVaporValue;
        float receptiveValue;
        float nonReceptiveValue;
        int x;
        int y;
        std::vector<Cell*> neighbours;

        Cell() = default;

        Cell(int x, int y, float value) : waterVaporValue(value),
                                          receptiveValue(0.0f),
                                          nonReceptiveValue(value),
                                          x(x),
                                          y(y) {
        }
    };

    enum State {
        RUNNING_AUTOMATON,
        IDLE,
        TURNING_RIGHT,
        TURNING_LEFT
    };

    constexpr const static float TURN_SPEED = 1.25f;
    constexpr const static float ICE = 1.0f; // cell is frozen if >= ICE
    constexpr const static float SIXTY_DEGREES_IN_RADIANS = 1.0472f;

    State state = RUNNING_AUTOMATON;

    int hexagonLatticeWidth;
    int hexagonLatticeHeight;

    int automatonStepEveryNthFrame;

    float alpha;
    float beta;
    float gamma;

    ModelLoader modelLoader;
    std::unique_ptr<SceneObject> rootSceneObject;
    PointLight pointLight;

    Model* snowflakeModel;
    Cell** cells;
    std::vector<Cell*> iceCells;
    std::vector<Cell*> boundaryCells;

    int updateCount = 0;

    float angleTurned = 0;

    D3DXVECTOR3 calculateCellInstancePosition(int x, int y);
    void automatonStep();
};
