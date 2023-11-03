#include "global.hlsli"
#include "Global_SPH.hlsli"

RWTexture2D<float> backwardDepthMap: register(u0);
RWTexture2D<float> horizontalBlurredBackward: register(u1);
RWTexture2D<float> frontDepthMap: register(u2);
RWTexture2D<float> horizontalBlurredFront: register(u3);

#define PI_OVER_8 0.392699082f
const static float thresholdRatio = 10.5;

void ModifiedGaussianFilter(inout float sampleDepth, inout float weight, inout float weight_other, inout float upper, inout float lower, float lower_clamp, float threshold)
{
	if (sampleDepth > upper) {
		weight = 0;
		weight_other = 0;
	}
	else {
		if (sampleDepth < lower) {
			sampleDepth = lower_clamp;
		}
		else {
			upper = max(upper, sampleDepth + threshold);
			lower = min(lower, sampleDepth - threshold);
		}
	}
}

[numthreads(16, 16, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{
	{
		float depth = frontDepthMap[DTid.xy];

		//커널 크기 정하기
		float threshold = radius * 0.5f * thresholdRatio;
		float ratio = viewPort.y / 2.0 / tan(PI_OVER_8);
		float K = filterRadius * ratio * radius * 0.5f * 0.1f;
		int filterSize = min(10, int(ceil(K / depth)));

		float upper = depth + threshold;
		float lower = depth - threshold;
		float lower_clamp = depth - radius * 0.5f;

		float upper1 = upper;
		float upper2 = upper;
		float lower1 = lower;
		float lower2 = lower;

		float sum = depth;
		float wsum = 1;

		float sigma = filterSize / 3.0f;
		float two_sigma2 = 2.0f * sigma * sigma;

		for (int x = 1; x <= filterSize; x += 1) {

			uint2 samplePos1 = uint2(clamp(DTid.x + x, 0, (int)viewPort.x - 1), DTid.y);
			float sampleDepth1 = frontDepthMap[samplePos1];

			uint2 samplePos2 = uint2(clamp(DTid.x - x, 0, (int)viewPort.x - 1), DTid.y);
			float sampleDepth2 = frontDepthMap[samplePos2];

			float weight = exp(-x * x / two_sigma2);

			float weight1 = weight;
			float weight2 = weight;

			ModifiedGaussianFilter(sampleDepth1, weight1, weight2, upper1, lower1, lower_clamp, threshold);
			ModifiedGaussianFilter(sampleDepth2, weight2, weight1, upper2, lower2, lower_clamp, threshold);
			
			sum += sampleDepth1 * weight1 + sampleDepth2 * weight2;
			wsum += weight1 + weight2;
		}
		if (wsum > 0) {
			sum /= wsum;
		}

		horizontalBlurredFront[DTid.xy] = sum;
	}

	{
		float depth = backwardDepthMap[DTid.xy];

		//커널 크기 정하기
		float threshold = radius * 0.5f * thresholdRatio;
		float ratio = viewPort.y / 2.0 / tan(PI_OVER_8);
		float K = filterRadius * ratio * radius * 0.5f * 0.1f;
		int filterSize = min(10, int(ceil(K / depth)));

		float upper = depth + threshold;
		float lower = depth - threshold;
		float lower_clamp = depth - radius * 0.5f;

		float upper1 = upper;
		float upper2 = upper;
		float lower1 = lower;
		float lower2 = lower;

		float sum = depth;
		float wsum = 1;

		float sigma = filterSize / 3.0f;
		float two_sigma2 = 2.0f * sigma * sigma;

		for (int x = 1; x <= filterSize; x += 1) {

			uint2 samplePos1 = uint2(clamp(DTid.x + x, 0, (int)viewPort.x - 1), DTid.y);
			float sampleDepth1 = backwardDepthMap[samplePos1];

			uint2 samplePos2 = uint2(clamp(DTid.x - x, 0, (int)viewPort.x - 1), DTid.y);
			float sampleDepth2 = backwardDepthMap[samplePos2];

			float weight = exp(-x * x / two_sigma2);

			float weight1 = weight;
			float weight2 = weight;

			ModifiedGaussianFilter(sampleDepth1, weight1, weight2, upper1, lower1, lower_clamp, threshold);
			ModifiedGaussianFilter(sampleDepth2, weight2, weight1, upper2, lower2, lower_clamp, threshold);

			sum += sampleDepth1 * weight1 + sampleDepth2 * weight2;
			wsum += weight1 + weight2;
		}
		if (wsum > 0) {
			sum /= wsum;
		}

		horizontalBlurredBackward[DTid.xy] = sum;
	}
}