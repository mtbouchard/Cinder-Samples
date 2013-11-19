#pragma once

#include "cinder/Filesystem.h"
#include "cinder/gl/GlslProg.h"

namespace ph { namespace render {

typedef std::shared_ptr<class Shader> ShaderRef;

class Shader
{
public:
	Shader(void);
	Shader(const std::string& name);
	~Shader(void);

	static ShaderRef		create();
	static ShaderRef		create( const std::string& name );

	const ci::fs::path&		getPath() const;

	bool					load();

	ci::gl::GlslProg&		getGlslProg() { return mGlslProg; }

protected:
	std::string parseShader( const ci::fs::path& path, bool optional = true, int level = 0 );
	std::string parseVersion( const std::string& code );

private:
	std::string		mVertexFile;
	std::string		mFragmentFile;
	std::string		mGeometryFile;

	bool			bHasGeometryShader;
	
	mutable ci::fs::path	mPath;

	unsigned int			mGlslVersion;
	ci::gl::GlslProg		mGlslProg;
};

} } // namespace ph::render