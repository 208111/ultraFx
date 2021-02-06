
#include "E:\D3D9\d3d9.h"
#include "E:\D3D9\d3dx9.h"
#include <Windows.h>
#include <fstream>

#include "resource.h"

#pragma comment (lib, "E:\\D3D9\\d3d9.lib")
#pragma comment (lib, "E:\\D3D9\\d3dx9.lib")
#pragma comment (lib, "winmm.lib")

static char $[0x10000];

static LPDIRECT3DTEXTURE9	$T[4];
static LPD3DXSPRITE			drawing;
static D3DXVECTOR2			position;

static LPD3DXFONT			$F[4];
static HFONT				$Fh[16];
static USHORT				tick = 16;
static LPSTR				test = "ultraFx",
							uFxC = "uFxSys";

static LPDIRECT3D9			d3d;
static LPDIRECT3DDEVICE9	d3ddev;

static UINT		score;
static USHORT	health;
static BYTE		lives;

static HBITMAP	common_gfx[4];
static HWND		int_hwnd[4];

static char CHR[0x10000];
static char PRG[0x10000];
static char GFX[0x10000];
static char SFX[0x10000];

//#define CRYPT8(str) { CRYPT8_(str "\0\0\0\0\0\0\0\0") }
//#define CRYPT8_(str) (str)[0] + 1, (str)[1] + 2, (str)[2] + 3, (str)[3] + 4, (str)[4] + 5, (str)[5] + 6, (str)[6] + 7, (str)[7] + 8, '\0'

void xor(char*in,size_t length,char*key,size_t keylen)
{
	size_t i;
	for (i = 0; i < length; i++)
		in[i] ^= key[i % keylen];
	//return (int*)(in + i);
}

UINT xorkle = 0xD;
char*xorkey = "\x13\x07\x04\x1A\x12\x0F\x04\x00\x0A\x04\x00\x12\x18";//"the speakeasy";//"\x8B\x97\x9A\xDF\x8C\x8F\x9A\x9E\x94\x9A\x9E\x8C\x86";

using namespace std;

static bool playSnd(LPCSTR fname)
{
	return PlaySound(fname,
		GetModuleHandle(NULL),
		SND_ASYNC | SND_FILENAME | SND_NOWAIT);
}

static void NewTexture(LPCSTR fname, D3DFORMAT fmt, LPDIRECT3DTEXTURE9 *tex)
{
	D3DXCreateTextureFromFileEx(d3ddev, fname, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT, 0, fmt, D3DPOOL_DEFAULT, D3DX_DEFAULT,
		D3DX_DEFAULT, D3DCOLOR_XRGB(255, 0, 255), NULL, NULL, tex);
}

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	static HWND hWnd;
	static WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = uFxC;

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
		uFxC,
		"ultraFx",
		WS_OVERLAPPED | WS_SYSMENU + SS_BITMAP,
		GetSystemMetrics(SM_CXSCREEN) / 2 - 1024 / 2,
		GetSystemMetrics(SM_CYSCREEN) / 2 - 768 / 2,
		1024 + 6,
		768 + 34,
		NULL,
		NULL,
		hInstance,
		NULL);

	{
		int_hwnd[0] = CreateWindowExA(0, "static", "Loading!!!", WS_CHILD | WS_VISIBLE, 16, 16, 64, 16, hWnd, 0, hInstance, 0);
		//common_gfx[0] = LoadBitmap(hInstance, MAKEINTRESOURCE(COM_GFX_LD0));
		//common_gfx[1] = LoadBitmap(hInstance, MAKEINTRESOURCE(COM_GFX_LD1));
		//int_hwnd[1] = CreateWindowExA(0, "static", "COM_GFX_LD0", WS_CHILD | WS_VISIBLE, 16, 40, 21, 22, hWnd, 0, hInstance, common_gfx[0]);
		//int_hwnd[2] = CreateWindowExA(0, "static", "COM_GFX_LD1", WS_CHILD | WS_VISIBLE, 16, 66, 21, 22, hWnd, 0, hInstance, 0);
		//SendMessage(int_hwnd[1], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)common_gfx[0]);
	}

	ShowWindow(hWnd, nCmdShow);

	//for (int i = 0; i < xorkle; i++)
	//{
		//xorkey[i] ^= 0xFF;
	//}

	{
		ifstream file;
		file.open("STR", ios::in | ios::binary);
		//file.seekg(0x100);
		if (file.read($, 0x200))
		{
			xor ($, 0x200, xorkey, xorkle);
			//sprintf_s($,"%d",file.gcount());
			SetWindowText(hWnd, $+0x100);
		}

		DestroyWindow(int_hwnd[0]);

		d3d = Direct3DCreate9(D3D_SDK_VERSION);

		D3DPRESENT_PARAMETERS d3dpp;

		ZeroMemory(&d3dpp, sizeof(d3dpp));

		d3dpp.Windowed = true;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = hWnd;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		d3d->CreateDevice(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp,
			&d3ddev);

		d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);

		d3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);

		D3DXCreateSprite(d3ddev, &drawing);
	}
	

	static MSG msg;

	while (TRUE)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		{
			Sleep(16);

			/*d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, 255, 0.0f, 0);

			d3ddev->BeginScene();

			drawing->Begin();

			//drawing->Draw(textures[0], NULL, NULL, 0, 0, NULL, 0xFFFFFFFF);

			drawing->End();

			d3ddev->EndScene();

			d3ddev->Present(NULL, NULL, NULL, NULL);*/
		}
	}

	d3ddev->Release();
	d3d->Release();

	//abort(); // lol

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_QUIT:
	case WM_CLOSE:
	case WM_DESTROY:
	{
		//abort();
		PostQuitMessage(0);
		return 0;
	} break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
