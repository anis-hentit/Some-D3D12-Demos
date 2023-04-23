//=============================================================================
// Performs a separable Guassian blur with a blur radius up to 5 pixels.
//=============================================================================

cbuffer cbSettings : register(b0)
{
	// We cannot have an array entry in a constant buffer that gets mapped onto
	// root constants, so list each element.  

	int gBlurRadius;

	// Support up to 11 blur weights.
	float w0;
	float w1;
	float w2;
	float w3;
	float w4;
	float w5;
	float w6;
	float w7;
	float w8;
	float w9;
	float w10;
};

static const int gMaxBlurRadius = 5;


Texture2D gInput            : register(t1);
RWTexture2D<float4> gOutput : register(u1);

#define N 256
#define CacheSize (N + 2*gMaxBlurRadius)
groupshared float4 gCache[CacheSize];


float BilateralBlurWeight(float range, float bilateralSigma)
{
	const float PI = 3.14159265f;
	float twoSigmaBi2 = 2 * bilateralSigma * bilateralSigma;
	return rsqrt(2 * PI) * rsqrt(bilateralSigma) * exp((-range * range) / twoSigmaBi2);
}


[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Put weights in an array for indexing 
	float weights[11] = { w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,w10 };

	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel. (BlurRadius threads on the left and on the right)

	if (groupThreadID.x < gBlurRadius)
	{
		// clamp out of bound samples that occur at left image border
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}

	if (groupThreadID.x >= N - gBlurRadius)
	{	// clamp out of bound samples that occur at right image border
		int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x - 1);
		gCache[groupThreadID.x + 2 * gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];

	}

	// this statement takes care of the threads that are not gonna load two texels ( so threads between gBlurRadius and N-gBlurRadius). it also
		// clamps out of bound samples that occur at image borders(DispatchThreadId.x is out of texture),( more precisely the min statement takes care of
		// cases where we have threads that are out of texture size but their GroupTheadID.x doesnt have to be between gBlurRadius and N-gBlurRadius,
        // it can be either < gBlurRadius or >= N-gBlurRadius

	gCache[groupThreadID.x + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	
	float4 blurColor = { 0.0f,0.0f,0.0f,0.0f };

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurRadius + i;
		blurColor += gCache[k] * weights[i + gBlurRadius];
		
	}

	

	gOutput[dispatchThreadID.xy] = blurColor;
}




[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if (groupThreadID.y < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders (top border).
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
	}
	if (groupThreadID.y >= N - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders (bottom border).
		int y = min(dispatchThreadID.y + gBlurRadius, gInput.Length.y - 1);
		gCache[groupThreadID.y + 2 * gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
	}

	// this statement takes care of the threads that are not gonna load two texels ( so threads between gBlurRadius and N-gBlurRadius). it also
		// clamps out of bound samples that occur at image borders(DispatchThreadId.y is out of texture),( more precisely the min statement takes care of
		// cases where we have threads that are out of texture size but their GroupTheadID.y doesnt have to be between gBlurRadius and N-gBlurRadius,
		// it can be either < gBlurRadius or >= N-gBlurRadius

	gCache[groupThreadID.y + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];


	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	//
	// Now blur each pixel.
	//

	
	float4 blurColor = float4(0, 0, 0, 0);

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.y + gBlurRadius + i;

		blurColor += gCache[k] * weights[i + gBlurRadius];
		
	}

	

	gOutput[dispatchThreadID.xy] = blurColor;
}
