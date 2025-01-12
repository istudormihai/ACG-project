uniform vec3 thrusterColor; // Uniform to pass color

void main()
{
    // Use thruster color for the fragment color
    gl_FragColor = vec4(thrusterColor, 1.0f);
}
