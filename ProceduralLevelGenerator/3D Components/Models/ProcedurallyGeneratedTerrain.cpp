#include "ProcedurallyGeneratedTerrain.h"

ProcedurallyGeneratedTerrain::ProcedurallyGeneratedTerrain(int width, int height) : width(width), height(height) {
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
        }
    }

    calculateHeightMapNormals();

    generateVerticesAndIndicesFromHeightMap();

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

void ProcedurallyGeneratedTerrain::mutate() {
    vertices.clear();
    indices.clear();

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int index = (height * j) + i;

            heightMap[index].x = static_cast<float>(i);
            heightMap[index].y = static_cast<float>(sin(static_cast<float>(i) / (width / 12)) * 3.0); //magic numbers ahoy, just to ramp up the height of the sin function so its visible.
            heightMap[index].z = static_cast<float>(j);
        }
    }

    calculateHeightMapNormals();
    generateVerticesAndIndicesFromHeightMap();
}

void ProcedurallyGeneratedTerrain::calculateHeightMapNormals() {
    // Go through all the faces in the mesh and calculate their normals.
    const int faceCountInHeight = height - 1;
    const int faceCountInWidth = width - 1;

    std::vector<D3DXVECTOR3> faceNormals(faceCountInHeight * faceCountInWidth);

    for (int z = 0; z < faceCountInHeight; z++) {
        for (int x = 0; x < faceCountInWidth; x++) {
            // Get three vertices from the face.
            int index1 = (z * height) + x;
            int index2 = (z * height) + (x + 1);
            int index3 = ((z + 1) * height) + x;

            auto vertex1 = heightMap[index1];
            auto vertex2 = heightMap[index2];
            auto vertex3 = heightMap[index3];

            // Calculate the two vectors for this face.
            auto x1 = vertex1.x - vertex3.x;
            auto y1 = vertex1.y - vertex3.y;
            auto z1 = vertex1.z - vertex3.z;
            auto x2 = vertex3.x - vertex2.x;
            auto y2 = vertex3.y - vertex2.y;
            auto z2 = vertex3.z - vertex2.z;

            int index = (z * (height - 1)) + x;

            // Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
            faceNormals[index] = (D3DXVECTOR3((y1 * z2) - (z1 * y2),
                                              (z1 * x2) - (x1 * z2),
                                              (x1 * y2) - (y1 * x2)));
        }
    }

    // Now go through all the vertices and take an average of each face normal 	
    // that the vertex touches to get the averaged normal for that vertex.
    float sum[3];
    int count;
    int index;
    float length;
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            sum[0] = 0.0f;
            sum[1] = 0.0f;
            sum[2] = 0.0f;
            
            count = 0;

            // Bottom left face.
            if (((x - 1) >= 0) && ((z - 1) >= 0)) {
                index = ((z - 1) * (height - 1)) + (x - 1);

                sum[0] += faceNormals[index].x;
                sum[1] += faceNormals[index].y;
                sum[2] += faceNormals[index].z;
                count++;
            }

            // Bottom right face.
            if ((x < (width - 1)) && ((z - 1) >= 0)) {
                index = ((z - 1) * (height - 1)) + x;

                sum[0] += faceNormals[index].x;
                sum[1] += faceNormals[index].y;
                sum[2] += faceNormals[index].z;
                count++;
            }

            // Upper left face.
            if (((x - 1) >= 0) && (z < (height - 1))) {
                index = (z * (height - 1)) + (x - 1);

                sum[0] += faceNormals[index].x;
                sum[1] += faceNormals[index].y;
                sum[2] += faceNormals[index].z;
                count++;
            }

            // Upper right face.
            if ((x < (width - 1)) && (z < (height - 1))) {
                index = (z * (height - 1)) + x;

                sum[0] += faceNormals[index].x;
                sum[1] += faceNormals[index].y;
                sum[2] += faceNormals[index].z;
                count++;
            }

            // Take the average of the faces touching this vertex.
            sum[0] = sum[0] / static_cast<float>(count);
            sum[1] = sum[1] / static_cast<float>(count);
            sum[2] = sum[2] / static_cast<float>(count);

            // Calculate the length of this normal.
            length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

            // Get an index to the vertex location in the height map array.
            index = (z * height) + x;

            // Normalize the final shared normal for this vertex and store it in the height map array.
            heightMap[index].nx = (sum[0] / length);
            heightMap[index].ny = (sum[1] / length);
            heightMap[index].nz = (sum[2] / length);
        }
    }
}

void ProcedurallyGeneratedTerrain::generateVerticesAndIndicesFromHeightMap() {
    int currentIndex = 0;
    const int faceCountInHeight = height - 1;
    const int faceCountInWidth = width - 1;
    for (int z = 0; z < faceCountInHeight; z++) {
        for (int x = 0; x < faceCountInWidth; x++) {
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
}
