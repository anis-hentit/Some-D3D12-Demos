#include "SobelFilter.h"
#include <combaseapi.h>
#include <d3d12.h>
#include <dxgiformat.h>
#include <minwinbase.h>


SobelFilter::SobelFilter(ID3D12Device* device,
                          UINT width, UINT height,
                            DXGI_FORMAT format)
{
    md3dDevice = device;
    mWidth = width;
    mHeight = height;
    mFormat = format;

    BuildResource();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE SobelFilter::OutputSrv()
{
    return mSobelGpuSrv;
}

UINT SobelFilter::DescriptorCount() const
{
    return 2;
}

void SobelFilter::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
                                   CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
                                   UINT descriptorSize)
{
    // Save references to descriptors
	mSobelCpuSrv = hCpuDescriptor;
    mSobelGpuSrv = hGpuDescriptor;

    mSobelCpuUav = hCpuDescriptor.Offset(1,descriptorSize);
    mSobelGpuUav = hGpuDescriptor.Offset(1,descriptorSize);

    BuildDescriptors();

}

void SobelFilter::OnResize(UINT Width, UINT Height)
{
    if((mWidth != Width) || (mHeight != Height))
    {
        mWidth = Width;
        mHeight = Height;

        BuildResource();

        // New resource, so we need new descriptors
        BuildDescriptors();
    }
}


void SobelFilter::BuildDescriptors()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format  = mFormat;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = mFormat;
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;


    md3dDevice->CreateShaderResourceView(mSobelFilter.Get(),&srvDesc,mSobelCpuSrv);
    md3dDevice->CreateUnorderedAccessView(mSobelFilter.Get(), nullptr, &uavDesc,mSobelCpuUav);

}


void SobelFilter::BuildResource()
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
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;


    ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,&texDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mSobelFilter)));
}


void SobelFilter::Execute(ID3D12GraphicsCommandList* cmdList,
                         ID3D12RootSignature* rootSig,
                        ID3D12PipelineState* pso,
                        CD3DX12_GPU_DESCRIPTOR_HANDLE input)
{
    cmdList->SetComputeRootSignature(rootSig);
    cmdList->SetPipelineState(pso);


    cmdList->SetComputeRootDescriptorTable(5, input);
    cmdList->SetComputeRootDescriptorTable(6,mSobelGpuUav);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSobelFilter.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

    // How many groups do we need to dispatch to cover image, where each
    // group covers 16x16 pixels.
    UINT numGroupX = (UINT)ceilf(mWidth/16.0f);
    UINT numGroupY = (UINT) ceilf(mHeight/16.0f);

    cmdList->Dispatch(numGroupX, numGroupY, 1);

    cmdList->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(mSobelFilter.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_GENERIC_READ));


}
