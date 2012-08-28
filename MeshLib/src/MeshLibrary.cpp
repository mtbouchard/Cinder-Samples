#include "MeshLibrary.h"
#include "cinder/Quaternion.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace std;

gl::VboMesh MeshLibrary::createCylinder( float ratio, int slices, int stacks, int rings )
{
	const float pi = (float) M_PI;

	vector<Vec3f>		positions;
	vector<Vec3f>		normals;
	vector<Vec2f>		texcoords;
	vector<uint32_t>	indices;

	int32_t		i, j;
	uint32_t	rowA, rowB;

	double t = app::getElapsedSeconds();

	int nstacks = stacks + 1;
	int nslices = slices + 1;
	int nrings = rings + 1;

	// reserve memory to avoid memory reallocation
	positions.reserve(nstacks * nslices);
	normals.reserve(nstacks * nslices);
	texcoords.reserve(nstacks * nslices);
	indices.reserve(stacks * slices * 6);	

	// cache sin and cos
	vector<float>	sinI(nslices), cosI(nslices);

	for(i=0;i<nslices;++i) 
		sincosf( 2 * pi * i / slices - pi, &sinI[i], &cosI[i] );

	// generate positions, normals and texture coordinates

	//	base
	for(j=0;j<nrings;++j)
	{
		float r = 0.5f * (j/(float)rings);

		for(i=0;i<nslices;++i)
		{
			normals.push_back( -Vec3f::yAxis() );
			positions.push_back( Vec3f( sinI[i] * r, -0.5f, cosI[i] * r ) );
			texcoords.push_back( Vec2f(i/(float)slices, 1.0f) );
		}
	}
	
	// sides
	for(j=0;j<nstacks;++j)
	{
		float n = j/(float)stacks;
		float r = 0.5f + n * (0.5f * ratio - 0.5f); 

		for(i=0;i<nslices;++i)
		{
			normals.push_back( Vec3f( sinI[i], 0.0f, cosI[i] ).normalized() );
			positions.push_back( Vec3f( sinI[i] * r, n - 0.5f, cosI[i] * r ) );
			texcoords.push_back( Vec2f(i/(float)slices, 1.0f - n) );
		}
	}

	// top
	for(j=0;j<nrings;++j)
	{
		float r = 0.5f * (1.0f - j/(float)rings) * ratio;

		for(i=0;i<nslices;++i)
		{
			normals.push_back( Vec3f::yAxis() );
			positions.push_back( Vec3f( sinI[i] * r, 0.5f, cosI[i] * r ) );
			texcoords.push_back( Vec2f(i/(float)slices, 0.0f) );
		}
	}

	// generate indices
	for(j=0;j<nstacks+2*nrings-1;++j)
	{
		rowA = j * nslices;
		rowB = rowA + nslices;

		for(i=0;i<slices;++i)
		{
			indices.push_back( rowA + i );
			indices.push_back( rowA + i + 1 );
			indices.push_back( rowB + i );

			indices.push_back( rowA + i + 1 );
			indices.push_back( rowB + i + 1 );
			indices.push_back( rowB + i );
		}
	}

	// create mesh
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticNormals();
	layout.setStaticIndices();
	layout.setStaticTexCoords2d();

	gl::VboMesh mesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
	mesh.bufferPositions( positions );
	mesh.bufferNormals( normals );
	mesh.bufferIndices( indices );
	mesh.bufferTexCoords2d(0, texcoords);

	app::console() << ( app::getElapsedSeconds() - t ) << std::endl;

	return mesh;
}

gl::VboMesh MeshLibrary::createIcosahedron()
{
	const float t = 0.5f + 0.5f * math<float>::sqrt(5.0f);
	const float one = 1.0f / math<float>::sqrt(1.0f+t*t);
	const float tau = t * one;
	const float pi = (float) M_PI;

	vector<Vec3f>		positions;
	vector<Vec3f>		normals;
	vector<Vec2f>		texcoords;
	vector<uint32_t>	indices;

	normals.push_back( Vec3f(one, 0, tau) );	
	normals.push_back( Vec3f(one, 0, -tau) );	
	normals.push_back( Vec3f(-one, 0, -tau) );		
	normals.push_back( Vec3f(-one, 0, tau) );		

	normals.push_back( Vec3f(tau, one, 0) );
	normals.push_back( Vec3f(-tau, one, 0) );	
	normals.push_back( Vec3f(-tau, -one, 0) );
	normals.push_back( Vec3f(tau, -one, 0) );

	normals.push_back( Vec3f(0, tau, one) );
	normals.push_back( Vec3f(0, -tau, one) );	
	normals.push_back( Vec3f(0, -tau, -one) );		
	normals.push_back( Vec3f(0, tau, -one) );		

	for(size_t i=0;i<12;++i) 
		positions.push_back( normals[i] * 0.5f );

	indices.push_back(0); indices.push_back(8); indices.push_back(3);
	indices.push_back(0); indices.push_back(3); indices.push_back(9);
	indices.push_back(1); indices.push_back(2); indices.push_back(11);
	indices.push_back(1); indices.push_back(10); indices.push_back(2);
	
	indices.push_back(4); indices.push_back(0); indices.push_back(7);
	indices.push_back(4); indices.push_back(7); indices.push_back(1);
	indices.push_back(6); indices.push_back(3); indices.push_back(5);
	indices.push_back(6); indices.push_back(5); indices.push_back(2);
	
	indices.push_back(8); indices.push_back(4); indices.push_back(11);
	indices.push_back(8); indices.push_back(11); indices.push_back(5);
	indices.push_back(9); indices.push_back(10); indices.push_back(7);
	indices.push_back(9); indices.push_back(6); indices.push_back(10);
	
	indices.push_back(8); indices.push_back(0); indices.push_back(4);
	indices.push_back(11); indices.push_back(4); indices.push_back(1);
	indices.push_back(0); indices.push_back(9); indices.push_back(7);
	indices.push_back(1); indices.push_back(7); indices.push_back(10);
	
	indices.push_back(3); indices.push_back(8); indices.push_back(5);
	indices.push_back(2); indices.push_back(5); indices.push_back(11);
	indices.push_back(3); indices.push_back(6); indices.push_back(9);
	indices.push_back(2); indices.push_back(10); indices.push_back(6);

	// approximate texture coordinates by converting to spherical coordinates	
	vector<Vec3f>::const_iterator itr;
	for(itr=normals.begin();itr!=normals.end();++itr) {
		float u = 0.5f + 0.5f * math<float>::atan2( itr->x, itr->z ) / pi;
		float v = 0.5f - math<float>::asin( itr->y ) / pi;

		texcoords.push_back( Vec2f(u, v) );
	}

	// create mesh
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticNormals();
	layout.setStaticIndices();
	layout.setStaticTexCoords2d();

	gl::VboMesh mesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
	mesh.bufferPositions( positions );
	mesh.bufferNormals( normals );
	mesh.bufferIndices( indices );
	mesh.bufferTexCoords2d(0, texcoords);

	return mesh;
}

gl::VboMesh MeshLibrary::createSphere( int slices, int stacks )
{
	const float pi = (float) M_PI;

	vector<Vec3f>		positions;
	vector<Vec3f>		normals;
	vector<Vec2f>		texcoords;
	vector<uint32_t>	indices;

	int32_t		i, j;
	uint32_t	rowA, rowB;

	double t = app::getElapsedSeconds();

	int nstacks = stacks + 1;
	int nslices = slices + 1;

	// reserve memory to avoid memory reallocation
	positions.reserve(nstacks * nslices);
	normals.reserve(nstacks * nslices);
	texcoords.reserve(nstacks * nslices);
	indices.reserve(stacks * slices * 6);

	// cache sin and cos
	vector<float>	sinI(nslices), cosI(nslices);
	vector<float>	sinJ(nstacks), cosJ(nstacks);

	for(i=0;i<nslices;++i) 
		sincosf( 2 * pi * i / slices - pi, &sinI[i], &cosI[i] );

	for(j=0;j<nstacks;++j) 
		sincosf( pi * j / stacks, &sinJ[j], &cosJ[j] );

	// generate positions, normals and texture coordinates
	for(j=0;j<nstacks;++j)
	{
		for(i=0;i<nslices;++i)
		{
			normals.push_back( Vec3f( sinI[i] * sinJ[j], cosJ[j], cosI[i] * sinJ[j] ) );
			positions.push_back( normals.back() * 0.5f );
			texcoords.push_back( Vec2f(i/(float)slices, j/(float)stacks) );
		}
	}

	// generate indices
	for(j=0;j<stacks;++j)
	{
		rowA = j * nslices;
		rowB = rowA + nslices;

		for(i=0;i<slices;++i)
		{
			indices.push_back( rowA + i );
			indices.push_back( rowB + i );
			indices.push_back( rowA + i + 1 );

			indices.push_back( rowA + i + 1 );
			indices.push_back( rowB + i );
			indices.push_back( rowB + i + 1 );
		}
	}

	// create mesh
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticNormals();
	layout.setStaticIndices();
	layout.setStaticTexCoords2d();

	gl::VboMesh mesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
	mesh.bufferPositions( positions );
	mesh.bufferNormals( normals );
	mesh.bufferIndices( indices );
	mesh.bufferTexCoords2d(0, texcoords);

	app::console() << ( app::getElapsedSeconds() - t ) << std::endl;

	return mesh;
}

gl::VboMesh MeshLibrary::createTorus( float ratio, int slices, int segments )
{
	const float pi = (float) M_PI;

	vector<Vec3f>		positions;
	vector<Vec3f>		normals;
	vector<Vec2f>		texcoords;
	vector<uint32_t>	indices;

	int32_t		i, j;
	uint32_t	rowA, rowB;

	double t = app::getElapsedSeconds();

	int nslices = slices + 1;
	int nsegments = segments + 1;

	// reserve memory to avoid memory reallocation
	positions.reserve(nsegments * nslices);
	normals.reserve(nsegments * nslices);
	texcoords.reserve(nsegments * nslices);
	indices.reserve(segments * slices * 6);

	// cache sin and cos
	vector<float>	sinI(nslices), cosI(nslices);
	vector<float>	sinJ(nsegments), cosJ(nsegments);

	for(i=0;i<nslices;++i) 
		sincosf( 2 * pi * i / slices - pi, &sinI[i], &cosI[i] );

	for(j=0;j<nsegments;++j) 
		sincosf( 2 * pi * j / segments, &sinJ[j], &cosJ[j] );

	// generate positions, normals and texture coordinates
	float outer = 0.5f / (1.0f + ratio);
	float inner = outer * ratio;

	for(i=0;i<nslices;++i)
	{
		for(j=0;j<nsegments;++j)
		{
			positions.push_back( Vec3f( cosI[i] * (outer + inner * cosJ[j]), sinI[i] * (outer + inner * cosJ[j]), sinJ[j] * inner ) );
			normals.push_back( Vec3f( cosI[i] * cosJ[j], sinI[i] * cosJ[j], sinJ[j] ) );
			texcoords.push_back( Vec2f(i/(float)slices, j/(float)segments) );
		}
	}

	// generate indices
	for(i=0;i<slices;++i)
	{
		rowA = i * nsegments;
		rowB = rowA + nsegments;

		for(j=0;j<segments;++j)
		{
			indices.push_back( rowA + j );
			indices.push_back( rowB + j );
			indices.push_back( rowA + j + 1 );

			indices.push_back( rowA + j + 1 );
			indices.push_back( rowB + j );
			indices.push_back( rowB + j + 1 );
		}
	}

	// create mesh
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticNormals();
	layout.setStaticIndices();
	layout.setStaticTexCoords2d();

	gl::VboMesh mesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
	mesh.bufferPositions( positions );
	mesh.bufferNormals( normals );
	mesh.bufferIndices( indices );
	mesh.bufferTexCoords2d(0, texcoords);

	app::console() << ( app::getElapsedSeconds() - t ) << std::endl;

	return mesh;
}

void MeshLibrary::sincosf( float radians, float *sin, float *cos )
{
#if(defined WIN32)
	// see: http://forums.codeguru.com/showthread.php?328669-MSVC-inline-assembly-query
	// the following asm has been converted to use floats

	_asm
	{
		fld DWORD PTR [radians]
		fsincos
		mov ebx,[cos]                
		fstp DWORD PTR [ebx]          
		mov ebx,[sin]
		fstp DWORD PTR [ebx]
	}
#else
	*sin = sinf(radians);
	*cos = cosf(radians);
#endif
}

void MeshLibrary::subdivide( vector<Vec3f> &positions, vector<Vec3f> &normals, vector<Vec2f> &texcoords, vector<uint32_t> &indices, bool normalize )
{
	bool hasNormals = ( ! normals.empty() );
	bool hasTexCoords = ( ! texcoords.empty() );

	// create a copy of the index buffer
	vector<uint32_t>	temp( indices );

	// clear current index buffer and allocate enough memory
	indices.clear();
	indices.reserve( temp.size() * 4 );

	// 
	uint32_t i1, i2, i3;
	uint32_t n1, n2, n3;

	vector<uint32_t>::const_iterator itr;
	for(itr=temp.begin();itr!=temp.end();)
	{
		// retrieve face
		i1 = *itr;	++itr;
		i2 = *itr;	++itr;
		i3 = *itr;	++itr;

		// create vertices
		if(normalize) {
			n1 = positions.size(); positions.push_back( positions[i1].lerp(0.5f, positions[i2]).normalized() * 0.5f );
			n2 = positions.size(); positions.push_back( positions[i2].lerp(0.5f, positions[i3]).normalized() * 0.5f );
			n3 = positions.size(); positions.push_back( positions[i3].lerp(0.5f, positions[i1]).normalized() * 0.5f );
		} 
		else {
			n1 = positions.size(); positions.push_back( positions[i1].lerp(0.5f, positions[i2]) );
			n2 = positions.size(); positions.push_back( positions[i2].lerp(0.5f, positions[i3]) );
			n3 = positions.size(); positions.push_back( positions[i3].lerp(0.5f, positions[i1]) );
		}

		// create normals and texture coordinates
		if(hasNormals) {
			normals.push_back( normals[i1].lerp(0.5f, normals[i2]) );
			normals.push_back( normals[i2].lerp(0.5f, normals[i3]) );
			normals.push_back( normals[i3].lerp(0.5f, normals[i1]) );
		}

		if(hasTexCoords) {
			texcoords.push_back( texcoords[i1].lerp(0.5f, texcoords[i2]) );
			texcoords.push_back( texcoords[i2].lerp(0.5f, texcoords[i3]) );
			texcoords.push_back( texcoords[i3].lerp(0.5f, texcoords[i1]) );
		}

		// create new polygons
		indices.push_back(i1); indices.push_back(n1); indices.push_back(n3);
		indices.push_back(n1); indices.push_back(i2); indices.push_back(n2);
		indices.push_back(n3); indices.push_back(n2); indices.push_back(i3);
		indices.push_back(n1); indices.push_back(n2); indices.push_back(n3);
	}
}
