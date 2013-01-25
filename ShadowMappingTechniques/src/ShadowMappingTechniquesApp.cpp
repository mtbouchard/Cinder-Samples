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
	
	void resize( ResizeEvent event );
	
	void mouseMove( MouseEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
private:
	//! renders all objects in our scene
	void renderScene();
private:
	// camera
	CameraPersp		mCamera;
	MayaCamUI		mMayaCam;

	// mesh
	TriMesh			mTriMeshTree;
	TriMesh			mTriMeshWindmill;

	// shadow
	ExponentialShadowMap	mShadowMap;
};

void ShadowMappingTechniquesApp::prepareSettings(Settings *settings)
{
	settings->setTitle("Cinder Sample");
}

void ShadowMappingTechniquesApp::setup()
{
	mCamera.setEyePoint( Vec3f(25, 50, -100) );
	mCamera.setCenterOfInterestPoint( Vec3f(0, 0, 0) );

	//
	try { 
		mTriMeshTree.read( loadAsset("tree.msh") );
		mTriMeshWindmill.read( loadAsset("windmill.msh") );
	}
	catch(...) {}

	//
	mShadowMap.setup();
}

void ShadowMappingTechniquesApp::update()
{
	mShadowMap.update( mCamera );
}

void ShadowMappingTechniquesApp::draw()
{
	gl::clear(); 

	gl::enableDepthRead();
	gl::enableDepthWrite();

	// render shadow map
	mShadowMap.bindDepth();
	renderScene();
	mShadowMap.unbindDepth();

	// render scene using shadow map
	gl::pushMatrices();
	gl::setMatrices( mCamera );

	mShadowMap.bindShadow();
	renderScene();
	mShadowMap.unbindShadow();

	gl::popMatrices();

	// restore render states
	gl::disableDepthWrite();
	gl::disableDepthRead();
}

void ShadowMappingTechniquesApp::resize( ResizeEvent event )
{
	mCamera.setAspectRatio( event.getAspectRatio() );
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
		mShadowMap.enableBlur( ! mShadowMap.isBlurEnabled() );
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
	gl::translate( -15, 0, 0 );
	gl::draw( mTriMeshTree );
	gl::popModelView();

	gl::pushModelView();
	gl::translate( 15, 0, 0 );
	gl::rotate( Vec3f::yAxis() * 160.0f );
	gl::scale( 0.025f, 0.025f, 0.025f );
	gl::draw( mTriMeshWindmill );
	gl::popModelView();

	gl::color( Color(0.5f, 0.5f, 0.5f) );
	gl::drawCube( Vec3f::zero(), Vec3f(250, 1, 250) );
}

CINDER_APP_BASIC( ShadowMappingTechniquesApp, RendererGl )
