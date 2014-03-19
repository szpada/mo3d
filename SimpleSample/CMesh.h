#include "DXUT.h"

class CMesh
{
	// Variables
	DWORD			    numMaterials;	// Number of materials
	static const DWORD  MESHFVF;

	DWORD				numFaces;		// Number of faces
	DWORD				numVerts;		// Number of vertices

	// Objects
	D3DMATERIAL9*		meshMaterials;	// Materials for mesh
	LPDIRECT3DTEXTURE9* meshTextures;	// Textures for mesh
	LPD3DXMESH			meshObject;		// D3DX Mesh object

	// Texture
	IDirect3DTexture9* m_pTexture;

public:
	// Get value
	DWORD GetNumFaces() { return numFaces; }
	DWORD GetNumVerts() { return numVerts; }
	DWORD GetNumMaterials() { return numMaterials; }

	// create mesh
	HRESULT CMesh::Create( LPCWSTR wszFileName, IDirect3DDevice9* pd3dDevice );

	// render with set position
	HRESULT CMesh::Render( LPDIRECT3DDEVICE9 d3dDevice,D3DXMATRIX matPos, D3DXVECTOR3 pos );
	// render
	HRESULT CMesh::Render( LPDIRECT3DDEVICE9 d3dDevice, D3DXMATRIX matPos );
	// render
	HRESULT CMesh::Render( LPDIRECT3DDEVICE9 d3dDevice, D3DXVECTOR3 pos );

	// destroy
	HRESULT CMesh::Destroy();

	// konstruktor klasy
	CMesh();
	// destruktor klasy
	~CMesh();	
};