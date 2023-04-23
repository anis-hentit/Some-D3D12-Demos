#include "RenderTarget.h"





RenderTarget::RenderTarget(ID3D12Device* device,UINT width, UINT height, DXGI_FORMAT format)
{
    mWidth = width;
    mHeight = height;
    md3dDevice = device;
    mFormat = format;

    BuildResource();
}


ID3D12Resource* RenderTarget::Resource()
{
    return mOffScreenTex.Get();
}


CD3DX12_GPU_DESCRIPTOR_HANDLE RenderTarget::Srv()
{
    return mhGpuSrv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderTarget::Rtv()
{
    return mhCpuRtv;
}


void RenderTarget::BuildResource()
{
  D3D12_RESOURCE_DESC texDesc;
  ZeroMemory(&texDesc,sizeof(D3D12_RESOURCE_DESC));
  texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  texDesc.Alignment = 0;
  texDesc.Width = mWidth;
  texDesc.Height = mHeight;
  texDesc.DepthOrArraySize = 1;
  texDesc.MipLevels = 1;
  texDesc.Format = mFormat;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

 ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mOffScreenTex)));

}

void RenderTarget::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
                                    CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
                                    CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv)
{
  mhCpuSrv = hCpuSrv;
  mhGpuSrv = hGpuSrv;
  mhCpuRtv = hCpuRtv;

  BuildDescriptors();
}

void RenderTarget::OnResize(UINT newWidth, UINT newHeight)
{
  if (mWidth != newWidth || mHeight != newHeight)
  {
    mWidth = newWidth;
    mHeight = newHeight;

    BuildResource();
    BuildDescriptors();
  }

}


void RenderTarget::BuildDescriptors()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
 srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
 srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
 srvDesc.Format = mFormat;
 srvDesc.Texture2D.MipLevels = 1;
 srvDesc.Texture2D.MostDetailedMip = 0;

 md3dDevice->CreateShaderResourceView(mOffScreenTex.Get(),&srvDesc, mhCpuSrv);
 md3dDevice->CreateRenderTargetView(mOffScreenTex.Get(),nullptr,mhCpuRtv);
}
