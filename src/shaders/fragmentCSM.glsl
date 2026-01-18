#version 330 core

in vec3 vertexColor;
in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;
in float viewDepth;

out vec4 FragColor;

struct PointLight{
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
}; 

struct DirectionalLightCSM{
    vec3 direction;
    vec3 color;
    float intensity;
};

// Directional Light
#define MAX_CASCADES 5

uniform DirectionalLightCSM dirLight;
uniform bool useDirectionalLight;
uniform mat4 lightVPs[MAX_CASCADES];
uniform float cascadeSplits[MAX_CASCADES];
uniform sampler2D shadowMaps[MAX_CASCADES];
uniform int NUM_CASCADES;


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

int selectCascade ()
{
    //return 2;
    for(int i = 0; i < NUM_CASCADES; i++){
        if(viewDepth <= cascadeSplits[i])
        return i;
    }
    return NUM_CASCADES - 1;
}

float ShadowCalculation(vec4 fragPosLS, vec3 n, int cascade)
{
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5;    
    
    float currentDepth = projCoords.z;

    // Bias (normal-based)
    float bias = max(0.0001 * (1.0 - dot(n, -dirLight.direction)), 0.00005);
     bias = 0; // debug

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[cascade], 0);

    // 3x3 PCF
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(
                shadowMaps[cascade],
                projCoords.xy + vec2(x, y) * texelSize
            ).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;

    // debug (no pcf use)
    //float texDepth = texture(shadowMaps[cascade], projCoords.xy).r;
    //shadow = currentDepth - bias > texDepth ? 1.0 : 0.0;
    return shadow;
}

void main() {
    vec3 result = vec3(0.0);
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos-fragPos);


    // ===== DirectionalLight contribution =====
    float shadow = 0.0;

    if (useDirectionalLight)
    {
        vec3 lightDir = normalize(-dirLight.direction);

        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * dirLight.color * dirLight.intensity;

        // Specular
        float spec = 0.0;
        if (useBlinn)
        {
            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        }
        else
        {
            vec3 reflectDir = reflect(-lightDir, norm);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        }
        vec3 specular = spec * dirLight.color * 0.5;

        int cascade = selectCascade();
        vec4 fragLS = lightVPs[cascade] * vec4(fragPos, 1);
        shadow = ShadowCalculation(fragLS, norm, cascade);

        result +=  (1.0 - shadow) * (diffuse + specular);
    }

    // ===== PointLight contribution =====
    for(int i = 0; i<numPointLights; i++)
    {
        vec3 lightDir = normalize(pointLights[i].position - fragPos);

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
        result += (diffuse + specular)*attenuation;
    }

    // Ambient
    vec3 ambient = ambientLightIntensity * vec3(1.0,1.0,1.0);

    // ===== tooned lighting =====
    result = floor(result*toonBands)/float(toonBands);

    // ====== orinal color =====
    vec3 color = vertexColor;
    if(useTexture){
        color = texture(diffuseTex, texCoord).rgb;
    }
    
    vec3 finalcolor = result*color;
    vec3 finalcolor_reduced = floor(finalcolor*float(colorBands))/float(colorBands);
    FragColor = vec4(finalcolor_reduced,1.0);

    //debug
    // vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // FragColor = vec4(1-ShadowCalculation(fragPosLightSpace, norm),0,0,1);
    // FragColor = vec4(projCoords.xy+1,0,1);
    // FragColor = vec4(vec3(-projCoords.z),1);
    // FragColor = vec4(fragPosLightSpace.xyz / fragPosLightSpace.w, 1.0);

    //debug
    // vec3 p = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // if (p.x < -1 || p.x > 1 || p.y < -1 || p.y > 1 || p.z < -1 || p.z > 1)
    // {
    //    FragColor = vec4(1,0,0,1); // RED = outside shadow frustum
    //    return;
    //}
    int cascade = selectCascade();
    vec4 fragLS = lightVPs[cascade] * vec4(fragPos, 1);
    vec3 projCoords = fragLS.xyz / fragLS.w;
    projCoords = projCoords * 0.5 + 0.5;    

    float texDepth = texture(shadowMaps[cascade], projCoords.xy).r/2.0;

    //FragColor = vec4(vec3(float(cascade)/NUM_CASCADES), 1.0);
    //FragColor = vec4(vec3(cascadeSplits[cascade]/70.0f), 1.0);
    //FragColor = vec4(vec3(projCoords.z), 1.0);
    //FragColor = vec4(vec3(texDepth),1.0);
    //FragColor = vec4(vec3(viewDepth/70.0f), 1.0);

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 
        )
    {
        //FragColor = vec4(1.0,float(cascade)/NUM_CASCADES,0.0,1.0);
    }

}
