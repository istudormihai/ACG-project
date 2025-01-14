#version 330 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 TexCoord;

uniform sampler2D texture1;   // Texture unit 1
uniform bool isThruster;       // Flag to indicate if it's the thruster
uniform float time;            // Time variable to animate the fire effect

void main()
{
    if (isThruster) {
        // Create a gradient effect using sine and time
        float intensity = 0.5f + 0.5f * sin(time * 3.0f);  // Sine wave oscillation for color change
        vec3 red = vec3(1.0f, 0.0f, 0.0f);    // Red color
        vec3 orange = vec3(1.0f, 0.25f, 0.0f); // Orange color
        vec3 yellow = vec3(1.0f, 0.5f, 0.0f); // Yellow color

        // Interpolate between red, orange, and yellow
        vec3 fireColor = mix(red, orange, intensity);      // Interpolate between red and orange
        fireColor = mix(fireColor, yellow, intensity);    // Interpolate between red+orange and yellow

        FragColor = vec4(fireColor, 1.0f);  // Apply the interpolated fire color
    } else {
        // For objects that are not thrusters, use the texture
        FragColor = texture(texture1, TexCoord);
    }
}
