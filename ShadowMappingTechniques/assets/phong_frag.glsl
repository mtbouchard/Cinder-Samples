#version 110

uniform sampler2D		diffuse_map;
uniform sampler2DShadow	shadow_map;

uniform vec2			pixel_size;

varying vec4 V;
varying vec3 N;
varying vec4 Q;
	
float lookup( vec2 offset )
{
	// values are multiplied by ShadowCoord.w because shadow2DProj does a W division for us.
	return shadow2DProj( shadow_map, Q + vec4( offset.x * pixel_size.x * Q.w, offset.y * pixel_size.y * Q.w, 0.05, 0.0 ) ).w;
}

void main()
{		
	vec2 uv = gl_TexCoord[0].st;
	
	vec3 L = normalize( gl_LightSource[0].position.xyz - V.xyz );   
	vec3 E = normalize( -V.xyz ); 
	vec3 R = normalize( -reflect( L, N ) );  

	// ambient term 
	vec4 ambient = vec4( 0.05, 0.0, 0.0, 1.0 );    

	// diffuse term
	vec4 diffuse = gl_Color; // texture2D( diffuse_map, uv );
	diffuse *= max( dot( N, L ), 0.0 );
	diffuse = clamp( diffuse, 0.0, 1.0 );
	
	// shadow term (60% color, 40% shadow)
	vec3 coord = 0.5 * (Q.xyz / Q.w + 1.0);
	float d = ( max( abs( 0.5 - coord.x ), abs( 0.5 - coord.y ) ) - 0.45 ) / 0.05;

	float shadow = 1.0; // 0.6
	//if( Q.w > 1.0 ) shadow += 0.4 * max(d, shadow2D( shadow_map, coord ).r );

	// specular term
	vec4 specular = vec4( 0.2, 0.15, 0.1, 1.0 ); 
	specular *= pow( max( dot( R, E ), 0.0 ), 50.0 );
	specular = clamp( specular, 0.0, 1.0 ); 

	// final color
	gl_FragColor = ambient + shadow * (diffuse + specular);
}