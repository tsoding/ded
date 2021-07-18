uniform vec2 resolution;
uniform float camera_scale;
uniform vec2 camera_pos;

vec2 camera_project(vec2 point)
{
    return 2.0 * (point - camera_pos) * camera_scale / resolution;
}
