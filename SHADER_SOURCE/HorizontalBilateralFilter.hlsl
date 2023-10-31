#include "global.hlsli"
#include "Global_SPH.hlsli"

RWTexture2D<float> depthMap: register(u0);
RWTexture2D<float> horizontalBlur: register(u1);

[numthreads(32, 8, 1)]
void CS_MAIN( uint3 DTid : SV_DispatchThreadID )
{
	float depth = depthMap[DTid.xy];
	
	float sum = 0;
	float wsum = 0;

	if (DTid.x >= viewPort.x || DTid.y >= viewPort.y)
	{
		return;
	}
	for (int x = -filterRadius; x <= filterRadius; x += 1) {

		uint2 samplePos = uint2(clamp((int)(DTid.x) + x, 0, (int)viewPort.x - 1), DTid.y);
		float sampleDepth = depthMap[DTid.xy];
		// spatial domain
		float r = x;
		float w = exp(-r * r / (2 * filterRadius * filterRadius));
		// range domain
		float r2 = (sampleDepth - depth) / blurDepthFalloff;
		float g = exp(-r2 * r2 / 2);
		sum += sampleDepth * w * g;
		wsum += w * g;
	}
	if (wsum > 0) {
		sum /= wsum;
	}

	horizontalBlur[DTid.xy] = sum;
}