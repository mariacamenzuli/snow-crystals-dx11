Texture2D shaderTexture;
SamplerState SampleType;

cbuffer ConvolutionBuffer {
    float4x4 kernelMatrix;
    float screenWidth;
    float screenHeight;
    float denominator;
    float offset;
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
    float kernel[9] = { kernelMatrix[0][0], kernelMatrix[0][1], kernelMatrix[0][2],
                        kernelMatrix[1][0], kernelMatrix[1][1], kernelMatrix[1][2],
                        kernelMatrix[2][0], kernelMatrix[2][1], kernelMatrix[2][2], };

    float neighborPixelsRedChannel[9];
    getNeighborPixels(input.tex, 0, neighborPixelsRedChannel);

    float neighborPixelsGreenChannel[9];
    getNeighborPixels(input.tex, 1, neighborPixelsGreenChannel);

    float neighborPixelsBlueChannel[9];
    getNeighborPixels(input.tex, 2, neighborPixelsBlueChannel);

    float neighborPixelsAverage[9];
    getMean(neighborPixelsRedChannel, neighborPixelsGreenChannel, neighborPixelsBlueChannel, neighborPixelsAverage);

    float4 color = 0;

    if (kernelMatrix[3][3] > 0) {
        color.x = convolve(kernel, neighborPixelsAverage, denominator, offset);
        color.y = convolve(kernel, neighborPixelsAverage, denominator, offset);
        color.z = convolve(kernel, neighborPixelsAverage, denominator, offset);
        color.w = 1;
    } else {
        color.x = convolve(kernel, neighborPixelsRedChannel, denominator, offset);
        color.y = convolve(kernel, neighborPixelsGreenChannel, denominator, offset);
        color.z = convolve(kernel, neighborPixelsBlueChannel, denominator, offset);
        color.w = 1;
    }

    return color;
}