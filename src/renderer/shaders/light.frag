#version 410 core

#define MAX_SHADER_POINT_LIGHTS 4
#define MAX_SHADER_SPOT_LIGHTS 4

const int ALPHA_MODE_OPAQUE = 0;
const int ALPHA_MODE_MASK = 1;
const int ALPHA_MODE_BLEND = 2;

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
    vec4 diffuseColor;
    int alphaMode;
    float alphaCutoff;
}; 
  
uniform Material material;


struct PointLight {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform PointLight pointLights[MAX_SHADER_POINT_LIGHTS];
uniform int pointLightCount;
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

uniform SpotLight spotLights[MAX_SHADER_SPOT_LIGHTS];
uniform int spotLightCount;

vec3 calculateDirectionalLight(
    DirectionalLight light,
    vec3 normal,
    vec3 viewDirection
) {
    vec3 lightDirection = normalize(-light.direction);

    float diffuseStrength = max(dot(normal, lightDirection), 0.0);

    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float specularStrength = pow(max(dot(viewDirection, reflectionDirection), 0.0), material.shininess);

    vec4 diffuseSample = texture(material.diffuse, TexCoords) * material.diffuseColor;
    vec3 diffuseTexture = diffuseSample.rgb;

    // vec3 specularTexture = vec3(texture(material.specular, TexCoords));

    vec3 specularTexture = vec3(0.3);

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

    vec4 diffuseSample = texture(material.diffuse, TexCoords) * material.diffuseColor;
    vec3 diffuseTexture = diffuseSample.rgb;

    // vec3 specularTexture = vec3(texture(material.specular, TexCoords));
    // vec3 diffuseTexture = vec3(1.0);

    vec3 specularTexture = vec3(0.3);

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

    vec4 diffuseSample = texture(material.diffuse, TexCoords) * material.diffuseColor;
    vec3 diffuseTexture = diffuseSample.rgb;

    // vec3 specularTexture = vec3(texture(material.specular, TexCoords));

    // vec3 diffuseTexture = vec3(1.0);

    vec3 specularTexture = vec3(0.3);

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

    vec4 diffuseSample = texture(material.diffuse, TexCoords) * material.diffuseColor;

    if (material.alphaMode == ALPHA_MODE_MASK  && diffuseSample.a < material.alphaCutoff)
    {
        discard;
    }

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

    for (int i = 0; i < spotLightCount; i++)
    {
        result += calculateSpotLight(
            spotLights[i],
            normal,
            FragPos,
            viewDirection
        );
    }
    
    FragColor = vec4(result, diffuseSample.a);
    // FragColor = texture(material.diffuse, TexCoords);
}