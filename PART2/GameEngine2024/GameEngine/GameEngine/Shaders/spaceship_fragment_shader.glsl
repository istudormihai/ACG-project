#version 330 core
out vec4 FragColor;
in vec3 fragPos;
in vec3 normal;
in vec2 TexCoord;

uniform sampler2D texture1;  // Texture unit 1


void main()
{
    FragColor = texture(texture1, TexCoord);
}
