#version 330

in vec2 fragTexCoord;
in vec3 fragNorm;
in vec4 fragColor;
in vec3 fragPos;

out vec4 outColor;

uniform vec3 sunPos;
uniform float coneAngleDegs;

void main() {
  float coneAngle = radians(coneAngleDegs);
  vec3 viewDir    = normalize(fragPos);
  vec3 lightDir   = normalize(sunPos - fragPos);
  float angle     = acos(dot(viewDir, lightDir));
  float intensity = smoothstep(coneAngle, coneAngle * 0.75, angle);
  outColor        = mix(vec4(0.0, 0.0, 0.0, 1.0), fragColor, intensity);
}
