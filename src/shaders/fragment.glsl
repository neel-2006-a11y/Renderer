#version 330 core

in vec3 vertexColor;
in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;


out vec4 FragColor;

struct PointLight{
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

uniform int colorBands;

#define MAX_LIGHTS 8
uniform int toonBands;
uniform float ambientLightIntensity;
uniform int numPointLights;
uniform PointLight pointLights[MAX_LIGHTS];

uniform float shininess;
uniform bool useBlinn;

uniform vec3 viewPos;

uniform sampler2D diffuseTex;
uniform bool useTexture;

void main() {
    vec3 result = vec3(0.0);
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos-fragPos);

    for(int i = 0; i<numPointLights; i++)
    {
        vec3 lightDir = normalize(pointLights[i].position - fragPos);

        // Ambient
        vec3 ambient = ambientLightIntensity * pointLights[i].color;

        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff*pointLights[i].color*pointLights[i].intensity;

        //Specular
        float spec = 0.0;
        if(useBlinn){
            // Blinn-Phong: halfway vertexColor
            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(halfwayDir,norm),0.0),shininess);
        }else{
            vec3 reflectDir = reflect(-lightDir, norm);
            spec = pow(max(dot(viewDir, reflectDir),0.0),shininess);
        }
        vec3 specular = spec * pointLights[i].color * 0.5;

        float distance = length(pointLights[i].position-fragPos);
        float attenuation = 1.0/(pointLights[i].constant+pointLights[i].linear*distance+pointLights[i].quadratic*distance*distance);
        result += (ambient + diffuse + specular)*attenuation;
    }
    result = floor(result*toonBands)/float(toonBands);
    vec3 color = vertexColor;
    if(useTexture){
        //color = vec4(1.0,1.0,1.0,1.0);
        color = texture(diffuseTex, texCoord).rgb;
    }
    
    vec3 finalcolor = result*color;
    vec3 finalcolor_reduced = floor(finalcolor*float(colorBands))/float(colorBands);
    FragColor = vec4(finalcolor_reduced,0.0);
}
