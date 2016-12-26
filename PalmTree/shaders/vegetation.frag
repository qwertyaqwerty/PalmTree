#version 330 core

in vec2 TexCoords;
in vec3 fragNormal;
in vec3 fragPosition;

out vec4 color;

uniform sampler2D texture_diffuse1;

uniform sampler2D leafThickness;

uniform int meshId;

uniform vec3 lightDirection;
uniform vec3 eyePosition;

float lambert(vec3 fragNormal) {
	return max(0.2, dot(-normalize(lightDirection), normalize(fragNormal)));
}

float specular(vec3 fragNormal) {
	vec3 eyeVec = normalize(fragPosition - eyePosition);
	vec3 reflectVec = reflect(eyeVec, normalize(fragNormal));
	float dotpro = dot(-normalize(lightDirection), reflectVec);
	return pow(max(0, dotpro), 50.0);
}

float subsurface(vec3 fragNormal) {
	vec3 eyeVec = normalize(fragPosition - eyePosition);
	float backdot = max(0, dot(-normalize(lightDirection), normalize(fragNormal)) * 0.6 + 0.4);
	float eyedot = max(0, dot(normalize(lightDirection), eyeVec));
	float powerDot = pow(eyedot, 4);
	float thickness = texture(leafThickness, TexCoords).r * 0.7;
	return backdot * powerDot * (1 - thickness);
}

vec4 leafShading()
{
	vec3 newNorm = fragNormal;
	if (dot(fragNormal, lightDirection) > 0) newNorm = -fragNormal;
	vec4 diffColor = texture(texture_diffuse1, TexCoords) * lambert(newNorm) * 1.0;
	vec4 specularColor = vec4(1.0, 1.0, 1.0, 1.0) * specular(newNorm) * 0.5;
	vec4 subsurfaceColor = texture(texture_diffuse1, TexCoords) * subsurface(newNorm) * 1.0;
	return diffColor + specularColor + subsurfaceColor;
}

vec4 truncShading()
{
	//lambert
	vec4 diffColor = texture(texture_diffuse1, TexCoords);
	return diffColor * lambert(fragNormal);
}

void main()
{
	if (meshId != 2)
		color = truncShading();
	else
		color = leafShading();
}
