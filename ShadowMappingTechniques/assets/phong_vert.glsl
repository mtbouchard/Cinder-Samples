#version 110

uniform mat4 shadow_matrix;

varying vec4 V;
varying vec3 N;
varying vec4 Q;

void main()
{
	// calculate eye space vertex
	V = gl_ModelViewMatrix * gl_Vertex;

	// calculate light space vertex
	//Q = shadow_matrix * V; 

	// calculate eye space normal
	N = normalize(gl_NormalMatrix * gl_Normal);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
	gl_FrontColor = gl_Color;
}
