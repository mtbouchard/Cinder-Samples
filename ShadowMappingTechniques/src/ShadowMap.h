#pragma once

#include "cinder/Cinder.h"
#include "cinder/Matrix.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Light.h"

class ShadowMap
{
public:
	ShadowMap(void) : mIsBlurEnabled(true) {}
	virtual ~ShadowMap(void) {}

	virtual void	setup() {}
	virtual void	update(  const ci::CameraPersp &cam ) {
		mLight->update( cam );
		mShadowMatrix = mLight->getShadowTransformationMatrix( cam );
	}

	virtual void	drawMap() {		
		int w = ci::app::getWindowHeight();
		int h = ci::app::getWindowWidth();

		ci::gl::color( ci::Color::white() );
		ci::gl::draw( mFboDepth.getTexture(), ci::Rectf(0, 256, 256, 0) );
	}

	virtual void	drawDepth() {
		int w = ci::app::getWindowHeight();
		int h = ci::app::getWindowWidth();

		ci::gl::color( ci::Color::white() );
		ci::gl::draw( mFboDepth.getTexture(), ci::Rectf(0, 256, 256, 0) );
	}

	virtual void	drawLight() {
		ci::gl::color( ci::Color::white() );
		ci::gl::drawSphere( mLight->getPosition(), 2.0f, 20 );
		ci::gl::drawVector( mLight->getPosition(), ci::Vec3f::zero(), 0.5f, 0.05f );
	}

	virtual void	bindDepth() {}
	virtual void	unbindDepth() {}

	virtual void	bindShadow() {}
	virtual void	unbindShadow() {}

	bool	isBlurEnabled() const { return mIsBlurEnabled; }
	void	enableBlur( bool enabled = true ) { mIsBlurEnabled = enabled; }

	ci::gl::GlslProg	getShader() const { return mShaderShadow; }

	ci::CameraPersp		getCamera() const { return mLight->getShadowCamera(); }
public:
	static const int SHADOW_MAP_RESOLUTION = 2048;
protected:
	bool				mIsBlurEnabled;

	ci::gl::Fbo			mFboDepth;
	ci::gl::Fbo			mFboBlur;

	ci::gl::GlslProg	mShaderDepth;
	ci::gl::GlslProg	mShaderBlur;
	ci::gl::GlslProg	mShaderShadow;

	ci::gl::Light*		mLight;

	ci::Matrix44f		mBiasMatrix;
	ci::Matrix44f		mShadowMatrix;
};

