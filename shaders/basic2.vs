#version 330

layout (location = 0) in vec2 vsiPosition;
layout (location = 1) in vec2 vsiTexCoord;

uniform float ball_x;
uniform float ball_y;
uniform float scale;

out vec2 vsoTexCoord;

void main(void) {
  gl_Position = vec4(vsiPosition.x * scale + ball_x, vsiPosition.y * scale + ball_y, 0.0, 1.0);
  vsoTexCoord = vec2(vsiTexCoord.s, 1.0 - vsiTexCoord.t);
}
