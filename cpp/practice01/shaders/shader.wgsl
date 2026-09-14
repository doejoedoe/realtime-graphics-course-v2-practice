@vertex
fn vertexMain(@builtin(vertex_index) vertexIndex: u32)
    -> @builtin(position) vec4f
{
    var positions = array<vec2f, 3>(
        vec2f( 0.0, 0.5),
        vec2f(-0.5, -0.5),
        vec2f( 0.5, -0.5),
    );
    return vec4f(positions[vertexIndex], 0.0, 1.0);
} 

@fragment
fn fragmentMain(@builtin(position) fragCoord: vec4f) -> @location(0) vec4f {
    let size = 20.0;
    if((i32(fragCoord.x / size) + i32(fragCoord.y / size)) % 2 == 0) {
        return vec4f(0.95, 0.35, 0.15, 1.0);
    }
    else {
        return vec4f(0.35, 0.95, 0.15, 1.0);    
    }
}