#version 330

in vec2 fragTexCoord;
in vec3 fragNorm;
in vec4 fragColor;
in vec3 fragPos;

out vec4 outColor;

uniform vec3 sunPos;
uniform float coneAngleDegs;
uniform sampler2D texture0;

void main() {
  float coneAngle = radians(coneAngleDegs);
  vec3 viewDir    = normalize(fragPos);
  vec3 lightDir   = normalize(sunPos - fragPos);
  float angle     = acos(dot(viewDir, lightDir));

  float intensity = smoothstep(coneAngle, coneAngle * 0.4, angle) *
                    max(dot(lightDir, vec3(0, 1, 0)), 0.0);
  
  intensity = max(intensity, 0.1);
  
  vec3 colorTemps[4] = vec3[4](
    vec3(1.0, 1.0, 1.0),
    vec3(0.7, 0.7, 0.2),
    vec3(1.0, 0.6, 0.2),
    vec3(0.2, 0.2, 0.5)
  );
  
  float factor = (intensity - 0.1) / 0.7;
  
  vec3 colorTemp;
  if (factor < 0.33) {
    colorTemp = mix(colorTemps[3], colorTemps[2], factor / 0.33);
  } else if (factor < 0.66) {
    colorTemp = mix(colorTemps[2], colorTemps[1], (factor - 0.33) / 0.33);
  } else {
    colorTemp = mix(colorTemps[1], colorTemps[0], (factor - 0.66) / 0.34);
  }
  
  outColor = mix(vec4(0.1 * fragColor.xyz, fragColor.w),
                 fragColor * vec4(colorTemp, 1.0), intensity);
}
