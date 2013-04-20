#include "VarianceShadowMap.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;

VarianceShadowMap::VarianceShadowMap(void)
{
}

VarianceShadowMap::~VarianceShadowMap(void)
{
}

void VarianceShadowMap::setup()
{
	try { mShaderDepth = gl::GlslProg( getDepthVS().c_str(), getDepthFS().c_str() ); }
	catch( const std::exception &e ) { console() << "Could not compile DEPTH shader: " << e.what() << std::endl; }

	try { mShaderShadow = gl::GlslProg( getShadowVS().c_str(), getShadowFS().c_str() ); }
	catch( const std::exception &e ) { console() << "Could not compile SHADOW shader: " << e.what() << std::endl; }

	try { 
		gl::Fbo::Format fmt;
		fmt.setColorInternalFormat( GL_RG32F );
		fmt.setSamples(4);
		fmt.setCoverageSamples(4);
		fmt.enableDepthBuffer( true );
		fmt.enableMipmapping( false );

		mFboDepth = gl::Fbo( SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, fmt );
	}
	catch( ... ) { }

	// initialize light
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->lookAt( Vec3f( 10, 100, -100 ), Vec3f( 0, 0, 0 ) );
	mLight->setAmbient( Color( 0.1f, 0.0f, 0.0f ) );
	mLight->setDiffuse( Color( 0.5f, 0.5f, 0.5f ) );
	mLight->setSpecular( Color( 0.5f, 0.5f, 0.5f ) );
	mLight->setShadowParams( 75.0f, 10.0f, 200.0f );
	mLight->enable();

	//
	mBiasMatrix = Matrix44f(
		0.5f, 0.0f, 0.0f, 0.5f,
		0.0f, 0.5f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.5f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f, true );
}

void VarianceShadowMap::update( const CameraPersp &cam )
{	
	double t = 0.1 * getElapsedSeconds();
	mLight->lookAt( Vec3f( 100 * sin( t ), 100, 100 * cos( t ) ), Vec3f::zero() );

	mLight->update( cam );
	mShadowMatrix = mLight->getShadowTransformationMatrix( cam );
}

void VarianceShadowMap::draw()
{
	int w = getWindowHeight();
	int h = getWindowWidth();

	gl::color( Color::white() );
	gl::draw( mFboDepth.getTexture(), Rectf(0, 256, 256, 0) );
}

void VarianceShadowMap::drawLight()
{
	gl::color( Color::white() );
	gl::drawSphere( mLight->getPosition(), 2.0f, 20 );
	gl::drawVector( mLight->getPosition(), Vec3f::zero(), 0.5f, 0.05f );
}

void VarianceShadowMap::bindDepth()
{	
	glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | GL_VIEWPORT_BIT );

	//glEnable( GL_CULL_FACE );
	//glCullFace( GL_FRONT );

	mFboDepth.bindFramebuffer();
	
	glViewport( 0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION );
	gl::clear();
	
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::pushMatrices();
	mLight->setShadowRenderMatrices();

	mShaderDepth.bind();
}

void VarianceShadowMap::unbindDepth()
{
	mShaderDepth.unbind();

	gl::popMatrices();

	mFboDepth.unbindFramebuffer();

	glPopAttrib();
}

void VarianceShadowMap::bindShadow()
{
	mFboDepth.bindTexture( 16 );

	mShaderShadow.bind();
	mShaderShadow.uniform( "shadow_matrix", mShadowMatrix );
	mShaderShadow.uniform( "shadow_map", 16 );
}

void VarianceShadowMap::unbindShadow()
{
	mShaderShadow.unbind();

	mFboDepth.unbindTexture();
}

std::string VarianceShadowMap::getDepthVS() const
{
	// vertex shader
	const char *vs = 
		"#version 110\n"
		"\n"
		"varying vec4 V;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = ftransform();\n"
		"	V = gl_Position;\n"
		"}\n";

	return std::string(vs);
}

std::string VarianceShadowMap::getDepthFS() const
{
	// fragment shader
	const char *fs = 
		"#version 110\n"
		"\n"
		"varying vec4 V;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	float depth = V.z / V.w;\n"
		//"	float depth = V.z;\n"
		"	depth = 0.5 * depth + 0.5;\n"
		"\n"
		"	float moment1 = depth;\n"
		"	float moment2 = depth * depth;\n"
		"\n"
		"	float dx = dFdx(depth);\n"
		"	float dy = dFdy(depth);\n"
		"	moment2 += 0.25 * ( dx * dx + dy * dy );\n"
		"\n"
		"	gl_FragColor = vec4( moment1, moment2, 0.0, 0.0 );\n"
		"}\n";

	return std::string(fs);
}

std::string VarianceShadowMap::getShadowVS() const
{
	// vertex shader
	const char *vs = 
		"#version 110\n"
		"\n"
		"uniform mat4 shadow_matrix;\n"
		"\n"
		"varying vec4 V;\n"
		"varying vec3 N;\n"
		"varying vec4 Q;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	V = gl_ModelViewMatrix * gl_Vertex;\n"
		"	Q = shadow_matrix * V;\n"
		"	N = normalize(gl_NormalMatrix * gl_Normal);\n"
		"\n"
		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = ftransform();\n"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
		"}\n";	

	return std::string(vs);
}

std::string VarianceShadowMap::getShadowFS() const
{
	// fragment shader
	const char *fs = 
		"#version 110\n"
		"\n"
		"uniform sampler2D diffuse_map;\n"
		"uniform sampler2D shadow_map;\n"
		"\n"
		"varying vec4 V;\n"
		"varying vec3 N;\n"
		"varying vec4 Q;\n"
		"\n"
		"float chebyshevUpperBound( float distance, vec2 uv )\n"
		"{\n"
		"	vec2 moments = texture2D( shadow_map, uv ).rg;\n"
		"\n"
		"	// one-tailed inequality valid if distance > moments.x\n"
		"	if (distance <= moments.x) return 1.0;\n"
		"\n"
		"	// compute variance\n"
		"	float variance = moments.y - ( moments.x * moments.x );\n"
		"	variance = max( variance, 0.000001 );\n"
		"\n"
		"	// compute probabilistic upper bound\n"
		"	float d = distance - moments.x;\n"
		"	return variance / ( variance + d * d );\n"
		"}\n"
		"\n"
		"void main()\n"
		"{\n"
		"	// calculate shadow factor\n"
		"	vec4 q = (Q / Q.w) * 0.5 + 0.5;\n"
		"\n"
		"	float shadow = chebyshevUpperBound( q.z, q.xy );\n"
		"	shadow = 0.5 * shadow + 0.5;\n"
		"\n"		
		"	vec2 uv = gl_TexCoord[0].st;\n"
		"\n"		
		"	vec3 L = normalize( gl_LightSource[0].position.xyz - V.xyz );\n"   
		"	vec3 E = normalize( -V.xyz );\n" 
		"	vec3 R = normalize( -reflect( L, N ) );\n"
		"\n"		
		"	// ambient term\n" 
		"	vec4 ambient = vec4( 0.05, 0.0, 0.0, 1.0 );\n" 
		"\n"		
		"	// diffuse term\n"
		"	vec4 diffuse = gl_Color; // texture2D( diffuse_map, uv );\n"
		"	diffuse *= max( dot( N, L ), 0.0 );\n"
		"	diffuse = clamp( diffuse, 0.0, 1.0 );\n" 
		"\n"		
		"	// specular term\n"
		"	vec4 specular = vec4( 0.2, 0.15, 0.1, 1.0 );\n" 
		"	specular *= pow( max( dot( R, E ), 0.0 ), 50.0 );\n"
		"	specular = clamp( specular, 0.0, 1.0 );\n"
		"\n"
		"	gl_FragColor = ambient + shadow * diffuse + specular;\n"
		"}\n";

	return std::string(fs);
}
