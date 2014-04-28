//--------------------------------------------------------------------------------------
// File: Airport.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"
#include "CMesh.h"
#include "Airplane.h"

#include <iostream>

//#define DEBUG_VS   // Uncomment this line to debug D3D9 vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug D3D9 pixel shaders 

static enum CAMERA_MODE
{
	CAMERA_STANDARD,	
	CAMERA_ROTATE_MODE1,
	CAMERA_AIRPLANE,
	CAMERA_AIRPLANE2,
	CAMERA_ALTERNATIVE,
	CAMERA_MAX,
};

static enum AIRPLANE_MODE
{
	AIRPLANE_RING,
	AIRPLANE_APPROACH,
	AIRPLANE_MAX,
};

struct CAM_MODE
{
	CAMERA_MODE mode;
	WCHAR wszName[32];
}
g_wszCameraMode[] = 
{
	{ CAMERA_STANDARD,L"Static mode2"},
	{ CAMERA_ROTATE_MODE1,L"Rotate"},
	{ CAMERA_AIRPLANE,L"Airplane"},
	{ CAMERA_AIRPLANE,L"Airplane2"},
	{ CAMERA_ALTERNATIVE,L"Static mode1"},
	{ CAMERA_MAX,L"MAX"},
};

struct MESHLISTDATA
{
	WCHAR   wszName[MAX_PATH];
	WCHAR   wszFile[MAX_PATH];
	DWORD dwNumMat;  // Number of materials.  To be filled in when loading this mesh.
} 
// umieszczajac obiekt w tej sekcji, jest on sttyczny(?), i umieszczony przed terminalem, na poziomie ziemi. 
//Sprawdzone na przyk³adzie modelu B735; dla torusa, nie widaæ, brak struktury.
g_BuildingMeshListData[] =
{
	//{ L"Hangar", L"Media\\Ground\\torus.x", 0 },
	{ L"Hangar", L"Media\\Building\\Hangar\\model.x", 0 },
	{ L"Tower", L"Media\\Building\\Tower\\model.x", 0 },
	//{ L"Terminal", L"Media\\Building\\Terminal\\model.x", 0 },  
	{ L"Cokolwiek", L"Media\\Airplane\\B737\\model1.x", 100000 },
};

MESHLISTDATA g_AirplaneMeshListData[] =
{
	//{ L"AN26 DHL", L"Media\\Airplane\\AN26_DHL\\model.x", 0 },
	//{ L"AN26 EXIN", L"Media\\Airplane\\AN26_EXIN\\model.x", 0 },
	{ L"AN26 DHL", L"Media\\Ground\\torus.x", 0 },
	{ L"AN26 EXIN", L"Media\\Ground\\torus.x", 0 },
	
};

MESHLISTDATA g_VehicleMeshListData[] =
{
	{ L"Car 001 white", L"Media\\Vehicle\\Car001\\model_white.x", 0 },
	{ L"Car 001 red", L"Media\\Vehicle\\Car001\\model_red.x", 0 },
	{ L"Car 001 black", L"Media\\Vehicle\\Car001\\model_black.x", 0 },
	{ L"Car 001 gray", L"Media\\Vehicle\\Car001\\model_gray.x", 0 },
};

MESHLISTDATA g_MovingMeshListData[] =
{
	{ L"B735", L"Media\\Airplane\\B737\\model.x", 0 },
	{ L"B735_2", L"Media\\Airplane\\B737\\model2.x", 0 },
	//{ L"B735", L"Media\\Vegetation\\model1.x", 1000000},
	//{ L"B735_2", L"Media\\Vegetation\\model2.x", 1000000 },
};

//----------------------------------------------------------------------------
/*************** PORUSZAJACE SIE POJAZDY - POCZATEK ***********************/
//----------------------------------------------------------------------------
MESHLISTDATA g_MovingBagCartMeshListData[] =
{
	{ L"Bag Cart", L"Media\\Vehicle\\BagCart\\bag_cart_changed.x", 0 },	
};

MESHLISTDATA g_MovingFireTruckMeshListData[] =
{
	{ L"Fire Truck", L"Media\\Vehicle\\FireTruck\\fire_truck.x", 0 },	
};
//----------------------------------------------------------------------------
/*************** PORUSZAJACE SIE POJAZDY - KONIEC ***********************/
//----------------------------------------------------------------------------


MESHLISTDATA g_VegetationMeshListData[] =
{
	{ L"Trees", L"Media\\Vegetation\\model1.x", 0 },
	{ L"Trees", L"Media\\Vegetation\\model2.x", 0 }
};

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CModelViewerCamera          g_Camera;               // A model viewing camera

CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper*            g_pTxtHelper = NULL;
CDXUTTextHelper*            g_pTxtHelperAbout = NULL;
CDXUTDialog                 g_HUD;                  // dialog for standard controls
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls

// Direct3D 9 resources
ID3DXFont*                  g_pFont9 = NULL;
ID3DXFont*                  g_pFontAbout9 = NULL;

ID3DXSprite*                g_pSprite9 = NULL;
ID3DXEffect*                g_pEffect9 = NULL;
D3DXHANDLE                  g_hmWorldViewProjection;
D3DXHANDLE                  g_hmWorld;
D3DXHANDLE                  g_hfTime;

//SWIATLO
D3DLIGHT9					light;
D3DLIGHT9					additional_light;
float						light_r = 1.0f;
float						light_g = 1.0f;
float						light_b = 1.0f;
float						light_a = 1.0f;
bool						light_directional = true;


int							light_type = 2;
float						light_additional_x = 100;
float						light_additional_y = 100;
float						light_additional_z = 100;
////
// Objekty na scenie

CMesh*						c_mesh_skybox;
CMesh*						c_mesh_terrain;
CMesh*						c_mesh_ground;

CGrowableArray <CMesh*>		g_BuildingMeshes;	// budynki
CGrowableArray <CMesh*>		g_AirplaneMeshes;	// samoloty
CGrowableArray <CMesh*>		g_MovingMeshes;		// lataj¹cy samolot
CGrowableArray <CMesh*>		g_VehicleMeshes;	// pojazdy
CGrowableArray <CMesh*>		g_VegetationMeshes; // pojazdy

//----------------------------------------------------------------------------
/*************** PORUSZAJACE SIE POJAZDY - POCZATEK ***********************/
//----------------------------------------------------------------------------
CGrowableArray <CMesh*>		g_MovingBagCartMeshes; // poruszajacy sie wozek bagazowy
CGrowableArray <CMesh*>		g_MovingFireTruckMeshes; // poruszajacy sie woz strazacki
//----------------------------------------------------------------------------
/*************** PORUSZAJACE SIE POJAZDY - KONIEC ***********************/
//----------------------------------------------------------------------------

bool						g_bShowGround = false;				// Czy renderowaæ pas startowy (c_mesh_ground)
bool						g_bShowVehicle = true;				// Czy renderowaæ pojazdy (g_VehicleMeshes)
bool						g_bShowInfo = false;				// Czy wyœwietlaæ informacje na ekranie
bool						g_bShowButton = false;				// Czy wyœwietlaæ przyciski
bool						g_bShowVegetation = true;			// Czy wyœwietlaæ drzewa
short int					g_iCameraMode = CAMERA_AIRPLANE;	// Zmiana trybu kamery
short int					g_iAirplaneMode = AIRPLANE_RING;	// Zmiana czynnoœci samolotu
float						g_fFogDensity = 0.0f;				// Intensywnoœæ mg³y
float						g_fFogStart = 0.0f;					// Start mg³y
float						g_fFogEnd = 10.0f;					// Koniec mg³y
short int					g_iFogMode = 1;						// Tryb mg³y

//D3DXMATRIXA16             g_mCenterWorld;						// Center WORLD	
//float						g_fPosX,g_fPosY,g_fPosZ	= 0.0f;		// Pozycja

float						g_fWidth, g_fHeight = 0.0f;			// Screen size

IDirect3DTexture9*			g_pDefaultTex;						// default texture
////


//----------------------------------------------------------------------------
/*************** PORUSZAJACE SIE POJAZDY - POCZATEK ***********************/
//----------------------------------------------------------------------------
// Zmienne dla poruszajacych sie pojazdow
float						y_rotation = D3DX_PI/2.0f; // rotacja o 90 stopni
float						bc_x_coordinate = -30.0f;	// wspolrzedna startowa x dla wozka bagazowego
float						bc_z_coordinate = -190.0f;	// wspolrzedna startowa z dla wozka bagazowego
float						y_coordinate = 0.6f;	// wspolna wspolrzedna y dla obu pojazdow
float						ft_x_coordinate = 0.0f;	// wspolrzedna startowa x dla wozu strazackiego
float						ft_z_coordinate = -20.0f;	// wspolrzedna startowa z dla wozu strazackiego
int							bc_road_part = 1;	//droga wozka bagazowego {1, 2, 3, 4}
int							ft_road_part = 1;	//droga wozu strazackiego {1, 2, 3, 4}
float						bc_counter = 0.0f;		// licznik dla wozka bagazowego
float						ft_counter = 0.0f;		// licznik dla wozu strazackiego
int							bc_stop_counter = 0;	// licznik postoju wozka bagazowego
int							ft_stop_counter = 0;	// licznik postoju wozu strazackiego

// Stale dla rotacji pojazdow wzgledem osi Y
#define Y_ROTATION_90			D3DX_PI/2.0f
#define Y_ROTATION_180			D3DX_PI
#define Y_ROTATION_270			D3DX_PI*3.0f/2.0f
#define Y_ROTATION_0			D3DX_PI*2.0f
//----------------------------------------------------------------------------
/*************** PORUSZAJACE SIE POJAZDY - KONIEC ***********************/
//----------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_CHANGEFOGMODE		4
#define IDC_FOGDENSITY			5
#define IDC_FOGSTART			6
#define IDC_FOGEND				7
#define IDC_ACTUALFOGMODE_TEXT  8
#define IDC_FOGDENSITY_TEXT		9
#define IDC_FOGSTART_TEXT		10
#define IDC_FOGEND_TEXT			11
#define IDC_LIGHT_R				12
#define IDC_LIGHT_G				13
#define IDC_LIGHT_B				14
#define IDC_LIGHT_TYPE			15
#define IDC_LIGHT_ADDITIONAL_X	16
#define IDC_LIGHT_ADDITIONAL_Y	17
#define IDC_LIGHT_ADDITIONAL_Z	18
#define IDC_LIGHT_TEXT			19
#define IDC_LIGHT_SELECTION		20
#define IDC_LIGHT_ADDITIONAL_LABEL_X		21
#define IDC_LIGHT_ADDITIONAL_LABEL_Y		22
#define IDC_LIGHT_ADDITIONAL_LABEL_Z		23


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
	bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext );
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext );
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D9LostDevice( void* pUserContext );
void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

void InitApp();
void RenderText();
void RenderAboutText();
void SetupVertexFog(IDirect3DDevice9* pd3dDevice, DWORD Color, BOOL UseRange);


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// DXUT will create and use the best device (either D3D9 or D3D10) 
	// that is available on the system depending on which D3D callbacks are set below

	// Set DXUT callbacks
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

	DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
	DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
	DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
	DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
	DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
	DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );

	InitApp();
	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true );
	DXUTCreateWindow( L"EPKT version 1.07 : www.zmitac.aei.polsl.pl : ISL 2012/2013" );
	DXUTCreateDevice( true, 800, 600 );
	DXUTMainLoop(); // Enter into the DXUT render loop

	return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	g_SettingsDlg.Init( &g_DialogResourceManager );
	g_HUD.Init( &g_DialogResourceManager );
	g_SampleUI.Init( &g_DialogResourceManager );

	g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
	g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
	g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22, VK_F3 );
	g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );
	g_HUD.AddButton( IDC_CHANGEFOGMODE, L"Change Fog Mode", 35, iY += 24, 125, 22 );
	g_HUD.AddStatic( IDC_ACTUALFOGMODE_TEXT, L"Actual Fog Mode: EXP1", 35, iY += 24, 125, 22);
	g_HUD.AddStatic( IDC_FOGDENSITY_TEXT, L"Fog Density:", 35, iY += 24, 125, 22);
	g_HUD.AddSlider( IDC_FOGDENSITY, 35, iY += 24, 125, 22, 0, 100, 0);
	g_HUD.AddStatic( IDC_FOGSTART_TEXT, L"Fog Start:", 35, iY += 24, 125, 22);
	g_HUD.AddSlider( IDC_FOGSTART, 35, iY += 24, 125, 22, 0, 100, 0);
	g_HUD.AddSlider( IDC_FOGEND, 35, iY += 24, 125, 22, 0, 100, 100);
	g_HUD.AddStatic( IDC_FOGEND_TEXT, L"Fog End:", 35, iY += 24, 125, 22);

	//GUI DO STEROWANIA SWIATLEM
	g_HUD.AddButton( IDC_LIGHT_SELECTION, L"Choose light", -233, 10, 125, 22 );
	g_HUD.AddStatic( IDC_LIGHT_TEXT, L"Light settings", -193, 32, 132, 22);
	g_HUD.AddButton( IDC_LIGHT_TYPE, L"change type", -193, 56, 132, 22);
	g_HUD.AddButton( IDC_LIGHT_R, L"RED", -95, 78, 66, 22);
	g_HUD.AddButton( IDC_LIGHT_G, L"GREEN", -160, 78, 66, 22);
	g_HUD.AddButton( IDC_LIGHT_B, L"BLUE", -225, 78, 66, 22);
	//dodatkowe swiatlo
	g_HUD.AddStatic( IDC_LIGHT_ADDITIONAL_LABEL_X, L"X", -370, 32, 22, 22);
	g_HUD.AddStatic( IDC_LIGHT_ADDITIONAL_LABEL_Y, L"Y", -370, 56, 22, 22);
	g_HUD.AddStatic( IDC_LIGHT_ADDITIONAL_LABEL_Z, L"Z", -370, 78, 22, 22);

	g_HUD.AddSlider( IDC_LIGHT_ADDITIONAL_X, -349, 32, 125, 22, 0, 100, 0);
	g_HUD.AddSlider( IDC_LIGHT_ADDITIONAL_Y, -349, 56, 125, 22, 0, 100, 0);
	g_HUD.AddSlider( IDC_LIGHT_ADDITIONAL_Z, -349, 78, 125, 22, 0, 100, 0);
	//*************************************************************************
	g_HUD.GetSlider(IDC_FOGDENSITY)->SetVisible(true);
	g_HUD.GetStatic(IDC_FOGDENSITY_TEXT)->SetVisible(true);
	g_HUD.GetSlider(IDC_FOGSTART)->SetVisible(false);
	g_HUD.GetStatic(IDC_FOGSTART_TEXT)->SetVisible(false);
	g_HUD.GetSlider(IDC_FOGEND)->SetVisible(false);
	g_HUD.GetStatic(IDC_FOGEND_TEXT)->SetVisible(false);

	g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;

	////
	c_mesh_skybox = new CMesh();	// create skybox mesh
	c_mesh_terrain = new CMesh();	// create terrain mesh
	c_mesh_ground = new CMesh();	// create ground
	////
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
	WCHAR wsz[256];

	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos( 5, 60 );
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.8f ) );
	g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( true) );
	g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

	swprintf_s( wsz, 256, L"Camera mode: %s", g_wszCameraMode[g_iCameraMode].wszName );
	//g_pTxtHelper->DrawTextLine(g_wszCameraMode[g_iCameraMode].wszName);
	g_pTxtHelper->DrawTextLine(wsz);

	// convert to WCHAR
	//swprintf_s( wsz, 256, L"Number of verticles: %u", c_mesh_terrain->GetNumVerts() );
	//g_pTxtHelper->DrawTextLine(wsz);
	g_pTxtHelper->End();



}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderAboutText()
{
	// About text
	g_pTxtHelperAbout->Begin();
	g_pTxtHelperAbout->SetInsertionPos( 5, 0 );
	g_pTxtHelperAbout->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.9f ) );
	g_pTxtHelperAbout->DrawTextLine( L"ISL 2012/2013" );
	g_pTxtHelperAbout->DrawTextLine( L"www.zmitac.aei.polsl.pl" );	
	g_pTxtHelperAbout->End();
}


//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	// Skip backbuffer formats that don't support alpha blending
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	// No fallback defined by this app, so reject any device that 
	// doesn't support at least ps2.0
	if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
		return false;

	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{

	// DirectX 9
	if( pDeviceSettings->ver == DXUT_D3D9_DEVICE )
	{
		// Change AutoDepthStencilFormat
		pDeviceSettings->d3d9.pp.AutoDepthStencilFormat = D3DFMT_D16;

		IDirect3D9* pD3D = DXUTGetD3D9Object();
		D3DCAPS9 Caps;
		pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps );

		// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
		// then switch to SWVP.
		if( ( Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
			Caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
		{
			pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		// Debugging vertex shaders requires either REF or software vertex processing 
		// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
		if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
		{
			pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
			pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
#endif
#ifdef DEBUG_PS
		pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
	}

	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		if( ( DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF ) ||
			( DXUT_D3D10_DEVICE == pDeviceSettings->ver &&
			pDeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_REFERENCE ) )
			DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext )
{
	HRESULT hr;

	V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
	V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );


	// Get width
	g_fWidth = pBackBufferSurfaceDesc->Width;
	// Get height
	g_fHeight = pBackBufferSurfaceDesc->Height;

	// Info font
	V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Arial", &g_pFont9 ) );

	// About font
	V_RETURN( D3DXCreateFont( pd3dDevice, 28, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Arial", &g_pFontAbout9 ) );

	// Create the 1x1 white default texture
	V_RETURN( pd3dDevice->CreateTexture( 1, 1, 1, 0, D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED, &g_pDefaultTex, NULL ) );
	D3DLOCKED_RECT lr;
	V_RETURN( g_pDefaultTex->LockRect( 0, &lr, NULL, 0 ) );
	*( LPDWORD )lr.pBits = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	V_RETURN( g_pDefaultTex->UnlockRect( 0 ) );

	// Read the D3DX effect file
	WCHAR str[MAX_PATH];
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

#ifdef DEBUG_VS
	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif
#ifdef D3DXFX_LARGEADDRESS_HANDLE
	dwShaderFlags |= D3DXFX_LARGEADDRESSAWARE;
#endif

	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"Airport.fx" ) );
	V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags,
		NULL, &g_pEffect9, NULL ) );

	g_hmWorldViewProjection = g_pEffect9->GetParameterByName( NULL, "g_mWorldViewProjection" );
	g_hmWorld = g_pEffect9->GetParameterByName( NULL, "g_mWorld" );
	g_hfTime = g_pEffect9->GetParameterByName( NULL, "g_fTime" );

	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye( 10.0f, 1.0f, 1.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, 0.0f );

	g_Camera.SetViewParams( &vecEye, &vecAt );

	g_Camera.SetModelCenter(D3DXVECTOR3(0.0f, 0.1f, 0.0f));

	g_Camera.SetEnablePositionMovement(true);
	g_Camera.SetEnableYAxisMovement(true);

	// Budynki
	for( int i = 0; i < sizeof( g_BuildingMeshListData ) / sizeof( MESHLISTDATA ); ++i )
	{
		CMesh* NewMesh = new CMesh();
		NewMesh->Create(g_BuildingMeshListData[i].wszFile, pd3dDevice);
		g_BuildingMeshes.Add(NewMesh);
	}

	//Samoloty
	for( int i = 0; i < sizeof( g_AirplaneMeshListData ) / sizeof( MESHLISTDATA ); ++i )
	{
		CMesh* NewMesh = new CMesh();
		NewMesh->Create(g_AirplaneMeshListData[i].wszFile, pd3dDevice);
		g_AirplaneMeshes.Add(NewMesh);
	}

	//Latajacy Samolot
	for( int i = 0; i < sizeof( g_MovingMeshListData ) / sizeof( MESHLISTDATA ); ++i )
	{
		CMesh* NewMesh = new CMesh();
		NewMesh->Create(g_MovingMeshListData[i].wszFile, pd3dDevice);
		g_MovingMeshes.Add(NewMesh);
	}


	//----------------------------------------------------------------------------
	/*************** PORUSZAJACE SIE POJAZDY - POCZATEK ***********************/
	//----------------------------------------------------------------------------
	//Poruszajacy sie wozek bagazowy
	for( int i = 0; i < sizeof( g_MovingBagCartMeshListData ) / sizeof( MESHLISTDATA ); ++i )
	{
		CMesh* NewMesh = new CMesh();
		NewMesh->Create(g_MovingBagCartMeshListData[i].wszFile, pd3dDevice);
		g_MovingBagCartMeshes.Add(NewMesh);
	}

	//Poruszajacy sie woz strazacki
	for( int i = 0; i < sizeof( g_MovingFireTruckMeshListData ) / sizeof( MESHLISTDATA ); ++i )
	{
		CMesh* NewMesh = new CMesh();
		NewMesh->Create(g_MovingFireTruckMeshListData[i].wszFile, pd3dDevice);
		g_MovingFireTruckMeshes.Add(NewMesh);
	}
	//----------------------------------------------------------------------------
	/*************** PORUSZAJACE SIE POJAZDY - KONIEC ***********************/
	//----------------------------------------------------------------------------


	// Pojazdy
	for( int i = 0; i < sizeof( g_VehicleMeshListData ) / sizeof( MESHLISTDATA ); ++i )
	{
		CMesh* NewMesh = new CMesh();
		NewMesh->Create(g_VehicleMeshListData[i].wszFile, pd3dDevice);
		g_VehicleMeshes.Add(NewMesh);
	}

	//Roslinnoœæ
	for( int i = 0; i < sizeof( g_VegetationMeshListData ) / sizeof( MESHLISTDATA ); ++i )
	{
		CMesh* NewMesh = new CMesh();
		NewMesh->Create(g_VegetationMeshListData[i].wszFile, pd3dDevice);
		g_VegetationMeshes.Add(NewMesh);
	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice,
	const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

	V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
	V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

	if( g_pFont9 ) V_RETURN( g_pFont9->OnResetDevice() );
	if( g_pFontAbout9 ) V_RETURN( g_pFont9->OnResetDevice() );
	if( g_pEffect9 ) V_RETURN( g_pEffect9->OnResetDevice() );

	V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pSprite9 ) );
	g_pTxtHelper = new CDXUTTextHelper( g_pFont9, g_pSprite9, NULL, NULL, 15 );
	g_pTxtHelperAbout = new CDXUTTextHelper( g_pFontAbout9, g_pSprite9, NULL, NULL, 28 );

	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;

	// Powiêkszenie Min Max
	g_Camera.SetRadius( 1.0f, -100.0f, 1.4f );
	// Zakres widocznoœci od do
	//g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.01f, 1000.0f );
	g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.001f, 100000.0f );

	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
	g_HUD.SetSize( 170, 170 );
	g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 350 );
	g_SampleUI.SetSize( 170, 300 );

	// Create mesh
	////
	c_mesh_skybox->Create(L"media\\skybox\\skybox.x", pd3dDevice);
	c_mesh_terrain->Create(L"media\\terrain\\model2.x", pd3dDevice);
	c_mesh_ground->Create(L"media\\ground\\model.x", pd3dDevice);	

	//// Enable texture filtering
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	//pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	////

	////	

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	HRESULT hr;
	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mWorld2;
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mProj;
	D3DXMATRIXA16 mWorldViewProjection;
	
	static Airplane airplane;

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if( g_SettingsDlg.IsActive() )
	{
		g_SettingsDlg.OnRender( fElapsedTime );
		return;
	}

	// Clear the render target and the zbuffer 
	V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 45, 50, 170 ), 1.0f, 0 ) );

	// Gdy jest mo¿liwoœæ ko¿ystaj z w-buffer
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_USEW );

	// Kolor czarny kolorem transparentnym
	pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000001);
	pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	//pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	// Render the scene
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		// Get the projection & view matrix from the camera class
		//mWorld = g_mCenterWorld * *g_Camera.GetWorldMatrix();
		mWorld = *g_Camera.GetWorldMatrix();
		mProj = *g_Camera.GetProjMatrix();
		mView = *g_Camera.GetViewMatrix();

		// To camera rotate mode1
		D3DXMATRIXA16 mWorldTmp;
		D3DXMATRIXA16 mRotationTmp;

		switch(g_iCameraMode)
		{
			//standard camera
		case CAMERA_STANDARD:
			mWorldViewProjection = mWorld * mView * mProj; break;
			
			//static camera
		case CAMERA_ALTERNATIVE:
			D3DXMatrixLookAtLH( &mView, &D3DXVECTOR3(  0.835f, 0.115f, 0.835f ),
				&D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
				&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
			mWorldViewProjection = mWorld * mView * mProj; break;
			
			//rotate camera mode1
		case CAMERA_ROTATE_MODE1:
			D3DXMatrixLookAtLH( &mView, &D3DXVECTOR3( 0.835*cos(fTime * 2.0 * D3DX_PI / 100.0f ) , 0.115f, 0.835*sin(fTime * 2.0 * D3DX_PI / 100.0f ) ),
				&D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
				&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
			mWorldViewProjection = mWorld * mView * mProj; break;
			
			//camera on airplane
		case CAMERA_AIRPLANE:
			switch(g_iAirplaneMode)
			{
				case AIRPLANE_RING:
					airplane.setViewRing1(fTime, &mView);
					mWorldViewProjection = mWorld * mView * mProj;
				break;

				case AIRPLANE_APPROACH:
					D3DXMatrixIdentity( &mWorld );
					airplane.setViewApproach1(fTime, &mView);
					mWorldViewProjection = mWorld * mView * mProj; 
				break;
			}
		break;

			//camera on airplane v.2
		case CAMERA_AIRPLANE2:
			switch(g_iAirplaneMode)
			{
				case AIRPLANE_RING:
					airplane.setViewRing2(fTime, &mView);
					mWorldViewProjection = mWorld * mView * mProj; 
				break;

				case AIRPLANE_APPROACH:
					D3DXMatrixIdentity( &mWorld );
					airplane.setViewApproach2(fTime, &mView);
					mWorldViewProjection = mWorld * mView * mProj; 
				break;
			}
		break;
		}

		// Update the effect's variables.  Instead of using strings, it would 
		// be more efficient to cache a handle to the parameter by calling 
		// ID3DXEffect::GetParameterByName

		V( g_pEffect9->SetMatrix( g_hmWorldViewProjection, &mWorldViewProjection ) );
		V( g_pEffect9->SetMatrix( g_hmWorld, &mWorld ) );

		V( g_pEffect9->SetFloat( g_hfTime, ( float )fTime ) );

		//D3DXMatrixRotationY( &mWorldViewProjection, timeGetTime() / 4000.0f );

		/*
		//// TODO: Zmodyfikowaæ œwiat³a na scenie
		D3DLIGHT9 light;
		D3DXVECTOR3 vecLightDirUnnormalized( 0.0f, -1.0f, 1.0f );
		ZeroMemory( &light, sizeof( D3DLIGHT9 ) );
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Diffuse.r = 1.0f;
		light.Diffuse.g = 1.0f;
		light.Diffuse.b = 1.0f;
		D3DXVec3Normalize( ( D3DXVECTOR3* )&light.Direction, &vecLightDirUnnormalized );
		//light.Position.x = 1.0f;
		//light.Position.y = -1.0f;
		//light.Position.z = 1.0f;
		//light.Range = 1.0f;
		V( pd3dDevice->SetLight( 0, &light ) );
		V( pd3dDevice->LightEnable( 0, TRUE ) );
		////
		*/


		// Start light

		// Fill in a light structure defining our light
		ZeroMemory( &light, sizeof(D3DLIGHT9) );

		if(light_directional) light.Type = D3DLIGHT_DIRECTIONAL;
		else light.Type = D3DLIGHT_SPOT;
		light.Diffuse.r  = light_r;
		light.Diffuse.g  = light_g;
		light.Diffuse.b  = light_b;
		light.Diffuse.a  = light_a;

		light.Range      = 1000.0f;

		// Create a direction for our light - it must be normalized  
		D3DXVECTOR3 vecDir;
		vecDir = D3DXVECTOR3(0.0f,-0.3f,0.5);
		//vecDir = vecDir * mWorldViewProjection;
		D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );

		// Tell the device about the light and turn it on
		pd3dDevice->SetLight( 0, &light );
		if(light_type == 1) pd3dDevice->LightEnable( 0, FALSE );
		else pd3dDevice->LightEnable( 0, TRUE );

		//ADDITIONAL LIGHT
		// Fill in a light structure defining our light
		ZeroMemory( &additional_light, sizeof(D3DLIGHT9) );

		additional_light.Type = D3DLIGHT_POINT;
		additional_light.Diffuse.r  = 1;
		additional_light.Diffuse.g  = 1;
		additional_light.Diffuse.b  = 1;
		additional_light.Diffuse.a  = 1;

		additional_light.Range      = 1000.0f;

		additional_light.Position.x = light_additional_x;
		additional_light.Position.y = light_additional_y;
		additional_light.Position.z = light_additional_z;

		pd3dDevice->SetLight( 1, &additional_light );
		// Tell the device about the light and turn it on
		if(light_type == 0) pd3dDevice->LightEnable( 1, FALSE );
		else pd3dDevice->LightEnable( 1, TRUE );
		// end Light


		////
		// Renderowanie skybox
		pd3dDevice->SetRenderState( D3DRS_LIGHTING, false ); // dezaktywacja oœwietlenia
		pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
		c_mesh_skybox->Render(pd3dDevice,mWorldViewProjection);
		pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
		pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
		pd3dDevice->SetRenderState( D3DRS_LIGHTING, true ); // aktywacja oœwietlenia

		////

		// renderowanie terenu
		c_mesh_terrain->Render(pd3dDevice,mWorldViewProjection);

		// warunkowe renderowanie pasa startowego
		if(g_bShowGround){
			c_mesh_ground->Render(pd3dDevice,mWorldViewProjection);
		}

		// renderowanie budynków
		for( int i = 0; i < g_BuildingMeshes.GetSize(); ++i )
		{
			g_BuildingMeshes[i]->Render(pd3dDevice,mWorldViewProjection);
		}

		// renderowanie samolotów
		for( int i = 0; i < g_AirplaneMeshes.GetSize(); ++i )
		{
			g_AirplaneMeshes[i]->Render(pd3dDevice,mWorldViewProjection);
		}

		//renderowanie latajacego samolotu
		switch(g_iAirplaneMode)
		{
			case AIRPLANE_RING:
			{
				airplane.setWorldRing(fTime, &mWorld, &mWorld2);
				mWorldViewProjection = mWorld2 * mView * mProj;

				g_MovingMeshes[0]->Render(pd3dDevice, mWorldViewProjection);

				mWorldViewProjection = mWorld * mView * mProj;
				
				break;
			}

			case AIRPLANE_APPROACH:
			{
				airplane.setWorldApproach(fTime, &mWorld, &mWorld2);
				mWorldViewProjection = mWorld2 * mView * mProj;

				g_MovingMeshes[1]->Render(pd3dDevice, mWorldViewProjection);

				mWorldViewProjection = mWorld * mView * mProj;

				break;
			}
		}

		//----------------------------------------------------------------------------
		/*************** PORUSZAJACE SIE POJAZDY - POCZATEK ***********************/
		//----------------------------------------------------------------------------
		// renderowanie poruszajacego sie wozka bagazowego
		for( int i = 0; i < g_MovingBagCartMeshes.GetSize(); ++i )
		{
			D3DXMATRIXA16 mTranslation;
			D3DXMATRIXA16 mRotationY;

			D3DXMatrixScaling(&mWorld2, 0.001, 0.001, 0.001);

			if (bc_road_part == 1)
			{
				bc_z_coordinate = -190.0f + bc_counter*0.2f;
				D3DXMatrixRotationY( &mRotationY, Y_ROTATION_90 );
				if (bc_z_coordinate >= -90)
				{
					bc_counter = 0.0f;
					bc_road_part = 2;
					bc_z_coordinate = bc_x_coordinate;
					bc_x_coordinate = 90.0f;
				}
			}
			if (bc_road_part == 2)
			{
				bc_z_coordinate = -30.0f + bc_counter*0.2f;
				D3DXMatrixRotationY( &mRotationY, Y_ROTATION_180 );
				if (bc_z_coordinate >= 20)
				{
					bc_counter = 0.0f;
					bc_z_coordinate = bc_x_coordinate;
					bc_x_coordinate = -20.0f;
					bc_road_part = 3;
				}
			}
			if(bc_road_part == 3)
			{
				if (bc_stop_counter > 200)
				{
					bc_z_coordinate = 90.0f + bc_counter*0.2f;
					D3DXMatrixRotationY( &mRotationY, Y_ROTATION_270 );
					if (bc_z_coordinate >= 190.0f)
					{
						bc_stop_counter = 0;
						bc_counter = 0.0f;
						bc_road_part = 4;
						bc_z_coordinate = bc_x_coordinate;
						bc_x_coordinate = -190.0f;
					}
				}
				else
				{
					D3DXMatrixRotationY( &mRotationY, Y_ROTATION_270 );
					bc_stop_counter += 1;
					bc_counter = 0.0f;
				}
			}
			if(bc_road_part == 4)
			{

				if (bc_stop_counter > 200)
				{
					bc_z_coordinate = -20.0f + bc_counter*0.2f;
					D3DXMatrixRotationY( &mRotationY, Y_ROTATION_0 );
					if (bc_z_coordinate >= 30.0f)
					{
						bc_stop_counter = 0;
						bc_counter = 0.0f;
						bc_road_part = 1;
						bc_z_coordinate = bc_x_coordinate;
						bc_x_coordinate = -30.0f;
					}
				}
				else
				{
					D3DXMatrixRotationY( &mRotationY, Y_ROTATION_0 );
					bc_stop_counter += 1;
					bc_counter = 0.0f;
				}
			}
			bc_counter += 1.0f;
			D3DXMatrixTranslation(&mTranslation, bc_x_coordinate, y_coordinate, bc_z_coordinate);
			mWorld2 = mTranslation*mRotationY*mWorld2*mWorld;
			mWorldViewProjection = mWorld2 * mView * mProj;
			g_MovingBagCartMeshes[i]->Render(pd3dDevice,mWorldViewProjection);
			mWorldViewProjection = mWorld * mView * mProj;
		}

		// renderowanie poruszajacego sie wozu strazackiego
		for( int i = 0; i < g_MovingFireTruckMeshes.GetSize(); ++i )
		{
			D3DXMATRIXA16 mTranslation;
			D3DXMATRIXA16 mWorld2;
			D3DXMATRIXA16 mRotationY;

			D3DXMatrixScaling(&mWorld2, 0.001, 0.001, 0.001);

			if (ft_road_part == 1)
			{
				if (ft_stop_counter > 400)
				{
					ft_z_coordinate = -20.0f + ft_counter*0.2f;
					if (ft_z_coordinate >= 20)
					{
						ft_stop_counter = 0;
						ft_counter = 0.0f;
						ft_road_part = 2;
						ft_z_coordinate = 0.0f;
						ft_x_coordinate = -20.0f;
					}
				}
				else
				{
					ft_stop_counter += 1;
					ft_counter = 0.0f;
				}
			}
			if (ft_road_part == 2)
			{
				ft_z_coordinate = ft_counter*0.2f;
				D3DXMatrixRotationY( &mRotationY, Y_ROTATION_90 );
				if (ft_z_coordinate >= 440)
				{
					ft_counter = 0.0f;
					ft_z_coordinate = -440.0f;
					ft_x_coordinate = 20.0f;
					ft_road_part = 3;
				}
			}
			if(ft_road_part == 3)
			{
				if (ft_stop_counter > 400)
				{
					ft_z_coordinate = -440.0f + ft_counter*0.2f;
					D3DXMatrixRotationY( &mRotationY, Y_ROTATION_270 ); 
					if (ft_z_coordinate >= 0.0f)
					{
						ft_stop_counter = 0;
						ft_counter = 0.0f;
						ft_road_part = 4;
						ft_z_coordinate = -20.0f;
						ft_x_coordinate = 0.0f;
					}
				}
				else
				{
					D3DXMatrixRotationY( &mRotationY, Y_ROTATION_270 );
					ft_stop_counter += 1;
					ft_counter = 0.0f;
				}
			}
			if(ft_road_part == 4)
			{
					ft_z_coordinate = -20.0f + ft_counter*0.2f;
					D3DXMatrixRotationY( &mRotationY, Y_ROTATION_180 );
					if (ft_z_coordinate >= 20.0f)
					{
						ft_stop_counter = 0;
						ft_counter = 0.0f;
						ft_road_part = 1;
						ft_z_coordinate = -20.0f;
						ft_x_coordinate = 0.0f;
					}
			}

			D3DXMatrixTranslation(&mTranslation, ft_x_coordinate, y_coordinate, ft_z_coordinate);

			if (ft_road_part == 1)
				mWorld2 = mTranslation*mWorld2*mWorld;
			else
				mWorld2 = mTranslation*mRotationY*mWorld2*mWorld;
			mWorldViewProjection = mWorld2 * mView * mProj;
			g_MovingFireTruckMeshes[i]->Render(pd3dDevice, mWorldViewProjection);
			mWorldViewProjection = mWorld * mView * mProj;
			ft_counter += 1.0f;
		}
		//----------------------------------------------------------------------------
		/*************** PORUSZAJACE SIE POJAZDY - KONIEC ***********************/
		//----------------------------------------------------------------------------



		// renderowanie pojazdów
		if(g_bShowVehicle){
			for( int i = 0; i < g_VehicleMeshes.GetSize(); ++i )
			{
				g_VehicleMeshes[i]->Render(pd3dDevice,mWorldViewProjection);
			}
		}

		// renderowanie roœlinnoœci
		if(g_bShowVegetation){
			for( int i = 0; i < g_VegetationMeshes.GetSize(); ++i )
			{
				D3DXMATRIXA16 mWorld2;
				D3DXMatrixScaling(&mWorld2, 0.001, 0.001, 0.001);
				mWorld2 = mWorld2*mWorld;
				mWorldViewProjection = mWorld2 * mView * mProj;
				g_VegetationMeshes[i]->Render(pd3dDevice,mWorldViewProjection);
				mWorldViewProjection = mWorld * mView * mProj;
			}
		}
		////

		//renderowanie mg³y
		SetupVertexFog(pd3dDevice, 0x00999999, D3DFOG_EXP);

		DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" ); // These events are to help PIX identify what the code is doing

		// render text info
		if(g_bShowInfo){
			RenderText();
		}
		RenderAboutText();

		// render HUD
		if(g_bShowButton){
			V( g_SampleUI.OnRender( fElapsedTime ) );
			V( g_HUD.OnRender( fElapsedTime ) );
		}

		DXUT_EndPerfEvent();

		V( pd3dDevice->EndScene() );
	}
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext )
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	// Pass messages to settings dialog if its active
	if( g_SettingsDlg.IsActive() )
	{
		g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
		return 0;
	}

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;
	*pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	// Pass all remaining windows messages to camera so it can respond to user input
	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	if( bKeyDown )
	{
		switch( nChar )
		{
			// show/hide ground
		case 'G':
			g_bShowGround = !g_bShowGround; break;
			// show/hide buttons
		case 'B':
			g_bShowButton = !g_bShowButton; break;
			// show/hide info
		case 'I':
			g_bShowInfo = !g_bShowInfo; break;
			// show/hide vehicle
		case 'V':
			g_bShowVehicle = !g_bShowVehicle; break;
			// show/hide vehicle
		case 'T':
			g_bShowVegetation = !g_bShowVegetation; break;
			// Change camera mode
		case 'C':
			g_iCameraMode++;
			if (g_iCameraMode == CAMERA_MAX)
				g_iCameraMode = CAMERA_STANDARD;
			break;
			//Change airplane mode
		case 'A':
			g_iAirplaneMode++;
			if (g_iAirplaneMode == AIRPLANE_MAX)
				g_iAirplaneMode = AIRPLANE_RING;
			g_iCameraMode = CAMERA_AIRPLANE;
			break;

			/*case 'J':
			g_fPosX += 0.1f; break;

			case 'K':
			g_fPosY += 0.1f; break;

			case 'L':
			g_fPosZ += 0.1f; break;*/		

		}
	}
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen(); break;
	case IDC_TOGGLEREF:
		DXUTToggleREF(); break;
	case IDC_CHANGEDEVICE:
		g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
	case IDC_CHANGEFOGMODE:
		if (g_iFogMode == 3)
			g_iFogMode = 1;
		else
			g_iFogMode++;
		if (g_iFogMode == 3)
		{
			g_HUD.GetSlider(IDC_FOGDENSITY)->SetVisible(false);
			g_HUD.GetStatic(IDC_FOGDENSITY_TEXT)->SetVisible(false);
			g_HUD.GetSlider(IDC_FOGSTART)->SetVisible(true);
			g_HUD.GetStatic(IDC_FOGSTART_TEXT)->SetVisible(true);
			g_HUD.GetSlider(IDC_FOGEND)->SetVisible(true);
			g_HUD.GetStatic(IDC_FOGEND_TEXT)->SetVisible(true);
			g_HUD.GetStatic(IDC_ACTUALFOGMODE_TEXT)->SetText(L"Actual Fog Mode: LINEAR");
		}
		else
		{
			g_HUD.GetSlider(IDC_FOGDENSITY)->SetVisible(true);
			g_HUD.GetStatic(IDC_FOGDENSITY_TEXT)->SetVisible(true);
			g_HUD.GetSlider(IDC_FOGSTART)->SetVisible(false);
			g_HUD.GetStatic(IDC_FOGSTART_TEXT)->SetVisible(false);
			g_HUD.GetSlider(IDC_FOGEND)->SetVisible(false);
			g_HUD.GetStatic(IDC_FOGEND_TEXT)->SetVisible(false);
			if (g_iFogMode == 1)
				g_HUD.GetStatic(IDC_ACTUALFOGMODE_TEXT)->SetText(L"Actual Fog Mode: EXP1");
			else
				g_HUD.GetStatic(IDC_ACTUALFOGMODE_TEXT)->SetText(L"Actual Fog Mode: EXP2");
		}
		break;
	case IDC_FOGDENSITY:
		if (g_iFogMode == 1)
			g_fFogDensity = (float)g_HUD.GetSlider(IDC_FOGDENSITY)->GetValue() / 30;
		else
			g_fFogDensity = (float)g_HUD.GetSlider(IDC_FOGDENSITY)->GetValue() / 50;
		break;
	case IDC_FOGSTART:
		g_fFogStart = (float)g_HUD.GetSlider(IDC_FOGSTART)->GetValue() / 10;
		break;
	case IDC_FOGEND:
		g_fFogEnd = (float)g_HUD.GetSlider(IDC_FOGEND)->GetValue() / 10;
		break;
	case IDC_LIGHT_R:
		if(light_r == 1.0f){
			g_HUD.GetButton(IDC_LIGHT_R)->SetText(L"RED OFF");
			light_r = 0.0f;
		}
		else{
			g_HUD.GetButton(IDC_LIGHT_R)->SetText(L"RED ON");
			light_r = 1.0f;
		}
		break;
	case IDC_LIGHT_G:
		if(light_g == 1.0f){
			g_HUD.GetButton(IDC_LIGHT_G)->SetText(L"GREEN OFF");
			light_g = 0.0f;
		}
		else{
			g_HUD.GetButton(IDC_LIGHT_G)->SetText(L"GREEN ON");
			light_g = 1.0f;
		}
		break;
	case IDC_LIGHT_B:
		if(light_b == 1.0f){
			g_HUD.GetButton(IDC_LIGHT_B)->SetText(L"BLUE OFF");
			light_b = 0.0f;
		}
		else{
			g_HUD.GetButton(IDC_LIGHT_B)->SetText(L"BLUE ON");
			light_b = 1.0f;
		}
		break;
	case IDC_LIGHT_TYPE:
		light_directional = !light_directional;
		if(light_directional) g_HUD.GetButton(IDC_LIGHT_TYPE)->SetText(L"Directional light");
		else g_HUD.GetButton(IDC_LIGHT_TYPE)->SetText(L"Point light");
		break;
	case IDC_LIGHT_ADDITIONAL_X:
		light_additional_x = (float)g_HUD.GetSlider(IDC_LIGHT_ADDITIONAL_X)->GetValue()/100.0f;
		break;
	case IDC_LIGHT_ADDITIONAL_Y:
		light_additional_y = (float)g_HUD.GetSlider(IDC_LIGHT_ADDITIONAL_Y)->GetValue()/100.0f;
		break;
	case IDC_LIGHT_ADDITIONAL_Z:
		light_additional_z = (float)g_HUD.GetSlider(IDC_LIGHT_ADDITIONAL_Z)->GetValue()/100.0f;
		break;
	case IDC_LIGHT_SELECTION:
		light_type++;
		if(light_type >= 3) light_type = 0;

		if(light_type == 0)	g_HUD.GetButton(IDC_LIGHT_SELECTION)->SetText(L"Main only");
		else if(light_type == 1) g_HUD.GetButton(IDC_LIGHT_SELECTION)->SetText(L"Additional only");
		else if(light_type == 2) g_HUD.GetButton(IDC_LIGHT_SELECTION)->SetText(L"Both");
		break;
	}
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
	g_DialogResourceManager.OnD3D9LostDevice();
	g_SettingsDlg.OnD3D9LostDevice();
	if( g_pFont9 ) g_pFont9->OnLostDevice();
	if( g_pFontAbout9 ) g_pFont9->OnLostDevice();
	if( g_pEffect9 ) g_pEffect9->OnLostDevice();
	SAFE_RELEASE( g_pSprite9 );
	SAFE_DELETE( g_pTxtHelper );
	SAFE_DELETE( g_pTxtHelperAbout );

	// Zwolnienie obiektów
	c_mesh_skybox->Destroy();
	c_mesh_terrain->Destroy();
	c_mesh_ground->Destroy();

	// Zwolnienie domyœlnej tekstury
	SAFE_RELEASE(g_pDefaultTex);
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
	g_DialogResourceManager.OnD3D9DestroyDevice();
	g_SettingsDlg.OnD3D9DestroyDevice();
	SAFE_RELEASE( g_pEffect9 );
	SAFE_RELEASE( g_pFont9 );
	SAFE_RELEASE( g_pFontAbout9 );

	// Release g_BuildingMeshes
	for( int i = 0; i < g_BuildingMeshes.GetSize(); ++i )
		g_BuildingMeshes[i]->Destroy();	
	g_BuildingMeshes.RemoveAll();

	// Release g_AirplaneMeshes
	for( int i = 0; i < g_AirplaneMeshes.GetSize(); ++i )
		g_AirplaneMeshes[i]->Destroy();	
	g_AirplaneMeshes.RemoveAll();

	// Release g_MovingMeshes
	for( int i = 0; i < g_MovingMeshes.GetSize(); ++i )
		g_MovingMeshes[i]->Destroy();	
	g_MovingMeshes.RemoveAll();

	//----------------------------------------------------------------------------
	/*************** PORUSZAJACE SIE POJAZDY - POCZATEK ***********************/
	//----------------------------------------------------------------------------
	// Release g_MovingBagCartMeshes
	for( int i = 0; i < g_MovingBagCartMeshes.GetSize(); ++i )
		g_MovingBagCartMeshes[i]->Destroy();	
	g_MovingBagCartMeshes.RemoveAll();

	// Release g_MovingFireTruckMeshes
	for( int i = 0; i < g_MovingFireTruckMeshes.GetSize(); ++i )
		g_MovingFireTruckMeshes[i]->Destroy();	
	g_MovingFireTruckMeshes.RemoveAll();
	//----------------------------------------------------------------------------
	/*************** PORUSZAJACE SIE POJAZDY - KONIEC ***********************/
	//----------------------------------------------------------------------------

	// Release g_VehicleMeshes
	for( int i = 0; i < g_VehicleMeshes.GetSize(); ++i )
		g_VehicleMeshes[i]->Destroy();	
	g_VehicleMeshes.RemoveAll();

	// Release g_VegetationMeshes
	for( int i = 0; i < g_VegetationMeshes.GetSize(); ++i )
		g_VegetationMeshes[i]->Destroy();	
	g_VegetationMeshes.RemoveAll();
}

void SetupVertexFog(IDirect3DDevice9* pd3dDevice, DWORD Color,  BOOL UseRange)
{
    float Start = 0.5f,    // Linear fog distances
          End   = 0.8f;
 
    // Enable fog blending.
    pd3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
 
    // Set the fog color.
    pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, Color);
    
    // Set fog parameters.
	if(D3DFOG_LINEAR == g_iFogMode)
    {
		pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, g_iFogMode);
		pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&g_fFogStart));
		pd3dDevice->SetRenderState(D3DRS_FOGEND,   *(DWORD *)(&g_fFogEnd));
    }
    else
    {
		pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, g_iFogMode);
		pd3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&g_fFogDensity));
    }

    // Enable range-based fog if desired (only supported for
    //   vertex fog). For this example, it is assumed that UseRange
    //   is set to a nonzero value only if the driver exposes the 
    //   D3DPRASTERCAPS_FOGRANGE capability.
    // Note: This is slightly more performance intensive
    //   than non-range-based fog.
    if(UseRange)
        pd3dDevice->SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);
}