
struct Data
{
	float x,y,z;
};

StructuredBuffer<Data> inputBuffer : register (t0);
RWStructuredBuffer<float> output : register (u0);

[numthreads(32, 1, 1)]
void main( int3 DTid : SV_DispatchThreadID )
{
	output[DTid.x] = length(float3(inputBuffer[DTid.x].x, inputBuffer[DTid.x].y, inputBuffer[DTid.x].z));
}