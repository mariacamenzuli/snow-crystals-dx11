#include "ModelLoader.h"
#include "../3D Components/Models/ObjModel.h"
#include "../3D Components/Models/CubeModel.h"
#include "../3D Components/Models/ProcedurallyGeneratedTerrain.h"

ModelLoader::ModelLoader() = default;

ModelLoader::~ModelLoader() = default;

Model* ModelLoader::getModel(ModelId modelId) {
    const auto found = modelMap.find(modelId);
    if (found != modelMap.end()) {
        return found->second.get();
    } else {
        const Material cubeMaterial(D3DXVECTOR4(0.4f, 0.6f, 0.4f, 1.0f), D3DXVECTOR4(0.4f, 0.6f, 0.4f, 1.0f), D3DXVECTOR4(0.015532f, 0.005717f, 0.002170f, 1.0f));
        switch (modelId) {
        case ModelId::CUBE:
            return modelMap.insert(std::make_pair(modelId, std::make_unique<CubeModel>(CubeModel(cubeMaterial)))).first->second.get();
        default:
            throw std::runtime_error("Tried to obtain reference to an unknown model");
        }
    }
}

Model* ModelLoader::createProceduralTerrain(int height, int width) {
    return modelMap.insert(std::make_pair(ModelId::PROCEDURAL_TERRAIN, std::make_unique<ProcedurallyGeneratedTerrain>(ProcedurallyGeneratedTerrain(height, width)))).first->second.get();
}
