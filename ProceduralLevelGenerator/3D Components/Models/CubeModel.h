#pragma once

#include "Model.h"

class CubeModel : public Model {
public:
    CubeModel(Material material);
    ~CubeModel();

    int getVertexCount() override;
    int getIndexCount() override;
    Vertex* getVertices() override;
    unsigned long* getIndices() override;
    std::vector<MaterialIndexRange> getMaterialIndexRanges() override;
private:
    const int vertexCount = 36;
    const int indexCount = 36;
    Vertex vertices[36];
    unsigned long indices[36];
    std::vector<MaterialIndexRange> materialIndexRanges;
};
