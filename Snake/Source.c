#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define ID_TIMER 1
#define START_BUTTON 101
#define RESTART_BUTTON 102
#define SNAKE_WIDTH 10
#define MAX_SIZE 23

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
	HBRUSH hBrush = CreateSolidBrush(RGB(127, 219, 255));
	wndclass.hbrBackground = hBrush;
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

	DeleteObject(hBrush);
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
	HDC hdc;
	PAINTSTRUCT ps;

	// Lưu thông tin khung giới hạn vùng di chuyển
	static RECT rect;

	// Vùng hiện điểm
	static RECT score_rect;

	// Các điểm của rắn
	static POINT snake[MAX_SIZE];

	// Độ dài của rắn. Khởi tạo = 3
	static int size = 3;

	// Lưu điểm làm mồi
	static POINT point;

	// Hướng di chuyển
	static int direction = VK_RIGHT;

	// Cờ đánh dấu có đang chơi hay không
	static BOOL playing = FALSE;

	// Điểm số
	static int score = 0;

	// Cờ đánh dấu có phải tạo mồi mới hay không
	static BOOL newpoint = TRUE;

	// Hàng đợi hướng di chuyển.
	// Có thể bấm phím di chuyển nhanh quá, chương trình chưa kịp xử lý nên phải cho vào hàng đợi
	static int queue_dir[10];

	// Kích thước hàng đợi
	static int count_queue = 0;

	switch (message)
	{
	case WM_CREATE:

		// Tạo các nút
		CreateWindow(L"BUTTON", L"BẮT ĐẦU", WS_VISIBLE | WS_CHILD, 25, 20, 100, 24, hwnd, (HMENU)START_BUTTON, GetModuleHandle(NULL), NULL);
		CreateWindow(L"BUTTON", L"CHƠI LẠI", WS_VISIBLE | WS_CHILD, 25, 146, 100, 24, hwnd, (HMENU)RESTART_BUTTON, GetModuleHandle(NULL), NULL);

		// Khởi tạo con rắn
		snake[2].x = 230;
		snake[2].y = 120;
		snake[1].x = snake[2].x + SNAKE_WIDTH;
		snake[1].y = snake[2].y;
		snake[0].x = snake[1].x + SNAKE_WIDTH;
		snake[0].y = snake[2].y;

		// Khởi tạo vùng hiện điểm
		score_rect.left = 25;
		score_rect.top = 45;
		score_rect.right = 125;
		score_rect.bottom = 145;

	case WM_SIZE:

		// Lấy thông tin khung giới hạn vùng di chuyển
		GetClientRect(hwnd, &rect);
		rect.left = rect.left + 150;
		rect.top = rect.top + 20;
		rect.right = rect.right - 20;
		rect.bottom = rect.bottom - 20;

		// Làm tròn để rắn di chuyển đúng hàng, 
		rect.left -= (rect.left % SNAKE_WIDTH);
		rect.top -= (rect.top % SNAKE_WIDTH);
		rect.right -= (rect.right % SNAKE_WIDTH);
		rect.bottom -= (rect.bottom % SNAKE_WIDTH);

		// Vẽ lại vùng Client
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_KEYDOWN:

		// Bắt phím di chuyển khi đang chơi
		if (playing == TRUE)
		{
			// Kiểm tra phím di chuyển hợp lệ mới lưu vào hàng đợi
			if ((wParam == VK_LEFT || wParam == VK_RIGHT) && (direction == VK_LEFT || direction == VK_RIGHT))
				return 0;
			if ((wParam == VK_UP || wParam == VK_DOWN) && (direction == VK_UP || direction == VK_DOWN))
				return 0;

			// Lưu phím di chuyển vào hàng đợi
			switch (wParam)
			{
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
				queue_dir[count_queue] = wParam;
				count_queue++;
				break;
			}
		}
		return 0;

	case WM_COMMAND:

		// Bắt sự kiện bấm các nút
		switch (LOWORD(wParam))
		{
		case START_BUTTON:

			// Đánh dấu đang chơi
			playing = TRUE;
			
			// Đặt timer gửi thông điệp WM_TIMER mỗi 20ms 1 lần
			SetTimer(hwnd, ID_TIMER, 50, NULL);
			
			// SetFocus cho cửa sổ chính để nhận được các thông điệp bàn phím
			SetFocus(hwnd);
			break;

		case RESTART_BUTTON:

			// Thiết đặt lại các giá trị ban đầu
			size = 3;
			score = 0;
			playing = FALSE;
			newpoint = TRUE;
			direction = VK_RIGHT;
			count_queue = 0;

			snake[2].x = 230;
			snake[2].y = 120;
			snake[1].x = snake[2].x + SNAKE_WIDTH;
			snake[1].y = snake[2].y;
			snake[0].x = snake[1].x + SNAKE_WIDTH;
			snake[0].y = snake[2].y;

			// Vẽ lại vùng Client
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		}
		return 0;

	case WM_TIMER:
		switch (wParam)
		{
		case ID_TIMER:
		{
			// Cờ kiểm tra xem rắn "chết" hay chưa
			BOOL alive = TRUE;

			// Kiểm tra xem rắn đâm vào cạnh không
			if ((snake[0].x >= rect.right) || (snake[0].x <= rect.left) || (snake[0].y >= rect.bottom) || (snake[0].y <= rect.top))
			{
				alive = FALSE;
			}

			// Kiểm tra xem rắn có đâm vào thân hay không
			for (int i = 3; i < size; i++)
			{
				if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
				{
					alive = FALSE;
				}
			}

			// Nếu rắn "chết" thì ngừng timer và đưa ra thông báo
			if (alive == FALSE)
			{
				KillTimer(hwnd, ID_TIMER);
				MessageBoxW(hwnd, TEXT("Thua cuộc!"), TEXT("Kết quả"), 0);
				return 0;
			}

			// Kiểm tra rắn có ăn mồi hay không
			if (snake[0].x == point.x && snake[0].y == point.y)
			{
				// Nếu có thì tăng kích thước rắn
				size++;

				// Lùi các phần tử của rắn lại 1 đơn vị
				for (int i = size - 1; i > 0; i--)
				{
					snake[i] = snake[i - 1];

				}

				// Gán đầu rắn là mồi vừa ăn
				snake[0] = point;

				// Tăng điểm
				score++;

				// Gán cờ vẽ lại con mồi khác
				newpoint = TRUE;

				// Vẽ lại vùng hiển thị điểm
				InvalidateRect(hwnd, &score_rect, TRUE);

				// Nếu rắn đạt độ dài tối đa cho phép thì ngừng timer và đưa ra thông báo
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

			// Lấy hướng đi trong hàng đợi hướng đi, nếu hết rồi thì chọn hướng cuối cùng.
			if (count_queue > 0)
			{
				direction = queue_dir[0];
				count_queue--;
			}

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

			// Vẽ lại vùng rắn di chuyển
			InvalidateRect(hwnd, &rect, TRUE);
			break;
		}
		}
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		// Hiển thị điểm
		Rectangle(hdc, score_rect.left, score_rect.top, score_rect.right, score_rect.bottom);

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
		HBRUSH hBrushGreen = CreateSolidBrush(RGB(46, 204, 64));
		SelectObject(hdc, hBrushGreen);

		for (int i = 0; i < size; i++)
		{
			Rectangle(hdc, snake[i].x, snake[i].y, snake[i].x + SNAKE_WIDTH, snake[i].y + SNAKE_WIDTH);
		}

		// Kiểm tra xem có cần vẽ mồi mới
		if (newpoint == TRUE)
		{
			// Tạo nguồn để tính ra số ngẫu nghiên theo thuật toán của hàm rand()
			srand(time(NULL));

			// Chọn toạ độ ngẫu nhiên cho mồi
			while (TRUE)
			{
				point.x = random(rect.left + 150, rect.right - 20);
				point.y = random(rect.top + 20, rect.bottom - 20);

				//Kiểm tra xem thức ăn có thuộc vào rắn 
				int i = 0;
				while (i < size)
				{
					if (point.x >= snake[i].x && point.y <= snake[i].x + SNAKE_WIDTH && point.y >= snake[i].y && point.y <= snake[i].y + SNAKE_WIDTH)
						break;
					i++;
				}

				// Nếu i vượt qua độ dài của rắn thì mồi không thuộc rắn --> thoát vòng lặp
				if (i >= size) break;
			}

			// Sau khi tính được toạ độ mới thì bỏ cờ để không vẽ lại thức ăn
			newpoint = FALSE;
		}

		// Vẽ thức ăn

		HBRUSH hBrushRed = CreateSolidBrush(RGB(255, 65, 54));
		SelectObject(hdc, hBrushRed);
		Rectangle(hdc, point.x, point.y, point.x + SNAKE_WIDTH, point.y + SNAKE_WIDTH);

		DeleteObject(hBrushRed);
		DeleteObject(hBrushGreen);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}