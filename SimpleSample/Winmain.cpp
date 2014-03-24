#include <windows.h>
#include "D3DApp.h"

struct VertexPositionColor
{
	VertexPositionColor() {}
	VertexPositionColor(float _x, float _y, float _z, D3DCOLOR c)
	{
		x = _x; y = _y; z = _z; color = c;
	}
	float x, y, z; //position
	D3DCOLOR color; //color
	static const DWORD FVF;
};

const DWORD VertexPositionColor::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

class TestApp : public D3DApp
{
public:
	//Constructor
	TestApp(HINSTANCE hInstance);
	//Destructor
	~TestApp();

	//Methods

	bool Init() override;
	void Update(float dt) override;
	void Render() override;
};

IDirect3DVertexBuffer9* VB;
IDirect3DIndexBuffer9* IB;

D3DXMATRIX World;

TestApp::TestApp(HINSTANCE hInstance):D3DApp(hInstance)
{

}

TestApp::~TestApp()
{

}

bool TestApp::Init()
{
	if(!D3DApp::Init())
	{
		return false;
	}

	//Shaded Gouraud -> 8 unique verts
	//Textured cube -> 24 unique verts u, v

	VertexPositionColor verts[8] =
	{
		//Front
		VertexPositionColor(-1.0f, 1.0f, -1.0f, d3dColors::Lime),   //0
		VertexPositionColor(1.0f, 1.0f, -1.0f, d3dColors::Blue),    //1
		VertexPositionColor(-1.0f, -1.0f, -1.0f, d3dColors::Red),   //2
		VertexPositionColor(1.0f, -1.0f, -1.0f, d3dColors::Purple), //3

		//Back
		VertexPositionColor(-1.0f, 1.0f, 1.0f, d3dColors::Orange),  //4
		VertexPositionColor(1.0f, 1.0f, 1.0f, d3dColors::Olive),    //5
		VertexPositionColor(-1.0f, -1.0f, 1.0f, d3dColors::Aqua),   //6
		VertexPositionColor(1.0f, -1.0f, 1.0f, d3dColors::AntiqueWhite) //7
	};

	WORD indices[36] = 
	{
		//Front face
		0, 1, 2,
	    2, 1, 3,

		//Back face
		5, 4, 7,
		7, 4, 6,

		//Right face
		1, 5, 3,
		3, 5, 7,

		//Left face
		4, 0, 6,
		6, 0, 2,

		//Top face
		4, 5, 0, 
		0, 5, 1,
		
		//Bottom face
		2, 3, 6,
		6, 3, 7

	};

	m_pDevice3D->CreateVertexBuffer(8 * sizeof(VertexPositionColor), 0, VertexPositionColor::FVF, D3DPOOL_MANAGED,
		&VB, NULL);

	m_pDevice3D->CreateIndexBuffer(36 * sizeof(WORD),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&IB,
		NULL);

	VOID* pVerts;
	VB->Lock(0, sizeof(verts), (void**)&pVerts, 0);
	memcpy(pVerts, verts, sizeof(verts));
	VB->Unlock();

	VOID* pIndices;
	IB->Lock(0, sizeof(indices), (void**)&pIndices, 0);
	memcpy(pIndices, indices, sizeof(indices));
	IB->Unlock();

	D3DXMATRIX view;
	D3DXMATRIX proj;

	//SET VIEW
	D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
	D3DXVECTOR3 target = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	D3DXMatrixLookAtLH(&view, &position, &target, &up);
	m_pDevice3D->SetTransform(D3DTS_VIEW, &view);

	//SET PROJECTION
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4, static_cast<float>(m_uiClientWidth)/m_uiClientHeight,
								1.0f, 1000.0f);

	m_pDevice3D->SetTransform(D3DTS_PROJECTION, &proj);

	m_pDevice3D->SetRenderState(D3DRS_LIGHTING, false);

	m_pDevice3D->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	return true;
}

float rotY;
float rotX;
float rotZ;

D3DXMATRIX Scale, Rx, Ry, Rz, Translation;

void TestApp::Update(float dt)
{
	rotY += 0.001f;
	rotX += 0.001f;
	rotZ += 0.001f;

	D3DXMatrixRotationY(&Ry, rotY);
	D3DXMatrixRotationX(&Rx, rotX);
	D3DXMatrixRotationZ(&Rz, rotZ);
	D3DXMatrixScaling(&Scale, 1.0f, 2.0f, 1.0f);
	D3DXMatrixTranslation(&Translation, 0.0f, 0.0f, rotY);

	World = Scale * Rx * Ry * Rz * Translation;
	
	
}

void TestApp::Render()
{
	m_pDevice3D->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, d3dColors::Black, 1.0f, 0);

	m_pDevice3D->BeginScene();

	m_pDevice3D->SetTransform(D3DTS_WORLD, &World);

	m_pDevice3D->SetStreamSource(0, VB, 0, sizeof(VertexPositionColor));
	m_pDevice3D->SetIndices(IB);
	m_pDevice3D->SetFVF(VertexPositionColor::FVF);
	m_pDevice3D->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 36, 0, 12);

	m_pDevice3D->EndScene();

	m_pDevice3D->Present(0, 0, 0, 0);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	TestApp* tApp = new TestApp(hInstance);

	if(!tApp->Init())
		return 1;

	return tApp->Run();
}