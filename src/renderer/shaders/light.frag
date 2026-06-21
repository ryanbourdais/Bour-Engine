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

#define MAX_POINT_LIGHTS 4
struct PointLight {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int pointLightCount;
// uniform vec3 objectColor;
uniform vec3 viewPos;


struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight directionalLight;

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float innerCutoff;
    float outerCutoff;
};

uniform SpotLight spotLight;

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

vec3 calculatePointLight(
    PointLight light,
    vec3 normal,
    vec3 fragmentPosition,
    vec3 viewDirection
) {
    vec3 lightDirection = normalize(light.position - fragmentPosition);

    float diffuseStrength = max(dot(normal, lightDirection), 0.0);

    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float specularStrength = pow(max(dot(viewDirection, reflectionDirection), 0.0), material.shininess);

    float distance = length(light.position - fragmentPosition);

    float attenuation = 1.0 / (
        light.constant +
        light.linear * distance +
        light.quadratic * distance * distance
    );

    vec3 diffuseTexture = vec3(texture(material.diffuse, TexCoords));

    vec3 specularTexture = vec3(texture(material.specular, TexCoords));

    vec3 ambient = light.ambient * diffuseTexture;

    vec3 diffuse = light.diffuse * diffuseStrength * diffuseTexture;

    vec3 specular = light.specular * specularStrength * specularTexture;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
{
    vec3 lightDirection = normalize(light.position - fragmentPosition);

    float diffuseStrength = max(dot(normal, lightDirection), 0.0);

    vec3 reflectionDirection = reflect(-lightDirection, normal);
    
    float specularStrength = pow(max(dot(viewDirection, reflectionDirection), 0.0), material.shininess);

    float distance = length(light.position - fragmentPosition);

    float attenuation = 1.0 / (
        light.constant +
        light.linear * distance +
        light.quadratic * distance * distance
    );

    float theta = dot(lightDirection, normalize(-light.direction));

    float epsilon = light.innerCutoff - light.outerCutoff;

    float coneIntensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    vec3 diffuseTexture = vec3(texture(material.diffuse, TexCoords));

    vec3 specularTexture = vec3(texture(material.specular, TexCoords));

    vec3 ambient = light.ambient * diffuseTexture;

    vec3 diffuse = light.diffuse * diffuseStrength * diffuseTexture;

    vec3 specular = light.specular * specularStrength * specularTexture;

    ambient *= attenuation * coneIntensity;
    diffuse *= attenuation * coneIntensity;
    specular *= attenuation * coneIntensity;

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

    for (int i = 0; i < pointLightCount; i++)
    {
        result += calculatePointLight(
            pointLights[i],
            normal,
            FragPos,
            viewDirection
        );
    }

    result += calculateSpotLight(
        spotLight,
        normal,
        FragPos,
        viewDirection
    );
    
    FragColor = vec4(result, 1.0);
}