#version 330

in vec3 pos;
in vec4 clr;

out vec4 out_clr;

uniform mat4 mvp;

void main() {
    out_clr = clr;
    gl_Position = mvp * vec4(pos, 1.0);
}
