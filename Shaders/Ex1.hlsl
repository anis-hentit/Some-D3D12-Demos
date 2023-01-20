
struct Data
{
	float x,y,z;
};

Buffer<float3> inputBuffer : register (t0);
RWBuffer<float> output : register (u2);

[numthreads(32, 1, 1)]
void main( int3 DTid : SV_DispatchThreadID )
{
	output[DTid.x] = length(inputBuffer[DTid.x]);
}