#version 410 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 
  
uniform Material material;

struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;  
// uniform vec3 objectColor;
uniform vec3 viewPos;


struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight directionalLight;

vec3 calculateDirectionalLight(
    DirectionalLight light,
    vec3 normal,
    vec3 viewDirection
) {
    vec3 lightDirection = normalize(-light.direction);

    float diffuseStrength = max(dot(normal, lightDirection), 0.0);

    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float specularStrength = pow(max(dot(viewDirection, reflectionDirection), 0.0), material.shininess);

    vec3 diffuseTexture = vec3(texture(material.diffuse, TexCoords));

    vec3 specularTexture = vec3(texture(material.specular, TexCoords));

    vec3 ambient = light.ambient * diffuseTexture;

    vec3 diffuse = light.diffuse * diffuseStrength * diffuseTexture;

    vec3 specular = light.specular * specularStrength * specularTexture;

    return ambient + diffuse + specular; 

}

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDirection = normalize(viewPos - FragPos);

    vec3 result = calculateDirectionalLight(
        directionalLight,
        normal,
        viewDirection
    );

    FragColor = vec4(result, 1.0);
}