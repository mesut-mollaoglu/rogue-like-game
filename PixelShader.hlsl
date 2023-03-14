Texture2D<float4> tex : register(t0);
SamplerState samplerState;

struct TextureSize {
    float width;
    float height;
};

cbuffer constants : register(b0) {
    float2 offset;
    float4 horizontalScale;
}

float4 main(float2 texcoord : TEXCOORD, float4 color : COLOR) : SV_TARGET{
    TextureSize texSize;
    texSize.width = 152;
    texSize.height = 148;
    float2 uv = float2(texcoord.x * horizontalScale.x + 0.5, 0.5 - texcoord.y);
    uv.x -= horizontalScale.x * offset.x;
    uv.y += offset.y;
    float2 newUV = floor(uv * texSize.width * 0.75) / (texSize.width * 0.75);
    float4 val = tex.Sample(samplerState, newUV);
    float3 red = float3(0.393f, 0.769f, 0.189f);
    float3 green = float3(0.349f, 0.686f, 0.168f);
    float3 blue = float3(0.272f, 0.534f, 0.131f);
    float3 output;
    output.r = dot(val.rgb, red);
    output.g = dot(val.rgb, green);
    output.b = dot(val.rgb, blue);
    clip(round(val.a) == 0 ? -1 : 1);
    float4 ret = float4(output, val.a);
    if (horizontalScale.y == 1.0f) ret.a = lerp(0.0f, ret.a, 0.5f);
    return ret;
}