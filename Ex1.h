#pragma once
#include "../../../Common/d3dUtil.h"
#include <time.h>
#include <cstdlib>
#include <DirectXMath.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

struct Data
{
	float x, y, z;
};

class Ex1
{
public :
	Ex1(ID3D12Device* device, ComPtr<ID3D12GraphicsCommandList> mCommandList, DXGI_FORMAT Format = DXGI_FORMAT_R32G32B32_FLOAT);
	Ex1(const Ex1& rhs) = delete;
	Ex1& operator=(const Ex1& rhs) = delete;
	Ex1() = default;

	void BuildResources(ComPtr<ID3D12GraphicsCommandList> mCommandList);
	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize);// version that caches the descriptors handles and size
	void BuildDescriptors();// overloaded version that actually builds the descriptors
	void Execute(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* RootSig, ID3D12PipelineState* Ex1Pso,
		ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap);
	void BuildVectors();
	void OutputFile();
private:
	ID3D12Device* md3dDevice = nullptr;
	std::vector<XMFLOAT3> mVector;
	
	DXGI_FORMAT mFormat = DXGI_FORMAT_R32G32B32_FLOAT;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mInputStructBufferCpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mOutputStructBufferCpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mInputStructBufferGpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mOutputStructBufferGpuUav;

	
	Microsoft::WRL::ComPtr<ID3D12Resource> mInputStructBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutputStructBuffer = nullptr;

	// temporary uploader to use createDefaultbuffer func for input structBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> BufferUploader = nullptr;
	// for read back in cpu
	Microsoft::WRL::ComPtr<ID3D12Resource> mBufferReadBack = nullptr;

};

