// Global Variables
cbuffer TransformationMatricesBuffer {
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// Type Definitions
// - Input
struct VertexDescriptor {
    float4 position : POSITION;
};

// - Output
struct PixelDescriptor {
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
    float distanceFromPointToCamera : DIST;
};

PixelDescriptor transformToScreenSpace(VertexDescriptor vertex) {
    PixelDescriptor pixel;

    // Change the position vector to be 4 units for proper matrix calculations.
    vertex.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    pixel.position = mul(vertex.position, worldMatrix);
    pixel.position = mul(pixel.position, viewMatrix);

    pixel.distanceFromPointToCamera = length(pixel.position);

    pixel.position = mul(pixel.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
    pixel.depthPosition = pixel.position;

    return pixel;
}