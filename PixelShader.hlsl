Texture2D tex;
SamplerState samplerState;

cbuffer constants : register(b0) {
    float2 offset;
    float4 horizontalScale;
}

float4 main(float2 texcoord : TEXCOORD, float4 color : COLOR) : SV_TARGET{
    float2 uv = float2(texcoord.x + 0.5, 0.5 - texcoord.y);
    uv.x -= offset.x;
    uv.y += offset.y;
    float4 val = tex.Sample(samplerState, uv);
    if (val.a <= 0.125) discard;
    return val;
}