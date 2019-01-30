#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <DXGI.h>
#include <D3D11.h>
#include <exception>
#include <string>
#include <wrl.h>
#include <D3DX10math.h>

#include "../3D Components/Scene.h"
#include "Camera.h"
#include "Shaders/LightShader.h"
#include "Texture.h"
#include "RenderTargetTextureCube.h"
#include "Shaders/DepthShader.h"

class D3D11Renderer {
public:
    D3D11Renderer(HWND windowHandle,
                  bool fullscreenEnabled,
                  bool vsyncEnabled,
                  float screenNear,
                  float screenDepth,
                  const int screenWidth,
                  const int screenHeight,
                  const int shadowMapSize);
    ~D3D11Renderer();

    void setScene(Scene* scene);
    void setCamera(Camera* camera);
    void renderFrame();
    ID3D11Device* getDevice();
    void writeCurrentShadowMapToDds();

private:
    struct MonitorDescriptor {
        unsigned int refreshRateNumerator;
        unsigned int refreshRateDenominator;
    };

    struct VideoCardDescriptor {
        int memory;
        std::string description;
    };

    struct PhysicalDeviceDescriptor {
        VideoCardDescriptor videoCard;
        MonitorDescriptor monitor;
    };

    int screenWidth;
    int screenHeight;
    bool fullscreenEnabled;
    bool vsyncEnabled;
    float screenNear;
    float screenDepth;

    Scene* scene = nullptr;
    Camera* camera = nullptr;

    std::map<std::string, std::unique_ptr<Texture>> textureMap;

    PhysicalDeviceDescriptor hardwareInfo;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterState;
    D3D11_VIEWPORT viewport;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

    D3DXMATRIX projectionMatrix;
    LightShader lightShader;
    DepthShader depthShader;
    RenderTargetTextureCube shadowMap;

    PhysicalDeviceDescriptor queryPhysicalDeviceDescriptors();
    void createSwapChainAndDevice(HWND windowHandle);
    void createDepthAndStencilBuffer();
    void createRasterizerState();
    void setupViewport();
    void setupVertexAndIndexBuffers();
    void renderShadowMap(D3DXMATRIX* pointLightProjectionMatrix);
    void setBackbufferAsRenderTargetAndClear();

    static std::vector<Model::Vertex> getAllVertices(Scene* scene);
    static std::vector<unsigned long> getAllIndices(Scene* scene);
};

class D3D11RendererException : public std::exception {
private:
    long errorCode;
    const char* msg;

public:
    D3D11RendererException(const char* msg, int errorCode) : errorCode(errorCode), msg(msg) {
    }

    long getErrorCode() const { return errorCode; }

    const char* what() const noexcept override {
        return msg;
    }
};
