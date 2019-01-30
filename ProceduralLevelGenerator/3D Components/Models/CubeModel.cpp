#include "CubeModel.h"

CubeModel::CubeModel(Material material) {
    // face 1 -ve z
    vertices[0].position = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
    vertices[0].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[1].position = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
    vertices[1].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    vertices[1].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[2].position = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    vertices[2].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    vertices[2].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[3].position = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    vertices[3].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    vertices[3].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[4].position = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
    vertices[4].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    vertices[4].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[5].position = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
    vertices[5].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    vertices[5].texture = D3DXVECTOR2(0.0f, 0.0f);

    // face 2 +ve y
    vertices[6].position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
    vertices[6].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    vertices[6].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[7].position = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    vertices[7].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    vertices[7].texture = D3DXVECTOR2(3.0f, 3.0f);

    vertices[8].position = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
    vertices[8].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    vertices[8].texture = D3DXVECTOR2(0.0f, 3.0f);

    vertices[9].position = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    vertices[9].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    vertices[9].texture = D3DXVECTOR2(3.0f, 0.0f);

    vertices[10].position = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    vertices[10].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    vertices[10].texture = D3DXVECTOR2(3.0f, 3.0f);

    vertices[11].position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
    vertices[11].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    vertices[11].texture = D3DXVECTOR2(0.0f, 0.0f);

    // face 3 +v z
    vertices[12].position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
    vertices[12].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    vertices[12].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[13].position = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    vertices[13].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    vertices[13].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[14].position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
    vertices[14].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    vertices[14].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[15].position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
    vertices[15].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    vertices[15].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[16].position = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    vertices[16].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    vertices[16].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[17].position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
    vertices[17].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    vertices[17].texture = D3DXVECTOR2(0.0f, 0.0f);

    // face 4 -v y
    vertices[18].position = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    vertices[18].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
    vertices[18].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[19].position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
    vertices[19].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
    vertices[19].texture = D3DXVECTOR2(1.0f, 1.0f);

    vertices[20].position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
    vertices[20].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
    vertices[20].texture = D3DXVECTOR2(0.0f, 1.0f);

    vertices[21].position = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
    vertices[21].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
    vertices[21].texture = D3DXVECTOR2(1.0f, 0.0f);

    vertices[22].position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
    vertices[22].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
    vertices[22].texture = D3DXVECTOR2(1.0f, 1.0f);

    vertices[23].position = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    vertices[23].normal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
    vertices[23].texture = D3DXVECTOR2(0.0f, 0.0f);

    // face 5 +ve x
    vertices[24].position = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    vertices[24].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    vertices[24].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[25].position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
    vertices[25].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    vertices[25].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[26].position = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
    vertices[26].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    vertices[26].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[27].position = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    vertices[27].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    vertices[27].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[28].position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
    vertices[28].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    vertices[28].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[29].position = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    vertices[29].normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    vertices[29].texture = D3DXVECTOR2(0.0f, 0.0f);

    // face 6 -ve x
    vertices[30].position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
    vertices[30].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
    vertices[30].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[31].position = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    vertices[31].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
    vertices[31].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[32].position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
    vertices[32].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
    vertices[32].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[33].position = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
    vertices[33].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
    vertices[33].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[34].position = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    vertices[34].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
    vertices[34].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[35].position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
    vertices[35].normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
    vertices[35].texture = D3DXVECTOR2(0.0f, 0.0f);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 3;
    indices[4] = 4;
    indices[5] = 5;
    indices[6] = 6;
    indices[7] = 7;
    indices[8] = 8;
    indices[9] = 9;
    indices[10] = 10;
    indices[11] = 11;
    indices[12] = 12;
    indices[13] = 13;
    indices[14] = 14;
    indices[15] = 15;
    indices[16] = 16;
    indices[17] = 17;
    indices[18] = 18;
    indices[19] = 19;
    indices[20] = 20;
    indices[21] = 21;
    indices[22] = 22;
    indices[23] = 23;
    indices[24] = 24;
    indices[25] = 25;
    indices[26] = 26;
    indices[27] = 27;
    indices[28] = 28;
    indices[29] = 29;
    indices[30] = 30;
    indices[31] = 31;
    indices[32] = 32;
    indices[33] = 33;
    indices[34] = 34;
    indices[35] = 35;

    materialIndexRanges.push_back(MaterialIndexRange(0, 35, material));
}

CubeModel::~CubeModel() = default;

int CubeModel::getVertexCount() {
    return vertexCount;
}

int CubeModel::getIndexCount() {
    return indexCount;
}

Model::Vertex* CubeModel::getVertices() {
    return vertices;
}

unsigned long* CubeModel::getIndices() {
    return indices;
}

std::vector<Model::MaterialIndexRange> CubeModel::getMaterialIndexRanges() {
    return materialIndexRanges;
}
