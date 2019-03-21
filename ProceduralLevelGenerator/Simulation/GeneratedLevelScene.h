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
        ICE, AIR
    };

    struct Cell {
        CellState state;
        SceneObject* sceneObject;

        void kill() {
            state = CellState::AIR;
            sceneObject->hide();
        }

        void spawn() {
            state = CellState::ICE;
            sceneObject->show();
        }
    };

    const static int CELL_CUBE_WIDTH = 20;

    ModelLoader modelLoader;
    std::unique_ptr<SceneObject> rootSceneObject;
    PointLight pointLight;
    Cell cells[CELL_CUBE_WIDTH][CELL_CUBE_WIDTH];
};
