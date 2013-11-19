#include "Shader.h"

#include "cinder/Utilities.h"
#include "cinder/app/AppBasic.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

using namespace ci;
using namespace ph;
using namespace ph::render;

Shader::Shader(void) :
	bHasGeometryShader(false),
	mGlslVersion(0)
{
}

Shader::Shader(const std::string& name) :
	bHasGeometryShader(false),
	mGlslVersion(0)
{
	mVertexFile = name + "_vert.glsl";
	mFragmentFile = name + "_frag.glsl";
	mGeometryFile = name + "_geom.glsl";
}

Shader::~Shader(void)
{
}

ShaderRef Shader::create()
{
	return std::make_shared<Shader>();
}

ShaderRef Shader::create(const std::string& name)
{
	return std::make_shared<Shader>(name);
}

bool Shader::load()
{
	// get a reference to our path (and create it in the process)
	const fs::path& path = getPath();
	if(path.empty()) return false;

	// check if all files are present
	if(!fs::exists(path / mFragmentFile)) return false;
	
	bHasGeometryShader = fs::exists(path / mGeometryFile);
	//if(bHasGeometryShader) TODO: check if geometry settings are defined

	// parse source
	std::string vertexSource = parseShader( path / mVertexFile );
	std::string fragmentSource = parseShader( path / mFragmentFile );

	if(bHasGeometryShader) {
		std::string geometrySource = parseShader( path / mGeometryFile );

		mGlslProg = gl::GlslProg( vertexSource.c_str(), fragmentSource.c_str(), geometrySource.c_str(),
									GL_TRIANGLES, GL_TRIANGLE_STRIP, 3); // TODO
	}
	else {
		mGlslProg = gl::GlslProg( vertexSource.c_str(), fragmentSource.c_str() );
	}

	return true;
}

const fs::path& Shader::getPath() const
{
	// return path if already found
	if( !mPath.empty() ) return mPath;

	// find path:
	//  1. in assets folder
	//  2. in assets/shaders folder
	//  3. next to executable
	mPath = app::getAssetPath("");
	if( fs::exists(mPath / mVertexFile) ) return mPath;

	mPath = app::getAssetPath("") / "shaders";
	if( fs::exists(mPath / mVertexFile) ) return mPath;

	mPath = app::getAppPath();
	if( fs::exists(mPath / mVertexFile) ) return mPath;

	// not found (throw exception?)
	mPath.clear();
	return mPath;
}

std::string Shader::parseShader( const fs::path& path, bool optional, int level )
{
	std::stringstream output;

	if( level > 32 )
	{
		throw std::exception("Reached the maximum inclusion depth.");
		return std::string();
	}

	static const boost::regex includeRegexp( "^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*" );

	std::ifstream input( path.c_str() );
	if( !input.is_open() )
	{
		if( optional )
			return std::string();

		if( level == 0 )
			throw std::exception("Failed to open shader file.");
		else
			throw std::exception("Failed to open shader include file.");

		return std::string();
	}

	// go through each line and process includes
	std::string		line;
	boost::smatch	matches;

	while( std::getline( input, line ) )
	{
		if( boost::regex_search( line, matches, includeRegexp ) )
			output << parseShader( path.parent_path() / matches[1].str(), false, level + 1 );
		else
			output << line;

		output << std::endl;
	}

	input.close();

		// make sure #version is the first line of the shader
	if( level == 0)
		return parseVersion( output.str() );
	else
		return output.str();
}

std::string Shader::parseVersion( const std::string& code )
{
    static const boost::regex versionRegexp( "^[ ]*#[ ]*version[ ]+([123456789][0123456789][0123456789]).*$" );

	if(code.empty())
		return std::string();
       
    mGlslVersion = 120;  

    std::stringstream completeCode( code );
    std::ostringstream cleanedCode;

    std::string line;
    boost::smatch matches; 
    while( std::getline( completeCode, line ) )
    {
        if( boost::regex_match( line, matches, versionRegexp ) ) 
        {
			unsigned int versionNum = ci::fromString< unsigned int >( matches[1] );	
            mGlslVersion = std::max( versionNum, mGlslVersion );

            continue;
        }

        cleanedCode << line << std::endl;
    }
	
    std::stringstream vs;
    vs << "#version " << mGlslVersion << std::endl << cleanedCode.str();

    return vs.str();
}
