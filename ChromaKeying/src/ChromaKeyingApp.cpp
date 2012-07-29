#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/qtime/QuickTime.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ChromaKeyingApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();

	void mouseDown( MouseEvent event );	
private:
	float			mThreshold;
	float			mMix;

	Color			mKeyingColor;
	Color			mColorizeColor;

	gl::Texture		mForeground;
	gl::Texture		mBackground;

	gl::GlslProg	mShader;

	params::InterfaceGl	mParams;

	qtime::MovieSurface	mMovie;
};

void ChromaKeyingApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize(1024, 768);
}

void ChromaKeyingApp::setup()
{
	mThreshold = 0.5f;
	mMix = 0.2f;

	mKeyingColor = Color(0.0f, 1.0f, 0.0f);
	mColorizeColor = Color(0.3f, 0.6f, 0.2f);

	try {
		//mForeground = gl::Texture( loadImage( loadAsset("foreground.jpg") ) );
		mBackground = gl::Texture( loadImage( loadAsset("background.jpg") ) );

		mShader = gl::GlslProg( loadAsset("chromakey_vert.glsl"), loadAsset("chromakey_frag.glsl") );
	}
	catch( const std::exception &e ) {
		console() << e.what() << std::endl;
	}

	mParams = params::InterfaceGl("Controls", Vec2i(300, 400));
	mParams.addParam("Keying Color", &mKeyingColor);
	mParams.addParam("Threshold", &mThreshold, "min=0.0 max=1.0 step=0.01");
	mParams.addSeparator();
	mParams.addParam("Colorize Color", &mColorizeColor);
	mParams.addParam("Mixing", &mMix, "min=0.0 max=1.0 step=0.01");

	//
	mMovie = qtime::MovieSurface( loadAsset("sample.mp4") );
	if(mMovie) {
		mMovie.setLoop(true);
		mMovie.play();
	}
}

void ChromaKeyingApp::update()
{
	//
	if( mMovie && mMovie.checkPlayable() && mMovie.checkNewFrame() ) {
		Surface surface = mMovie.getSurface();
		
		if(surface)
			mForeground = gl::Texture( surface ); 
	}
}

void ChromaKeyingApp::draw()
{
	gl::clear(); 

	gl::enableAlphaBlending();

	gl::enable( GL_TEXTURE_2D );

	if(mBackground)
		gl::draw( mBackground, getWindowBounds() );

	if(mForeground) {
		mForeground.bind(0);

		mShader.bind();
		mShader.uniform("image", 0);
		mShader.uniform("colorizeColor", mColorizeColor);
		mShader.uniform("mixFactor", mMix);
		mShader.uniform("keyColor", mKeyingColor);
		mShader.uniform("thresh", mThreshold );

		gl::drawSolidRect( getWindowBounds() );

		mShader.unbind();

		mForeground.unbind();
	}

	gl::disableAlphaBlending();

	mParams.draw();
}

void ChromaKeyingApp::mouseDown( MouseEvent event )
{
}

CINDER_APP_BASIC( ChromaKeyingApp, RendererGl )
