Texture2D<float4> tex : register(t0);
SamplerState samplerState;

cbuffer constants : register(b0) {
    float2 offset;
    float4 horizontalScale;
}

float2 GetTexCoord(float2 texcoord, float4 nHorizontalScale, float2 nOffset, float width1, float height1) {
    float2 uv = float2(texcoord.x * nHorizontalScale.x + 0.5, 0.5 - texcoord.y);
    uv.x -= nHorizontalScale.x * nOffset.x;
    uv.y += nOffset.y;
    float2 newUV = floor(uv * width1 * 0.75) / (width1 * 0.75);
    return newUV;
}

float4 GetSample(float2 texcoord, Texture2D texName) {
    float4 val = texName.Sample(samplerState, texcoord);
    clip(round(val.a) == 0 ? -1 : 1);
    return val;     
}

float4 main(float2 mainTexCoord : TEXCOORD0) : SV_TARGET0{
    float2 mainUV = GetTexCoord(mainTexCoord, horizontalScale, offset, 192, 138);
    float4 res = GetSample(mainUV, tex);
    return res;
}