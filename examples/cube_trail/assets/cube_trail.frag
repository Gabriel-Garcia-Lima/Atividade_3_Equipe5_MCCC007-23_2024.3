#version 300 es

precision mediump float;

in vec2 fragTexCoord;

uniform vec4 color;
uniform sampler2D diffuseTex;

out vec4 outColor;

void main() {
  vec4 texColor = texture(diffuseTex, fragTexCoord);
  outColor = texColor * color;
}