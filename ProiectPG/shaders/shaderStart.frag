#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec4 fPointPos;

out vec4 fColor;

//lighting
uniform	vec3 lightPointPosEye1;
uniform	vec3 lightPointColor1;

uniform	vec3 lightPointPosEye2;
uniform	vec3 lightPointColor2;

uniform	vec3 lightPointPosEye3;
uniform	vec3 lightPointColor3;

uniform	vec3 lightPointPosEye4;
uniform	vec3 lightPointColor4;
uniform bool projectileSpawned;


uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform bool lightMode;
uniform vec3 viewPos;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform samplerCube skybox;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 1.0f;

float computeShadow()
{
	float bias = max(0.0005f * (1.0f - dot(fNormal, lightDir)), 0.0005f);
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	return currentDepth - bias > closestDepth ? 1.0f : 0.0f;
}


void computeLightComponentsDir()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 halfVector = normalize(lightDirN + viewDirN);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular =  specularStrength * specCoeff * lightColor;

	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;
}

vec3 computeLightComponentsPoint(vec3 lightPointPos, float constant, float linear, float quadratic, vec3 color)
{		
	vec3 cameraPosEye = vec3(0.0f);

	float dist = length(lightPointPos - fPointPos.xyz);
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightPointPos - fPointPos.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPointPos.xyz);

	//compute ambient light
	vec3 ambientPoint = att * ambientStrength * color;
	
	//compute diffuse light
	vec3 diffusePoint = att * max(dot(normalEye, lightDirN), 0.0f) * color;
	
	//compute specular light
	vec3 halfVector = normalize(lightDirN + viewDirN);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	vec3 specularPoint = att * specularStrength * specCoeff * color;

	ambientPoint *= texture(diffuseTexture, fTexCoords).rgb;
	diffusePoint *= texture(diffuseTexture, fTexCoords).rgb;
	specularPoint *= texture(specularTexture, fTexCoords).rgb;

	return ambientPoint + diffusePoint + specularPoint;
}

void main() 
{
	vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
	if(colorFromTexture.a < 0.1)
		discard;
	
	vec3 colorPoint1;
	vec3 colorPoint2;
	vec3 colorPoint3;
	vec3 colorPoint4 = projectileSpawned ? computeLightComponentsPoint(lightPointPosEye4, constant, 0.09f, 0.032f, lightPointColor4) : vec3(0.0f, 0.0f, 0.0f);;

	if(lightMode)
	{
		computeLightComponentsDir();
	}
	else
	{
		colorPoint1 = computeLightComponentsPoint(lightPointPosEye1, constant, 0.35f, 0.44f, lightPointColor1);
		colorPoint2 = computeLightComponentsPoint(lightPointPosEye2, constant, 0.22f, 0.20f, lightPointColor2);
		colorPoint3 = computeLightComponentsPoint(lightPointPosEye3, constant, 0.045f, 0.0075f, lightPointColor3);
	}

	float shadow = lightMode ? computeShadow() : 0.0f;
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	vec3 finalColor = min(color + colorPoint1 + colorPoint2 + colorPoint3 + colorPoint4, 1.0f);

    fColor = vec4(finalColor, 1.0f);
}