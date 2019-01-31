#pragma once

#include "Model.h"

class ProcedurallyGeneratedTerrain : public Model {
public:
    ProcedurallyGeneratedTerrain(int width, int height);
    ~ProcedurallyGeneratedTerrain();

    int getVertexCount() override;
    int getIndexCount() override;
    Vertex* getVertices() override;
    unsigned long* getIndices() override;
    std::vector<MaterialIndexRange> getMaterialIndexRanges() override;

    void mutate();

private:
    struct HeightMapPoint {
        float x, y, z;
        float nx, ny, nz;
    };

    int width;
    int height;
    std::vector<HeightMapPoint> heightMap;
    std::vector<Vertex> vertices;
    std::vector<unsigned long> indices;
    std::vector<MaterialIndexRange> materialIndexRanges;

    void calculateHeightMapNormals();
    void generateVerticesAndIndicesFromHeightMap();
};

