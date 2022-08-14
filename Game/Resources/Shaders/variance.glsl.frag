
#version 450

layout(location = 0) out vec4 outDepth;

void main()
{
	outDepth = vec4(gl_FragDepth, gl_FragDepth * gl_FragDepth, 0, 0);
}