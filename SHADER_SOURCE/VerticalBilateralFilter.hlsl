#include "global.hlsli"
#include "Global_SPH.hlsli"

RWTexture2D<float> depthMap: register(u0);
RWTexture2D<float> verticalBlur: register(u1);

[numthreads(32, 8, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{
	float depth = depthMap[DTid.xy];

	float sum = 0;
	float wsum = 0;

	if (DTid.x >= viewPort.x || DTid.y >= viewPort.y)
	{
		return;
	}
	for (int y = -filterRadius; y <= filterRadius; y += 1) {

		uint2 samplePos = uint2(DTid.x, clamp((int)(DTid.y) + y, 0, (uint)viewPort.y - 1));
		float sample = depthMap[samplePos];
		// spatial domain
		float r = y;
		float w = exp(-r * r / (2 * filterRadius * filterRadius));
		// range domain
		float r2 = (sample - depth) / blurDepthFalloff;
		float g = exp(-r2 * r2);
		sum += sample * w * g;
		wsum += w * g;
	}
	if (wsum > 0.0) {
		sum /= wsum;
	}

	verticalBlur[DTid.xy] = sum;
}