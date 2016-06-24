#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define ID_TIMER		1
#define REFRESH_BUTTON	101
#define PAUSE_BUTTON	102
#define LO_SPEED_BUTTON 201
#define MD_SPEED_BUTTON 202
#define LG_SPEED_BUTTON 203
#define LO_SPEED		100
#define MD_SPEED		50
#define LG_SPEED		30
#define SNAKE_WIDTH		10
#define MAX_SIZE		203

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

void GetRect(HWND hwnd, LPRECT rect)
{
	// Lấy thông tin khung giới hạn vùng di chuyển và tính toán sao cho rắn đi được hết hàng và cột.
	GetClientRect(hwnd, rect);
	rect->left = 130 + SNAKE_WIDTH;
	rect->top = SNAKE_WIDTH;
	rect->right = rect->right - ((rect->right - rect->left) % SNAKE_WIDTH);
	rect->bottom = rect->bottom - (rect->bottom % SNAKE_WIDTH);
}

int random(int min, int max)
{
	int r = rand();
	int n = (max - min) / SNAKE_WIDTH;
	int rs = (r % n) * SNAKE_WIDTH + min;
	return rs;
}

POINT getNewPoint(LPRECT rect, POINT snake[], int size)
{
	// Tạo nguồn để tính ra số ngẫu nghiên theo thuật toán của hàm rand()
	srand(time(NULL));

	POINT point;
	// Chọn toạ độ ngẫu nhiên cho mồi
	while (TRUE)
	{
		point.x = random(rect->left, rect->right, SNAKE_WIDTH);
		point.y = random(rect->top, rect->bottom, SNAKE_WIDTH);

		//Kiểm tra xem thức ăn có thuộc vào rắn 
		int i = 0;
		while (i < size)
		{
			if (point.x == snake[i].x && point.y == snake[i].y)
				break;
			i++;
		}

		// Nếu i vượt qua độ dài của rắn thì mồi không thuộc rắn --> thoát vòng lặp
		if (i >= size) break;
	}

	return point;
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

	// Tốc độ
	static int speed = MD_SPEED;

	// Lưu điểm làm mồi
	static POINT point;

	// Hướng di chuyển
	static int direction = VK_RIGHT;

	// Cờ đánh dấu có đang chơi hay không
	static BOOL playing = FALSE;

	// Điểm số
	static int score = 0;

	// Hàng đợi hướng di chuyển.
	// Có thể bấm phím di chuyển nhanh quá, chương trình chưa kịp xử lý nên phải cho vào hàng đợi
	static int queue_dir[20];

	// Kích thước hàng đợi
	static int count_queue = 0;

	switch (message)
	{
	case WM_CREATE:

		// Lấy thông tin khung giới hạn vùng di chuyển
		GetRect(hwnd, &rect);

		// Tạo các nút
		CreateWindow(L"BUTTON", L"CHẠY / DỪNG", WS_VISIBLE | WS_CHILD | BS_FLAT, 25, 20, 100, 24, hwnd, (HMENU)PAUSE_BUTTON, GetModuleHandle(NULL), NULL);
		CreateWindow(L"BUTTON", L"LÀM MỚI", WS_VISIBLE | WS_CHILD | BS_FLAT, 25, 146, 100, 24, hwnd, (HMENU)REFRESH_BUTTON, GetModuleHandle(NULL), NULL);
		CreateWindow(L"BUTTON", L"CHẬM", WS_VISIBLE | WS_CHILD | BS_FLAT, 25, 180, 100, 24, hwnd, (HMENU)LO_SPEED_BUTTON, GetModuleHandle(NULL), NULL);
		CreateWindow(L"BUTTON", L"VỪA", WS_VISIBLE | WS_CHILD | BS_FLAT, 25, 210, 100, 24, hwnd, (HMENU)MD_SPEED_BUTTON, GetModuleHandle(NULL), NULL);
		CreateWindow(L"BUTTON", L"NHANH", WS_VISIBLE | WS_CHILD | BS_FLAT, 25, 240, 100, 24, hwnd, (HMENU)LG_SPEED_BUTTON, GetModuleHandle(NULL), NULL);

		// Khởi tạo con rắn
		snake[2].x = rect.left + SNAKE_WIDTH;
		snake[2].y = rect.top + SNAKE_WIDTH * 5;
		snake[1].x = snake[2].x + SNAKE_WIDTH;
		snake[1].y = snake[2].y;
		snake[0].x = snake[1].x + SNAKE_WIDTH;
		snake[0].y = snake[2].y;

		// Khởi tạo mồi
		point = getNewPoint(&rect, snake, size);

		// Khởi tạo vùng hiện điểm
		score_rect.left = 25;
		score_rect.top = 45;
		score_rect.right = 125;
		score_rect.bottom = 145;

		return 0;

	case WM_SIZE:

		// Lấy thông tin khung giới hạn vùng di chuyển
		GetRect(hwnd, &rect);

		// Vẽ lại vùng Client
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_KEYDOWN:

		// Bắt phím di chuyển khi đang chơi
		if (playing == TRUE)
		{
			int last_dir;
			if (count_queue == 0) last_dir = direction;
			else last_dir = queue_dir[count_queue - 1];
			// Kiểm tra phím di chuyển hợp lệ mới lưu vào hàng đợi
			if ((wParam == VK_LEFT || wParam == VK_RIGHT) && (last_dir == VK_LEFT || last_dir == VK_RIGHT))
				return 0;
			if ((wParam == VK_UP || wParam == VK_DOWN) && (last_dir == VK_UP || last_dir == VK_DOWN))
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
		if (playing == TRUE && LOWORD(wParam) != PAUSE_BUTTON)
		{
			MessageBoxW(hwnd, TEXT("Game chưa kết thúc!"), TEXT("Cảnh báo"), 0);
			break;
		}

		switch (LOWORD(wParam))
		{
		case PAUSE_BUTTON:

			if (playing == FALSE)
			{
				// Đánh dấu đang chơi
				playing = TRUE;

				// Đặt timer gửi thông điệp WM_TIMER để chạy tiếp game
				SetTimer(hwnd, ID_TIMER, speed, NULL);
			}
			else
			{
				// Huỷ timer để dừng game
				KillTimer(hwnd, ID_TIMER);
				playing = FALSE;
			}

			break;

		case LO_SPEED_BUTTON:
			speed = LO_SPEED;
			MessageBoxW(hwnd, TEXT("Tốc độ: Chậm!"), TEXT("Cảnh báo"), 0);
			break;

		case MD_SPEED_BUTTON:
			speed = MD_SPEED;
			MessageBoxW(hwnd, TEXT("Tốc độ: Vừa!"), TEXT("Cảnh báo"), 0);
			break;

		case LG_SPEED_BUTTON:
			speed = LG_SPEED;
			MessageBoxW(hwnd, TEXT("Tốc độ: Nhanh!"), TEXT("Cảnh báo"), 0);
			break;

		case REFRESH_BUTTON:
			if (playing == FALSE)
			{
				// Thiết đặt lại các giá trị ban đầu
				size = 3;
				score = 0;
				playing = FALSE;
				direction = VK_RIGHT;
				count_queue = 0;

				// Khởi tạo rắn
				snake[2].x = rect.left + SNAKE_WIDTH;
				snake[2].y = rect.top + SNAKE_WIDTH * 5;
				snake[1].x = snake[2].x + SNAKE_WIDTH;
				snake[1].y = snake[2].y;
				snake[0].x = snake[1].x + SNAKE_WIDTH;
				snake[0].y = snake[2].y;

				// Khởi tạo mồi
				point = getNewPoint(&rect, snake, size);

				// Vẽ lại vùng Client
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		}

		// SetFocus cho cửa sổ chính để nhận được các thông điệp bàn phím
		SetFocus(hwnd);

		return 0;

	case WM_TIMER:
		switch (wParam)
		{
		case ID_TIMER:
		{
			// Vùng cần vẽ lại
			RECT rct;
			
			// Cờ kiểm tra xem rắn "chết" hay chưa
			BOOL alive = TRUE;

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
				// Bỏ dấu cờ đang chơi
				playing = FALSE;
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

				// Vẽ lại vùng hiển thị điểm
				InvalidateRect(hwnd, &score_rect, TRUE);

				// Lấy vùng hiển thị mồi cũ
				rct.left = point.x;
				rct.top = point.y;
				rct.right = point.x + SNAKE_WIDTH + 1;
				rct.bottom = point.y + SNAKE_WIDTH + 1;

				// Vẽ lại vùng của mồi cũ để xoá đi
				InvalidateRect(hwnd, &rct, TRUE);

				// Tính lại toạ độ con mồi khác
				point = getNewPoint(&rect, snake, size);

				// Lấy vùng hiển thị mồi mới
				rct.left = point.x;
				rct.top = point.y;
				rct.right = point.x + SNAKE_WIDTH + 1;
				rct.bottom = point.y + SNAKE_WIDTH + 1;

				// Vẽ lại vùng của mồi mới để hiển thị
				InvalidateRect(hwnd, &rct, TRUE);
			}

			// Nếu rắn đạt độ dài tối đa cho phép thì ngừng timer và đưa ra thông báo
			if (size == MAX_SIZE)
			{
				playing = FALSE;
				KillTimer(hwnd, ID_TIMER);
				MessageBox(hwnd, TEXT("Rắn đã đạt độ dài tối đa cho phép. Bạn thắng!"), TEXT("Thông báo"), 0);
				return 0;
			}

			// Dịch chuyển
			// Lấy vùng hiển thị đuôi rắn
			rct.left = snake[size-1].x;
			rct.top = snake[size - 1].y;
			rct.right = snake[size - 1].x + SNAKE_WIDTH + 1;
			rct.bottom = snake[size - 1].y + SNAKE_WIDTH + 1;

			// Vẽ lại vùng đuôi rắn để xoá đi
			InvalidateRect(hwnd, &rct, TRUE);

			// Gán lại toạn độ các phần tử phía sau bằng phần tử đứng trước nó.
			for (int i = size - 1; i > 0; i--)
				snake[i] = snake[i - 1];

			// Lấy hướng đi trong hàng đợi hướng đi, nếu hết rồi thì giữ nguyên hướng trước đó.
			if (count_queue > 0)
			{
				if ((queue_dir[0] >= 37 && queue_dir[0] <= 40) &&
					!((direction == VK_LEFT || direction == VK_RIGHT) &&
					(queue_dir[0] == VK_LEFT || queue_dir[0] == VK_RIGHT)) &&
					!((direction == VK_UP || direction == VK_DOWN) &&
					(queue_dir[0] == VK_UP || queue_dir[0] == VK_DOWN)))
				{
					direction = queue_dir[0];
				}
				count_queue--;
				for (int i = 0; i < count_queue; i++) queue_dir[i] = queue_dir + 1;
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


			// Kiểm tra xem rắn đâm vào cạnh không
			if (snake[0].x >= rect.right)
			{
				snake[0].x = rect.left;
			}
			if (snake[0].x < rect.left)
			{
				snake[0].x = rect.right - SNAKE_WIDTH;
			}
			if (snake[0].y >= rect.bottom)
			{
				snake[0].y = rect.top;
			}
			if (snake[0].y < rect.top)
			{
				snake[0].y = rect.bottom - SNAKE_WIDTH;
			}

			// Lưu lại vùng hiển thị của đầu rắn
			rct.left = snake[0].x;
			rct.top = snake[0].y;
			rct.right = snake[0].x + SNAKE_WIDTH + 1;
			rct.bottom = snake[0].y + SNAKE_WIDTH + 1;

			// Vẽ lại vùng đầu rắn để hiển thị ở vị trí mới
			InvalidateRect(hwnd, &rct, TRUE);

			break;
		}
		}
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		// Hiển thị điểm
		Rectangle(hdc, score_rect.left, score_rect.top, score_rect.right + 1, score_rect.bottom + 1);

		TextOutW(hdc, 60, 50, TEXT("Điểm"), lstrlen(TEXT("Điểm")));

		HFONT hFont = CreateFont(50, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");
		SelectObject(hdc, hFont);
		
		wchar_t diem[10];
		wsprintf(diem, TEXT("%3d"), score);
		TextOutW(hdc, 35, 80, diem, lstrlen(diem));
		
		DeleteObject(hFont);

		// Vẽ khung giới hạn trò chơi
		Rectangle(hdc, rect.left, rect.top, rect.right + 1, rect.bottom + 1);

		// Vẽ rắn
		HBRUSH hBrushGreen = CreateSolidBrush(RGB(46, 204, 64));
		SelectObject(hdc, hBrushGreen);

		for (int i = 0; i < size; i++)
		{
			Rectangle(hdc, snake[i].x, snake[i].y, snake[i].x + SNAKE_WIDTH + 1, snake[i].y + SNAKE_WIDTH + 1);
		}

		// Vẽ mồi
		HBRUSH hBrushRed = CreateSolidBrush(RGB(255, 65, 54));
		SelectObject(hdc, hBrushRed);
		Rectangle(hdc, point.x, point.y, point.x + SNAKE_WIDTH + 1, point.y + SNAKE_WIDTH + 1);

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