#include "global.hlsli"

RWTexture2D<float> FrontDepthMap: register(u0);
RWTexture2D<float> BackwardDepthMap: register(u1);
RWTexture2D<float> ThicknessMap: register(u2);

[numthreads(16, 16, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{
	ThicknessMap[DTid.xy] = clamp(BackwardDepthMap[DTid.xy] - FrontDepthMap[DTid.xy], 0, farClip);
}