cbuffer constants : register(b0) {
    float2 position;
    float2 flipScale;
    float4 color;
};

cbuffer projection : register(b1) {
    float4x4 world;
    float4x4 view;
    float4x4 proj;
}

cbuffer health : register(b2) {
    float max;
    float min;
    float current;
    float padding;
}

struct VS_Input {
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D<float4> tex : register(t0);
SamplerState samplerState : register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    float4x4 worldViewProj = mul(world, mul(view, proj));
    output.pos = mul(float4(input.pos.x * flipScale.x, input.pos.y * flipScale.y, input.pos.z, 1.0f), worldViewProj);
    output.pos += float4(position, 0.0f, 0.0f);
    output.uv = float2((flipScale.x == -1.0f) ? 1.0f - input.uv.x : input.uv.x, (flipScale.y == -1.0f) ? 1.0f - input.uv.y : input.uv.y);
    return output;
}

float2 RotateMatrix(float2 uv, float angle) {
    float2 coord = uv * 2 - 1;
    float c = cos(radians(angle));
    float s = sin(radians(angle));
    float2x2 mat = float2x2(c, -s, s, c);
    return mul(mat, coord);
}

float4 ps_main(VS_Output input) : SV_Target
{
    if (color.a != 0) return color;
    float2 uv = input.uv;
    if (flipScale.x == -1) uv.x = 1 - input.uv.x;
    if (flipScale.y == -1) uv.y = 1 - input.uv.y;
    float4 val = tex.Sample(samplerState, uv);
    clip(round(val.a) == 0 ? -1 : 1);
    return val;
}

float4 healthbar(VS_Output input) : SV_Target0{
    float4 ret;
    float col = current / abs(max - min);
    ret = (input.uv.x > col) ? float4(0, 0, 0, 1) : float4(0.1, 0.1, 0.1, 0.9);
    float4 val = tex.Sample(samplerState, input.uv);
    clip(round(val.a) == 0 ? -1 : 1);
    return val + ret;
}

float4 font_main(VS_Output input) : SV_TARGET
{
    float4 val = tex.Sample(samplerState, input.uv);
    return (val.r == 0.0f) ? (1, 1, 1, 0) : val * color;
}