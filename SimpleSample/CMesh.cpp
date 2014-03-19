#include "DXUT.h"
#include "CMesh.h"
#include "SDKmisc.h"

extern IDirect3DTexture9*       g_pDefaultTex;

//-----------------------------------------------------------------------------
// Name: CMesh()
// Desc: Constructor
//-----------------------------------------------------------------------------
CMesh::CMesh()
{
	numMaterials = 0;
	numFaces = 0;
	numVerts = 0;
	m_pTexture = NULL;
}

//-----------------------------------------------------------------------------
// Name: ~CMesh()
// Desc: Destruktor
//-----------------------------------------------------------------------------
CMesh::~CMesh()
{
	this->Destroy();
}

//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Create mesh
//-----------------------------------------------------------------------------
HRESULT CMesh::Create( LPCWSTR wszFileName, IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr;

	WCHAR str[MAX_PATH];
	WCHAR wszMeshPath[MAX_PATH];
	WCHAR m_wszMeshFile[MAX_PATH];

	ID3DXBuffer* pAdjacency;
	ID3DXBuffer* pMaterials;

	// Save the mesh filename
	wcscpy_s( m_wszMeshFile, MAX_PATH, wszFileName );

	// Find .x file
	V_RETURN( DXUTFindDXSDKMediaFileCch( wszMeshPath, MAX_PATH, m_wszMeshFile ) );

	// Load mesh from x
	V_RETURN( D3DXLoadMeshFromX( wszMeshPath, D3DXMESH_MANAGED, pd3dDevice,
		&pAdjacency, &pMaterials, NULL,
		&numMaterials, &meshObject ) );

	// Get numer of faces
	numFaces =  meshObject->GetNumFaces();
	numVerts =	meshObject->GetNumVertices();

	// Extract the path of the mesh file
	WCHAR* pLastBSlash = wcsrchr( wszMeshPath, L'\\' );
	if( pLastBSlash )
	{
		*( pLastBSlash + 1 ) = L'\0';
	}
	else
	{
		wcscpy_s( wszMeshPath, MAX_PATH, L".\\" );
	}

	// Extract material properties and texture names
	D3DXMATERIAL* materials = ( D3DXMATERIAL* )pMaterials->GetBufferPointer();
	meshMaterials = new D3DMATERIAL9[ numMaterials ];
	meshTextures  = new LPDIRECT3DTEXTURE9[ numMaterials ];

	for( UINT i = 0; i < numMaterials; ++i )
	{
		hr = S_OK;

		// Copy the material
		meshMaterials[i] = materials[i].MatD3D;

		// Set ambient colors
		meshMaterials[i].Ambient = meshMaterials[i].Diffuse;

		meshTextures[i] = NULL;

		if( materials[i].pTextureFilename != NULL && lstrlenA(materials[i].pTextureFilename) > 0 )
		{
			// Try the mesh's directory first.
			wcscpy_s( str, MAX_PATH, wszMeshPath );
			MultiByteToWideChar( CP_ACP, 0, materials[i].pTextureFilename, -1, str + lstrlenW( str ), MAX_PATH );
			
			// If the texture file is not in the same directory as the mesh, search the SDK paths.
			if( INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW( str ) )
			{
				WCHAR wszTexName[MAX_PATH];
				// Search the SDK paths
				MultiByteToWideChar( CP_ACP, 0, materials[i].pTextureFilename, -1, wszTexName, MAX_PATH );
				hr = DXUTFindDXSDKMediaFileCch( str, MAX_PATH, wszTexName );
			}

			//DXUTGetGlobalResourceCache().CreateTextureFromFile( pd3dDevice, str, &meshTextures[i] );

			if(SUCCEEDED(hr)){
				if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, str, &meshTextures[i]) ) )
				meshTextures[i] = g_pDefaultTex; // gdy nie utworzy, bierze domyœln¹ teksture
			}

		} 
		else 
		{
			//meshTextures[i] = g_pDefaultTex;
		}
	}
	
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Tranforms the mesh, then renders it
//-----------------------------------------------------------------------------
HRESULT CMesh::Render( LPDIRECT3DDEVICE9 d3dDevice, D3DXMATRIX matPos )
{
	// Apply tranformation
	d3dDevice->SetTransform( D3DTS_WORLD, &matPos );

	// Now render
	for( DWORD i = 0; i < numMaterials; i++ )
	{
		// Set the material and texture for this subset
		d3dDevice->SetMaterial( &meshMaterials[i] );
		d3dDevice->SetTexture( 0, meshTextures[i] );

		// Draw the subset
		meshObject->DrawSubset( i );
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Tranforms the mesh, then renders it
//-----------------------------------------------------------------------------
HRESULT CMesh::Render( LPDIRECT3DDEVICE9 d3dDevice,D3DXMATRIX matPos, D3DXVECTOR3 pos )
{
	// Apply tranformation
	D3DXMatrixTranslation( &matPos, pos.x, pos.y, pos.z );
	d3dDevice->SetTransform( D3DTS_WORLD, &matPos );

	// Now render
	for( DWORD i = 0; i < numMaterials; i++ )
	{
		// Set the material and texture for this subset
		d3dDevice->SetMaterial( &meshMaterials[i] );
		d3dDevice->SetTexture( 0, meshTextures[i] );

		// Draw the subset
		meshObject->DrawSubset( i );
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Tranforms the mesh, then renders it
//-----------------------------------------------------------------------------
HRESULT CMesh::Render( LPDIRECT3DDEVICE9 d3dDevice, D3DXVECTOR3 pos )
{
	// Translate first
	D3DXMATRIX matTranslate;
	D3DXMatrixTranslation( &matTranslate, pos.x, pos.y, pos.z );
	return Render( d3dDevice, matTranslate );
}

//-----------------------------------------------------------------------------
// Name: Destroy()
// Desc: Cleanup all the objects created
//-----------------------------------------------------------------------------
HRESULT CMesh::Destroy()
{
	
	if( meshMaterials != NULL )
		delete[] meshMaterials;

	if( meshTextures )
	{
		for( DWORD i = 0; i < numMaterials; i++ )
			SAFE_RELEASE( meshTextures[i] );

		delete[] meshTextures;
	}

	SAFE_RELEASE(meshObject);

	//SAFE_RELEASE(g_pDefaultTex);

	return S_OK;
}