#pragma once

#include "../../../Common/d3dUtil.h"
#include <d3d12.h>



class SobelFilter
{

 public :
      SobelFilter(ID3D12Device* device, UINT Width, UINT Height, DXGI_FORMAT Format);
      SobelFilter(const SobelFilter& rhs) = delete;
      SobelFilter& operator=(const SobelFilter& rhs) = delete;
      


      void BuildDescriptors(
                           CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
                           CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
                           UINT descriptorSize
                           ); // overload that cashes the handles and size

      CD3DX12_GPU_DESCRIPTOR_HANDLE OutputSrv();

      UINT DescriptorCount() const;

      void Execute(ID3D12GraphicsCommandList* cmdList,
                   ID3D12RootSignature* rootSig,
                   ID3D12PipelineState* pso,
                   CD3DX12_GPU_DESCRIPTOR_HANDLE input);

      void OnResize(UINT NewWidth, UINT NewHeight);

 private:
      void BuildDescriptors();// overload that builds descriptors
      void BuildResource();


 private:

      ID3D12Device* md3dDevice= nullptr;
      UINT mWidth = 0;
      UINT mHeight = 0;
      DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

      CD3DX12_CPU_DESCRIPTOR_HANDLE mSobelCpuUav;
      CD3DX12_GPU_DESCRIPTOR_HANDLE mSobelGpuUav;

      CD3DX12_CPU_DESCRIPTOR_HANDLE mSobelCpuSrv;
      CD3DX12_GPU_DESCRIPTOR_HANDLE mSobelGpuSrv;

      Microsoft::WRL::ComPtr<ID3D12Resource> mSobelFilter = nullptr;


};
