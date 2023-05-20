cbuffer constants : register(b0)
{
    float2 offset;
    float2 flipScale;
    float4 fColor;
};

cbuffer proj : register(b1)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

struct VS_INPUT
{
    float2 inPos : POSITION;
    float4 inColor : COLOR;
    float2 inTexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float4 outColor : COLOR;
    float2 outTexCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4x4 worldViewProj = mul(world, mul(view, proj));
    output.outPosition = mul(float4(input.inPos.x * flipScale.x, input.inPos.y * flipScale.y, 0.0f, 1.0f), worldViewProj);
    output.outPosition += float4(offset, 0.0f, 0.0f);
    output.outColor = input.inColor;
    output.outTexCoord = input.inTexCoord;
    return output;
}
