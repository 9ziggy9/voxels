#version 330

in vec2 fragTexCoord;
in vec3 fragNorm;
in vec4 fragColor;
in vec3 fragPos;

out vec4 outColor;

vec3 sphCenter = vec3(0.0, 0.0, 0.0);
float sphRadius = 25.0;

void main() {
  float distanceToCenter = distance(fragPos, sphCenter);
  if (distanceToCenter < sphRadius) {
    outColor = fragColor;
  } else {
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
