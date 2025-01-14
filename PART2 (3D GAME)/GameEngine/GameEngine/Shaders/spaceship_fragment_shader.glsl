#version 330 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 TexCoord;

uniform sampler2D texture1;   
uniform bool isThruster;       
uniform float time;            

void main()
{
    if (isThruster) {
        float intensity = 0.5f + 0.5f * sin(time * 3.0f);  
        vec3 red = vec3(1.0f, 0.0f, 0.0f);    
        vec3 orange = vec3(1.0f, 0.25f, 0.0f); 
        vec3 yellow = vec3(1.0f, 0.5f, 0.0f); 

        vec3 fireColor = mix(red, orange, intensity);      
        fireColor = mix(fireColor, yellow, intensity);    

        FragColor = vec4(fireColor, 1.0f);  
    } else {
        FragColor = texture(texture1, TexCoord);
    }
}
