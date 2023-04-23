

Texture2D gInput            : register(t5);// bound in Compute rootsig (for offscreen texture)
RWTexture2D<float4> gOutput : register(u5);// bound in Compute rootsig

// Approximates luminance ("brightness") from an RGB value.  These weights are derived from
// experiment based on eye sensitivity to different wavelengths of light.
float CalcLuminance(float3 color)
{
    return dot(color, float3(0.299f,0.587f, 0.114f));
}

[numthreads(16,16,1)]
void SobelCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    //Sample the pixels in the neighborhood of this pixel
    float4 c[3][3];
    for(int i =0; i < 3; ++i)
    {
      for(int j = 0; j < 3; ++j)
          {
              // remember that the x axis goes to right and y axis goes down in thread grids.
              int2 xy = dispatchThreadID.xy + int2(-1 + j, -1 + i);
              c[i][j] = gInput[xy];
          }
    }

    // For each color channel, estimate the partial x derivative using Sobel scheme
    float4 Gx = -1.0f*c[0][0]- 2.0f*c[1][0] - 1.0f*c[2][0] + 1.0f*c[0][2] + 2.0f*c[1][2] + 1.0f*c[2][2];

    // Same thing for partial y derivative
    float4 Gy = 1.0f*c[0][0] + 2.0f*c[0][1] + 1.0f*c[0][2] - 1.0f*c[2][0] - 2.0f*c[2][1]- 1.0f*c[2][2];

    // Gradient is (Gx,Gy), Gx and Gy are float4 (one float for each channel rgba).
    // For each channel, compute the magnitude to get the maximum rate of change
    // It computes the mag of the gradient of each color channel : redGrad=(Gxr,Gyr), greenGrad = (Gxg,Gyg) ect...
    // mag.r will hold magnitude of redGrad, mag.g will hold magnitude of greenGrad and so on.
    float4 mag = sqrt(Gx*Gx + Gy*Gy);

    //Make edges black and nonedge white.
    mag = saturate(CalcLuminance(mag.rgb));

    gOutput[dispatchThreadID.xy] = mag;
}
