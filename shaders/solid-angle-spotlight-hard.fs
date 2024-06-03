#version 330

in vec2 fragTexCoord;
in vec3 fragNorm;
in vec4 fragColor;
in vec3 fragPos;

out vec4 outColor;

vec3 lightDir = vec3(0.0, 0.0, 1.0);
float coneAngle = radians(30.0);

void main() {
  vec3 viewDir = normalize(fragPos);
  float angle  = acos(dot(viewDir, lightDir));
  if (angle < coneAngle) {
    outColor = fragColor;
  } else {
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
