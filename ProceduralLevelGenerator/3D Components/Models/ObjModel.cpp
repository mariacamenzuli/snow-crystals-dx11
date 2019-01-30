#include "ObjModel.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <regex>

const std::regex regexPositionOnly(R"((\d+(?:\.\d+)?))");
const std::regex regexPositionAndTexture(R"((\d+(?:\.\d+)?)\/(\d+(?:\.\d+)?))");
const std::regex regexPositionAndNormal(R"((\d+(?:\.\d+)?)\/\/(\d+(?:\.\d+)?))");
const std::regex regexPositionTextureNormal(R"((\d+(?:\.\d+)?)\/(\d+(?:\.\d+)?)\/(\d+(?:\.\d+)?))");

ObjModel::ObjModel() = default;

ObjModel::~ObjModel() = default;

ObjModel ObjModel::loadFromFile(const std::string& filename, float vertexCorrectionX, float vertexCorrectionY, float vertexCorrectionZ) {
    std::ifstream inputStream;

    inputStream.open(filename);

    if (inputStream.fail()) {
        throw std::runtime_error("Failed to open file [" + filename + "] to load .obj model");
    }

    std::vector<D3DXVECTOR3> vertexPositions;
    std::vector<D3DXVECTOR3> normals;
    std::vector<D3DXVECTOR2> textureCoordinates;

    ObjModel model;

    int indexCount = 0;
    std::map<std::string, Material> materialMap;
    std::vector<MaterialIndexRange> materialIndexRanges;
    std::string lastUsedMaterial;
    std::string currentMaterial;
    while (!inputStream.eof()) {
        std::string line;
        std::getline(inputStream, line);

        if (line.empty()) {
            continue;
        }

        if (lineStartsWith(line, "mtllib ")) {
            auto materialFileName = line.substr(7);
            size_t filePathEnd = filename.find_last_of("/\\");
            materialMap = loadMaterialFile(filename.substr(0, filePathEnd) + "/" + materialFileName);
        }

        if (lineStartsWith(line, "usemtl ")) {
            auto materialName = line.substr(7);
            currentMaterial = materialName;
        }

        if (lineStartsWith(line, "v ")) {
            std::istringstream stringStream(line.substr(2));
            D3DXVECTOR3 vertex;
            stringStream >> vertex.x >> vertex.y >> vertex.z;
            // Invert the Z vertex to change to left hand system.
            vertex.z = vertex.z * -1.0f;
            vertex.x += vertexCorrectionX;
            vertex.y += vertexCorrectionY;
            vertex.z += vertexCorrectionZ;
            vertexPositions.push_back(vertex);
        }

        if (lineStartsWith(line, "vn ")) {
            std::istringstream stringStream(line.substr(3));
            D3DXVECTOR3 normal;
            stringStream >> normal.x >> normal.y >> normal.z;
            // Invert the Z normal to change to left hand system.
            normal.z = normal.z * -1.0f;
            normals.push_back(normal);
        }

        if (lineStartsWith(line, "vt ")) {
            std::istringstream stringStream(line.substr(3));
            D3DXVECTOR2 textureCoordinate;
            stringStream >> textureCoordinate.x >> textureCoordinate.y;
            // Invert the V texture coordinates to left hand system.
            textureCoordinate.y = 1.0f - textureCoordinate.y;
            textureCoordinates.push_back(textureCoordinate);
        }

        if (lineStartsWith(line, "f ")) {
            std::istringstream stringStream(line.substr(2));
            std::vector<std::string> faceVertices;
            std::string faceVertex;
            while (stringStream.rdbuf()->in_avail() != 0) {
                stringStream >> faceVertex;
                faceVertices.push_back(faceVertex);
            }

            int startIndex;
            if (indexCount == 0) {
                startIndex = 0;
            } else {
                startIndex = indexCount;
            }
            for (int i = faceVertices.size() - 1; i > 1; i--) {
                auto vertex1 = createVertex(faceVertices.at(i), vertexPositions, normals, textureCoordinates);
                model.vertices.push_back(vertex1);
                model.indices.push_back(indexCount);
                indexCount++;

                auto vertex2 = createVertex(faceVertices.at(i - 1), vertexPositions, normals, textureCoordinates);
                model.vertices.push_back(vertex2);
                model.indices.push_back(indexCount);
                indexCount++;

                auto vertex3 = createVertex(faceVertices.at(0), vertexPositions, normals, textureCoordinates);
                model.vertices.push_back(vertex3);
                model.indices.push_back(indexCount);
                indexCount++;
            }

            if (!currentMaterial.empty()) {
                if (currentMaterial == lastUsedMaterial) {
                    materialIndexRanges.at(materialIndexRanges.size() - 1).endInclusive = indexCount - 1;
                } else {
                    materialIndexRanges.push_back(MaterialIndexRange(startIndex, indexCount, materialMap.find(currentMaterial)->second));
                    lastUsedMaterial = currentMaterial;
                }
            }
        }
    }

    inputStream.close();

    model.materialIndexRanges = materialIndexRanges;

    return model;
}

int ObjModel::getVertexCount() {
    return vertices.size();
}

int ObjModel::getIndexCount() {
    return indices.size();
}

Model::Vertex* ObjModel::getVertices() {
    return vertices.data();
}

unsigned long* ObjModel::getIndices() {
    return indices.data();
}

std::vector<Model::MaterialIndexRange> ObjModel::getMaterialIndexRanges() {
    return materialIndexRanges;
}

bool ObjModel::lineStartsWith(std::string text, std::string prefix) {
    return text.find(prefix) == 0;
}

Model::Vertex ObjModel::createVertex(std::string vertexDescriptor, std::vector<D3DXVECTOR3>& vertexPositions, std::vector<D3DXVECTOR3>& normals, std::vector<D3DXVECTOR2>& textureCoordinates) {
    Vertex vertex;
    std::smatch regexMatches;

    if (std::regex_search(vertexDescriptor, regexMatches, regexPositionTextureNormal)) {
        vertex.position = vertexPositions[std::stoi(regexMatches[1]) - 1];
        vertex.texture = textureCoordinates[std::stoi(regexMatches[2]) - 1];
        vertex.normal = normals[std::stoi(regexMatches[3]) - 1];
    } else if (std::regex_search(vertexDescriptor, regexMatches, regexPositionAndNormal)) {
        vertex.position = vertexPositions[std::stoi(regexMatches[1]) - 1];
        vertex.texture.x = 0.0f;
        vertex.texture.y = 0.0f;
        vertex.normal = normals[std::stoi(regexMatches[2]) - 1];
    } else if (std::regex_search(vertexDescriptor, regexMatches, regexPositionAndTexture)) {
        vertex.position = vertexPositions[std::stoi(regexMatches[1]) - 1];
        vertex.texture = textureCoordinates[std::stoi(regexMatches[2]) - 1];
        vertex.normal.x = 0.0f;
        vertex.normal.y = 0.0f;
        vertex.normal.z = 0.0f;
    } else {
        vertex.position = vertexPositions[std::stoi(vertexDescriptor) - 1];
        vertex.texture.x = 0.0f;
        vertex.texture.y = 0.0f;
    }

    return vertex;
}

std::map<std::string, Material> ObjModel::loadMaterialFile(const std::string& filename) {
    std::ifstream inputStream;

    inputStream.open(filename);

    if (inputStream.fail()) {
        throw std::runtime_error("Failed to open file [" + filename + "] to load .mtl material file");
    }

    std::vector<D3DXVECTOR3> vertexPositions;
    std::vector<D3DXVECTOR3> normals;
    std::vector<D3DXVECTOR2> textureCoordinates;

    std::map<std::string, Material> materials;

    std::string currentMaterialName;
    D3DXVECTOR4 currentAmbientColor;
    D3DXVECTOR4 currentDiffuseColor;
    D3DXVECTOR4 currentSpecularColor;
    while (!inputStream.eof()) {
        std::string line;
        std::getline(inputStream, line);

        if (line.empty()) {
            continue;
        }

        if (lineStartsWith(line, "newmtl ")) {
            if (!currentMaterialName.empty()) {
                materials.insert(std::make_pair(currentMaterialName, Material(currentAmbientColor, currentDiffuseColor, currentSpecularColor)));
            }

            currentMaterialName = line.substr(7);
        }

        if (lineStartsWith(line, "Ka ")) {
            std::istringstream stringStream(line.substr(3));
            stringStream >> currentAmbientColor.x >> currentAmbientColor.y >> currentAmbientColor.z;
            currentAmbientColor.w = 1.0f;
        }

        if (lineStartsWith(line, "Kd ")) {
            std::istringstream stringStream(line.substr(3));
            stringStream >> currentDiffuseColor.x >> currentDiffuseColor.y >> currentDiffuseColor.z;
            currentDiffuseColor.w = 1.0f;
        }

        if (lineStartsWith(line, "Ks ")) {
            std::istringstream stringStream(line.substr(3));
            stringStream >> currentSpecularColor.x >> currentSpecularColor.y >> currentSpecularColor.z;
        }

        if (lineStartsWith(line, "Ns ")) {
            std::istringstream stringStream(line.substr(3));
            stringStream >> currentSpecularColor.w;
        }
    }

    materials.insert(std::make_pair(currentMaterialName, Material(currentAmbientColor, currentDiffuseColor, currentSpecularColor)));

    inputStream.close();

    return materials;
}
