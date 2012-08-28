#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "MeshLibrary.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MeshLibApp : public AppBasic {
public:
	void setup();	
	void update();
	void draw();

	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );

	void keyDown( KeyEvent event );

	void resize( ResizeEvent event );
protected:
	void render();
private:
	gl::VboMesh		mMesh;

	gl::Texture		mTexture;

	CameraStereo	mCam;
	MayaCamUI		mMayaCam;
};

void MeshLibApp::setup()
{
	mCam.setEyePoint( Vec3f(0.0, 0.0f, 10.0f) );
	mCam.setCenterOfInterestPoint( Vec3f::zero() );
	mCam.setFov( 20.0f );

	mMayaCam.setCurrentCam( mCam );

	mMesh = MeshLibrary::createSphere();

	gl::Texture::Format fmt;
	//fmt.enableMipmapping();
	//fmt.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );

	mTexture = gl::Texture( loadImage( loadAsset("earth.png") ), fmt );
}

void MeshLibApp::update()
{
	mCam.setFocus( math<float>::min(2.0f, mCam.getEyePoint().length() * 0.9f) );
}

void MeshLibApp::draw()
{
	int w = getWindowWidth();
	int h = getWindowHeight();

	gl::clear(); 
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::pushMatrices();
	{
		glPushAttrib( GL_VIEWPORT_BIT );

		mCam.enableStereoLeft();
		gl::setViewport( Area(0, 0, w/2, h) );
		gl::setMatrices( mCam );
		render();

		mCam.enableStereoRight();
		gl::setViewport( Area(w/2, 0, w, h) );
		gl::setMatrices( mCam );
		render();

		glPopAttrib();
	}
	gl::popMatrices();

	gl::disableDepthWrite();
	gl::disableDepthRead();
}

void MeshLibApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void MeshLibApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), false, event.isRightDown() );
	mCam.setEyePoint( mMayaCam.getCamera().getEyePoint() );
	mCam.setCenterOfInterestPoint( mMayaCam.getCamera().getCenterOfInterestPoint() );
}

void MeshLibApp::keyDown( KeyEvent event )
{
	switch( event.getCode() ) {
	case KeyEvent::KEY_ESCAPE:
		quit();
		break;
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_1:
		mMesh = MeshLibrary::createCylinder(1.0f, 60, 10, 5);
		break;
	case KeyEvent::KEY_2:
		mMesh = MeshLibrary::createSphere(60, 30);
		break;
	case KeyEvent::KEY_3:
		mMesh = MeshLibrary::createTorus(0.5f, 60, 60);
		break;
	case KeyEvent::KEY_9:
		mMesh = MeshLibrary::createIcosahedron();
		break;
	}
}

void MeshLibApp::resize( ResizeEvent event )
{
	mCam.setAspectRatio( event.getAspectRatio() );
	mMayaCam.setCurrentCam( mCam );
}

void MeshLibApp::render()
{
	// visual debugging
	gl::drawCoordinateFrame(0.5f);
	gl::enableWireframe();
	gl::color( Color(0.5f, 0.5f, 0.5f) );
	gl::draw( mMesh );
	gl::disableWireframe();

	for(int i=-10;i<=10;++i) {
		gl::drawLine( Vec3f(i, 0, -10), Vec3f(i, 0, 10) ); 
		gl::drawLine( Vec3f(-10, 0, i), Vec3f(10, 0, i) );
	}
	
	mTexture.enableAndBind();
	gl::enable( GL_CULL_FACE );
	gl::color( Color::white() );
	gl::draw( mMesh );
	gl::disable( GL_CULL_FACE );
	mTexture.unbind();	
}

CINDER_APP_BASIC( MeshLibApp, RendererGl )
