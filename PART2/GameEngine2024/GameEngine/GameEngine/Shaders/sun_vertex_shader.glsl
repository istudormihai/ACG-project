#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoords; // Variable to pass texture coordinates to the fragment shader

uniform mat4 MVP; // Model-view-projection matrix

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
    TexCoords = aTexCoord; // Pass the texture coordinates to the fragment shader
}
