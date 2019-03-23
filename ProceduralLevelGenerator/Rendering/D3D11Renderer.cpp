#include <vector>

#include "D3D11Renderer.h"
#include <stack>

D3D11Renderer::D3D11Renderer(HWND windowHandle,
                             bool fullscreenEnabled,
                             bool vsyncEnabled,
                             float screenNear,
                             float screenDepth,
                             const int screenWidth,
                             const int screenHeight,
                             const int shadowMapSize) : screenWidth(screenWidth),
                                                        screenHeight(screenHeight),
                                                        fullscreenEnabled(fullscreenEnabled),
                                                        vsyncEnabled(vsyncEnabled),
                                                        screenNear(screenNear),
                                                        screenDepth(screenDepth) {
    hardwareInfo = queryPhysicalDeviceDescriptors();
    createSwapChainAndDevice(windowHandle);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBufferPtr;
    HRESULT result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), static_cast<LPVOID*>(&backBufferPtr));
    if (FAILED(result)) {
        throw D3D11RendererException("failed to obtain a pointer to the swap chain back buffer", result);
    }

    result = device->CreateRenderTargetView(backBufferPtr.Get(), nullptr, renderTargetView.GetAddressOf());
    if (FAILED(result)) {
        throw D3D11RendererException("failed create the render target view", result);
    }

    createDepthAndStencilBuffer();

    // Bind the render target view and depth stencil buffer to the output render pipeline.
    deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    createRasterizerState();
    setupViewport();

    float fieldOfView = static_cast<float>(D3DX_PI) / 4.0f;
    float screenAspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

    // Create the projection matrix for 3D rendering.
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

    lightShader.initialize(device.Get(), deviceContext.Get());
    depthShader.initialize(device.Get(), deviceContext.Get());

    shadowMap.initialize(device.Get(), shadowMapSize);
}

D3D11Renderer::~D3D11Renderer() {
    // Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
    if (swapChain.Get()) {
        swapChain->SetFullscreenState(false, nullptr);
    }
}

void D3D11Renderer::setScene(Scene* scene) {
    this->scene = scene;
    setupVertexAndIndexBuffers();
    
    std::stack<SceneObject*> toVisit;
    toVisit.push(scene->getRootSceneObject());

    while (!toVisit.empty()) {
        SceneObject* sceneObject = toVisit.top();
        toVisit.pop();

        if (sceneObject->getModel() != nullptr) {
            for (auto const& materialIndexRange : sceneObject->getModel()->getMaterialIndexRanges()) {
                if (materialIndexRange.material.isTextured()) {
                    auto textureFilename = materialIndexRange.material.getTextureFileName();
                    const auto found = textureMap.find(textureFilename);
                    if (found == textureMap.end()) {
                        textureMap.insert(std::make_pair(textureFilename, std::make_unique<Texture>(Texture(device.Get(), textureFilename))));
                    }
                }
            }
        }

        auto children = sceneObject->getChildren();
        for (auto child : children) {
            toVisit.push(child);
        }
    }
}

void D3D11Renderer::setCamera(Camera* camera) {
    this->camera = camera;
}

void D3D11Renderer::renderFrame() {
    camera->calculateViewMatrix();
    D3DXMATRIX viewMatrix;
    camera->getViewMatrix(viewMatrix);

    D3DXMATRIX* pointLightProjectionMatrix = scene->getPointLight()->getProjectionMatrix();

    depthShader.setActive(deviceContext.Get());
    renderShadowMap(pointLightProjectionMatrix);
    setBackbufferAsRenderTargetAndClear();

    lightShader.setActive(deviceContext.Get());
    lightShader.updateDepthMapTexture(deviceContext.Get(), &shadowMap);
    lightShader.updateCameraBuffer(deviceContext.Get(), camera->getPosition());
    lightShader.updateAmbientLightBuffer(deviceContext.Get(), scene->getAmbientLight());
    lightShader.updatePointLightBuffer(deviceContext.Get(), scene->getPointLight()->getDiffuse(), scene->getPointLight()->getSpecular(), *scene->getPointLight()->getWorldMatrix());
    
    int indexStartLocation = 0;
    int vertexStartLocation = 0;
    
    std::stack<SceneObject*> toVisit;
    toVisit.push(scene->getRootSceneObject());
    
    while (!toVisit.empty()) {
        SceneObject* sceneObject = toVisit.top();
        toVisit.pop();
    
        if (sceneObject->getModel() != nullptr) {
            if (sceneObject->isVisible()) {
                lightShader.updateTransformationMatricesBuffer(deviceContext.Get(),
                                                               *sceneObject->getWorldMatrix(),
                                                               viewMatrix,
                                                               projectionMatrix);
    
                for (auto const& materialIndexRange : sceneObject->getModel()->getMaterialIndexRanges()) {
                    bool isTextured = false;
                    if (materialIndexRange.material.isTextured()) {
                        isTextured = true;
                        lightShader.updateTexture(deviceContext.Get(), textureMap.find(materialIndexRange.material.getTextureFileName())->second.get());
                    }
    
                    lightShader.updateMaterialBuffer(deviceContext.Get(),
                                                     materialIndexRange.material.getAmbientColor(),
                                                     materialIndexRange.material.getDiffuseColor(),
                                                     materialIndexRange.material.getSpecularColor(),
                                                     isTextured);
    
                    const auto indicesToDrawCount = (materialIndexRange.endInclusive + 1) - materialIndexRange.startInclusive;
                    deviceContext->DrawIndexed(indicesToDrawCount, indexStartLocation, vertexStartLocation);
    
                    indexStartLocation = indexStartLocation + indicesToDrawCount;
                }
            } else {
                indexStartLocation = indexStartLocation + sceneObject->getModel()->getIndexCount();
            }
    
            vertexStartLocation = vertexStartLocation + sceneObject->getModel()->getVertexCount();
        }
    
        auto children = sceneObject->getChildren();
        for (auto child : children) {
            toVisit.push(child);
        }
    }

    // Present the back buffer to the screen since rendering is complete.
    if (vsyncEnabled) {
        // Lock to screen refresh rate.
        swapChain->Present(1, 0);
    } else {
        // Present as fast as possible.
        swapChain->Present(0, 0);
    }
}

ID3D11Device* D3D11Renderer::getDevice() {
    return device.Get();
}

void D3D11Renderer::writeCurrentShadowMapToDds() {
    shadowMap.saveToFile(deviceContext.Get());
}

D3D11Renderer::PhysicalDeviceDescriptor D3D11Renderer::queryPhysicalDeviceDescriptors() {
    HRESULT result;

    Microsoft::WRL::ComPtr<IDXGIFactory> factory;
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(result)) {
        throw D3D11RendererException("failed to create a DirectX graphics interface factory", result);
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    result = factory->EnumAdapters(0, &adapter);
    if (FAILED(result)) {
        throw D3D11RendererException("failed to create an adapter for the primary graphics interface (video card)", result);
    }

    Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
    result = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(result)) {
        throw D3D11RendererException("failed to enumerate the primary output devices (monitor)", result);
    }

    unsigned int numModes;
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
    if (FAILED(result)) {
        throw D3D11RendererException("failed to get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)", result);
    }

    std::vector<DXGI_MODE_DESC> displayModeList(numModes);
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.data()
    );
    if (FAILED(result)) {
        throw D3D11RendererException("failed to get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)", result);
    }

    // Now go through all the display modes and find the one that matches the screen width and height.
    // When a match is found store the numerator and denominator of the refresh rate for that monitor.
    unsigned int numerator = 0;
    unsigned int denominator = 0;
    bool foundMatchingMode = false;
    for (int i = 0; i < numModes && !foundMatchingMode; i++) {
        if (displayModeList[i].Width == static_cast<unsigned int>(screenWidth)) {
            if (displayModeList[i].Height == static_cast<unsigned int>(screenHeight)) {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
                foundMatchingMode = true;
            }
        }
    }

    if (!foundMatchingMode) {
        throw D3D11RendererException("failed to find the refresh rate for the adapter output (monitor)", result);
    }

    DXGI_ADAPTER_DESC adapterDesc;
    result = adapter->GetDesc(&adapterDesc);
    if (FAILED(result)) {
        throw D3D11RendererException("failed to get the adapter (video card) description", result);
    }

    // Store the dedicated video card memory in megabytes.
    int videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // Convert the name of the video card to a character array and store it.
    std::wstring ws(adapterDesc.Description);
    std::string videoCardDescription(ws.begin(), ws.end());

    return {VideoCardDescriptor{videoCardMemory, videoCardDescription}, MonitorDescriptor{numerator, denominator}};
}

void D3D11Renderer::createSwapChainAndDevice(HWND windowHandle) {
    DXGI_SWAP_CHAIN_DESC swapChainDesc;

    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1; // Back buffer
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Set the refresh rate of the back buffer.
    if (vsyncEnabled) {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = hardwareInfo.monitor.refreshRateNumerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = hardwareInfo.monitor.refreshRateDenominator;
    } else {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // Set the usage of the back buffer.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Set the handle for the window to render to.
    swapChainDesc.OutputWindow = windowHandle;

    // Turn multisampling off.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    // Set to windowed mode.
    swapChainDesc.Windowed = !fullscreenEnabled; // would need to be false for full screen

    // Set the scan line ordering and scaling to unspecified.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Discard the back buffer contents after presenting.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Don't set the advanced flags.
    swapChainDesc.Flags = 0;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        &featureLevel,
        1,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        swapChain.GetAddressOf(),
        device.GetAddressOf(),
        nullptr,
        deviceContext.GetAddressOf());
    if (FAILED(result)) {
        throw D3D11RendererException("failed to create a DirectX device and swap chain", result);
    }
}

void D3D11Renderer::createDepthAndStencilBuffer() {
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = screenWidth;
    depthBufferDesc.Height = screenHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    HRESULT result = device->CreateTexture2D(&depthBufferDesc, nullptr, depthStencilBuffer.GetAddressOf());
    if (FAILED(result)) {
        throw D3D11RendererException("failed to create depth and stencil buffer", result);
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
    if (FAILED(result)) {
        throw D3D11RendererException("failed to create depth stencil state", result);
    }

    deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    result = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());
    if (FAILED(result)) {
        throw D3D11RendererException("failed to create depth stencil view", result);
    }
}

void D3D11Renderer::createRasterizerState() {
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    HRESULT result = device->CreateRasterizerState(&rasterDesc, rasterState.GetAddressOf());
    if (FAILED(result)) {
        throw D3D11RendererException("failed to create rasterizer state", result);
    }

    deviceContext->RSSetState(rasterState.Get());
}

void D3D11Renderer::setupViewport() {
    viewport.Width = static_cast<float>(screenWidth);
    viewport.Height = static_cast<float>(screenHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
}

void D3D11Renderer::setupVertexAndIndexBuffers() {
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;

    auto sceneVertices = getAllVertices(scene);

    // Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(Model::Vertex) * sceneVertices.size();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = sceneVertices.data();
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create vertex buffer for scene.");
    }

    // Bind the vertex buffer to the input-assembler stage.
    unsigned int stride = sizeof(Model::Vertex);
    unsigned int offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

    auto sceneIndices = getAllIndices(scene);

    // Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * sceneIndices.size();
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    indexData.pSysMem = sceneIndices.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create index buffer for scene.");
    }

    // Bind the index buffer to the input-assembler stage.
    deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3D11Renderer::renderShadowMap(D3DXMATRIX* pointLightProjectionMatrix) {
    shadowMap.clearRenderTarget(deviceContext.Get(), 0);
    shadowMap.clearRenderTarget(deviceContext.Get(), 1);
    shadowMap.clearRenderTarget(deviceContext.Get(), 2);
    shadowMap.clearRenderTarget(deviceContext.Get(), 3);
    shadowMap.clearRenderTarget(deviceContext.Get(), 4);
    shadowMap.clearRenderTarget(deviceContext.Get(), 5);

    int indexStartLocation = 0;
    int vertexStartLocation = 0;
    
    std::stack<SceneObject*> toVisit;
    toVisit.push(scene->getRootSceneObject());
    
    while (!toVisit.empty()) {
        SceneObject* sceneObject = toVisit.top();
        toVisit.pop();
    
        if (sceneObject->getModel() != nullptr) {
            if (sceneObject->isVisible()) {
                // +VE X
                shadowMap.setAsRenderTarget(deviceContext.Get(), 0);
                depthShader.updateTransformationMatricesBuffer(deviceContext.Get(),
                                                               *sceneObject->getWorldMatrix(),
                                                               scene->getPointLight()->getViewMatrixPositiveX(),
                                                               *pointLightProjectionMatrix);
                deviceContext->DrawIndexed(sceneObject->getModel()->getIndexCount(), indexStartLocation, vertexStartLocation);
                
                // -VE X
                shadowMap.setAsRenderTarget(deviceContext.Get(), 1);
                depthShader.updateTransformationMatricesBuffer(deviceContext.Get(),
                                                               *sceneObject->getWorldMatrix(),
                                                               scene->getPointLight()->getViewMatrixNegativeX(),
                                                               *pointLightProjectionMatrix);
                deviceContext->DrawIndexed(sceneObject->getModel()->getIndexCount(), indexStartLocation, vertexStartLocation);
                
                // +VE Y
                shadowMap.setAsRenderTarget(deviceContext.Get(), 2);
                depthShader.updateTransformationMatricesBuffer(deviceContext.Get(),
                                                               *sceneObject->getWorldMatrix(),
                                                               scene->getPointLight()->getViewMatrixPositiveY(),
                                                               *pointLightProjectionMatrix);
                deviceContext->DrawIndexed(sceneObject->getModel()->getIndexCount(), indexStartLocation, vertexStartLocation);

                // -VE Y
                shadowMap.setAsRenderTarget(deviceContext.Get(), 3);
                depthShader.updateTransformationMatricesBuffer(deviceContext.Get(),
                                                               *sceneObject->getWorldMatrix(),
                                                               scene->getPointLight()->getViewMatrixNegativeY(),
                                                               *pointLightProjectionMatrix);
                deviceContext->DrawIndexed(sceneObject->getModel()->getIndexCount(), indexStartLocation, vertexStartLocation);

                // +VE Z
                shadowMap.setAsRenderTarget(deviceContext.Get(), 4);
                depthShader.updateTransformationMatricesBuffer(deviceContext.Get(),
                                                               *sceneObject->getWorldMatrix(),
                                                               scene->getPointLight()->getViewMatrixPositiveZ(),
                                                               *pointLightProjectionMatrix);
                deviceContext->DrawIndexed(sceneObject->getModel()->getIndexCount(), indexStartLocation, vertexStartLocation);
                
                // -VE Z
                shadowMap.setAsRenderTarget(deviceContext.Get(), 5);
                depthShader.updateTransformationMatricesBuffer(deviceContext.Get(),
                                                               *sceneObject->getWorldMatrix(),
                                                               scene->getPointLight()->getViewMatrixNegativeZ(),
                                                               *pointLightProjectionMatrix);
                deviceContext->DrawIndexed(sceneObject->getModel()->getIndexCount(), indexStartLocation, vertexStartLocation);
            }
    
            indexStartLocation = indexStartLocation + sceneObject->getModel()->getIndexCount();
            vertexStartLocation = vertexStartLocation + sceneObject->getModel()->getVertexCount();
        }
    
        auto children = sceneObject->getChildren();
        for (auto child : children) {
            toVisit.push(child);
        }
    }
}

void D3D11Renderer::setBackbufferAsRenderTargetAndClear() {
    deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
    deviceContext->RSSetViewports(1, &viewport);

    float backBufferStartingColor[4] = { 0.0f, 0.0f, 0.01f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView.Get(), backBufferStartingColor);
    deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

std::vector<Model::Vertex> D3D11Renderer::getAllVertices(Scene* scene) {
    std::vector<Model::Vertex> vertices;

    std::stack<SceneObject*> toVisit;
    toVisit.push(scene->getRootSceneObject());

    while (!toVisit.empty()) {
        SceneObject* sceneObject = toVisit.top();
        toVisit.pop();

        if (sceneObject->getModel() != nullptr) {
            for (auto i = 0; i < sceneObject->getModel()->getVertexCount(); i++) {
                vertices.push_back(sceneObject->getModel()->getVertices()[i]);
            }
        }

        auto children = sceneObject->getChildren();
        for (auto child : children) {
            toVisit.push(child);
        }
    }

    return vertices;
}

std::vector<unsigned long> D3D11Renderer::getAllIndices(Scene* scene) {
    std::vector<unsigned long> indices;

    std::stack<SceneObject*> toVisit;
    toVisit.push(scene->getRootSceneObject());

    while (!toVisit.empty()) {
        SceneObject* sceneObject = toVisit.top();
        toVisit.pop();

        if (sceneObject->getModel() != nullptr) {
            for (auto i = 0; i < sceneObject->getModel()->getIndexCount(); i++) {
                indices.push_back(sceneObject->getModel()->getIndices()[i]);
            }
        }

        auto children = sceneObject->getChildren();
        for (auto child : children) {
            toVisit.push(child);
        }
    }

    return indices;
}
