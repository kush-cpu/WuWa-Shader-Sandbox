#include "dx12_renderer.h"
#include <cassert>

void DX12Renderer::Initialize(HWND hwnd) {
    // Init DXGI + Device
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    ComPtr<IDXGIAdapter1> adapter;
    factory->EnumAdapters1(0, &adapter);

    D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

    // Command Queue
    D3D12_COMMAND_QUEUE_DESC desc = {};
    device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue));

    // SwapChain (basic for now)
    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.Width = 1280;
    scDesc.Height = 720;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> sc1;
    factory->CreateSwapChainForHwnd(
        commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &sc1);

    sc1.As(&swapChain);
}

void DX12Renderer::RenderFrame() {
    // TODO: add command buffer submission & render logic
}

void DX12Renderer::Cleanup() {
    // TODO: release resources
}
