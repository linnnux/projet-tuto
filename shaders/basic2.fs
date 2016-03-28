#version 330
uniform sampler2D myTexture;
uniform vec4 couleur;
in  vec2 vsoTexCoord;
out vec4 fragColor;

void main(void) {
  vec4 t = texture(myTexture, vsoTexCoord).rgba;
  fragColor = vec4(t.rgb * couleur.rgb, t.a);
}
