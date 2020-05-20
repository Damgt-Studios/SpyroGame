
#pragma pack_matrix(row_major)

struct InputVertex
{
    float3 pos : POSITION;
    float3 tex : TEXCOORD;
    float3 normal : NORMAL;
    float3 tanget : TANGENT;
};

struct OutputVertex
{
    float4 pos : SV_Position;
    float3 tex : TEXCOORD;
    float3 normal : NORMAL;
    float3 tanget : TANGENT;
};

cbuffer SHADER_VARIABLES : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 cameraPosition;
};

OutputVertex main( InputVertex vertex )
{
    OutputVertex output = (OutputVertex) 0;
    output.pos = float4(vertex.pos, 1);
    output.tex = vertex.tex;
    output.normal = vertex.normal;
    output.tanget = vertex.tanget;
    
    output.pos = mul(output.pos, worldMatrix);
    output.pos = mul(output.pos, viewMatrix);
    output.pos = mul(output.pos, projectionMatrix);
    return output;

}