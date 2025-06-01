// dx12_triangle_demo.cpp
// Phase 1: Basic Triangle Rendering using DX12 + HLSL

#include <windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;
using namespace DirectX;

// Vertex structure
struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

// Global handles
ComPtr<ID3D12Device> device;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<IDXGISwapChain3> swapChain;
ComPtr<ID3D12DescriptorHeap> rtvHeap;
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12PipelineState> pipelineState;
ComPtr<ID3D12RootSignature> rootSignature;
ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vbView = {};
ComPtr<ID3D12Resource> gVertexBuffer;
D3D12_VERTEX + BUFFER_VIEW gVertexBufferView;

HWND hwnd;
UINT rtvDescriptorSize;

void InitWindow(HINSTANCE hInstance, int nCmdShow);
void InitD3D();
void LoadAssets();
void Render();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    InitWindow(hInstance, nCmdShow);
    InitD3D();
    LoadAssets();

    // Main loop
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
        }
    }
    return 0;
}

void InitWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DX12WindowClass";
    RegisterClassEx(&wc);

    hwnd = CreateWindow(wc.lpszClassName, L"DX12 Triangle", WS_OVERLAPPEDWINDOW,
        100, 100, 1280, 720, nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
}

void InitD3D() {
    ComPtr<IDXGIFactory4> dxgiFactory;
    CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

    // Load compiled shader blobs
    ComPtr<ID3DBlob> vertexShader, pixelShader;

    {
        // Load from file
        std::ifstream vsFile("shaders/basic_vs.cso", std::ios::binary);
        std::ifstream psFile("shaders/basic_ps.cso", std::ios::binary);
        std::vector<char> vsData((std::istreambuf_iterator<char>(vsFile)), std::istreambuf_iterator<char>());
        std::vector<char> psData((std::istreambuf_iterator<char>(psFile)), std::istreambuf_iterator<char>());

        D3DCreateBlob(vsData.size(), &vertexShader);
        memcpy(vertexShader->GetBufferPointer(), vsData.data(), vsData.size());

        D3DCreateBlob(psData.size(), &pixelShader);
        memcpy(pixelShader->GetBufferPointer(), psData.data(), psData.size());
    }

    struct Vertex {
        float x, y, z;
    };

    Vertex triangleVertices[] = {
        { 0.0f,  0.25f, 0.0f },
        { 0.25f, -0.25f, 0.0f },
        { -0.25f, -0.25f, 0.0f }
    };

    const UINT vertexBufferSize = sizeof(triangleVertices);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    gDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&gVertexBuffer));

    void* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource
    gVertexBuffer->Map(0, &readRange, &pVertexDataBegin);
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    gVertexBuffer->Unmap(0, nullptr);

    gVertexBufferView.BufferLocation = gVertexBuffer->GetGPUVirtualAddress();
    gVertexBufferView.StrideInBytes = sizeof(Vertex);
    gVertexBufferView.SizeInBytes = vertexBufferSize;


    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.Width = 1280;
    scDesc.Height = 720;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> tempSwapChain;
    dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &tempSwapChain);
    tempSwapChain.As(&swapChain);

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
}

void LoadAssets() {
    // Compile shaders
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    D3DCompileFromFile(L"shaders/basic.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
    D3DCompileFromFile(L"shaders/basic.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

    // Root signature (empty)
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serializedRootSig;
    D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig, nullptr);
    device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

    // Input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    psoDec.InputLayout = { inputElementsDescs, _countof(inputElementsDescs) };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { nullptr, 0 }; // No layout yet
    psoDesc.pRootSignature = gRootSignature;
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    gDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&gPipelineState));


    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator.Get(), pipelineState.Get(),
        IID_PPV_ARGS(&commandList));

    // Create triangle
    Vertex vertices[] = {
        { {  0.0f,  0.5f, 0.0f }, { 1, 0, 0, 1 } },
        { {  0.5f, -0.5f, 0.0f }, { 0, 1, 0, 1 } },
        { { -0.5f, -0.5f, 0.0f }, { 0, 0, 1, 1 } },
    };

    const UINT vbSize = sizeof(vertices);

    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vbSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer));

    void* data;
    vertexBuffer->Map(0, nullptr, &data);
    memcpy(data, vertices, vbSize);
    vertexBuffer->Unmap(0, nullptr);

    vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbView.StrideInBytes = sizeof(Vertex);
    vbView.SizeInBytes = vbSize;
}

void Render() {
    gCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gCommandList->IASetVertexBuffers(0, 1, &gVertexBufferView);
    gCommandList->DrawInstanced(3, 1, 0, 0);

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
