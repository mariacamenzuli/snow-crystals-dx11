// Type Definitions
// - Input
struct PixelDescriptor {
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
    float distanceFromPointToCamera : DIST;
};

float4 calculateDepth(PixelDescriptor pixel) : SV_TARGET {
    // Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
    float depthValue = pixel.depthPosition.z / pixel.depthPosition.w;

    return float4(pixel.distanceFromPointToCamera, depthValue, depthValue, 1.0f);
}