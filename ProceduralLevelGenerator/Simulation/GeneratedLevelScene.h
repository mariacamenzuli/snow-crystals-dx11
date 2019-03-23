#pragma once

#include "../3D Components/Scene.h"
#include "ModelLoader.h"
#include "../Util/UserInputReader.h"

class GeneratedLevelScene : public Scene {
public:
    GeneratedLevelScene();
    ~GeneratedLevelScene();

    SceneObject* getRootSceneObject() override;
    D3DXVECTOR4 getAmbientLight() override;
    PointLight* getPointLight() override;
    void update(float deltaTime);
private:
    enum class CellState {
        ICE,
        AIR
    };

    struct Cell {
        CellState state;
        SceneObject* sceneObject;
        float value; // water vapor
        float receptiveValue;
        float nonReceptiveValue;
        int x;
        int y;
        std::vector<Cell*> neighbours;

        Cell() = default;

        Cell(int x, int y, CellState state, SceneObject* sceneObject, float value)
            : x(x),
              y(y),
              state(state),
              sceneObject(sceneObject),
              value(value),
              receptiveValue(0.0f),
              nonReceptiveValue(value) {
            if (state == CellState::AIR) {
                sceneObject->hide();
            }
        }

        void thaw() {
            state = CellState::AIR;
            sceneObject->hide();
        }

        void freeze() {
            state = CellState::ICE;
            sceneObject->show();
        }
    };

    const static int HEXAGON_LATTICE_WIDTH = 25;
    const static int HEXAGON_LATTICE_HEIGHT = 25;
    constexpr const static float ICE = 1.0f;
    constexpr const static float ALPHA = 1.0f;
    constexpr const static float BETA = 0.4f;
    constexpr const static float GAMMA = 0.000005f;

    ModelLoader modelLoader;
    std::unique_ptr<SceneObject> rootSceneObject;
    PointLight pointLight;
    Cell cells[HEXAGON_LATTICE_WIDTH][HEXAGON_LATTICE_HEIGHT];
    std::vector<Cell*> ice;
    std::vector<Cell*> boundary;
    int updateCount = 0;
};
