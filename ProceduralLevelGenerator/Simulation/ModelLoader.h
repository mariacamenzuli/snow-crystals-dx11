#pragma once

#include <map>
#include <memory>

#include "../3D Components/Models/Model.h"

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    enum class ModelId {
        CUBE,
        PROCEDURAL_TERRAIN,
        HEXAGON
    };

    Model* getModel(ModelId modelId);

private:
    std::map<ModelId, std::unique_ptr<Model>> modelMap;
};

