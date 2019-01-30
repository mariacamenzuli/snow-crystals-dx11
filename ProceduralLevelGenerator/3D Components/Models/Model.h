#pragma once

#include <D3DX10.h>
#include <vector>

#include "../Material.h"

class Model {
public:
    struct Vertex {
        D3DXVECTOR3 position;
        D3DXVECTOR3 normal;
        D3DXVECTOR2 texture;
    };

    struct MaterialIndexRange {
        int startInclusive;
        int endInclusive;
        Material material;

        MaterialIndexRange(int startInclusive, int endInclusive, const Material& material) : startInclusive(startInclusive), endInclusive(endInclusive), material(material) {
        }
    };

    virtual ~Model() = default;;

    virtual int getVertexCount() = 0;
    virtual int getIndexCount() = 0;
    virtual Vertex* getVertices() = 0;
    virtual unsigned long* getIndices() = 0;
    virtual std::vector<MaterialIndexRange> getMaterialIndexRanges() = 0;
};
