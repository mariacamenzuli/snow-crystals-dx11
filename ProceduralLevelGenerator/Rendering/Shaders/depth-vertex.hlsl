// Global Variables
cbuffer TransformationMatricesBuffer {
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    float isInstanced;
    float3 padding;
};

// Type Definitions
// - Input
struct VertexDescriptor {
    float4 position : POSITION;

    // instance data
    float4 instancePosition : INSTANCE_POSITION;
    uint instanceId : SV_InstanceID;
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

    //if (isInstanced > 0) {
        // Update the position of the vertices based on the data for this particular instance.
     //   vertex.position.x += vertex.instancePosition.x;
     //   vertex.position.y += vertex.instancePosition.y;
      //  vertex.position.z += vertex.instancePosition.z;
   // }

    // Calculate the position of the vertex against the world, view, and projection matrices.
    pixel.position = mul(vertex.position, worldMatrix);

    if (isInstanced > 0) {
        // Update the position of the vertices based on the data for this particular instance.
        pixel.position.x += vertex.instancePosition.x;
        pixel.position.y += vertex.instancePosition.y;
        pixel.position.z += vertex.instancePosition.z;
    }

    pixel.position = mul(pixel.position, viewMatrix);

    pixel.distanceFromPointToCamera = length(pixel.position);

    pixel.position = mul(pixel.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
    pixel.depthPosition = pixel.position;

    return pixel;
}