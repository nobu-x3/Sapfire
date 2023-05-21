#version 450

layout(location = 0) in vec3 in_position;

layout(set = 0, binding = 0) uniform sceneData{
    mat4 projection;
    mat4 view;
} scene_data;

layout(push_constant) uniform push_constants{
    mat4 model;
} u_push_constants;

void main() {
    gl_Position = scene_data.projection * scene_data.view * u_push_constants.model * vec4(in_position, 1.0);
}