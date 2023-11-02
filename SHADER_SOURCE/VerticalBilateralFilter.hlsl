#include "global.hlsli"
#include "Global_SPH.hlsli"

RWTexture2D<float> backwardDepthMap: register(u0);
RWTexture2D<float> horizontalBlurredBackward: register(u1);
RWTexture2D<float> frontDepthMap: register(u2);
RWTexture2D<float> horizontalBlurredFront: register(u3);

[numthreads(32, 8, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{

	{
		float depth = horizontalBlurredBackward[DTid.xy];

		float sum = 0;
		float wsum = 0;

		for (float y = -filterRadius; y <= filterRadius + 1e-4; y += 1) {

			uint2 samplePos = uint2(DTid.x, clamp((int)(DTid.y) + y, 0, (uint)viewPort.y - 1));
			float sample = horizontalBlurredBackward[samplePos];
			// spatial domain
			float r = y / filterRadius;
			
			// range domain
			float r2 = (sample - depth) / blurDepthFalloff;

			float w = exp(-r * r / 2 - r2 * r2 / 2);
			sum += sample * w;
			wsum += w;
		}
		if (wsum > 0.0) {
			sum /= wsum;
		}

		backwardDepthMap[DTid.xy] = sum;
	}

	{
		float depth = horizontalBlurredFront[DTid.xy];

		float sum = 0;
		float wsum = 0;

		for (float y = -filterRadius; y <= filterRadius + 1e-4; y += 1) {

			uint2 samplePos = uint2(DTid.x, clamp((int)(DTid.y) + y, 0, (uint)viewPort.y - 1));
			float sample = horizontalBlurredFront[samplePos];
			// spatial domain
			float r = y / filterRadius;

			// range domain
			float r2 = (sample - depth) / blurDepthFalloff;

			float w = exp(-r * r / 2 - r2 * r2 / 2);
			sum += sample * w;
			wsum += w;
		}
		if (wsum > 0.0) {
			sum /= wsum;
		}

		frontDepthMap[DTid.xy] = sum;
	}
}