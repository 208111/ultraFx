
#include "E:\D3D9\d3d9.h"
#include "E:\D3D9\d3dx9.h"
#include <Windows.h>
#include <intrin.h>

#include "resource.h"

#pragma comment (lib, "winmm.lib")

//#pragma comment(linker, "/FORCE:MULTIPLE")
//#pragma comment(linker, "/FILEALIGN:1024")
//#pragma comment(linker,     "/ALIGN:1024")// Merge sections
//#pragma comment(linker, "/MERGE:.rdata=.data")
//#pragma comment(linker, "/MERGE:.text=.data")// 252K WTF
#pragma optimize("gsy", on)

#define LODWORD(_qw)    ((DWORD)(_qw))
#define HIDWORD(_qw)    ((DWORD)(((_qw) >> 32) & 0xffffffff))

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
// whats the dependency identity for direct3d
// but why should i care

void * __cdecl operator new(unsigned int bytes)
{
	return HeapAlloc(GetProcessHeap(), 0, bytes);
}

void __cdecl operator delete(void *ptr)
{
	if (ptr) HeapFree(GetProcessHeap(), 0, ptr);
}

static char*$;

static LPDIRECT3DTEXTURE9	$T[4];
static LPD3DXSPRITE			drawing;
static D3DXVECTOR2			position;

static LPD3DXFONT			$F[4];
static HFONT				$Fh[16];
static USHORT				tick = 16;
static LPSTR				uFxC = "uFxSys";

static LPDIRECT3D9			d3d;
static LPDIRECT3DDEVICE9	d3ddev;

static UINT		score;
static USHORT	health;
static BYTE		lives;

static HBITMAP	common_gfx[4];
static HWND		int_hwnd[4];

static char*STR, *PRG, *GFX, *SFX;
static char END[] = "END     ÿÿÿÿÿÿÿÿ";

int* xor(char*in,size_t length,char*key,size_t keylen)
{
	size_t i;
	for (i = 0; i < length; i++)
		//if (in[i])
		in[i] ^= key[i % keylen];
	return (int*)(in+i); // <-- imitating old programming thing to piss you off
}

UINT xorkle = 0xD;
char*xorkey = "\x13\x07\x04\x1A\x12\x0F\x04\x00\x0A\x04\x00\x12\x18";//"the speakeasy";

#define MAKELLONG(a,b) (long long) a << 32 | b;

ULONGLONG muhLookupTable(char*mem, char*id)
{
	char* a = mem;
	char* b = new char[8];

	strncpy(b, id, 8);

	for (int i = 0; i < 8; i++)
		if (!b[i])
			b[i] = ' ';

	while (strncmp(a, END, 8))
	{
		if (!strncmp(a, b, 8))
			break;
		else
			a += 16;
	}

	int addr, len;
	CopyMemory(&len, a + 0x8, sizeof(DWORD));
	CopyMemory(&addr, a + 0xC, sizeof(DWORD));

	return MAKELLONG(_byteswap_ulong(len), _byteswap_ulong(addr));
}

static bool playSnd(char*SFXx)
{
	return PlaySound(SFXx,
		GetModuleHandle(NULL),
		SND_ASYNC | SND_MEMORY | SND_NOWAIT);
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
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(0);
	wc.lpszClassName = uFxC;

	RegisterClassEx(&wc);
	
	hWnd = CreateWindowEx(NULL,
		uFxC,
		0,
		WS_OVERLAPPED | WS_SYSMENU,
		GetSystemMetrics(SM_CXSCREEN) / 2 - 640 / 2,
		GetSystemMetrics(SM_CYSCREEN) / 2 - 480 / 2,
		640 + 6,
		480 + 34,
		NULL,
		NULL,
		hInstance,
		NULL);

	{
		$ = new char[0x10000];
		LoadString(hInstance, COM_STR_UFXT, $ + 0x93E0, 0x20);
		SetWindowText(hWnd, $ + 0x93E0);
		LoadString(hInstance, COM_STR_LD0, $ + 0x9400, 0x20);
		int_hwnd[0] = CreateWindowExA(0, "static", $ + 0x9400, WS_CHILD | WS_VISIBLE, 16, 16, 112, 16, hWnd, 0, hInstance, 0);
		common_gfx[0] = LoadBitmap(hInstance, MAKEINTRESOURCE(COM_GFX_LD0));
		common_gfx[1] = LoadBitmap(hInstance, MAKEINTRESOURCE(COM_GFX_LD1));
		int_hwnd[1] = CreateWindowExA(0, "static", "COM_GFX_LD0", WS_CHILD | WS_VISIBLE | SS_BITMAP, 16, 40, 21, 22, hWnd, 0, hInstance, common_gfx[0]);
		SendMessage(int_hwnd[1], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)common_gfx[0]);
	}

	ShowWindow(hWnd, nCmdShow);

	//for (int i = 0; i < xorkle; i++)
	//{
		//xorkey[i] ^= 0xFF;
	//}

	{
		UpdateWindow(hWnd);

		// D3D INIT
		{
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

		LoadString(hInstance, COM_STR_LD1, $ + 0x9420, 0x20);
		SetWindowText(int_hwnd[0], $ + 0x9420);
		UpdateWindow(hWnd);

		// FILE LOADING
		{
			HANDLE file;
			file = CreateFile("STR", GENERIC_READ, FILE_SHARE_READ, 0,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			STR = new char[0x10000];
			
			if (file != INVALID_HANDLE_VALUE)
				if (ReadFile(file, STR, 0x200, 0, 0))
					SetWindowText(hWnd, STR + LODWORD(muhLookupTable(STR, "TITLE")));
		}
		
		SetWindowText(int_hwnd[0], $ + 0x9420);
		SendMessage(int_hwnd[1], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)common_gfx[1]);
		UpdateWindow(hWnd);

		Sleep(40);
	}

	DestroyWindow(int_hwnd[0]);
	DestroyWindow(int_hwnd[1]);

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
