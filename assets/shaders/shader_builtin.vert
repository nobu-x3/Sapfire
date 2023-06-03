#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(set = 0, binding = 0) uniform sceneData{
    mat4 projection;
    mat4 view;
} scene_data;

layout(push_constant) uniform push_constants{
    mat4 model;
} u_push_constants;
layout(location = 0) out int out_mode;
// Data Transfer Object
layout(location = 1) out struct dto {
	vec2 tex_coord;
} out_dto;

void main() {
    out_dto.tex_coord = in_texcoord;
    gl_Position = scene_data.projection * scene_data.view * u_push_constants.model * vec4(in_position, 1.0);
}
