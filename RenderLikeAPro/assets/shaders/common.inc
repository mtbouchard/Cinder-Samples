// encode normal
vec2 encode (vec3 n)
{
    vec2 enc = normalize(n.xy) * (sqrt(-n.z*0.5+0.5));
    return (enc*0.5 + 0.5);
}

// decode normal
vec3 decode (in vec2 enc)
{
    vec4 nn = vec4(enc, 0, 0) * vec4(2, 2, 0, 0) + vec4(-1, -1, 1, -1);
    float l = dot(nn.xyz, -nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(l);
    return nn.xyz * 2.0 + vec3(0, 0, -1);
}

// depth:
//  as sampled from the depth texture
// params:
//  x: projectionMatrix.at(0, 0)
//  y: projectionMatrix.at(1, 1)
//  z: zFar / (zFar - zNear)
//  w: -(zFar * zNear) / (zFar - zNear)
float linearizeDepth(in float depth, in vec4 params) {
	return params.w / (depth - params.z);
}

// depth:
//  as calculated by linearizeDepth()
// ndc:
//  e.g. gl_TexCoord[0].xy * 2.0 - 1.0
// params:
//  x: projectionMatrix.at(0, 0)
//  y: projectionMatrix.at(1, 1)
//  z: zFar / (zFar - zNear)
//  w: -(zFar * zNear) / (zFar - zNear)
vec3 getViewPositionFromLinearDepth(in float depth, in vec2 ndc, in vec4 params)
{
	return vec3((ndc * depth) / params.xy, depth);
}