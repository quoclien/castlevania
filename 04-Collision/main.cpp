/* =============================================================
	INTRODUCTION TO GAME PROGRAMMING SE102

	SAMPLE 04 - COLLISION
o
	This sample illustrates how to:

		1/ Implement SweptAABB algorithm between moving objects
		2/ Implement a simple (yet effective) collision frame work

	Key functions:
		CGame::SweptAABB
		CGameObject::SweptAABBEx
		CGameObject::CalcPotentialCollisions
		CGameObject::FilterCollision

		CGameObject::GetBoundingBox

================================================================ */

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include<fstream>
#include <queue>

#include "debug.h"
#include "Game.h"
#include "GameObject.h"
#include "Textures.h"

#include "Simon.h"
#include "Brick.h"
#include "FirePots.h"
#include"BackGround.h"
#include"Weapons.h"
#include"dagger.h"

#define WINDOW_CLASS_NAME L"SampleWindow"
#define MAIN_WINDOW_TITLE L"04 - Collision"

#define BACKGROUND_COLOR D3DCOLOR_XRGB(36, 24, 140)
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 200
#define GROUND_HEIGHT 115
#define SCREEN_EDGE_LEFT 0
#define SCREEN_EDGE_RIGHT 740

#define MAX_FRAME_RATE 12000

#define ID_TEX_SIMON 0
#define ID_TEX_EFFECT 10	
#define ID_TEX_BRICK 20
#define ID_TEX_FIREPOTS 30
#define ID_TEX_ENTRANCESTAGE 40
#define ID_TEX_WHIP  50
#define ID_TEX_ITEM  60

CGame *game;

Simon *simon;
FirePots *firepots;
Weapons *whip;
Dagger *dagger;

vector<LPGAMEOBJECT> objects;
vector<LPGAMEOBJECT> objects2;
vector<LPGAMEOBJECT> effect;
class CSampleKeyHander : public CKeyEventHandler
{
	virtual void KeyState(BYTE *states);
	virtual void OnKeyDown(int KeyCode);
	virtual void OnKeyUp(int KeyCode);
};

CSampleKeyHander * keyHandler;

void CSampleKeyHander::OnKeyDown(int KeyCode)
{
	DebugOut(L"[INFO] KeyDown: %d\n", KeyCode);
	switch (KeyCode)
	{
	case DIK_SPACE:
		if (simon->Get_IsJumping()) return;
			simon->SetState(SIMON_STATE_JUMP);
		break;
	case DIK_A: // reset
		simon->SetState(SIMON_STATE_IDLE);
		simon->SetPosition(50.0f, 0.0f);
		simon->SetSpeed(0, 0);
		break;
	case DIK_F:
		if (simon->getDagger() > 0)
		{
			dagger->SetState(DAGGER_STATE_NORMAL);
			dagger->SetSpeed(0.5f, 0);
			dagger->Get_simon(simon);
			simon->SetDagger(simon->getDagger() - 1);
		}
		break;
	case DIK_Z:
		if (simon->GetState() == SIMON_STATE_SIT_IDLE)
		{
			if (simon->getSimonnx() == -1)
			{
				simon->SetState(SIMON_STATE_SIT_ATTACK_LEFT);
			}
			else
			{
				simon->SetState(SIMON_STATE_SIT_ATTACK_RIGHT);
			}
		}
		else
			simon->SetState(SIMON_STATE_ATTACK);
		whip->SetState(WHIP_STATE_ATTACK);
		break;
	case DIK_DOWN:
		simon->SetState(SIMON_STATE_SIT_IDLE);
		break;
	case DIK_Q:
		whip->setLevel();
		break;
	}
}

void CSampleKeyHander::OnKeyUp(int KeyCode)
{
	DebugOut(L"[INFO] KeyUp: %d\n", KeyCode);
	switch (KeyCode) {
	case DIK_DOWN:
		simon->SetState(SIMON_STATE_STAND);
		break;
	}

}

void CSampleKeyHander::KeyState(BYTE *states)
{
	// disable control key when Simon die 
	if (simon->GetState() == SIMON_STATE_DIE) return;
	if (game->IsKeyDown(DIK_RIGHT))
	{
		//if (simon->Get_IsAttack()||simon->Get_IsSit())
		//	return;
		if (simon->GetState() == SIMON_STATE_SIT_IDLE)
		{
			if (simon->getSimonnx() == -1)
			{
				simon->setSimonnx(1);
			}
		}
		else
			simon->SetState(SIMON_STATE_WALKING_RIGHT);
	}
	else if (game->IsKeyDown(DIK_LEFT))
	{
		//if (simon->Get_IsAttack() || simon->Get_IsSit())
		//	return;
		if (simon->GetState() == SIMON_STATE_SIT_IDLE)
		{
			if (simon->getSimonnx() == 1)
			{
				simon->setSimonnx(-1);
			}
		}
		else
		{
			simon->SetState(SIMON_STATE_WALKING_LEFT);
		}
	}
	else if(game->IsKeyDown(DIK_DOWN))
		simon->SetState(SIMON_STATE_SIT_IDLE);
	else
		simon->SetState(SIMON_STATE_IDLE);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

/*
	Load all game resources
	In this example: load textures, sprites, animations and mario object

	TO-DO: Improve this function by loading texture,sprite,animation,object from file
*/
void LoadResources()
{
	CTextures * textures = CTextures::GetInstance();
	textures->Add(ID_TEX_SIMON, L"textures\\Simon.png", D3DCOLOR_XRGB(0, 128, 128));
	textures->Add(ID_TEX_BRICK, L"textures\\brick.png", D3DCOLOR_XRGB(176, 224, 248));
	textures->Add(ID_TEX_EFFECT, L"textures\\Effect1.png.png", D3DCOLOR_XRGB(34, 177, 76));
	textures->Add(ID_TEX_FIREPOTS, L"textures\\FirePots.png", D3DCOLOR_XRGB(34, 177, 76));
	textures->Add(ID_TEX_ENTRANCESTAGE, L"textures\\Background_entrance.png", D3DCOLOR_XRGB(36, 24, 140));
	textures->Add(ID_TEX_WHIP, L"textures\\Whip.png", D3DCOLOR_XRGB(0, 128, 128));
	textures->Add(ID_TEX_BBOX, L"textures\\bbox.png", D3DCOLOR_XRGB(255, 225, 225));
	textures->Add(ID_TEX_ITEM, L"textures\\Item.png", D3DCOLOR_XRGB(128, 0, 0));


	CSprites * sprites = CSprites::GetInstance();
	CAnimations * animations = CAnimations::GetInstance();

	LPDIRECT3DTEXTURE9 texSimon = textures->Get(ID_TEX_SIMON);
	LPDIRECT3DTEXTURE9 texBrick = textures->Get(ID_TEX_BRICK);
	LPDIRECT3DTEXTURE9 texFirePots = textures->Get(ID_TEX_FIREPOTS);
	LPDIRECT3DTEXTURE9 texMap = textures->Get(ID_TEX_ENTRANCESTAGE);
	LPDIRECT3DTEXTURE9 texWhip = textures->Get(ID_TEX_WHIP);
	LPDIRECT3DTEXTURE9 texEffect = textures->Get(ID_TEX_EFFECT);
	LPDIRECT3DTEXTURE9 texItem = textures->Get(ID_TEX_ITEM);
	
	LPDIRECT3DTEXTURE9 idTextures = NULL;
	ifstream infile;
	infile.open("ReadFile\\Sprites.txt");
	int arr[6];
	while (infile)
	{
		infile >> arr[0] >> arr[1] >> arr[2] >> arr[3] >> arr[4] >> arr[5];
		switch (arr[5])
		{
		case 0:
			idTextures = texSimon;
			break;
		case 1:
			idTextures = texBrick;
			break;
		case 2:
			idTextures = texFirePots;
			break;
		case 3:
			idTextures = texMap;
			break;
		case 4:
			idTextures = texWhip;
			break;
		case 5:
			idTextures = texEffect;
			break;
		default:
			idTextures = texItem;
		}
		sprites->Add(arr[0], arr[1], arr[2], arr[3], arr[4], idTextures);
	}
	infile.close();


	LPANIMATION ani;
	int arr2[5]; int time, row, idAni;
	infile.open("ReadFile\\Animation.txt");
	while (infile) {
		infile >> time >> row >> idAni;
		for(int i=0;i<row;i++)
			infile >> arr2[i];
		ani = new CAnimation(time);
		for (int i = 0; i < row; i++)
			ani->Add(arr2[i]);
		animations->Add(idAni, ani);
	}
	infile.close();

	for (int i = 0; i <= 23; i++)
	{
		ani = new CAnimation(100);
		ani->Add(i);
		animations->Add(i, ani);
	}
	ifstream myfile{ "ReadFile\\Entrance.txt" };
	int mArray[50][50];
	for (int d = 5; d >= 0; d--) {
		for (int c = 0; c <= 23; c++) {
			myfile >> mArray[d][c];
			BackGround *back = new BackGround();
			if (mArray[d][c] != 0)
			{
				back->AddAnimation(mArray[d][c]);
				back->SetPosition(0 + c * 32.0f, 150 - d * 32.0f);
				objects.push_back(back);
			}
		}
	}
	myfile.close();

	infile.open("ReadFile\\AddAni.txt");
	int obj, row2; int id[15];
	
	while (infile){  
		infile >> obj >> row2;
		if(obj==1)
		{
			simon = new Simon();
			for (int i = 0; i < row2; i++)
			{
				infile >> id[i];
				simon->AddAnimation(id[i]);
			}
			simon->SetPosition(50.0f, 0);
			
		}
		if (obj == 2)
		{
			whip = new Weapons();
			for (int i = 0; i < row2; i++)
			{
				infile >> id[i];
				whip->AddAnimation(id[i]);
			}
		}
		if (obj == 3)
		{
			dagger = new Dagger();
			for (int i = 0; i < row2; i++)
			{
				infile >> id[i];
				dagger->AddAnimation(id[i]);
			}
		}
		
	}
	objects.push_back(simon);
	objects2.push_back(whip);
	objects2.push_back(dagger);

	for (int i = 0; i < 51; i++)
	{
		CBrick *brick = new CBrick();
		brick->AddAnimation(601);
		brick->SetPosition(0 + i * 15.0f, 150);
		objects.push_back(brick);
	}
	for (int i = 0; i < 4; i++)
	{
		FirePots *firepots = new FirePots();
		firepots->AddAnimation(603);
		firepots->AddAnimation(602);
		firepots->setID(i);
		firepots->SetPosition(130 + i * 100.0f, GROUND_HEIGHT+5.5f);
		objects2.push_back(firepots);
	}


	for (int i = 0; i < 4; i++)
	{
		Effect* ef = new Effect();
		ef->AddAnimation(1001);
		ef->SetPosition(130 + i * 100.0f, 125);
		effect.push_back(ef);
	}

	for (int i = 0; i < 4; i++)
	{
		Heart* it = new Heart(true);
		it->SetPosition(130 + i * 100.0f, 125);
		objects.push_back(it);
	}
}

/*
	Update world status for this frame
	dt: time period between beginning of last frame and beginning of this frame
*/
void Update(DWORD dt)
{
	// We know that Mario is the first object in the list hence we won't add him into the colliable object list
	// TO-DO: This is a "dirty" way, need a more organized way 
	whip->Get_simon(simon);
	vector<LPGAMEOBJECT> coObjects;
	vector<LPGAMEOBJECT> coObjects2;
	for (int i = 1; i < objects.size(); i++)
	{
		coObjects.push_back(objects[i]);
	}
	
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->Update(dt, &coObjects);
		if (dynamic_cast<FirePots*>(objects.at(i)))//is firepots
		{
			FirePots* fp = dynamic_cast<FirePots*>(objects.at(i));
			if (fp->GetState() == FIREPOTS_STATE_BREAK)
			{
				int id = fp->getID();
				effect[id]->Update(dt);
			}

		}
	}
	for (int i = 1; i < objects2.size(); i++)
	{
		coObjects2.push_back(objects2[i]);
	}
	for (int i = 0; i < objects2.size(); i++)
	{
		objects2[i]->Update(dt, &coObjects2);
	}

	float cx, cy;
	simon->GetPosition(cx, cy);
	//keep Simon inside the screen
	if (cx < SCREEN_EDGE_LEFT)
	{
		simon->SetPosition(SCREEN_EDGE_LEFT, cy);
	}
	else if (cx > SCREEN_EDGE_RIGHT)
	{
		simon->SetPosition(SCREEN_EDGE_RIGHT, cy);
	}

	// Update camera to follow simon
	cx -= SCREEN_WIDTH / 2;
	cy -= SCREEN_HEIGHT / 2;
	
	CGame::GetInstance()->SetCamPos(cx, 0.0f /*cy*/);
}

/*
	Render a frame
*/
void Render()
{
	CSprites * sprites = CSprites::GetInstance();
	LPDIRECT3DDEVICE9 d3ddv = game->GetDirect3DDevice();
	LPDIRECT3DSURFACE9 bb = game->GetBackBuffer();
	LPD3DXSPRITE spriteHandler = game->GetSpriteHandler();

	if (d3ddv->BeginScene())
	{
		// Clear back buffer with a color
		d3ddv->ColorFill(bb, NULL, BACKGROUND_COLOR);

		spriteHandler->Begin(D3DXSPRITE_ALPHABLEND);
		
		for (int i = 0; i < objects.size(); i++)
		{
			objects[i]->Render();
			if (dynamic_cast<FirePots*>(objects.at(i)))//is firepots
			{
				FirePots* fp = dynamic_cast<FirePots*>(objects.at(i));
				if (fp->GetState() == FIREPOTS_STATE_BREAK)
				{
					int id = fp->getID();
					effect[id]->Render();
				}

			}
		}
		for (int i = 0; i < objects2.size(); i++)
		{
			objects2[i]->Render();
		}
		for (int i = 0; i < effect.size(); i++)
		{
			effect[i]->Render();
		}
		spriteHandler->End();
		d3ddv->EndScene();
	}

	// Display back buffer content to the screen
	d3ddv->Present(NULL, NULL, NULL, NULL);
}

HWND CreateGameWindow(HINSTANCE hInstance, int nCmdShow, int ScreenWidth, int ScreenHeight)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;

	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	HWND hWnd =
		CreateWindow(
			WINDOW_CLASS_NAME,
			MAIN_WINDOW_TITLE,
			WS_OVERLAPPEDWINDOW, // WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			ScreenWidth,
			ScreenHeight,
			NULL,
			NULL,
			hInstance,
			NULL);

	if (!hWnd)
	{
		OutputDebugString(L"[ERROR] CreateWindow failed");
		DWORD ErrCode = GetLastError();
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

int Run()
{
	MSG msg;
	int done = 0;
	DWORD frameStart = GetTickCount();
	DWORD tickPerFrame = 1000 / MAX_FRAME_RATE;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DWORD now = GetTickCount();

		// dt: the time between (beginning of last frame) and now
		// this frame: the frame we are about to render
		DWORD dt = now - frameStart;

		if (dt >= tickPerFrame)
		{
			frameStart = now;

			game->ProcessKeyboard();

			Update(dt);
			Render();
		}
		else
			Sleep(tickPerFrame - dt);
	}

	return 1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd = CreateGameWindow(hInstance, nCmdShow, SCREEN_WIDTH, SCREEN_HEIGHT);

	game = CGame::GetInstance();
	game->Init(hWnd);

	keyHandler = new CSampleKeyHander();
	game->InitKeyboard(keyHandler);


	LoadResources();

	SetWindowPos(hWnd, 0, 0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	Run();

	return 0;
}