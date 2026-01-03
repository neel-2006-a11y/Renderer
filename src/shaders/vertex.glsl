#version 330 core

layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 1) in vec3 aColor;    // Vertex color
layout(location = 2) in vec3 aNormal;   // Vertex normal (optional)
layout(location = 3) in vec2 aTexCoord; // Texture coordinates (optional)

out vec3 vertexColor;
out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightVP;

uniform bool useDirectionalLight;

void main()
{

    // Compute the normal in world space
    normal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Pass color and texCoord to the fragment shader
    vertexColor = aColor;
    texCoord = aTexCoord;

    vec4 worldPos = model * vec4(aPos,1.0);

    fragPos = worldPos.xyz;

    fragPosLightSpace = lightVP * worldPos;

    gl_Position = projection * view * worldPos;
}
