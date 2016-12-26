#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 fragNormal;
out vec3 fragPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 wind;
uniform float time;
uniform float detailPhase;
uniform float detailScale;

uniform sampler2D leafEdge;
uniform sampler2D leafStiffness;

uniform int meshId;

float frac(float x) {
	return x - trunc(x);
}
vec4 frac(vec4 v) {
	return vec4(frac(v.x), frac(v.y), frac(v.z), frac(v.w));
}

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
	vec3 basePos = (model * vec4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
	float fBendScale = length(wind.xy);
	float fBF = position.z * fBendScale * 0.002;
	// Smooth bending factor and increase its nearby height limit.
	fBF += 1.0;
	fBF *= fBF;
	fBF = fBF * fBF - fBF;
	// Displace position
	vec3 vNewPos = vPos;
	vNewPos.xy += normalize(wind.xy) * fBF;
	// Rescale
	float fLength = length(vPos.xyz - basePos);
	vPos.xyz = normalize(vNewPos.xyz - basePos) * fLength + basePos;
	return vPos;
}

vec3 detailBending(vec3 vPos, vec3 vNormal) {
	// Phases (object, vertex, branch)
	float fVtxPhase = (position.x + position.y + position.z) * 2.0 * 0.01;
	// x is used for edges; y is used for branches
	vec2 vWavesIn = vec2(time, time) + vec2(fVtxPhase + detailPhase, fVtxPhase + detailPhase);
	// 1.975, 0.793, 0.375, 0.193 are good frequencies
	vec4 vWaves = (frac( vWavesIn.xxyy *
						   vec4(1.975, 0.793, 0.375, 0.193) ) *
						   2.0 - 1.0 ) * 0.5;
	vWaves = vec4(SmoothTriangleWave(vWaves.x), SmoothTriangleWave(vWaves.y),
	              SmoothTriangleWave(vWaves.z), SmoothTriangleWave(vWaves.w));
	vec2 vWavesSum = vWaves.xz + vWaves.yw;
	// Edge (xy) and branch bending (z)
	float fEdgeAtten = 1 - texture(leafEdge, TexCoords).r;
	float fBranchAtten = length(position.xy) / 3.0 * 0.03;
	float fzAtten = sqrt(fBranchAtten);
	vPos.xyz += vWavesSum.xxy * vec3(fEdgeAtten * fBranchAtten * 0.2 * vNormal.xy, fzAtten * 0.1) * detailScale;
	return vPos;
}

void main()
{
	vec3 model_pos = (model * vec4(position, 1.0f)).xyz;
	vec3 new_pos = mainBending(model_pos);
	fragNormal = normalize((transpose(inverse(model)) * vec4(normal, 1.0f)).xyz);
	if (meshId == 2) {
		new_pos = detailBending(new_pos, fragNormal);
	}
    gl_Position = projection * view * vec4(new_pos, 1.0f);

	fragPosition = new_pos;
    TexCoords = texCoords;
}
