#include "global.hlsli"
#include "Global_SPH.hlsli"

Texture2D<float> frontDepthMap: register(t0);
Texture2D<float> backwardDepthMap: register(t1);
Texture2D<float3> normalMap: register(t2);
TextureCube cubeMap : register(t3);
Texture2D backgroundTexture: register(t4);
Texture2D<float> obstacleDepthMap: register(t5);

const static float3 LightDirection = float3(0.707, -0.707, 0);

struct PSIn
{
    float4 Pos : SV_Position;
	float3 ScreenToWorld : WorldPosition;
    float3 EyePos : ViewPosition;
    float2 UV : TEXCOORD;
};

PSIn VS_MAIN(VSIn In)
{
    PSIn Out = (PSIn)0.f;

    Out.UV = In.UV;
	Out.Pos = float4(In.Pos.xy * 2, 1.f, 1.f);
	Out.ScreenToWorld = mul(mul(float4(In.Pos.xy * 2, 1, 1) * farClip, projectionInv), viewInv).xyz;
    Out.EyePos = mul(float4(0.f, 0.f, 0.f, 1.f), viewInv).xyz;

    return Out;
}

float4 PS_MAIN(PSIn In) : SV_Target
{
	const float3 waterColor = float3(0.1f, 0.4f, 0.8f);	
	float depth = frontDepthMap.Sample(linearSampler, In.UV);
	
	if (depth > farClip)
	{
		return backgroundTexture.Sample(linearSampler, In.UV);
	}

	//ViewPosition
	float2 Ndc = 2.f * In.UV - 1.f;
	Ndc.y *= -1.f;

	float2 coff = 1.f / float2(projection._11, projection._22);
	float3 eyePos = float3(Ndc * coff, 1) * depth;

	//WorldPosition
	float3 worldPos = mul(float4(eyePos, 1.f), viewInv).xyz;
	
	float3 viewNormal = normalMap.Sample(linearSampler, In.UV);
	viewNormal.xz *= -1.f;
	float3 normal = mul(float4(viewNormal, 0.f), viewInv).xyz;

	float3 VertexToEye = normalize(In.EyePos - worldPos);
	float3 halfVector = normalize(VertexToEye - LightDirection);
	float SpecularFactory = max(dot(halfVector, normal), 0);
	if (SpecularFactory > 0)
	{
		SpecularFactory = pow(SpecularFactory, SpecularPower);
	}

	float3 ViewDirection = -VertexToEye;
	float3 Reflection = reflect(ViewDirection, normal);
	float3 ReflectionTex = cubeMap.Sample(linearSampler, ViewDirection).xyz;
	float3 ReflectionColor = SpecularFactory * SpecularColor * SpecularIntensity * cubeMap.Sample(linearSampler, Reflection).xyz;

	float Radio = 1.0 / 1.33;
	float3 RefractionDir = refract(ViewDirection, normal, Radio);
	
	float3 absorbance = 1 - fluidColor;

	float backwardDepth = backwardDepthMap.Sample(linearSampler, In.UV);
	float obstacleDepth = obstacleDepthMap.Sample(linearSampler, In.UV);
	float thickness = min(backwardDepth, obstacleDepth) - min(depth, obstacleDepth);
	thickness = clamp(thickness, 0, farClip);

	if(thickness <= 0.f)
		return backgroundTexture.Sample(linearSampler, In.UV);

	float3 absorbtionColor = exp(-absorbance * thickness * absorbanceCoff);

	float3 RefractionPos = RefractionDir * thickness * 0.5f + worldPos;
	float4 RefractionNdc = mul(mul(float4(RefractionPos, 1.f), view), projection);
	float2 RefractionUV = RefractionNdc.xy / RefractionNdc.w;
	RefractionUV.y *= -1;
	RefractionUV = RefractionUV * 0.5f + 0.5f;

	float3 RefractionColor = absorbtionColor * backgroundTexture.Sample(linearSampler, RefractionUV);

	const float f0 = (1.33 - 1) * (1.33 - 1) / ((1.33 + 1) * (1.33 + 1));
	float fresnel = f0 + (1.f - f0) * pow(1.0 - max(dot(normal, VertexToEye), 0.0), 5.0);
	float4 color = float4(lerp(RefractionColor, ReflectionColor, fresnel), 1.f);

	return color;
}