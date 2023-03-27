#type vert
#version 450 core
layout (location = 0) in vec3 pos;

void main() {
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}

#type frag
#version 450 core
out vec4 final_color;

void main() {
    final_color = vec4(1.0, 1.0, 1.0, 1.0);
}
