#include <Windows.h>
#include "Render.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

RenderSys rs;

HRESULT InitWindow(HINSTANCE hInst, int nCmdShow, HWND* hWnd)
{
	WNDCLASSEX wc = { 0 };
	wchar_t name[] = L"Name";

	wc.cbSize = sizeof(wc);
	wc.hInstance = hInst;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = name;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Cann't register class", L"Error", MB_OK);
		return E_FAIL;
	}

	(*hWnd) = CreateWindow(name, L"Hello World", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, (HWND)NULL, (HMENU)NULL, (HINSTANCE)hInst, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, L"Cann't create window", L"Error", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{

	HWND hWnd;
	MSG msg = { 0 };

	if (FAILED(InitWindow(hInst, nCmdShow, &hWnd)))
	{
		return 0;
	}

	if (FAILED(rs.Initialize(hWnd)))
	{
		MessageBox(NULL, L"Initialization faild! Something goes wrong.", L"Error!", MB_OK);
		rs.Release();
		return 0;
	}
	//rs.drawCubeScene();
	rs.genSphere(3);

	ShowWindow(hWnd, nCmdShow);

	try
	{
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				rs.Render();
			}
		}
	}
	catch (...)
	{
	}

	rs.Release();


	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;

	switch (uMsg)
	{
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			DestroyWindow(hWnd);
			break;
		}
		};
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
