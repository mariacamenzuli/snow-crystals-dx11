#pragma once

#include <map>
#include <memory>

#include "../3D Components/Models/Model.h"
#include "../3D Components/Models/ProcedurallyGeneratedTerrain.h"

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    enum class ModelId {
        CUBE,
        PROCEDURAL_TERRAIN
    };

    Model* getModel(ModelId modelId);
    ProcedurallyGeneratedTerrain* createProceduralTerrain(int height, int width);

private:
    std::map<ModelId, std::unique_ptr<Model>> modelMap;
};

