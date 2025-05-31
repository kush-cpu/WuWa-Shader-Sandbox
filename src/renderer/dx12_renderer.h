#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

class DX12Renderer {
public:
    void Initialize(HWND hwnd);
    void RenderFrame();
    void Cleanup();

private:
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    // Add more components as we go: RTVs, heaps, command allocators, etc.
};
