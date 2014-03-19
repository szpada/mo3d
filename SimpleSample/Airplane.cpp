#include "DXUT.h"
#include "Airplane.h"
#include "CMesh.h"


Airplane::Airplane(void)
{
	approach = false;
}

Airplane::~Airplane(void)
{
}

void Airplane::setWorldRing(float fTime, D3DXMATRIXA16* mWorld, D3DXMATRIXA16* mWorld2)
{
	if(approach)
	{
		approach = false;
	}
	D3DXMatrixRotationZ( &mRotationZ, -0.2f );
	D3DXMatrixRotationY( &mRotationY, fTime * S );
	D3DXMatrixTranslation(&mTranslation, R, H, 0.0f);

	*mWorld2 = mRotationZ*mTranslation*mRotationY*(*mWorld);
}

void Airplane::setViewRing1(float fTime, D3DXMATRIXA16* mView)
{
	D3DXMatrixLookAtLH( mView, 
		&D3DXVECTOR3( Rv1*cos((fTime-0.05) * -S) , Hv1, Rv1*sin((fTime-0.05) * -S) ),
		&D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
		&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
}

void Airplane::setViewRing2(float fTime, D3DXMATRIXA16* mView)
{
	D3DXMatrixLookAtLH( mView, 
		&D3DXVECTOR3( Rv2*cos((fTime-1.0f) * -S) , Hv2, Rv2*sin((fTime-1.0f) * -S) ),
		&D3DXVECTOR3( R*cos((fTime+10.0f) * 2.0f * -S ) , Hv2-0.1f, R*sin((fTime+10.0f) * 2.0f * -S ) ),
		&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
}

void Airplane::setWorldApproach(float fTime, D3DXMATRIXA16* mWorld, D3DXMATRIXA16* mWorld2)
{
	if(!approach)
	{
		approach = true;
		tStart = fTime;
	}

	D3DXMatrixRotationY(&mRotationY, Rot);

	D3DXMatrixRotationZ(&mRotationZ, anim.GetFi(tActual)*A );
	D3DXMatrixTranslation(&mTranslation, Xscale*anim.GetX(tActual)+Xtrans, Yscale*anim.GetY(tActual)+Ytrans, Ztrans);

	*mWorld2 = mRotationY*mRotationZ*mTranslation*(*mWorld);
}

void Airplane::setViewApproach1(float fTime, D3DXMATRIXA16* mView)
{
	tActual = ((fTime - tStart) / Sa) + animStart;
	D3DXMatrixLookAtLH( mView, 
		&D3DXVECTOR3( Xscale*anim.GetX(tActual)+Xtrans-0.05f, Yscale*anim.GetY(tActual)+0.01f, Ztrans - 0.01f),
		&D3DXVECTOR3( Xscale*anim.GetX(tActual)+Xtrans+0.1f, Yscale*anim.GetY(tActual), Ztrans),
		&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
}

void Airplane::setViewApproach2(float fTime, D3DXMATRIXA16* mView)
{
	tActual = ((fTime - tStart) / Sa) + animStart;
	D3DXMatrixLookAtLH( mView, 
		&D3DXVECTOR3( Xscale*anim.GetX(tActual)+Xtrans+0.05f, Yscale*anim.GetY(tActual)+0.01f, Ztrans - 0.01f),
		&D3DXVECTOR3( Xscale*anim.GetX(tActual)+Xtrans-0.0f, Yscale*anim.GetY(tActual), Ztrans),
		&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
}