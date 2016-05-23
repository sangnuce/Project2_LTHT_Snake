#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define ID_TIMER 1
#define START_BUTTON 101
#define RESTART_BUTTON 102
#define SNAKE_WIDTH 5
#define MAX_SIZE 200

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("Snake");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,	// window class name
		TEXT("Rắn săn mồi"),		// window caption
		WS_OVERLAPPEDWINDOW,		// window style
		CW_USEDEFAULT,              // initial x position
		CW_USEDEFAULT,              // initial y position
		CW_USEDEFAULT,              // initial x size
		CW_USEDEFAULT,              // initial y size
		NULL,                       // parent window handle
		NULL,						// window menu handle
		hInstance,					// program instance handle
		NULL);						// creation parameters
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

int random(int min, int max)
{
	int r = rand();
	int rs = r % (max - min) + min;
	rs = rs - (rs % SNAKE_WIDTH);
	if (rs < min) rs += SNAKE_WIDTH;
	if (rs + SNAKE_WIDTH > max) rs -= SNAKE_WIDTH;
	return rs;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static RECT rect;
	HDC hdc;
	PAINTSTRUCT ps;
	static POINT snake[MAX_SIZE];
	static POINT point;
	static int size = 3;
	static int direction = VK_RIGHT;
	static BOOL playing = FALSE;
	static int score = 0;
	static BOOL newpoint = TRUE;

	switch (message)
	{
	case WM_CREATE:
		CreateWindow(L"BUTTON", L"BẮT ĐẦU", WS_VISIBLE | WS_CHILD, 25, 20, 100, 24, hwnd, (HMENU)START_BUTTON, GetModuleHandle(NULL), NULL);
		CreateWindow(L"BUTTON", L"CHƠI LẠI", WS_VISIBLE | WS_CHILD, 25, 150, 100, 24, hwnd, (HMENU)RESTART_BUTTON, GetModuleHandle(NULL), NULL);
		snake[2].x = 230;
		snake[2].y = 120;
		snake[1].x = snake[2].x + SNAKE_WIDTH;
		snake[1].y = snake[2].y;
		snake[0].x = snake[1].x + SNAKE_WIDTH;
		snake[0].y = snake[2].y;

	case WM_SIZE:
		GetClientRect(hwnd, &rect);
		rect.left = rect.left + 150 - (rect.left % SNAKE_WIDTH);
		rect.top = rect.top + 20 - (rect.top % SNAKE_WIDTH);
		rect.right = rect.right - 20 - (rect.right % SNAKE_WIDTH);
		rect.bottom = rect.bottom - 20 - (rect.bottom % SNAKE_WIDTH);

		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_KEYDOWN:
		if (playing == TRUE)
		{
			if ((wParam == VK_LEFT || wParam == VK_RIGHT) && (direction == VK_LEFT || direction == VK_RIGHT))
				return 0;
			if ((wParam == VK_UP || wParam == VK_DOWN) && (direction == VK_UP || direction == VK_DOWN))
				return 0;

			switch (wParam)
			{
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
				direction = wParam;
				break;
			}
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case START_BUTTON:
			playing = TRUE;
			SetTimer(hwnd, ID_TIMER, 20, NULL);
			
			// SetFocus cho cửa sổ chính để nhận các thông điệp bàn phím
			SetFocus(hwnd);
			break;
		case RESTART_BUTTON:
			size = 3;
			score = 0;
			playing = FALSE;
			newpoint = TRUE;
			direction = VK_RIGHT;

			snake[2].x = 230;
			snake[2].y = 120;
			snake[1].x = snake[2].x + SNAKE_WIDTH;
			snake[1].y = snake[2].y;
			snake[0].x = snake[1].x + SNAKE_WIDTH;
			snake[0].y = snake[2].y;

			InvalidateRect(hwnd, NULL, TRUE);
			break;
		}
		return 0;

	case WM_TIMER:
		switch (wParam)
		{
		case ID_TIMER:
		{
			BOOL alive = TRUE;

			// Kiểm tra xem rắn đâm vào cạnh không
			if ((snake[0].x + SNAKE_WIDTH >= rect.right) || (snake[0].x <= rect.left) || (snake[0].y + SNAKE_WIDTH >= rect.bottom) || (snake[0].y <= rect.top))
			{
				alive = FALSE;
			}

			// Kiểm tra xem rắn có đâm vào thân hay không

			for (int i = 2; i < size; i++)
			{
				if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
				{
					alive = FALSE;
				}
			}

			if (alive == FALSE)
			{
				KillTimer(hwnd, ID_TIMER);
				MessageBoxW(hwnd, TEXT("Thua cuộc!"), TEXT("Kết quả"), 0);
				return 0;
			}

			// Kiểm tra ăn điểm
			if (snake[0].x == point.x && snake[0].y == point.y)
			{
				size++;
				for (int i = size - 1; i > 0; i--)
				{
					snake[i] = snake[i - 1];

				}
				snake[0] = point;
				score++;
				newpoint = TRUE;
				if (size == MAX_SIZE)
				{
					KillTimer(hwnd, ID_TIMER);
					MessageBox(hwnd, TEXT("Ô kê, diu guyn. Rắn dài quá không cho chơi nữa!"), TEXT("Chúc mừng con nhợn!"), 0);
					return 0;
				}
			}

			// Dịch chuyển

			for (int i = size - 1; i > 0; i--)
				snake[i] = snake[i - 1];

			switch (direction)
			{
			case VK_LEFT:
				snake[0].x -= SNAKE_WIDTH;
				break;
			case VK_UP:
				snake[0].y -= SNAKE_WIDTH;
				break;
			case VK_RIGHT:
				snake[0].x += SNAKE_WIDTH;
				break;
			case VK_DOWN:
				snake[0].y += SNAKE_WIDTH;
				break;
			}

			InvalidateRect(hwnd, NULL, TRUE);
			break;
		}
		}
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		// Hiển thị điểm
		Rectangle(hdc, 25, 45, 125, 145);

		SetBkMode(hdc, TRANSPARENT);
		TextOutW(hdc, 60, 50, TEXT("Điểm"), lstrlen(TEXT("Điểm")));

		HFONT hFont = CreateFont(50, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");
		SelectObject(hdc, hFont);
		
		wchar_t diem[10];
		wsprintf(diem, TEXT("%3d"), score);
		TextOutW(hdc, 35, 80, diem, lstrlen(diem));
		
		DeleteObject(hFont);

		// Vẽ khung giới hạn trò chơi
		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

		// Vẽ rắn
		HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
		SelectObject(hdc, hBrush);
		for (int i = 0; i < size; i++)
		{
			Rectangle(hdc, snake[i].x, snake[i].y, snake[i].x + SNAKE_WIDTH, snake[i].y + SNAKE_WIDTH);
		}

		// Kiểm tra xem có cần tạo mới thức ăn
		if (newpoint == TRUE)
		{
			srand(time(NULL));
			BOOL check = TRUE;

			// Chọn toạ độ ngẫu nhiên cho thức ăn
			while (check)
			{
				point.x = random(rect.left + 150, rect.right - 20);
				point.y = random(rect.top + 20, rect.bottom - 20);

				//Kiểm tra xem thức ăn có bị dính vào rắn không
				int i = 0;
				while (i < size)
				{
					if (point.x >= snake[i].x && point.y <= snake[i].x + SNAKE_WIDTH && point.y >= snake[i].y && point.y <= snake[i].y + SNAKE_WIDTH)
						break;
					i++;
				}

				// Gắn cờ kiểm tra
				if (i < size) check = TRUE;
				else check = FALSE;
			}

			// Sau khi tính được toạ độ mới thì bỏ cờ để không vẽ lại thức ăn
			newpoint = FALSE;
		}

		// Vẽ thức ăn
		Rectangle(hdc, point.x, point.y, point.x + SNAKE_WIDTH, point.y + SNAKE_WIDTH);

		DeleteObject(hBrush);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}