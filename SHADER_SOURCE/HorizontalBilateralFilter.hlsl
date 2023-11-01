#include "global.hlsli"
#include "Global_SPH.hlsli"

RWTexture2D<float> backwardDepthMap: register(u0);
RWTexture2D<float> horizontalBlurredBackward: register(u1);
RWTexture2D<float> frontDepthMap: register(u2);
RWTexture2D<float> horizontalBlurredFront: register(u3);

[numthreads(32, 8, 1)]
void CS_MAIN( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x >= viewPort.x || DTid.y >= viewPort.y)
	{
		return;
	}

	{
		float depth = frontDepthMap[DTid.xy];

		float sum = 0;
		float wsum = 0;

		for (int x = -filterRadius; x <= filterRadius; x += 1) {

			uint2 samplePos = uint2(clamp((int)(DTid.x) + x, 0, (int)viewPort.x - 1), DTid.y);
			float sampleDepth = frontDepthMap[DTid.xy];
			// spatial domain
			float r = x / filterRadius;
			float w = exp(-r * r / 2);
			// range domain
			float r2 = (sampleDepth - depth) / blurDepthFalloff;
			float g = exp(-r2 * r2 / 2);
			sum += sampleDepth * w * g;
			wsum += w * g;
		}
		if (wsum > 0) {
			sum /= wsum;
		}

		horizontalBlurredFront[DTid.xy] = sum;
	}

	{
		float depth = backwardDepthMap[DTid.xy];

		float sum = 0;
		float wsum = 0;

		for (int x = -filterRadius; x <= filterRadius; x += 1) {

			uint2 samplePos = uint2(clamp((int)(DTid.x) + x, 0, (int)viewPort.x - 1), DTid.y);
			float sampleDepth = backwardDepthMap[DTid.xy];
			// spatial domain
			float r = x / filterRadius;
			float w = exp(-r * r / 2);
			// range domain
			float r2 = (sampleDepth - depth) / blurDepthFalloff;
			float g = exp(-r2 * r2 / 2);
			sum += sampleDepth * w * g;
			wsum += w * g;
		}
		if (wsum > 0) {
			sum /= wsum;
		}

		horizontalBlurredBackward[DTid.xy] = sum;
	}
}