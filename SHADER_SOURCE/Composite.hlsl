#include "global.hlsli"
#include "Global_SPH.hlsli"

Texture2D<float> frontDepthMap: register(t0);
Texture2D<float> thicknessMap: register(t1);
Texture2D<float3> normalMap: register(t2);
TextureCube cubeMap : register(t3);

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
		return cubeMap.Sample(linearSampler, normalize(In.ScreenToWorld - In.EyePos));
	}

	float3 absorbance = 1 - fluidColor;
	float thickness = thicknessMap.Sample(linearSampler, In.UV);
	float3 absorbtionColor = exp(-absorbance * thickness * absorbanceCoff);

	//ViewPosition
	float2 Ndc = 2.f * In.UV - 1.f;
	Ndc.y *= -1.f;

	float2 coff = 1.f / float2(projection._11, projection._22);
	float3 eyePos = float3(Ndc * coff, 1) * depth;

	//WorldPosition
	float3 worldPos = mul(float4(eyePos, 1.f), viewInv).xyz;

	
	float3 viewNormal = normalMap.Sample(linearSampler, In.UV);
	viewNormal.x *= -1.f;
	viewNormal.z *= -1.f;
	float3 normal = mul(float4(viewNormal, 0.f), viewInv).xyz;

	float3 VertexToEye = normalize(In.EyePos - worldPos);
	float3 LightReflect = normalize(reflect(LightDirection, normal));
	float SpecularFactory = dot(VertexToEye, LightReflect);
	if (SpecularFactory > 0)
	{
		SpecularFactory = pow(SpecularFactory, SpecularPower);
	}

	float3 ViewDirection = -VertexToEye;
	float3 Reflection = reflect(ViewDirection, normal);
	float3 ReflectionTex = cubeMap.Sample(linearSampler, ViewDirection).xyz;
	float3 ReflectionColor = SpecularFactory * SpecularColor;

	float Radio = 1.0 / 1.33;
	float3 Refraction = refract(ViewDirection, normal, Radio);
	
	//TODO 방향 수정
	float3 RefractionColor = absorbtionColor * cubeMap.Sample(linearSampler, Refraction).xyz;

	const float f0 = (1.33 - 1) * (1.33 - 1) / ((1.33 + 1) * (1.33 + 1));
	float fresnel = f0 + (1.f - f0) * pow(1.0 - max(dot(viewNormal, VertexToEye), 0.0), 5.0);
	float4 color = float4(RefractionColor, 1.f);

	return color;
}