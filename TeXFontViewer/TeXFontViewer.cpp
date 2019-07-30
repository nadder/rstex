// TeXFontViewer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TeXFontViewer.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
TBBUTTON tbButtons[12];
HIMAGELIST ghImgList;
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HWND CreateToolbar(HWND hWndParent);
HWND CreateStatusBar(HWND hwndParent, int idStatus, HINSTANCE hinst);
void read_pk_file(wchar_t const * filename);
bool wstr_ends_with(const wchar_t * str, const wchar_t * suffix);

void set_image_raster_bit(bool set, int bit_offset, eight_bits *raster);
bool get_image_raster_bit(int bit_offset, eight_bits *raster);
void MyCreateImageList();
BOOL OnMouseWheel(HWND hwnd, UINT nFlags, short zDelta, POINT pt);
void OnHScroll(HWND hwnd, UINT nSBCode, UINT nPos);
void OnVScroll(HWND hwnd, UINT nSBCode, UINT nPos);
void OnSize(HWND hwnd, int cx, int cy);
HWND g_hwndToolbar;
HWND g_hwndStatusbar;
HWND g_hwnd;
HWND hwndView;
#define ID_STATUS_BAR 1106 // we create status bar manually, could perhaps be a resourse instead
RECT usableClientRect;
int cur_char = -1;
extern char_raster_info char_info[256];
extern eight_bits *image_raster[256];
extern int num_chars;
void render_raster(HDC hdc, int x, int y);
void zoom_raster(int zoom_factor, int width, int height, eight_bits *raster, eight_bits **out_raster);
void UpdateStatusBar();
int zoom_factor = 4;
bool show_chardx;
bool show_charwd;
bool show_ref;
bool show_grid;

int yMaxScroll;
int xMaxScroll;
int yCurrentScroll;
int xCurrentScroll;
int yMinScroll;
int xMinScroll;

int orgPointX = 0;
int orgPointY = 0;
int horz_margin = 50;
int vert_margin = 50;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TEXFONTVIEWER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEXFONTVIEWER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEXFONTVIEWER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TEXFONTVIEWER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hwnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


void HandleZoom(int z)
{
	if (z < 1) z = 1;
	if (z > 20) z = 20;

	if (zoom_factor != z) {
		zoom_factor = z;
		UpdateStatusBar();
		RECT rc;
		GetClientRect(hwndView, &rc);
		OnSize(hwndView, rc.right, rc.bottom);
		InvalidateRect(hwndView, NULL, TRUE);
	}
}

void HandleChar(WPARAM wParam)
{
	if (num_chars > 0 && (wParam == 'n' || wParam == 'N')) {
		cur_char++;
		if (wParam == 'N')
			cur_char -= 2;
		if (cur_char >= num_chars) cur_char = 0;
		if (cur_char < 0) cur_char += num_chars;
		wchar_t buffer[256];
		_swprintf(buffer, L"Char (index:%d, code:%d)", cur_char, char_info[cur_char].code);
		SendMessage(g_hwndStatusbar, SB_SETTEXT, 1 | (SBT_NOBORDERS << 8), (LPARAM)buffer);
		UpdateStatusBar();

		InvalidateRect(hwndView, NULL, TRUE);
	}
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		if (GetWindowLong(hWnd, GWL_HWNDPARENT) == NULL) {
			INITCOMMONCONTROLSEX cmnex = {0};
			cmnex.dwSize = sizeof cmnex;
			cmnex.dwICC = ICC_WIN95_CLASSES|ICC_COOL_CLASSES;
			InitCommonControlsEx(&cmnex);
			g_hwndToolbar = CreateToolbar(hWnd);
			g_hwndStatusbar = CreateStatusBar(hWnd,ID_STATUS_BAR, hInst);
			UpdateStatusBar();

			const wchar_t *class_name = L"view_window";
			WNDCLASS wc = {0};
			wc.lpfnWndProc = WndProc;
			wc.hInstance = hInst;
			wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.lpszMenuName = NULL;
			wc.lpszClassName = class_name;
			RegisterClass(&wc);

			hwndView =CreateWindowEx(0, class_name, L"view_window", 
			WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			hWnd, NULL, hInst, NULL);
			ShowWindow(hwndView, SW_SHOW);

		}
		break;

	case WM_MOUSEWHEEL: {
		if (hWnd == hwndView) {
			WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
			if (fwKeys == MK_CONTROL) {
				short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				int old_zoom_factor = zoom_factor;

				HandleZoom(zoom_factor + zDelta / 120);


			}
			else {
				POINT pt;
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);
				OnMouseWheel(hWnd, GET_KEYSTATE_WPARAM(wParam),
					GET_WHEEL_DELTA_WPARAM(wParam), pt);
			}
		}
		break;

		case WM_VSCROLL:
			if (hWnd == hwndView)
			{
				OnVScroll(hWnd, LOWORD(wParam), HIWORD(wParam));

			}
			break;

		case WM_HSCROLL:
			if (hWnd == hwndView)
			{
				OnHScroll(hWnd, LOWORD(wParam), HIWORD(wParam));

 			}
			break;


	}
    case WM_COMMAND:
		if (hWnd != hwndView)
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

			case IDC_SHOW_CHARDX:
				show_chardx = !show_chardx;
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			case IDC_SHOW_CHARWD:
				show_charwd = !show_charwd;
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			case IDC_SHOW_REF:
				show_ref = !show_ref;
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			case IDC_SHOW_GRID:
				show_grid = !show_grid;
				InvalidateRect(hWnd, NULL, TRUE);
				break;

			case IDC_RIGHT: 
			case IDC_LEFT: 
			case IDC_UP: 
			case IDC_DOWN: 
			{

				switch(wmId) {
					case IDC_RIGHT: horz_margin += 10; break;
					case IDC_LEFT: horz_margin -= 10; break;
					case IDC_UP: vert_margin -= 10; break;
					case IDC_DOWN: vert_margin += 10; break;
				}

				RECT rc;
				GetClientRect(hwndView, &rc);
				OnSize(hwndView, rc.right, rc.bottom);
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			}
			case IDC_ZOOMIN:
			case IDC_ZOOMOUT:
				HandleZoom(wmId == IDC_ZOOMIN ? zoom_factor+1 : zoom_factor - 1);
				break;

			case IDC_NEXTCHAR:
				HandleChar('n');
				break;
			case IDC_PREVCHAR:
				HandleChar('N');
				break;

			case IDM_OPEN: {
					wchar_t the_filename[1024] = {0};
					OPENFILENAME ofn = {0};
					ofn.lStructSize = sizeof ( ofn );
					ofn.hwndOwner = hWnd  ;
					ofn.lpstrFile = the_filename ;
					ofn.nMaxFile = sizeof the_filename  / sizeof *the_filename;
					ofn.lpstrFilter = L"Fonts (gf,pk,mf)\0*.*gf;*.*pk;*.mf\0"
						              "All Files (*.*)\0*.*\0";
					ofn.nFilterIndex =1;
					ofn.lpstrFileTitle = NULL ;
					ofn.nMaxFileTitle = 0 ;
					ofn.lpstrInitialDir=NULL ;
					ofn.Flags = 0 ;
					BOOL ret = GetOpenFileName(&ofn);
					if (ret) 
					{
						// if user selects mf file we try to run mf and gftopk and then open the pk file
						bool mf_file = wstr_ends_with(the_filename, L".mf") || wstr_ends_with(the_filename, L".MF");
						bool gf_file = wstr_ends_with(the_filename, L"gf") || wstr_ends_with(the_filename, L"GF");
						if (mf_file || gf_file) {
							wchar_t cmd[512];
							if (mf_file) {
								
								_swprintf(cmd, L"mf \"\\mode=localfont; input %s\"", the_filename);
								_wsystem(cmd);

								the_filename[wcslen(the_filename)-2] = 0;
								wcscat(the_filename, L"600gf");
							}
							wchar_t out_name[512];
							wcscpy(out_name, the_filename);
							out_name[wcslen(out_name)-2] = 0;
							wcscat(out_name, L"pk");
							_swprintf(cmd, L"gftopk %s %s", the_filename, out_name);
							_wsystem(cmd);
							wcscpy(the_filename, out_name);
							
						}

						if (wstr_ends_with(the_filename, L"pk") || wstr_ends_with(the_filename, L"PK"))
						{
							cur_char = -1;
							try {
								read_pk_file(the_filename);
							}
							catch(...) {
								break;
							}
							wchar_t buf[512];
							_swprintf(buf, L"%s - %s", szTitle, the_filename);
							SetWindowText(hWnd, buf);
							if (num_chars > 0)
								cur_char = 0;
							UpdateStatusBar();
							RECT rc;
							GetClientRect(hwndView, &rc);
							OnSize(hwndView, rc.right, rc.bottom);
							InvalidateRect(hWnd, NULL, TRUE);
						}
					}
				break;
			}
			
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

	case WM_NOTIFY: 
	if (hWnd != hwndView){
			switch (((LPNMHDR)lParam)->code) 
			{
        case TBN_GETBUTTONINFO:  
            {
                LPTBNOTIFY lpTbNotify = (LPTBNOTIFY)lParam;

                // Pass the next button from the array. There is no need to filter out buttons
                // that are already used—they will be ignored.
                
                int buttonCount = sizeof(tbButtons) / sizeof(TBBUTTON);
                
                if (lpTbNotify->iItem < buttonCount)
                {
                    lpTbNotify->tbButton = tbButtons[lpTbNotify->iItem];
                    return TRUE;
                }
                
                else
                
                {
                    return FALSE;  // No more buttons.
                }
            }
            
            break;       

			case TBN_QUERYINSERT:
            
			case TBN_QUERYDELETE:
				return TRUE; 
			}
		}
		break;

	case WM_SIZE: 
		if (hWnd != hwndView)
		{
			SendMessage(g_hwndToolbar, TB_AUTOSIZE, 0, 0);
			RECT toolbarRect;
			GetWindowRect(g_hwndToolbar, &toolbarRect);

			RECT rc;
			SendMessage(g_hwndStatusbar, message, 0, 0);
			GetClientRect(g_hwndStatusbar, &rc);
			long status_bar_height = rc.bottom;
			long status_bar_width = rc.right;
			long tool_bar_height = toolbarRect.bottom - toolbarRect.top;
			long tool_bar_width = rc.right;
			usableClientRect.left = 0;
			usableClientRect.right = LOWORD(lParam);
			usableClientRect.top = tool_bar_height;
			usableClientRect.bottom = HIWORD(lParam) - status_bar_height;
			// size child window

			MoveWindow(hwndView, usableClientRect.left, usableClientRect.top, LOWORD(lParam),HIWORD(lParam) - status_bar_height - tool_bar_height, TRUE);
		}

		if (hWnd == hwndView)
		{
			int cxClient = LOWORD(lParam);
			int cyClient = HIWORD(lParam);
			
			OnSize(hWnd, cxClient, cyClient);


		}

		break;

	case WM_CHAR:
		HandleChar(wParam);

		break;
    case WM_PAINT:
        {
			if (hWnd == hwndView)
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				//SetViewportOrgEx(hdc, -xCurrentScroll, -yCurrentScroll, NULL);
				        // Get vertical scroll bar position.
				FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1));
				if (num_chars > 0) {

					render_raster(hdc, orgPointX+horz_margin-xCurrentScroll, orgPointY+vert_margin-yCurrentScroll);
				}
				EndPaint(hWnd, &ps);
				int k = 10;
				k++;
			}
			else {
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
        }
        break;


    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void OnSize(HWND hwnd, int cx, int cy)
{
	int charRectWidth = char_info[cur_char].width*zoom_factor;
	int charRectHeight = char_info[cur_char].height*zoom_factor;

	yMaxScroll = max(charRectHeight + 2*vert_margin- cy, 0);
	yCurrentScroll = min(yCurrentScroll, yMaxScroll);
	SCROLLINFO si = {0};
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = yMinScroll;
	si.nMax = charRectHeight + 2*vert_margin;
	si.nPage = cy;
	si.nPos = yCurrentScroll;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);


	xMaxScroll = max(charRectWidth+horz_margin*2 - cx, 0);
	xCurrentScroll = min(xCurrentScroll, xMaxScroll);
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = xMinScroll;
	si.nMax = charRectWidth+horz_margin*2;
	si.nPage = cx;
	si.nPos = xCurrentScroll;
	SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


HWND CreateStatusBar(HWND hwndParent, int idStatus, HINSTANCE hinst)
{
	HWND hwndStatus;
	RECT rcClient;
	HLOCAL hloc;
	PINT paParts;
	int cParts=6;
	InitCommonControls();

	hwndStatus = CreateWindowEx(
		0,
		STATUSCLASSNAME,
		NULL,
		SBARS_SIZEGRIP |
		WS_CHILD | WS_VISIBLE,
		0,0,0,0,
		hwndParent,
		(HMENU) idStatus,
		hinst,
		NULL);

		GetClientRect(hwndParent, &rcClient);
		hloc = LocalAlloc(LHND, sizeof(int) * cParts);
		paParts = (PINT)LocalLock(hloc);

		paParts[0] = 100;
		paParts[1] = 260;
		paParts[2] = 550;
		paParts[3] = 700;
		paParts[4] = 900;
		paParts[5] = 1100;

		SendMessage(hwndStatus, SB_SETPARTS, (WPARAM)cParts, (LPARAM)
				paParts);

		LocalUnlock(hloc);
		LocalFree(hloc);
		return hwndStatus;
}

void UpdateStatusBar()
{
	wchar_t buffer[512] = {0};

	if (cur_char >= 0) {
		_swprintf(buffer, L"Width (tfm pxl:%.2f, tfm pt:%.2f, dx:%.2f)", char_info[cur_char].tfm_width, char_info[cur_char].tfm_width/char_info[cur_char].hppp, char_info[cur_char].horz_esc);
		SendMessage(g_hwndStatusbar, SB_SETTEXT, 2 | (SBT_NOBORDERS << 8), (LPARAM)buffer);

		_swprintf(buffer, L"Char (index:%d, code:%d)", cur_char, char_info[cur_char].code);
		SendMessage(g_hwndStatusbar, SB_SETTEXT, 1 | (SBT_NOBORDERS << 8), (LPARAM)buffer);

		_swprintf(buffer, L"Res ppi: %.2f", char_info[cur_char].hppp * 72.27);
		SendMessage(g_hwndStatusbar, SB_SETTEXT, 3 | (SBT_NOBORDERS << 8), (LPARAM)buffer);

		_swprintf(buffer, L"designsize pt: %.2f", char_info[cur_char].design_size);
		SendMessage(g_hwndStatusbar, SB_SETTEXT, 4 | (SBT_NOBORDERS << 8), (LPARAM)buffer);

		_swprintf(buffer, L"xoff:%d, yoff:%d", char_info[cur_char].x_off, char_info[cur_char].y_off);
		SendMessage(g_hwndStatusbar, SB_SETTEXT, 5 | (SBT_NOBORDERS << 8), (LPARAM)buffer);


	}

	_swprintf(buffer, L"zoom: %.0f%%", zoom_factor*100.0);
	SendMessage(g_hwndStatusbar, SB_SETTEXT, 0 | (SBT_NOBORDERS << 8), (LPARAM)buffer);




}



HIMAGELIST g_hImageList = NULL;

HWND CreateToolbar(HWND hWndParent)
{



    // Create the toolbar.
    HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 
                                      WS_CHILD | TBSTYLE_WRAPABLE, 0, 0, 0, 0,
                                      hWndParent, NULL, hInst, NULL);
    if (hWndToolbar == NULL)
        return NULL;

	MyCreateImageList();

	SendMessage(hWndToolbar, TB_SETIMAGELIST, 
        (WPARAM)0, 
        (LPARAM)ghImgList);



	tbButtons[0] = { MAKELONG(0, 0), IDC_SHOW_CHARDX,  TBSTATE_ENABLED, BTNS_AUTOSIZE|BTNS_CHECK, {0}, 0, (INT_PTR)L"chardx" };
	tbButtons[1] = { MAKELONG(1, 0), IDC_SHOW_CHARWD, TBSTATE_ENABLED, BTNS_AUTOSIZE|BTNS_CHECK, {0}, 0, (INT_PTR)L"charwd"};
	tbButtons[2] = { MAKELONG(2, 0), IDC_SHOW_REF, TBSTATE_ENABLED, BTNS_AUTOSIZE|BTNS_CHECK, {0}, 0, (INT_PTR)L"ref"};
	tbButtons[3] = { MAKELONG(3, 0), IDC_SHOW_GRID, TBSTATE_ENABLED, BTNS_AUTOSIZE|BTNS_CHECK, {0}, 0, (INT_PTR)L"grid"};
	tbButtons[4] = { MAKELONG(4, 0), IDC_RIGHT, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"right"};
	tbButtons[5] = { MAKELONG(5, 0), IDC_UP, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"up"};
	tbButtons[6] = { MAKELONG(6, 0), IDC_DOWN, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"down"};
	tbButtons[7] = { MAKELONG(7, 0), IDC_LEFT, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"left"};
	tbButtons[8] = { MAKELONG(8, 0), IDC_ZOOMIN, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"zoom in"};
	tbButtons[9] = { MAKELONG(9, 0), IDC_ZOOMOUT, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"zoom out"};
	tbButtons[10] = { MAKELONG(10, 0), IDC_NEXTCHAR, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"Next char (n)"};
	tbButtons[11] = { MAKELONG(11, 0), IDC_PREVCHAR, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"Prev char (N)"};


    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS,       (WPARAM)sizeof tbButtons / sizeof *tbButtons,       (LPARAM)tbButtons);


    // Resize the toolbar, and then show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0); 
    ShowWindow(hWndToolbar,  TRUE);
    
    return hWndToolbar;
}

/*  returns 1 iff str ends with suffix  */
bool wstr_ends_with(const wchar_t * str, const wchar_t * suffix) 
{

  if( str == NULL || suffix == NULL )
    return false;

  size_t str_len = wcslen(str);
  size_t suffix_len = wcslen(suffix);

  if(suffix_len > str_len)
    return false;

  return 0 == wcsncmp( str + str_len - suffix_len, suffix, suffix_len );
}



void draw_grid(HDC hdc, int start_x, int start_y, int end_x, int end_y, int grid_spacing)
{
	if (grid_spacing < 4) return;

	HPEN ltGrayPen = CreatePen(PS_SOLID, 1, RGB(220,220,220));
	HPEN penOld = SelectPen(hdc, ltGrayPen);

	int cur_y = start_y;
	while (cur_y <= end_y) {
		MoveToEx(hdc, start_x, cur_y, NULL);
		LineTo(hdc, end_x+1, cur_y);
		cur_y += grid_spacing;
	}

	int cur_x = start_x;
	while (cur_x <= end_x) {
		MoveToEx(hdc, cur_x, start_y, NULL);
		LineTo(hdc, cur_x, end_y+1);
		cur_x += grid_spacing;
	}
	SelectPen(hdc, penOld);
	DeletePen(ltGrayPen);
}


void render_raster(HDC hdc, int x, int y)
{
	// draws raster's upper left corner at x,y
	
	
	int width = char_info[cur_char].width;
	int height = char_info[cur_char].height;
	if (width == 0 && height == 0)
		return; // empty raster
	unsigned char *praster = image_raster[cur_char];
	int x_off = char_info[cur_char].x_off;
	int y_off = char_info[cur_char].y_off;

	// zoom bitmap
	
	eight_bits *zoomed_buf;
	zoom_raster(zoom_factor, width, height, praster, &zoomed_buf);
	width *= zoom_factor;
	height *= zoom_factor;

	// monochrome bitmap needs to have each scan line word aligned
	const int nfill = (16 - width % 16)%16;
	unsigned char *bitmap_buf = zoomed_buf;
	
	if (nfill > 0) {
		const int new_size = 2*((width+15)/16)*height;
		char unsigned * const new_buf = (unsigned char *)malloc(new_size);
		for (int yy = 0; yy < height; yy++) {
			for (int xx = 0; xx < width; xx++) {
				bool bitval = get_image_raster_bit(xx + yy*width, zoomed_buf);
				set_image_raster_bit(bitval, xx + (width+nfill)*yy, new_buf);
			}
			for (int pp = 0; pp < nfill; pp++) { // fill with zeros to make the row multiple of 16
				set_image_raster_bit(0, (width+nfill)*yy + width + pp, new_buf);
			}
		}
		free(zoomed_buf);
		bitmap_buf = new_buf;
	}
	

	HBITMAP hBm = CreateBitmap(width,height,1,1,bitmap_buf); // monochrome bitmap, each row is filled with zeros to make it aligned to 16-bits
	                                                      // leftmost pixel corresponds to most significant bit in first byte
	HDC hdcMem = CreateCompatibleDC(hdc);

	int up_left_x = x;
	int up_left_y = y;
	int origin_x = up_left_x + x_off*zoom_factor;
	int origin_y = up_left_y + (y_off+1)*zoom_factor;

	//RECT clRect;
	//GetClientRect(hwndView, &clRect);
	//int cx = clRect.right;
	//int cy = clRect.bottom;

	//int up_left_x = x;
	//int up_left_y = cy - y - height;

	//int origin_x = up_left_x + x_off*zoom_factor;
	//int origin_y = up_left_y + (y_off+1)*zoom_factor;


	HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBm);
	SetTextColor(hdc, RGB(255,255,255));
	SetBkColor(hdc, RGB(0,0,0)); // 1-bit becomes black color
	//BitBlt(hdc, x, y, width, height, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdc, up_left_x, up_left_y, width, height, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOld);
	DeleteObject(hBm);
	DeleteObject(hdcMem);
	free(bitmap_buf);


	// draw marker tfm_width to the right of origin
	int ppx = round(origin_x + char_info[cur_char].tfm_width*zoom_factor);
	int ppy = origin_y;
	int ppx_esc = round(origin_x + char_info[cur_char].horz_esc*zoom_factor);
	bool width_diff = char_info[cur_char].tfm_width - char_info[cur_char].horz_esc > 0.5 || char_info[cur_char].tfm_width - char_info[cur_char].horz_esc < -0.5;

	// grid
	if (show_grid)
		draw_grid(hdc, up_left_x, up_left_y, up_left_x + char_info[cur_char].width *zoom_factor, up_left_y + char_info[cur_char].height*zoom_factor, zoom_factor);



	HPEN hPen, hPenOld;
	if (show_chardx) {
		SelectBrush(hdc, GetStockBrush(GRAY_BRUSH));

		COLORREF escColor = RGB(0,0,0);
		hPen = CreatePen(PS_SOLID,2, escColor);		
		hPenOld = SelectPen(hdc, hPen);

		MoveToEx(hdc, origin_x, up_left_y, NULL);
		LineTo(hdc, ppx_esc, up_left_y);
		LineTo(hdc, ppx_esc, up_left_y + char_info[cur_char].height*zoom_factor);
		LineTo(hdc, origin_x, up_left_y + char_info[cur_char].height*zoom_factor);
		LineTo(hdc, origin_x, up_left_y);
		SelectPen(hdc, hPenOld);
		DeletePen(hPen);
	}
	// draw outline to show tfm width as well
	if (show_charwd) {
		COLORREF penColor;
		SetBkMode(hdc, TRANSPARENT);		
		penColor = RGB(30,220,10);
		hPen = CreatePen(PS_DOT,0, penColor);	
		hPenOld = SelectPen(hdc, hPen);
		MoveToEx(hdc, origin_x, up_left_y, NULL);
		LineTo(hdc, ppx, up_left_y);
		LineTo(hdc, ppx, up_left_y + char_info[cur_char].height*zoom_factor);
		LineTo(hdc, origin_x, up_left_y + char_info[cur_char].height*zoom_factor);
		LineTo(hdc, origin_x, up_left_y);
		SelectPen(hdc, hPenOld);
		DeletePen(hPen);
	}

	// draw origin
	if (show_ref) {
		HBRUSH hbrRed = CreateSolidBrush(RGB(255,0,0));
		int circleWidth = 1*zoom_factor;
		if (circleWidth < 7) circleWidth = 7;
		if (circleWidth > 17) circleWidth = 17;
		if (circleWidth % 2 == 0) circleWidth++;

		HBRUSH hbbrOld = SelectBrush(hdc, hbrRed);
		SelectPen(hdc, GetStockObject(NULL_PEN));
		Ellipse(hdc, origin_x - circleWidth / 2, origin_y - circleWidth / 2, 
			origin_x + circleWidth / 2 + 1, origin_y + circleWidth / 2 + 1);
		SelectBrush(hdc, hbbrOld);
		DeleteObject(hbrRed);

	}

}

void MyCreateImageList()
{
	HBITMAP hbmImage = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
	HIMAGELIST hImgList = ImageList_Create(30,35, ILC_MASK|ILC_COLOR8, sizeof tbButtons / sizeof *tbButtons,0);
	ImageList_AddMasked(hImgList, hbmImage, RGB(255,255,255));

	ghImgList = hImgList;
}



void OnVScroll(HWND hwnd, UINT nSBCode, UINT nPos)
{
		RECT rc;
		SCROLLINFO si = {0};
		int iVertPos;
		int yNewPos;
		int yDelta;
		int xDelta = 0;
		GetClientRect(hwnd, &rc);
		//rc.bottom -= status_bar_height;
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVertPos = si.nPos;
		switch (nSBCode) {
			case SB_TOP:
				si.nPos = si.nMin;
				break;

			case SB_BOTTOM:
				si.nPos = si.nMax;
				break;

			case SB_LINEUP:
				// Up arrow button on scrollbar was pressed.
				yNewPos = yCurrentScroll - 20;
				break;

			case SB_LINEDOWN:
				// Down arrow button on scrollbar was pressed.
				yNewPos = yCurrentScroll + 20;
				break;

			case SB_PAGEUP:
				// User clicked inbetween up arrow and thumb.
				yNewPos = yCurrentScroll - 100;
				break;

			case SB_PAGEDOWN:
				// User clicked inbetween thumb and down arrow.
				yNewPos = yCurrentScroll + 100;
				break;

			case SB_THUMBTRACK:
			case SB_THUMBPOSITION:
				// Scrollbar thumb is being dragged.
				yNewPos = nPos;
				break;

			default:
				yNewPos = yCurrentScroll;

		}

		yNewPos = max(0, yNewPos);
		yNewPos = min(yMaxScroll, yNewPos);
		si.fMask = SIF_POS;

		if (yNewPos == yCurrentScroll)
			return;

		yDelta = yNewPos - yCurrentScroll;

		yCurrentScroll = yNewPos;
		ScrollWindowEx(hwnd, -xDelta, -yDelta, &rc, NULL, NULL, NULL, SW_INVALIDATE);
		UpdateWindow(hwnd);
		InvalidateRect(g_hwndStatusbar, NULL, TRUE);
		si.cbSize = sizeof si;
		si.fMask = SIF_POS;
		si.nPos = yCurrentScroll;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void OnHScroll(HWND hwnd, UINT nSBCode, UINT nPos)
{
		int xDelta;
		int xNewPos;
		int yDelta = 0;
		RECT rc;
		SCROLLINFO si = {0};
		int iHorzPos;
		GetClientRect(hwnd, &rc);
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;
		switch (nSBCode) {
			// User clicked the scroll bar shaft left of the scroll box. 
			case SB_PAGEUP: 
				xNewPos = xCurrentScroll - 100; 
				break; 
 
			// User clicked the scroll bar shaft right of the scroll box. 
			case SB_PAGEDOWN: 
				xNewPos = xCurrentScroll + 100; 
				break; 
 
			// User clicked the left arrow. 
			case SB_LINEUP: 
				xNewPos = xCurrentScroll - 20; 
				break; 
 
			// User clicked the right arrow. 
			case SB_LINEDOWN: 
				xNewPos = xCurrentScroll + 20; 
				break; 
 
			// User dragged the scroll box. 
			case SB_THUMBTRACK:
			case SB_THUMBPOSITION: 
				xNewPos = nPos; 
				break; 
 
			default: 
				xNewPos = xCurrentScroll; 

		}
		// New position must be between 0 and the screen width. 
		xNewPos = max(0, xNewPos); 
		xNewPos = min(xMaxScroll, xNewPos); 
 
		// If the current position does not change, do not scroll.
		if (xNewPos == xCurrentScroll) 
			return; 
 

		// Determine the amount scrolled (in pixels). 
		xDelta = xNewPos - xCurrentScroll; 
 
		// Reset the current scroll position. 
		xCurrentScroll = xNewPos; 
 
		// Scroll the window. (The system repaints most of the 
		// client area when ScrollWindowEx is called; however, it is 
		// necessary to call UpdateWindow in order to repaint the 
		// rectangle of pixels that were invalidated.) 
		ScrollWindowEx(hwnd, -xDelta, -yDelta, (CONST RECT *) &rc, 
			(CONST RECT *) NULL, (HRGN) NULL, (PRECT) NULL, 
			SW_INVALIDATE); 
		UpdateWindow(hwnd); 
		InvalidateRect(g_hwndStatusbar, NULL, TRUE);
		// Reset the scroll bar. 
		si.cbSize = sizeof(si); 
		si.fMask  = SIF_POS; 
		si.nPos   = xCurrentScroll; 
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE); 
}


BOOL OnMouseWheel(HWND hwnd, UINT nFlags, short zDelta, POINT pt)
{
    int min_pos = 0, max_pos = 0;
    GetScrollRange(hwndView, SB_VERT, &min_pos, &max_pos);
    if ( min_pos == max_pos )
        return FALSE;
    int scrolling_incr = abs((int)zDelta) / WHEEL_DELTA;
    int lines_per_incr = 0;
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines_per_incr, 0);

    if ( lines_per_incr == WHEEL_PAGESCROLL )
    {
        OnVScroll(hwnd, zDelta > 0 ? SB_PAGEUP : SB_PAGEDOWN, 0);
        return TRUE;
    }
    int n_lines = scrolling_incr * lines_per_incr;
	n_lines = max(n_lines/2, 1);
    for(int i = 0; i < n_lines; ++i)
    {
        OnVScroll(hwnd, zDelta > 0 ? SB_LINEUP : SB_LINEDOWN, 0);
    }

    return TRUE;
}
