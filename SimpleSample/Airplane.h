#pragma once
#include "PlainAnimation.h"

class CMesh;

//parametry ruchu po okregu
static const float R = -0.8f;			//promien okregu po ktorym porusza sie samolot
static const float Rv1 = -0.835f;		//promien dla View 1
static const float Rv2 = -0.81f;		//promien dla View 2
static const float H = 0.2f;			//wysokosc samolotu nad gruntem
static const float Hv1 = 0.115f;		//wysokosc dla View 1
static const float Hv2 = 0.111f;		//wysokosc dla View 2
static const float S = D3DX_PI/50.0f;	//skala czasowa; im wieksze tym wolniej porusza sie samolot

//parametry ladowania
static const float Rot = 1.5708;		//obrot (w radianach) samolotu (1.5708 ~ 90*)
static const float A = D3DX_PI/720.0f;	//skala odchylenia samolotu (im wieksze tym samolot bardzije odchylony)
static const float Xscale = 3.0f;		//skala toru w osi X
static const float Yscale = 0.1f;		//skala toru w osi Y
static const float Xtrans = -2.4f;		//przesuniecie toru w osi X
static const float Ytrans = 0.001f;		//przesuniecie toru w osi Y
static const float Ztrans= 0.1418f;		//przesuniecie toru w osi Z
static const float animStart = 0.3f;	//poczatek animacji
static const float Sa = 30.0f;			//skala czasowe

class Airplane
{
	float tStart;
	float tActual;

	CPlainAnimation anim;
	bool approach;

	D3DXMATRIXA16 mRotationY;
	D3DXMATRIXA16 mRotationZ;
	D3DXMATRIXA16 mTranslation;

public:
	Airplane(void);
	~Airplane(void);

	void setWorldRing(float fTime, D3DXMATRIXA16* mWorld, D3DXMATRIXA16* mWorld2);
	void setViewRing1(float fTime, D3DXMATRIXA16* mWorldViewProjection);
	void setViewRing2(float fTime, D3DXMATRIXA16* mWorldViewProjection);

	void setWorldApproach(float fTime, D3DXMATRIXA16* mWorld, D3DXMATRIXA16* mWorld2);
	void setViewApproach1(float fTime, D3DXMATRIXA16* mView);
	void setViewApproach2(float fTime, D3DXMATRIXA16* mView);
};

