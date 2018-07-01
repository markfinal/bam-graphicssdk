#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    float4 position;
    float4 color;
} VertexIn;

typedef struct {
    float4 position [[position]];
    half4  color;
} VertexOut;

vertex VertexOut clip_space_colour_vertex_function(device VertexIn *vertices [[buffer(0)]],
                                 uint vid [[vertex_id]])
{
    VertexOut out;
    out.position = vertices[vid].position;
    out.color = half4(vertices[vid].color);
    return out;
}

fragment half4 pass_through_colour_fragment_function(VertexOut in [[stage_in]])
{
    return in.color;
}
