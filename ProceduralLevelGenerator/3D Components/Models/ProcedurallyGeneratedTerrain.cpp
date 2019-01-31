#include "ProcedurallyGeneratedTerrain.h"

ProcedurallyGeneratedTerrain::ProcedurallyGeneratedTerrain(int width, int height) {
    if (width < 2 || height < 2) {
        throw std::runtime_error("Width and height of procedurally generated terrain cannot be smaller than 2.");
    }

    float initialHeight = 0.0f;

    heightMap.resize(height * width);
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int index = (height * j) + i;

            heightMap[index].x = static_cast<float>(i);
            heightMap[index].y = static_cast<float>(initialHeight);
            heightMap[index].z = static_cast<float>(j);
            
            //todo
            heightMap[index].nx = 0;
            heightMap[index].ny = 1;
            heightMap[index].nz = 0;
        }
    }

    int currentIndex = 0;
    for (int z = 0; z < (height - 1); z++) {
        for (int x = 0; x < (width - 1); x++) {
            int bottomLeftIndex = (height * z) + x;
            int bottomRightIndex = (height * z) + (x + 1);
            int upperLeftIndex = (height * (z + 1)) + x;
            int upperRightIndex = (height * (z + 1)) + (x + 1);

            Vertex upperLeftVertex;
            upperLeftVertex.position = D3DXVECTOR3(heightMap[upperLeftIndex].x, heightMap[upperLeftIndex].y, heightMap[upperLeftIndex].z);
            upperLeftVertex.normal = D3DXVECTOR3(heightMap[upperLeftIndex].nx, heightMap[upperLeftIndex].ny, heightMap[upperLeftIndex].nz);
            vertices.push_back(upperLeftVertex);
            indices.push_back(currentIndex);
            currentIndex++;

            Vertex upperRightVertex;
            upperRightVertex.position = D3DXVECTOR3(heightMap[upperRightIndex].x, heightMap[upperRightIndex].y, heightMap[upperRightIndex].z);
            upperRightVertex.normal = D3DXVECTOR3(heightMap[upperRightIndex].nx, heightMap[upperRightIndex].ny, heightMap[upperRightIndex].nz);
            vertices.push_back(upperRightVertex);
            indices.push_back(currentIndex);
            currentIndex++;

            Vertex bottomLeftVertex;
            bottomLeftVertex.position = D3DXVECTOR3(heightMap[bottomLeftIndex].x, heightMap[bottomLeftIndex].y, heightMap[bottomLeftIndex].z);
            bottomLeftVertex.normal = D3DXVECTOR3(heightMap[bottomLeftIndex].nx, heightMap[bottomLeftIndex].ny, heightMap[bottomLeftIndex].nz);
            vertices.push_back(bottomLeftVertex);
            indices.push_back(currentIndex);
            currentIndex++;

            vertices.push_back(bottomLeftVertex);
            indices.push_back(currentIndex);
            currentIndex++;

            vertices.push_back(upperRightVertex);
            indices.push_back(currentIndex);
            currentIndex++;

            Vertex bottomRightVertex;
            bottomRightVertex.position = D3DXVECTOR3(heightMap[bottomRightIndex].x, heightMap[bottomRightIndex].y, heightMap[bottomRightIndex].z);
            bottomRightVertex.normal = D3DXVECTOR3(heightMap[bottomRightIndex].nx, heightMap[bottomRightIndex].ny, heightMap[bottomRightIndex].nz);
            vertices.push_back(bottomRightVertex);
            indices.push_back(currentIndex);
            currentIndex++;
        }
    }

    const Material terrainMaterial(D3DXVECTOR4(0.4f, 0.6f, 0.4f, 1.0f), D3DXVECTOR4(0.4f, 0.6f, 0.4f, 1.0f), D3DXVECTOR4(0.015532f, 0.005717f, 0.002170f, 1.0f));
    materialIndexRanges.push_back(MaterialIndexRange(0, getIndexCount(), terrainMaterial));
}

ProcedurallyGeneratedTerrain::~ProcedurallyGeneratedTerrain() = default;

int ProcedurallyGeneratedTerrain::getVertexCount() {
    return vertices.size();
}

int ProcedurallyGeneratedTerrain::getIndexCount() {
    return indices.size();
}

Model::Vertex* ProcedurallyGeneratedTerrain::getVertices() {
    return vertices.data();
}

unsigned long* ProcedurallyGeneratedTerrain::getIndices() {
    return indices.data();
}

std::vector<Model::MaterialIndexRange> ProcedurallyGeneratedTerrain::getMaterialIndexRanges() {
    return materialIndexRanges;
}
