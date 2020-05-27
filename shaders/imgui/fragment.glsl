#version 330

uniform sampler2D active_texture;

in vec4 v_Color;
in vec2 v_TexCoords;

out vec4 out_Color;

void main() {
	out_Color = texture(active_texture, v_TexCoords).rgba * v_Color;
}
