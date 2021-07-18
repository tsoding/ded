uniform vec2 resolution;
uniform vec2 camera;
uniform float time;
// uniform float camera_scale;

vec2 camera_project(vec2 point)
{
    float camera_scale = 1.0 + (sin(time) + 1.0) / 2.0;
    return 2.0 * (point - camera) * camera_scale / resolution;
}
