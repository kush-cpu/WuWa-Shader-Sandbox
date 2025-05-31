// basic.hlsl
struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0);
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 1.0); // Red color
}
