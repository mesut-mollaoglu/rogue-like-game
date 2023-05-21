Texture2D<float4> tex : register(t0);
SamplerState samplerState;

struct TextureSize {
    float width;
    float height;
};

cbuffer constants : register(b0) {
    float2 offset;
    float2 flipScale;
    float4 fColor;
}

float4 main(float2 texcoord : TEXCOORD, float4 color : COLOR) : SV_TARGET{
    if (fColor.a != 0) return fColor;
    TextureSize texSize;
    texSize.width = 152;
    float2 texCoord = float2(texcoord.x * flipScale.x + 0.5, 0.5 - texcoord.y * flipScale.y);
    float2 uv;
    uv.x = texCoord.x * cos(-fColor.x) - texCoord.y * sin(-fColor.x);
    uv.y = texCoord.x * sin(-fColor.x) + texCoord.y * cos(-fColor.x);
    uv.x -= flipScale.x * offset.x * cos(-fColor.x);
    uv.y += offset.y * cos(-fColor.x);
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
    return ret;
}