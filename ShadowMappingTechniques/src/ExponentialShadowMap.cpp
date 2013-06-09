#include "ExponentialShadowMap.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;

void ExponentialShadowMap::setup()
{
	try { mShaderDepth = gl::GlslProg( getDepthVS().c_str(), getDepthFS().c_str() ); }
	catch( const std::exception &e ) { console() << "Could not compile DEPTH shader: " << e.what() << std::endl; }
	
	try { mShaderBlur = gl::GlslProg( getBlurVS().c_str(), getBlurFS().c_str() ); }
	catch( const std::exception &e ) { console() << "Could not compile BLUR shader: " << e.what() << std::endl; }

	try { mShaderShadow = gl::GlslProg( getShadowVS().c_str(), getShadowFS().c_str() ); }
	catch( const std::exception &e ) { console() << "Could not compile SHADOW shader: " << e.what() << std::endl; }

	try { 
		gl::Fbo::Format fmt;
		fmt.setColorInternalFormat( GL_R32F );
		fmt.setMinFilter( GL_LINEAR );
		fmt.setMagFilter( GL_LINEAR );
		fmt.setSamples(4);
		fmt.setCoverageSamples(4);
		fmt.enableDepthBuffer( true );
		fmt.enableMipmapping( false );

		mFboDepth = gl::Fbo( SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, fmt );
		mFboBlur = gl::Fbo( SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, fmt );
	}
	catch( ... ) { }

	// initialize light
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->lookAt( Vec3f( 10, 100, -100 ), Vec3f( 0, 0, 0 ) );
	mLight->setAmbient( Color( 0.1f, 0.0f, 0.0f ) );
	mLight->setDiffuse( Color( 0.5f, 0.5f, 0.5f ) );
	mLight->setSpecular( Color( 0.5f, 0.5f, 0.5f ) );
	mLight->setShadowParams( 60.0f, 50.0f, 250.0f );
	mLight->enable();

	//
	mBiasMatrix = Matrix44f(
		0.5f, 0.0f, 0.0f, 0.5f,
		0.0f, 0.5f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.5f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f, true );
}

void ExponentialShadowMap::update( const CameraPersp &cam )
{	
	float t = 0.1f * float( getElapsedSeconds() );
	mLight->lookAt( Vec3f( 60 * sinf( t ), 60, 60 * cosf( t ) ), Vec3f::zero() );

	mLight->update( cam );
	mShadowMatrix = mLight->getShadowTransformationMatrix( cam );
}

void ExponentialShadowMap::bindDepth()
{	
	glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | GL_VIEWPORT_BIT );

	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	mFboDepth.bindFramebuffer();
	
	glViewport( 0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION );
	gl::clear();
	
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::pushMatrices();
	mLight->setShadowRenderMatrices();

	mShaderDepth.bind();
}

void ExponentialShadowMap::unbindDepth()
{
	mShaderDepth.unbind();

	mFboDepth.unbindFramebuffer();

	// blur pass
	if( mIsBlurEnabled ) 
	{
		gl::setMatricesWindow( mFboDepth.getSize() );

		mFboBlur.bindFramebuffer();
		{
			mFboDepth.bindTexture();
			{
				mShaderBlur.bind();
				mShaderBlur.uniform( "tex0", 0 );
				mShaderBlur.uniform( "offset", Vec2f( 1.0f / SHADOW_MAP_RESOLUTION, 0.0f ) );

				gl::clear();
				gl::drawSolidRect( mFboBlur.getBounds() );
			}
			mFboDepth.unbindTexture();
		}
		mFboBlur.unbindFramebuffer();

		mFboDepth.bindFramebuffer();
		{
			mFboBlur.bindTexture();
			{
				mShaderBlur.uniform( "offset", Vec2f( 0.0f, 1.0f / SHADOW_MAP_RESOLUTION ) );

				gl::clear();
				gl::drawSolidRect( mFboDepth.getBounds() );

				mShaderBlur.unbind();
			}
			mFboBlur.unbindTexture();
		}
		mFboDepth.unbindFramebuffer();
	}

	gl::popMatrices();

	//
	glDisable( GL_CULL_FACE );
	glPopAttrib();
}

void ExponentialShadowMap::bindShadow()
{
	mFboDepth.bindTexture( 16 );

	mShaderShadow.bind();
	mShaderShadow.uniform( "shadow_matrix", mShadowMatrix );
	mShaderShadow.uniform( "light_matrix", mLight->getShadowCamera().getModelViewMatrix() * mLight->getShadowCamera().getProjectionMatrix() );
	mShaderShadow.uniform( "shadow_map", 16 );
}

void ExponentialShadowMap::unbindShadow()
{
	mShaderShadow.unbind();

	mFboDepth.unbindTexture();
}

std::string ExponentialShadowMap::getDepthVS() const
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

std::string ExponentialShadowMap::getDepthFS() const
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
		"	gl_FragColor = vec4( depth, 0.0, 0.0, 0.0 );\n"
		"}\n";

	return std::string(fs);
}

std::string ExponentialShadowMap::getBlurVS() const
{
	// vertex shader
	const char *vs = 
		"#version 110\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = ftransform();\n"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
		"}\n";

	return std::string(vs);
}

std::string ExponentialShadowMap::getBlurFS() const
{
	// fragment shader
	const char *fs = 
		"#version 110\n"
		"\n"
		"uniform sampler2D	tex0;\n"
		"uniform vec2		offset;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	vec3 sum = vec3( 0.0, 0.0, 0.0 );\n"

		"	sum += texture2D( tex0, gl_TexCoord[0].st -  3.0 * offset ).rgb * 0.006;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  2.0 * offset ).rgb * 0.061;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -        offset ).rgb * 0.242;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st                 ).rgb * 0.383;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +        offset ).rgb * 0.242;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  2.0 * offset ).rgb * 0.061;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  3.0 * offset ).rgb * 0.006;\n"
		
		/*
		"	sum += texture2D( tex0, gl_TexCoord[0].st - 10.0 * offset ).rgb * 0.009167927656011385;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  9.0 * offset ).rgb * 0.014053461291849008;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  8.0 * offset ).rgb * 0.020595286319257878;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  7.0 * offset ).rgb * 0.028855245532226279;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  6.0 * offset ).rgb * 0.038650411513543079;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  5.0 * offset ).rgb * 0.049494378859311142;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  4.0 * offset ).rgb * 0.060594058578763078;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  3.0 * offset ).rgb * 0.070921288047096992;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -  2.0 * offset ).rgb * 0.079358891804948081;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st -        offset ).rgb * 0.084895951965930902;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st                 ).rgb * 0.086826196862124602;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +        offset ).rgb * 0.084895951965930902;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  2.0 * offset ).rgb * 0.079358891804948081;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  3.0 * offset ).rgb * 0.070921288047096992;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  4.0 * offset ).rgb * 0.060594058578763078;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  5.0 * offset ).rgb * 0.049494378859311142;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  6.0 * offset ).rgb * 0.038650411513543079;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  7.0 * offset ).rgb * 0.028855245532226279;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  8.0 * offset ).rgb * 0.020595286319257878;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st +  9.0 * offset ).rgb * 0.014053461291849008;\n"
		"	sum += texture2D( tex0, gl_TexCoord[0].st + 10.0 * offset ).rgb * 0.009167927656011385;\n"
		*/

		"\n"
		"	gl_FragColor.rgb = sum;\n"
		"	gl_FragColor.a = 1.0;\n"
		"}";

	return std::string(fs);
}

std::string ExponentialShadowMap::getShadowVS() const
{
	// vertex shader
	const char *vs = 
		"#version 110\n"
		"\n"
		"uniform mat4 shadow_matrix;\n"
		"uniform mat4 light_matrix;\n"
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

std::string ExponentialShadowMap::getShadowFS() const
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
		"void main()\n"
		"{\n"		
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
		"	// calculate shadow factor\n"
		"	vec4 q = (Q / Q.w) * 0.5 + 0.5;\n"
		"\n"
		"	// constant empirically found to be 80 for 32-bit floating points\n"
		"	const float k = 80.0;\n"
		"\n"
		"	float occluder = texture2D( shadow_map, q.xy ).r;\n"
		"	float receiver = Q.z / Q.w;\n"
		"	float shadow = exp(k * occluder) * exp(-k * receiver);\n"
		"	shadow = 0.5 + 0.5 * smoothstep(0.4, 1.0, shadow);\n"
		"	shadow = clamp( shadow, 0.0, 1.0 );\n"
		"\n"
		"	gl_FragColor = ambient + shadow * (diffuse + specular);\n"
		"}\n";

	return std::string(fs);
}
