Texture2D materialTexture : register(t0);
TextureCube depthMapTexture : register(t1);

SamplerState materialSampler : register(s0);
SamplerState depthMapSampler : register(s1);

cbuffer AmbientLightBuffer {
    float4 sceneAmbientColor;
};

cbuffer PointLightBuffer {
    float4 pointLightPosition;
    float4 pointLightDiffuse;
    float4 pointLightSpecular;
};

cbuffer MaterialBuffer {
    float4 materialAmbientColor;
    float4 materialDiffuseColor;
    float4 materialSpecularColor; // w component is specular power
    int materialIsTextured; // -ve if not textured, +ve if textured
};

// Type Definitions
// - Input
struct PixelDescriptor {
    float4 screenSpacePosition : SV_POSITION;
    float4 worldSpacePosition : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float3 viewDirection : VIEWDIR;
};

float4 sceneAmbience(PixelDescriptor pixel) {
    if (materialIsTextured >= 0) {
        float4 textureColor = materialTexture.Sample(materialSampler, pixel.tex);
        return  textureColor * sceneAmbientColor;
    } else {
        return materialAmbientColor * sceneAmbientColor;
    }
}

bool isInPointLightShadow(PixelDescriptor pixel) {
    float3 shadowMapSamplerVector = pixel.worldSpacePosition - pointLightPosition;

    // Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
    float shadowMapDepthValue = depthMapTexture.Sample(depthMapSampler, shadowMapSamplerVector).r;

    // Calculate the depth of the pixel.
    //float pixelDepthValue = pixel.lightViewPosition.z / pixel.lightViewPosition.w;
    float pixelDepthValue = length(shadowMapSamplerVector);

    // Set the bias value for fixing the floating point precision issues.
    float bias = 0.555f;

    // Subtract the bias from the lightDepthValue.
    pixelDepthValue = pixelDepthValue - bias;

    // Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    // If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
    if (pixelDepthValue < shadowMapDepthValue) {
        return false;
    } else {
        return true;
    }
}

float4 pointLight(PixelDescriptor pixel) {
    float4 color = 0;
    float lightIntensity;

    if (isInPointLightShadow(pixel)) {
        return color;
    }

    float3 pointToLightVector = pointLightPosition - pixel.worldSpacePosition;
    float pointToLightVectorLength = length(pointToLightVector);

    // make unit vector
    pointToLightVector /= pointToLightVectorLength;

    lightIntensity = saturate(dot(pixel.normal, pointToLightVector)); //todo: to saturate or not to saturate? when should we saturate?
    if (lightIntensity > 0.0f) {
        if (materialIsTextured >= 0) {
            float4 textureColor = materialTexture.Sample(materialSampler, pixel.tex);
            color += (lightIntensity * textureColor * pointLightDiffuse);
        } else {
            color += (lightIntensity * materialDiffuseColor * pointLightDiffuse);
        }

        float3 reflection = normalize(2 * lightIntensity * pixel.normal - pointToLightVector);
        float specularFactor = pow(saturate(dot(reflection, pixel.viewDirection)), materialSpecularColor.w);;

        color += specularFactor * materialSpecularColor * pointLightSpecular;
    }

    return color;
}

float4 colorPixel(PixelDescriptor pixel) : SV_TARGET {
    float4 pixelColor = 0;

    // start out with the ambient color
    pixelColor += sceneAmbience(pixel);

    // add any diffuse light from point light
    pixelColor += pointLight(pixel);

    return pixelColor;
}