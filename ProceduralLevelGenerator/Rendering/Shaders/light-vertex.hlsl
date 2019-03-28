// Global Variables
cbuffer TransformationMatricesBuffer {
    float4x4 objectWorldMatrix;
    float4x4 cameraViewMatrix;
    float4x4 cameraProjectionMatrix;
    float isInstanced;
    float3 padding;
};

cbuffer CameraBuffer {
    float4 cameraPosition;
};

// Type Definitions
// - Input
struct VertexDescriptor {
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;

    // instance data
    float4 instancePosition : INSTANCE_POSITION;
    uint instanceId : SV_InstanceID;
};

// - Output
struct PixelDescriptor {
    float4 screenSpacePosition : SV_POSITION;
    float4 worldSpacePosition : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float3 viewDirection : VIEWDIR;
};

PixelDescriptor transformToScreenSpace(VertexDescriptor vertex) {
    PixelDescriptor pixel;

    // Set the w component (since it is not going to be passed along to the shader)
    vertex.position.w = 1.0f;

    if (isInstanced > 0) {
        // Update the position of the vertices based on the data for this particular instance.
        vertex.position.x += vertex.instancePosition.x;
        vertex.position.y += vertex.instancePosition.y;
        vertex.position.z += vertex.instancePosition.z;
    }

    // Calculate the screen space position of the vertex against the world, view, and projection matrices
    pixel.worldSpacePosition = mul(vertex.position, objectWorldMatrix);
    pixel.screenSpacePosition = mul(pixel.worldSpacePosition, cameraViewMatrix);
    pixel.screenSpacePosition = mul(pixel.screenSpacePosition, cameraProjectionMatrix);

    // Store the texture coordinates for the pixel shader.
    pixel.tex = vertex.tex;

    // Calculate the normal vector against the world matrix only
    pixel.normal = mul(vertex.normal, (float3x3) objectWorldMatrix);

    // Normalize the normal vector.
    pixel.normal = normalize(pixel.normal);

    // Calculate the viewing direction vector.
    pixel.viewDirection = cameraPosition.xyz - pixel.worldSpacePosition.xyz;

    // Normalize the viewing direction vector.
    pixel.viewDirection = normalize(pixel.viewDirection);

    return pixel;
}