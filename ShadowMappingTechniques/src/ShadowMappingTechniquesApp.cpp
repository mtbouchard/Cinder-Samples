/*
 Copyright (c) 2010-2013, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ObjLoader.h"
#include "cinder/TriMesh.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"

#include "ExponentialShadowMap.h"
#include "VarianceShadowMap.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ShadowMappingTechniquesApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	
	void setup();
	void update();
	void draw();
	
	void resize();
	
	void mouseMove( MouseEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
private:
	//! renders all objects in our scene
	void renderScene();
	//
	ShadowMap& getShadowMap() { return mExponentialShadowMap; }
private:
	// camera
	CameraPersp		mCamera;
	MayaCamUI		mMayaCam;

	// mesh
	gl::VboMesh		mMesh;

	// shadow
	ExponentialShadowMap	mExponentialShadowMap;
	VarianceShadowMap		mVarianceShadowMap;
};

void ShadowMappingTechniquesApp::prepareSettings(Settings *settings)
{
	settings->setTitle("Shadow Mapping Techniques");
	settings->setWindowSize( 1024, 768 );
}

void ShadowMappingTechniquesApp::setup()
{
	mCamera.setEyePoint( Vec3f(25, 50, -70) );
	mCamera.setCenterOfInterestPoint( Vec3f(0, 0, 0) );

	// 3D model generously supplied by AngelStudios,
	//  see: http://www.turbosquid.com/3d-models/cityscape-old-houses-obj-free/738904
	try { 
		TriMesh mesh;
		mesh.read( loadAsset("city.msh") );

		mMesh = gl::VboMesh(mesh);
	}
	catch(...) {}

	//
	mExponentialShadowMap.setup();
	mVarianceShadowMap.setup();
}

void ShadowMappingTechniquesApp::update()
{
	mExponentialShadowMap.update( mCamera );
	mVarianceShadowMap.update( mCamera );
}

void ShadowMappingTechniquesApp::draw()
{
	gl::clear(); 

	gl::enableDepthRead();
	gl::enableDepthWrite();

	// render shadow map
	getShadowMap().bindDepth();
	renderScene();
	getShadowMap().unbindDepth();

	// render scene using shadow map
	gl::pushMatrices();
	gl::setMatrices( mCamera );

	getShadowMap().bindShadow();
	renderScene();
	getShadowMap().unbindShadow();

	getShadowMap().drawLight();

	gl::popMatrices();

	// restore render states
	gl::disableDepthWrite();
	gl::disableDepthRead();

	getShadowMap().drawDepth();
}

void ShadowMappingTechniquesApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
}

void ShadowMappingTechniquesApp::mouseMove( MouseEvent event )
{
}

void ShadowMappingTechniquesApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void ShadowMappingTechniquesApp::mouseDrag( MouseEvent event )
{
	mMayaCam.setCurrentCam( mCamera );
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
	mCamera = mMayaCam.getCamera();
}

void ShadowMappingTechniquesApp::mouseUp( MouseEvent event )
{
}

void ShadowMappingTechniquesApp::keyDown( KeyEvent event )
{
	switch( event.getCode() )
	{
	case KeyEvent::KEY_ESCAPE:
		quit();
		break;
	case KeyEvent::KEY_b:		
		getShadowMap().enableBlur( ! getShadowMap().isBlurEnabled() );
		break;
	case KeyEvent::KEY_f:
		setFullScreen( ! isFullScreen() );
		break;
	case KeyEvent::KEY_v:
		gl::enableVerticalSync( ! gl::isVerticalSyncEnabled() );
		break;
	}
}

void ShadowMappingTechniquesApp::keyUp( KeyEvent event )
{
}

void ShadowMappingTechniquesApp::renderScene()
{	
	gl::color( Color::white() );

	gl::pushModelView();
	gl::translate( -15, 0, 10 );
	gl::scale( 0.001f, 0.001f, 0.001f );
	gl::draw( mMesh );
	gl::popModelView();
}

CINDER_APP_BASIC( ShadowMappingTechniquesApp, RendererGl )
