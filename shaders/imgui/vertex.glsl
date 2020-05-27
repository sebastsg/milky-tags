#version 330

uniform mat4 model_view_projection;

in vec2 in_Position;
in vec2 in_TexCoords;
in vec4 in_Color;

out vec4 v_Color;
out vec2 v_TexCoords;

void main() {
	gl_Position = model_view_projection * vec4(in_Position.xy, 0.0f, 1.0f);
	v_Color = in_Color;
	v_TexCoords = in_TexCoords;
}
