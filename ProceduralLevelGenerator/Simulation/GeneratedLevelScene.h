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

    const static int HEXAGON_LATTICE_WIDTH = 100;
    const static int HEXAGON_LATTICE_HEIGHT = 100;

    constexpr const static float ICE = 1.0f; // cell is frozen if >= ICE

    constexpr const static float ALPHA = 1.0f;
    constexpr const static float BETA = 0.4f;
    constexpr const static float GAMMA = 0.000005f;

    ModelLoader modelLoader;
    std::unique_ptr<SceneObject> rootSceneObject;
    PointLight pointLight;

    Model* snowflakeModel;
    Cell cells[HEXAGON_LATTICE_WIDTH][HEXAGON_LATTICE_HEIGHT];
    std::vector<Cell*> iceCells;
    std::vector<Cell*> boundaryCells;

    int updateCount = 0;

    D3DXVECTOR3 calculateCellInstancePosition(int x, int y);
};
