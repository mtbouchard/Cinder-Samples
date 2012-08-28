#pragma once

#include "cinder/TriMesh.h"
#include "cinder/Vector.h"
#include "cinder/gl/Vbo.h"

class MeshLibrary
{
public:
	//! Creates a cylinder. The \a ratio parameter controls the ratio between base and top radius.
	static	ci::gl::VboMesh	createCylinder( float ratio=1.0f, int slices=12, int stacks=1, int rings=1 );
	//!
	static	ci::gl::VboMesh	createIcosahedron();
	//!
	static	ci::gl::VboMesh	createSphere( int slices=12, int stacks=12 );
	//! Creates a torus. The \a ratio parameter controls the ratio between inner and outer radius and should be in the range [0..1].
	static	ci::gl::VboMesh	createTorus( float ratio=0.5f, int slices=12, int segments=12 );

	// helpers
	static inline void		sincosf( float radians, float *sin, float *cos );
	//! Subdivides the mesh by splitting each polygon into 4 new polygons.
	static void				subdivide( std::vector<ci::Vec3f> &positions, std::vector<ci::Vec3f> &normals, 
		std::vector<ci::Vec2f> &texcoords, std::vector<uint32_t> &indices, bool normalize=false );
};

