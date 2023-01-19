#include "Ex1.h"

Ex1::Ex1(ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList> mCommandList, DXGI_FORMAT Format )
{
	md3dDevice = device;
	BuildVectors();
	BuildResources(mCommandList);
}

void Ex1::BuildResources(ComPtr<ID3D12GraphicsCommandList> mCommandList)
{
	int vbByteSize = mVector.size() * sizeof(Data);

	mInputStructBuffer = d3dUtil::CreateDefaultBuffer(md3dDevice,
		mCommandList.Get(), mVector.data(), vbByteSize, BufferUploader);


	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(float)*vbByteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(mOutputStructBuffer.GetAddressOf())));


	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(float)*vbByteSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(mBufferReadBack.GetAddressOf())));


}

void Ex1::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor, UINT descriptorSize)
{
	mInputStructBufferCpuSrv = hCpuDescriptor;
	mOutputStructBufferCpuUav = hCpuDescriptor.Offset(1, descriptorSize);

	mInputStructBufferGpuSrv = hGpuDescriptor;
	mOutputStructBufferGpuUav = hGpuDescriptor.Offset(1, descriptorSize);


	BuildDescriptors();
}

void Ex1::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = 64;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.StructureByteStride = sizeof(Data);

	md3dDevice->CreateShaderResourceView(mInputStructBuffer.Get(), &srvDesc, mInputStructBufferCpuSrv);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 64;
	uavDesc.Buffer.StructureByteStride = sizeof(float);
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	md3dDevice->CreateUnorderedAccessView(mOutputStructBuffer.Get(), nullptr, &uavDesc, mOutputStructBufferCpuUav);
}

void Ex1::Execute(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* RootSig, ID3D12PipelineState* Ex1Pso,
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap)
{

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);



	cmdList->SetComputeRootSignature(RootSig);
	cmdList->SetPipelineState(Ex1Pso);
	cmdList->SetComputeRootShaderResourceView(3,mInputStructBuffer->GetGPUVirtualAddress());
	cmdList->SetComputeRootUnorderedAccessView(4,mOutputStructBuffer->GetGPUVirtualAddress());

	cmdList->Dispatch(2, 1, 1);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOutputStructBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS
																								, D3D12_RESOURCE_STATE_COPY_SOURCE));
	cmdList->CopyResource(mBufferReadBack.Get(), mOutputStructBuffer.Get());

	


}

void Ex1::BuildVectors()
{
	mVector.resize(64);
	float x = 0.0f, y = 0.0f, z = 0.0f;

	srand(1);
	for (int i = 0; i < 64; ++i)
	{
		x = static_cast<float>(rand()) / 900;
		y = static_cast<float>(rand()) / 900;
		z = static_cast<float>(rand()) / 900;

		if (x * x + y * y + z * z < 100)
		{
			mVector[i].x = x;
			mVector[i].y = y;
			mVector[i].z = z;
		}
		else
			--i;

	}

}

void Ex1::OutputFile()
{
	float* mMapData = nullptr;

	ThrowIfFailed(mBufferReadBack->Map(0,nullptr,reinterpret_cast<void**>(&mMapData)));

	std::ofstream fout("Ex1result.txt");

	for (int i = 0 ; i < 64; ++i)
	{
		fout << mMapData[i] << std::endl;
	}

	mBufferReadBack->Unmap(0,nullptr);
}
