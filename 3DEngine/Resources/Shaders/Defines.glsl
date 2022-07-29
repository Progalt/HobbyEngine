


// This was useful forum I modified for my own use
// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
// https://discourse.panda3d.org/t/glsl-octahedral-normal-packing/15233

vec2 sign_not_zero(vec2 v) 
{
        return fma(step(vec2(0.0), v), vec2(2.0), vec2(-1.0));
}

vec2 encode (vec3 v)
{
	vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx))  * sign_not_zero(p)) : p;
}

vec3 decode(vec2 enc)
{
    vec3 v = vec3(enc.xy, 1.0 - abs(enc.x) - abs(enc.y));
    if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
	return normalize(v);
}

vec3 calculate_view_position(vec2 texture_coordinate, float depth_from_depth_buffer, mat4 inverse_projection_matrix)
{
    vec3 clip_space_position = vec3(texture_coordinate, depth_from_depth_buffer) * 2.0 - vec3(1.0);

    vec4 view_position = vec4(vec2(inverse_projection_matrix[0][0], inverse_projection_matrix[1][1]) * clip_space_position.xy,
                                   inverse_projection_matrix[2][3] * clip_space_position.z + inverse_projection_matrix[3][3]);

    return(view_position.xyz / view_position.w);
}