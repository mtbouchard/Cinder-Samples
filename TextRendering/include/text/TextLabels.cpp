/*
 Copyright (c) 2010-2012, Paul Houx - All rights reserved.
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

#include "cinder/Unicode.h"
#include "cinder/gl/VboMesh.h"

#include "text/TextLabels.h"

#include <boost/algorithm/string.hpp>

namespace ph { namespace text {

using namespace ci;
using namespace std;

void TextLabels::clear()
{
	mLabels.clear();
	mInvalid = true;
}

void TextLabels::addLabel( const vec3 &position, const std::u16string &text )
{
	mLabels.insert( pair<vec3, std::u16string>( position, text ) );
	mInvalid = true;
}

void TextLabels::clearMesh()
{
	mVboMesh.reset();
	
	mVertices.clear();
	mIndices.clear();
	mTexcoords.clear();
	mOffsets.clear();

	mInvalid = true;
}

void TextLabels::renderMesh()
{
	// parse all labels
	TextLabelListIter labelItr;
	for(labelItr=mLabels.begin();labelItr!=mLabels.end();++labelItr) {
		// render label
		mOffset = labelItr->first;
		setText( labelItr->second );

		Text::renderMesh();
	}
}

void TextLabels::renderString( const std::u16string &str, vec2 *cursor, float stretch )
{
	std::u16string::const_iterator itr;
	for(itr=str.begin();itr!=str.end();++itr) {
		// retrieve character code
		uint16_t id = (uint16_t) *itr;

		if( mFont->contains(id) ) {
			// get metrics for this character to speed up measurements
			Font::Metrics m = mFont->getMetrics(id);

			// skip whitespace characters
			if( ! isWhitespaceUtf16(id) ) {
				size_t index = mVertices.size();

				Rectf bounds = mFont->getBounds(m, mFontSize);
				mVertices.push_back( vec3(*cursor + bounds.getUpperLeft(),0) );
				mVertices.push_back( vec3(*cursor + bounds.getUpperRight(),0) );
				mVertices.push_back( vec3(*cursor + bounds.getLowerRight(),0) );
				mVertices.push_back( vec3(*cursor + bounds.getLowerLeft(),0) );
			
				bounds = mFont->getTexCoords(m);
				mTexcoords.push_back( bounds.getUpperLeft() );
				mTexcoords.push_back( bounds.getUpperRight() );
				mTexcoords.push_back( bounds.getLowerRight() );
				mTexcoords.push_back( bounds.getLowerLeft() );

				mIndices.push_back(index+0); mIndices.push_back(index+3); mIndices.push_back(index+1);
				mIndices.push_back(index+1); mIndices.push_back(index+3); mIndices.push_back(index+2);
			
				mOffsets.insert(mOffsets.end(), 4, mOffset);
			}

			if( id == 32 )
				cursor->x += stretch * mFont->getAdvance(m, mFontSize);
			else
				cursor->x += mFont->getAdvance(m, mFontSize);
		}
	}

	//
	mBoundsInvalid = true;
}

void TextLabels::createMesh()
{
	//
	if( mVertices.empty() || mIndices.empty() )
		return;

	//
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticIndices();
	layout.setStaticTexCoords2d(0);
	//layout.setStaticColorsRGBA();
	layout.setStaticTexCoords3d(1);

	mVboMesh = gl::VboMesh( mVertices.size(), mIndices.size(), layout, GL_TRIANGLES );
	mVboMesh.bufferPositions( &mVertices.front(), mVertices.size() );
	mVboMesh.bufferIndices( mIndices );
	mVboMesh.bufferTexCoords2d( 0, mTexcoords );
	//mVboMesh.bufferColorsRGBA( colors );
	mVboMesh.bufferTexCoords3d( 1, mOffsets );

	mInvalid = false;
}

std::string TextLabels::getVertexShader() const
{
	// vertex shader
	const char *vs = 
		"#version 120\n"
		"\n"
		"// viewport parameters (x, y, width, height)\n"
		"uniform vec4 viewport;\n"
		"\n"
		"vec3 toNDC(vec4 vertex)\n"
		"{\n"
		"	return vec3( vertex.xyz / vertex.w );\n"
		"}\n"
		"\n"
		"vec2 toScreenSpace(vec4 vertex)\n"
		"{\n"
		"	return vec2( vertex.xy / vertex.w ) * viewport.zw;\n"
		"}\n"
		"\n"
		"void main()\n"
		"{\n"
		"	// pass font texture coordinate to fragment shader\n"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
		"\n"
		"	// set the color\n"
		"	gl_FrontColor = gl_Color;\n"
		"\n"
		"	// convert label position to normalized device coordinates to find the 2D offset\n"
		"	vec3 offset = toNDC( gl_ModelViewProjectionMatrix * gl_MultiTexCoord1 );\n"
		"\n"
		"	// convert vertex from screen space to normalized device coordinates\n"
		"	vec3 vertex = vec3( gl_Vertex.xy * vec2(1.0, -1.0) / viewport.zw * 2.0, 0.0 );\n"
		"\n"
		"	// calculate final vertex position by offsetting it\n"
		"	gl_Position = vec4( vertex + offset, 1.0 );\n"
		"}";

	return std::string(vs);
}

bool TextLabels::bindShader()
{
	if( Text::bindShader() )
	{
		auto viewport = gl::getViewport();
		mShader->uniform( "viewport", vec4( viewport.first.x, viewport.first.y, viewport.second.x, viewport.second.y ) );
		return true;
	}

	return false;
}

} } // namespace ph::text