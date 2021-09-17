//windows message loop will handle the following:
//keyboard inputs
//window manipulation redraws
//
//extra threads will draw to buffer and then wm_paint and frame display will display buffer
//
//need to be able to blit sprites
//draw line
//draw triangle
#include <windows.h>
#include <time.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Gdi32.lib")

struct keyState {
	unsigned char down : 1;
	unsigned char hold : 1;
	unsigned char up : 1;
};

struct keyState Keys[254];

void log_(const char* txt) {
	time_t time_s = time(NULL);
	printf("%s--------------\n%s\n\n", ctime(&time_s), txt);
	fflush(stdout);
}

const char g_szClassName[] = "myWindowClass";

static HRGN rectRgn;
static HBRUSH fillBrush;
HDC hdc;
HDC memhdc;
HBITMAP bitmap;
int w;
int h;

LARGE_INTEGER t1, t2;
LARGE_INTEGER freq;

int fill(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);

int gdi_cleanup(void) {
	DeleteObject(rectRgn);
	DeleteObject(fillBrush);
	DeleteObject(bitmap);
	DeleteDC(memhdc);
	DeleteDC(hdc);
	return 0;
}

long double fElapsedTime;

void update(void);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


int start(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow, int w1, int h1) {
	log_("CALLED USER START");
	//Take some parameters, like screen resolution and window title...
	//HINSTANCE hInstance;
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;
	//HANDLE hMutex;
	
	w = w1;
	h = h1;
	//Registering the Window Class
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Creating the Window
	hwnd = CreateWindowEx(
		CS_OWNDC,
		g_szClassName,
		"Win32 Render Test",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, w, h,
		NULL, NULL, hInstance, NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	
	hdc = GetDC(hwnd);
	memhdc = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, w, h);
	SelectObject(memhdc, bitmap);
	
	QueryPerformanceFrequency(&freq);
	printf("freq: %lli\n", freq.QuadPart);
	freq.QuadPart /= 1000;
	QueryPerformanceCounter(&t1);
	
	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
	return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//Need to allow the user to modify this callback to allow for their own message handling..?
    switch(msg)
    {
		case WM_TIMER:
			QueryPerformanceCounter(&t2);	
			fElapsedTime = (t2.QuadPart - t1.QuadPart);
			fElapsedTime /= 1000;
			fElapsedTime /= freq.QuadPart;
			t1.QuadPart = t2.QuadPart;
			//printf("TIME: %Lf\n", fElapsedTime);
			//fflush(stdout);
			update();
			break;
			
		case WM_KEYDOWN:
			if (Keys[wParam].down) Keys[wParam].hold = 1;			
			else Keys[wParam].down = 1;			
			return 0;
			
		case WM_KEYUP:
			Keys[wParam].down = 0;
			Keys[wParam].hold = 0;
			Keys[wParam].up = 1;
			return 0;
			
		case WM_CLOSE:
			if (gdi_cleanup()) printf("gdi cleanup failure!");
            DestroyWindow(hwnd);
			break;
        case WM_DESTROY:
            PostQuitMessage(0);
			break;
		case WM_CREATE:
			SetTimer(hwnd, 1, 0, NULL);
			break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }	
    return 0;
}

int fill(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b) {	
	rectRgn = CreateRectRgn(x1, y1, x2, y2);
	fillBrush = CreateSolidBrush(RGB(r, g, b));
	FillRgn(memhdc, rectRgn, fillBrush);
	DeleteObject(rectRgn);
	DeleteObject(fillBrush);
	return 1;
}

void blit() {
	BitBlt(hdc, 0, 0, w, h, memhdc, 0, 0, SRCCOPY);
}

