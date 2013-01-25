#pragma once

#include "cinder/Cinder.h"
#include "cinder/Matrix.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Light.h"

class VarianceShadowMap
{
public:
	VarianceShadowMap(void);
	~VarianceShadowMap(void);

	void	setup();
	void	update(  const ci::CameraPersp &cam );
	void	draw();

	void	drawLight();

	void	bindDepth();
	void	unbindDepth();

	void	bindShadow();
	void	unbindShadow();

	bool	isBlurEnabled() const { return mIsBlurEnabled; }
	void	enableBlur( bool enabled = true ) { mIsBlurEnabled = enabled; }

	ci::gl::GlslProg	getShader() const { return mShaderShadow; }
private:
	std::string	getDepthVS() const;
	std::string	getDepthFS() const;
	
	std::string	getShadowVS() const;
	std::string	getShadowFS() const;
public:
	static const int SHADOW_MAP_RESOLUTION = 2048;
private:
	bool				mIsBlurEnabled;

	ci::gl::Fbo			mFboDepth;

	ci::gl::GlslProg	mShaderDepth;
	ci::gl::GlslProg	mShaderShadow;

	ci::gl::Light*		mLight;

	ci::Matrix44f		mBiasMatrix;
	ci::Matrix44f		mShadowMatrix;
};

