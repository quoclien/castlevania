#pragma once
#include "GameObject.h"
#include "Effect.h"
#include "Heart.h"

#define FIREPOTS_BBOX_WIDTH  16
#define FIREPOTS_BBOX_HEIGHT 32
#define FIREPOTS_STATE_BURN  100
#define FIREPOTS_STATE_BREAK 200
#define FIREPOTS_ANI_NOTHING 0
#define FIREPOTS_ANI_BURN    1
class FirePots : public CGameObject
{
	bool IsBreak;
	//DWORD timer = 0;
	//bool ended = false;
	int id;
public:
	FirePots();
	~FirePots();
	virtual void Render();
	void SetState(int stat);
	virtual void GetBoundingBox(float &l, float &t, float &r, float &b);
	void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	bool isBreak();
	void setID(int ID);
	int getID();
	
};
