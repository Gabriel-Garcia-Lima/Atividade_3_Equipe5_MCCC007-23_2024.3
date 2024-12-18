#version 300 es

precision mediump float;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform vec3 lightPosition;
uniform vec3 objectColor;
uniform bool useTexture;
uniform sampler2D diffuseTexture;

out vec4 outColor;

void main() {
  // Ambient
  float ka = 0.2;
  vec3 ambient = ka * objectColor;

  // Diffuse
  vec3 N = normalize(fragNormal);
  vec3 L = normalize(lightPosition - fragPosition);
  float kd = 0.7;
  float diff = max(dot(N, L), 0.0);
  vec3 diffuse = kd * diff * objectColor;

  // Specular
  float ks = 0.2;
  vec3 V = normalize(-fragPosition);
  vec3 R = reflect(-L, N);
  float spec = pow(max(dot(R, V), 0.0), 32.0);
  vec3 specular = ks * spec * vec3(1.0);

  vec3 color;
  if (useTexture) {
    vec4 texColor = texture(diffuseTexture, fragTexCoord);
    color = texColor.rgb * (ambient + diffuse) + specular;
  } else {
    color = ambient + diffuse + specular;
  }

  outColor = vec4(color, 1.0);
}