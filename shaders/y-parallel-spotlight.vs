#version 330

in vec3 vertPos;
in vec2 vertTexCoord;
in vec3 vertNorm;
in vec4 vertColor;

out vec2 fragTexCoord;
out vec3 fragNorm;
out vec4 fragColor;
out vec3 fragPos;

uniform mat4 mvp;

void main() {
  fragTexCoord = vertTexCoord;
  fragNorm     = vertNorm;
  fragColor    = vertColor;
  fragPos      = vertPos;
  gl_Position  = mvp * vec4(vertPos, 1.0);
}
