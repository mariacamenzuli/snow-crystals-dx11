Texture2D shaderTexture;
SamplerState SampleType;

cbuffer ScreenSizeBuffer {
    float screenWidth;
    float screenHeight;
    float2 padding;
};

struct PixelInputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float convolve(float kernel[9],
               float neighborPixels[9],
               float denominator,
               float offset) {
    float sum = 0.0;
    for (int i = 0; i < 9; i++) {
        sum += kernel[i] * neighborPixels[i];
    }
    return clamp(sum / denominator + offset, 0.0, 1.0);
}

float4 getPixel(float2 coords, float deltaX, float deltaY) {
    return shaderTexture.Sample(SampleType, coords + float2(deltaX, deltaY));
}

void getNeighborPixels(float2 tex, int channel, out float neighborPixels[9]) {
    float texelSizeX = 1.0 / screenWidth;
    float texelSizeY = 1.0 / screenHeight;

    int k = -1;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            k++;
            neighborPixels[k] = getPixel(tex,
                                         float(i) * texelSizeX,
                                         float(j) * texelSizeY)[channel];
        }
    }
}

void getMean(float neighborPixelsRedChannel[9],
             float neighborPixelsGreenChannel[9],
             float neighborPixelsBlueChannel[9],
             out float averageValues[9]) {
    for (int i = 0; i < 9; i++) {
        averageValues[i] = (neighborPixelsRedChannel[i] + neighborPixelsGreenChannel[i] + neighborPixelsBlueChannel[i]) / 3.;
    }
}

float4 main(PixelInputType input) : SV_TARGET {
    float kerEmboss[9] = { 2.,0.,0.,
                           0., -1., 0.,
                           0., 0., -1. };

    float kerSharpness[9] = { -1.,-1.,-1.,
                              -1., 9., -1.,
                              -1., -1., -1. };

    float kerGausBlur[9] = { 1.,2.,1.,
                             2., 4., 2.,
                             1., 2., 1. };

    float kerEdgeDetect[9] = { -1. / 8.,-1. / 8.,-1. / 8.,
                               -1. / 8., 1., -1. / 8.,
                               -1. / 8., -1. / 8., -1. / 8. };

    float neighborPixelsRedChannel[9];
    getNeighborPixels(input.tex, 0, neighborPixelsRedChannel);

    float neighborPixelsGreenChannel[9];
    getNeighborPixels(input.tex, 1, neighborPixelsGreenChannel);

    float neighborPixelsBlueChannel[9];
    getNeighborPixels(input.tex, 2, neighborPixelsBlueChannel);

    float neighborPixelsAverage[9];
    getMean(neighborPixelsRedChannel, neighborPixelsGreenChannel, neighborPixelsBlueChannel, neighborPixelsAverage);

    float4 color = 0;

    // Sharpness kernel
    //color.x = convolve(kerSharpness, neighborPixelsRedChannel, 1., 0.);
    //color.y = convolve(kerSharpness, neighborPixelsGreenChannel, 1., 0.);
    //color.z = convolve(kerSharpness, neighborPixelsBlueChannel, 1., 0.);
    //color.w = 1;

    // Gaussian blur kernel
    //color.x = convolve(kerGausBlur, neighborPixelsRedChannel, 16., 0.);
    //color.y = convolve(kerGausBlur, neighborPixelsGreenChannel, 16., 0.);
    //color.z = convolve(kerGausBlur, neighborPixelsBlueChannel, 16., 0.);
    //color.w = 1;

    // Edge Detection kernel
    //color.x = convolve(kerEdgeDetect, neighborPixelsAverage, 0.1, 0.);
    //color.y = convolve(kerEdgeDetect, neighborPixelsAverage, 0.1, 0.);
    //color.z = convolve(kerEdgeDetect, neighborPixelsAverage, 0.1, 0.);
    //color.w = 1;

    // Emboss kernel
    color.x = convolve(kerEmboss, neighborPixelsAverage, 1., 1. / 2.);
    color.y = convolve(kerEmboss, neighborPixelsAverage, 1., 1. / 2.);
    color.z = convolve(kerEmboss, neighborPixelsAverage, 1., 1. / 2.);
    color.w = 1;

    return color;
}