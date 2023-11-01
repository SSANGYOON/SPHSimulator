#include "global.hlsli"
#include "Global_SPH.hlsli"

RWTexture2D<float> depthMap: register(u0);
RWTexture2D<float3> normalMap: register(u1);

float3 GetEyePos(int2 xy)
{
	float depth = depthMap[xy];

	float4 Ndc = float4(2.f * (float2)xy / viewPort - float2(1.f, 1.f), (farClip - farClip * nearClip / depth) / (farClip - nearClip), 1.f) * depth;
	float3 eyePos = mul(Ndc, projectionInv).xyz;

	return eyePos;
}

[numthreads(32, 8, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{
	// read eye-space depth from texture
	float depth = depthMap[DTid.xy];
	if (depth >= farClip || depth <= nearClip) {
		normalMap[DTid.xy] = float3(0, 0, -1.f);
	}
	else {

		// calculate eye-space position from depth
		int2 screenPos = (int2)DTid.xy;
		float3 posEye = GetEyePos(screenPos);
		// calculate differences

		int2 minxy = int2(0, 0);
		int2 maxxy = (int2)(viewPort) - 1;

		int2 dir = int2(1, 0);

		float3 ddx = GetEyePos(clamp(screenPos + dir, minxy, maxxy)) - posEye;
		float3 ddx2 = posEye - GetEyePos(clamp(screenPos - dir, minxy, maxxy));
		if (abs(ddx.z) > abs(ddx2.z)) {
			ddx = ddx2;
		}

		dir = int2(0, 1);

		float3 ddy = GetEyePos(clamp(screenPos + dir, minxy, maxxy)) - posEye;
		float3 ddy2 = posEye - GetEyePos(clamp(screenPos - dir, minxy, maxxy));
		if (abs(ddy.z) > abs(ddy2.z)) {
			ddy = ddy2;
		}

		// calculate normal
		float3 n = cross(ddx, ddy);
		normalMap[DTid.xy] = normalize(n);
	}
}