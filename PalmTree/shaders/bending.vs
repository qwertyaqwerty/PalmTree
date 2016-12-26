#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 wind;
uniform float time;
uniform float detailPhase;
uniform float leafPhase;

float SmoothCurve( float x ) {
  return x * x *( 3.0 - 2.0 * x );
}
float TriangleWave( float x ) {
  return abs( frac( x + 0.5 ) * 2.0 - 1.0 );
}
float SmoothTriangleWave( float x ) {
  return SmoothCurve( TriangleWave( x ) );
}

vec3 mainBending(vec3 vPos)
{
	// Bend factor - Wind variation is done on the CPU.
	float fBendScale = sqrt(dot(wind.xy, wind.xy));
	float fBF = vPos.z * fBendScale * 0.008;
	// Smooth bending factor and increase its nearby height limit.
	fBF += 1.0;
	fBF *= fBF;
	fBF = fBF * fBF - fBF;
	// Displace position
	vec3 vNewPos = vPos;
	vNewPos.xy += normalize(wind.xy) * fBF;
	// Rescale
	float fLength = sqrt(dot(position, position));
	vPos.xyz = normalize(vNewPos.xyz)* fLength;
	return vPos;
}

void main()
{
	vec3 new_pos = mainBending(position);
    gl_Position = projection * view * model * vec4(new_pos, 1.0f);
    TexCoords = texCoords;
}
