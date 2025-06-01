#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
//#include "renderer/dx12_renderer.h"

using Microsoft::WRL::ComPtr;

// Win32 window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"WuWaShaderWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"WuWa Shader Sandbox",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);

    DX12Renderer renderer;
    renderer.Initialize(hwnd);

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            renderer.RenderFrame();
        }
    }

    renderer.Cleanup();
    return 0;
}

// ---- renderer/dx12_renderer.h ----
#ifndef DX12_RENDERER_H
#define DX12_RENDERER_H

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

class DX12Renderer {
public:
    void Initialize(HWND hwnd);
    void RenderFrame();
    void Cleanup();

private:
    static const UINT FrameCount = 2;

    Microsoft::WRL::ComPtr<IDXGISwapChain3> gSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> gDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> gCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> gRTVHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> gRenderTargets[FrameCount];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> gCommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> gCommandList;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> gPipelineState;

    UINT gRTVDescriptorSize = 0;
};

#endif

// ---- renderer/dx12_renderer.cpp ----
//#include <dx12_renderer.h>
#include <d3dx12.h>
#include <stdexcept>

void DX12Renderer::Initialize(HWND hwnd) {
    // Stub - assume the device, swap chain, heaps, etc., are created correctly
}

void DX12Renderer::RenderFrame() {
    UINT backBufferIndex = gSwapChain->GetCurrentBackBufferIndex();

    gCommandAllocator->Reset();
    gCommandList->Reset(gCommandAllocator.Get(), nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = gRenderTargets[backBufferIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    gCommandList->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        gRTVHeap->GetCPUDescriptorHandleForHeapStart(),
        backBufferIndex, gRTVDescriptorSize);
    gCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    gCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Drawing code would go here

    std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
    gCommandList->ResourceBarrier(1, &barrier);

    gCommandList->Close();
    ID3D12CommandList* cmdLists[] = { gCommandList.Get() };
    gCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    gSwapChain->Present(1, 0);
}

void DX12Renderer::Cleanup() {
    // Stub - release resources properly
}
