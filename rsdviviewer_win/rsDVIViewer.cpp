/*

Copyright (C) 2018 by Richard Sandberg.

This is the Windows specific version of rsdviviewer.
No Unix version exists yet, maybe I'll rewrite it in Qt
some day...

*/
#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <cstdio>
#include "resource.h"
#include "ReadDVI.h"

// double to int warning
#pragma warning(disable:4244)

// use new common controls
#pragma comment(linker, "/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' \
    publicKeyToken='6595b64144ccf1df' language='*'\"")


#define ID_STATUS_BAR 1106 // we create status bar manually, could perhaps be a resourse instead

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


char cur_dvi_filename[512];
HINSTANCE ghInst;
HWND ghWnd;
HWND hwndDVIView;
HDC ghDC;
HWND hwndStatus;
extern float resolution; // dpi
int dlg_new_viewing_page;

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	const char *class_name = "MainWindow";
	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName = class_name;
	RegisterClass(&wc);


	ghInst = hInstance;

	InitCommonControls();

	HWND hwnd = CreateWindowEx(0, class_name, "rsDVIViewer",
				WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, NULL, hInstance, NULL);
	if (!hwnd) return 0;

	ghWnd = hwnd;

	ShowWindow(hwnd, nCmdShow);
	ghDC = GetDC(hwnd);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

int status_bar_height;
int status_bar_width;

double zoom_factor = 1.0;

double page_size_width = 8.27; // inches
double page_size_height = 11.69; // inches
int page_pixel_width;
int page_pixel_height;
int page_pixel_offset_x;
int page_pixel_offset_y;


int scroll_step_v = 100;

int scrollX;
int scrollY;

int xMinScroll = 0;
int xCurrentScroll = 0;
int xMaxScroll = 0;

int yMinScroll = 0;
int yCurrentScroll = 0;
int yMaxScroll = 0;

int minimum_margin = 0; // fix upper left corner at (minimum_margin,minimum_margin)
int viewing_page = 0;

void DrawPage(HWND hwnd, 
	HDC hdc, 
	int page_num, // 0 based page number
	bool draw_border)
{
	BITMAP bm;
	SetMapMode(hdc, MM_ISOTROPIC);
	SetWindowExtEx(hdc, 327670, 327670, NULL);
	SetViewportExtEx(hdc, static_cast<int>(327670*zoom_factor), static_cast<int>(327670*zoom_factor), NULL);

	page_pixel_width = static_cast<int>(page_size_width*resolution);
	page_pixel_height = static_cast<int>(page_size_height*resolution);


	SetViewportOrgEx(hdc, -xCurrentScroll, -yCurrentScroll, NULL);

	if (draw_border)
		Rectangle(hdc, 0, 0, page_pixel_width, page_pixel_height);
	HRGN hRgn = CreateRectRgn(0,0, static_cast<int>(page_pixel_width*zoom_factor), static_cast<int>(page_pixel_height*zoom_factor));
	SelectClipRgn(hdc, hRgn);

	if (PageCharVector.size() == 0)
	return;

	std::vector<Character> &char_page = PageCharVector[page_num];

	for (auto& i : char_page) {
		GetObject(i.pFontChar->hBitmap, sizeof bm, &bm);
		HDC hdcBitmap = CreateCompatibleDC(hdc);
		int x_offset = i.pFontChar->min_m;
		int y_offset = -i.pFontChar->max_n-1;
		HBITMAP hOld = (HBITMAP)SelectObject(hdcBitmap, i.pFontChar->hBitmap);
		BitBlt(hdc, i.x+x_offset,i.y+y_offset, bm.bmWidth, bm.bmHeight, hdcBitmap, 0, 0, SRCAND);
		SelectObject(hdcBitmap, hOld);
		DeleteDC(hdcBitmap);
	}
	
	std::vector<Rule> &rule_page = PageRuleVector[page_num];
	for (auto& i : rule_page) {
		RECT rc;
		rc.left = i.x;
		rc.right = i.x+i.width+1;
		rc.bottom = i.y;
		rc.top = rc.bottom - i.height-1;
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	
	SelectClipRgn(hdc, NULL);
	DeleteObject(hRgn);

}

void DrawCharacterAt(HWND hwnd, HDC hdc, const Font *f, int index, int x, int y)
{
	BITMAP bm;
	RECT rc;
	GetClientRect(hwnd, &rc);
	SetMapMode(hdc, MM_ISOTROPIC);
	SetWindowExtEx(hdc, 327670, 327670, NULL);
	SetViewportExtEx(hdc, static_cast<int>(327670*zoom_factor), static_cast<int>(327670*zoom_factor), NULL);
	GetObject(f->Char[index].hBitmap, sizeof bm, &bm);
	HDC hdcBitmap = CreateCompatibleDC(hdc);
	int x_offset = f->Char[index].min_m;
	int y_offset = -f->Char[index].max_n-1;
	HBITMAP hOld = (HBITMAP)SelectObject(hdcBitmap, f->Char[index].hBitmap);
	BitBlt(hdc, x+x_offset,y+y_offset, bm.bmWidth, bm.bmHeight, hdcBitmap, 0, 0, SRCCOPY);
	SelectObject(hdcBitmap, hOld);
	DeleteDC(hdcBitmap);
}

HWND CreateStatusBar(HWND hwndParent, int idStatus, HINSTANCE hinst)
{
	HWND hwndStatus;
	RECT rcClient;
	HLOCAL hloc;
	PINT paParts;
	int cParts=2;
	InitCommonControls();

	hwndStatus = CreateWindowEx(
		0,
		STATUSCLASSNAME,
		NULL,
		SBARS_SIZEGRIP |
		WS_CHILD | WS_VISIBLE,
		0,0,0,0,
		hwndParent,
		(HMENU) (INT_PTR)idStatus,
		hinst,
		NULL);

		GetClientRect(hwndParent, &rcClient);
		hloc = LocalAlloc(LHND, sizeof(int) * cParts);
		paParts = (PINT)LocalLock(hloc);

		paParts[0] = 120;
		paParts[1] = -1;

		SendMessage(hwndStatus, SB_SETPARTS, (WPARAM)cParts, (LPARAM)
				paParts);

		LocalUnlock(hloc);
		LocalFree(hloc);
		return hwndStatus;
}


INT_PTR CALLBACK MyDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char buf[256];
	switch(msg) {
		case WM_INITDIALOG:
			// initialize dialog controls
		{
			sprintf(buf, "%.2f", resolution);
			SetDlgItemText(hwndDlg, IDC_EDIT1, buf);
		}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					{
						GetDlgItemText(hwndDlg, IDC_EDIT1, buf, sizeof buf);
						double dtmp = strtod(buf, NULL);
						if (dtmp != 0.0)
							resolution = static_cast<float>(dtmp);

					}
				// fall through

				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
					return TRUE;
			}
	}
	return FALSE;
}

INT_PTR CALLBACK MyDialogProc3(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char buf[256];
	switch(msg) {
		case WM_INITDIALOG:
			// initialize dialog controls
		{
			sprintf(buf, "%.2f", resolution);
			SetDlgItemInt(hwndDlg, IDC_EDIT_PAGE, viewing_page+1, TRUE);
		}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					{
						BOOL translated = FALSE;
						int temp = GetDlgItemInt(hwndDlg, IDC_EDIT_PAGE, &translated, TRUE);
						if (translated)
							dlg_new_viewing_page = temp;
					}
				// fall through

				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
					return TRUE;
			}
	}
	return FALSE;
}
INT_PTR CALLBACK MyDialogProc2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char buf[256];
	switch(msg) {
		case WM_INITDIALOG:
			// initialize dialog controls
		{
			sprintf(buf, "%.2f", page_size_width);
			SetDlgItemText(hwndDlg, IDC_EDIT_WIDTH, buf);
			sprintf(buf, "%.2f", page_size_height);
			SetDlgItemText(hwndDlg, IDC_EDIT_HEIGHT, buf);
		}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					{
						GetDlgItemText(hwndDlg, IDC_EDIT_WIDTH, buf, sizeof buf);
						double dtmp = strtod(buf, NULL);
						if (dtmp != 0.0)
							page_size_width = dtmp;

						GetDlgItemText(hwndDlg, IDC_EDIT_HEIGHT, buf, sizeof buf);
						dtmp = strtod(buf, NULL);
						if (dtmp != 0.0)
							page_size_height = dtmp;

					}
				// fall through

				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
					return TRUE;
			}
	}
	return FALSE;
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
		InvalidateRect(hwndStatus, NULL, TRUE);
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
		InvalidateRect(hwndStatus, NULL, TRUE);
		// Reset the scroll bar. 
		si.cbSize = sizeof(si); 
		si.fMask  = SIF_POS; 
		si.nPos   = xCurrentScroll; 
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE); 
}


BOOL OnMouseWheel(HWND hwnd, UINT nFlags, short zDelta, POINT pt)
{
    int min_pos = 0, max_pos = 0;
    GetScrollRange(hwndDVIView, SB_VERT, &min_pos, &max_pos);
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	static int deltaPos;
	static int scrollAmount = 50; // 50 pixels
	switch(uMsg) {

		case WM_CREATE:
			if (GetWindowLongPtr(hwnd, GWLP_HWNDPARENT) == NULL) {
				hwndStatus = CreateStatusBar(hwnd, ID_STATUS_BAR, ghInst);

				const char *class_name = "DVIViewWindow";
				WNDCLASS wc = {0};
				wc.lpfnWndProc = WndProc;
				wc.hInstance = ghInst;
				wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
				wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
				wc.style = CS_HREDRAW | CS_VREDRAW;
				wc.hCursor = LoadCursor(NULL, IDC_ARROW);
				wc.lpszMenuName = NULL;
				wc.lpszClassName = class_name;
				RegisterClass(&wc);

				hwndDVIView =CreateWindowEx(0, class_name, "rsDVIViewer", 
				WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				hwnd, NULL, ghInst, NULL);
				ShowWindow(hwndDVIView, SW_SHOW);
				if (!hwndDVIView) {
					int a = 10;
					a++;
				}
			}

			break;

		case WM_SIZE:
			if (hwnd == ghWnd)
			{
				// size child window
				RECT rc;
				SendMessage(hwndStatus, uMsg, 0, 0);
				GetClientRect(hwndStatus, &rc);
				status_bar_height = rc.bottom;
				status_bar_width = rc.right;
				MoveWindow(hwndDVIView, 0, 0, LOWORD(lParam),HIWORD(lParam) - status_bar_height, TRUE);
			}
			if (hwnd == hwndDVIView)
			{
				SCROLLINFO si = {0};
				int cxClient = LOWORD(lParam);
				int cyClient = HIWORD(lParam);
			
				yMaxScroll = max(page_size_height*resolution*zoom_factor+2*minimum_margin - (cyClient), 0);
				yCurrentScroll = min(yCurrentScroll, yMaxScroll);
				si.cbSize = sizeof si;
				si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
				si.nMin = yMinScroll;
				si.nMax = (page_size_height*resolution*zoom_factor+2*minimum_margin);
				si.nPage = cyClient;
				si.nPos = yCurrentScroll;
				SetScrollInfo(hwnd, SB_VERT, &si, TRUE);


				xMaxScroll = max(page_size_width*resolution*zoom_factor+2*minimum_margin - (cxClient), 0);
				xCurrentScroll = min(xCurrentScroll, xMaxScroll);
				si.cbSize = sizeof si;
				si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
				si.nMin = xMinScroll;
				si.nMax = (page_size_width*resolution*zoom_factor+2*minimum_margin);
				si.nPage = (cxClient);
				si.nPos = xCurrentScroll;
				SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
			}
			break;

		case WM_MOUSEWHEEL:
			{
				POINT pt;
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);
				OnMouseWheel(hwnd, GET_KEYSTATE_WPARAM(wParam),
					GET_WHEEL_DELTA_WPARAM(wParam), pt);
			}
			break;

		case WM_VSCROLL:
			if (hwnd == hwndDVIView)
			{
				OnVScroll(hwnd, LOWORD(wParam), HIWORD(wParam));

			}
			break;

		case WM_HSCROLL:
			if (hwnd == hwndDVIView)
			{
				OnHScroll(hwnd, LOWORD(wParam), HIWORD(wParam));

 			}
			break;

		case WM_MOUSEMOVE:
			{
				int xPos = GET_X_LPARAM(lParam);
				int yPos = GET_Y_LPARAM(lParam);
				char buffer[256];
				sprintf(buffer, "zoom: %.2f%%", zoom_factor*100.0);
				SendMessage(hwndStatus, SB_SETTEXT, 0 | (SBT_NOBORDERS << 8), (LPARAM)buffer);
				sprintf(buffer, "Screen (%d,%d)", xPos, yPos);
				SendMessage(hwndStatus, SB_SETTEXT, 1 | (SBT_NOBORDERS << 8), (LPARAM)buffer);
			}
			break;


		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_PAINT:
		if (hwnd == hwndDVIView)
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+2));
			DrawPage(hwnd, hdc, viewing_page, true);
			EndPaint(hwnd, &ps);
		}
		else {
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_FILE_EXIT:
					PostQuitMessage(0);
					break;

				case ID_FILE_PRINT:
				{
						DOCINFO di = {sizeof (DOCINFO), "DVIViewer Print"};
						HDC hdcPrinter;
						PRINTDLG pd = {0};
						pd.lStructSize = sizeof(pd);
						pd.hwndOwner = hwnd;
						pd.Flags = PD_RETURNDC;
						pd.nMinPage = 1;
						pd.nMaxPage = static_cast<WORD>(PageCharVector.size());
						
						if (!PrintDlg(&pd)) break;;
						hdcPrinter = pd.hDC;
						int logPixelsX = GetDeviceCaps(hdcPrinter, LOGPIXELSX);

						if (fabs((double)logPixelsX - resolution) > 1.0) {
							resolution = (float)logPixelsX;
							ReadDVIFile(cur_dvi_filename);
							SCROLLINFO si = {0};
							RECT rc;
							GetClientRect(hwndDVIView, &rc);

							yMaxScroll = max(page_size_height*resolution*zoom_factor+2*minimum_margin - rc.bottom, 0);
							yCurrentScroll = min(yCurrentScroll, yMaxScroll);
							xMaxScroll = max(page_size_width*resolution*zoom_factor+2*minimum_margin - rc.right, 0);
							xCurrentScroll = min(xCurrentScroll, xMaxScroll);

							si.cbSize = sizeof si;
							si.fMask = SIF_RANGE|SIF_POS;
							si.nMin = yMinScroll;
							si.nMax = (page_size_height*resolution*zoom_factor+2*minimum_margin);
							si.nPos = yCurrentScroll;
							SetScrollInfo(hwndDVIView, SB_VERT, &si, TRUE);

							si.cbSize = sizeof si;
							si.fMask = SIF_RANGE|SIF_POS;
							si.nMin = xMinScroll;
							si.nMax = (page_size_width*resolution*zoom_factor+2*minimum_margin);
							si.nPos = xCurrentScroll;
							SetScrollInfo(hwndDVIView, SB_HORZ, &si, TRUE);

							InvalidateRect(hwndDVIView, NULL, TRUE);

						}
						
						if (pd.Flags & PD_PAGENUMS) {
							StartDoc(hdcPrinter, &di);
							for (int i = pd.nFromPage; i <= pd.nToPage; i++) {
								StartPage(hdcPrinter);
								DrawPage(hwndDVIView, hdcPrinter, i-1, false);
								EndPage(hdcPrinter);
							}
							EndDoc(hdcPrinter);
						}
						else if (pd.Flags & PD_SELECTION)
						{
							// what to do here?
						}
						else {
							// print all pages
							StartDoc(hdcPrinter, &di);
							for (int i = 1; i <= PageCharVector.size(); i++) {
								StartPage(hdcPrinter);
								DrawPage(hwndDVIView, hdcPrinter, i-1, false);
								EndPage(hdcPrinter);
							}
							EndDoc(hdcPrinter);
						}
						
						DeleteDC(hdcPrinter);
				}
					break;

				case ID_FILE_GOTOPAGE:
					if (DialogBox(ghInst, MAKEINTRESOURCE(IDD_GOTOPAGE),
								hwnd, MyDialogProc3) == IDOK) {
						if (dlg_new_viewing_page <= PageCharVector.size() &&  dlg_new_viewing_page >= 1) {
							viewing_page = dlg_new_viewing_page - 1;
							InvalidateRect(hwndDVIView, NULL, TRUE);
						}
					}
					break;


				case ID_FILE_SETPAGESIZE:
					if (DialogBox(ghInst, MAKEINTRESOURCE(IDD_SETPAGESIZE),
								hwnd, MyDialogProc2) == IDOK) {
						SCROLLINFO si = {0};
						RECT rc;
						GetClientRect(hwndDVIView, &rc);

						yMaxScroll = max(page_size_height*resolution*zoom_factor+2*minimum_margin - rc.bottom, 0);
						yCurrentScroll = min(yCurrentScroll, yMaxScroll);
						xMaxScroll = max(page_size_width*resolution*zoom_factor+2*minimum_margin - rc.right, 0);
						xCurrentScroll = min(xCurrentScroll, xMaxScroll);

						si.cbSize = sizeof si;
						si.fMask = SIF_RANGE|SIF_POS;
						si.nMin = yMinScroll;
						si.nMax = (page_size_height*resolution*zoom_factor+2*minimum_margin);
						si.nPos = yCurrentScroll;
						SetScrollInfo(hwndDVIView, SB_VERT, &si, TRUE);

						si.cbSize = sizeof si;
						si.fMask = SIF_RANGE|SIF_POS;
						si.nMin = xMinScroll;
						si.nMax = (page_size_width*resolution*zoom_factor+2*minimum_margin);
						si.nPos = xCurrentScroll;
						SetScrollInfo(hwndDVIView, SB_HORZ, &si, TRUE);

						InvalidateRect(hwndDVIView, NULL, TRUE);

					}
					break;

				case ID_FILE_NEXTPAGE:
					if (PageCharVector.size() > viewing_page + 1) {
						viewing_page++;
						InvalidateRect(hwndDVIView, NULL, TRUE);
					}
					break;

				case ID_FILE_PREVPAGE:
					if (viewing_page > 0) {
						viewing_page--;
						InvalidateRect(hwndDVIView, NULL, TRUE);
					}
					break;

				case ID_FILE_SETRESOLUTION:
					if (DialogBox(ghInst, MAKEINTRESOURCE(IDD_SETRESOLUTION),
								hwnd, MyDialogProc) == IDOK) {
						RECT rc;
						GetClientRect(hwndDVIView, &rc);
						int cxClient = rc.right - rc.left;
						int cyClient = rc.bottom - rc.top;
						ReadDVIFile(cur_dvi_filename);
						SCROLLINFO si = {0};

						yMaxScroll = max(page_size_height*resolution*zoom_factor+2*minimum_margin - rc.bottom, 0);
						yCurrentScroll = min(yCurrentScroll, yMaxScroll);
						xMaxScroll = max(page_size_width*resolution*zoom_factor+2*minimum_margin - rc.right, 0);
						xCurrentScroll = min(xCurrentScroll, xMaxScroll);

						si.cbSize = sizeof si;
						si.fMask = SIF_RANGE|SIF_POS;
						si.nMin = yMinScroll;
						si.nMax = (page_size_height*resolution*zoom_factor+2*minimum_margin);
						si.nPos = yCurrentScroll;
						SetScrollInfo(hwndDVIView, SB_VERT, &si, TRUE);

						si.cbSize = sizeof si;
						si.fMask = SIF_RANGE|SIF_POS;
						si.nMin = xMinScroll;
						si.nMax = (page_size_width*resolution*zoom_factor+2*minimum_margin);
						si.nPos = xCurrentScroll;
						SetScrollInfo(hwndDVIView, SB_HORZ, &si, TRUE);

						InvalidateRect(hwndDVIView, NULL, TRUE);

					}
					break;

				case ID_FILE_ZOOMIN:
				{
					RECT rc;

					GetClientRect(hwndDVIView, &rc);
					zoom_factor *=2;
					SCROLLINFO si = {0};

					yMaxScroll = max(page_size_height*resolution*zoom_factor+2*minimum_margin - rc.bottom, 0);
					yCurrentScroll = min(yCurrentScroll, yMaxScroll);
					xMaxScroll = max(page_size_width*resolution*zoom_factor+2*minimum_margin - rc.right, 0);
					xCurrentScroll = min(xCurrentScroll, xMaxScroll);

					si.cbSize = sizeof si;
					si.fMask = SIF_RANGE|SIF_POS;
					si.nMin = yMinScroll;
					si.nMax = (page_size_height*resolution*zoom_factor+2*minimum_margin);
					si.nPos = yCurrentScroll;
					SetScrollInfo(hwndDVIView, SB_VERT, &si, TRUE);


					si.cbSize = sizeof si;
					si.fMask = SIF_RANGE|SIF_POS;
					si.nMin = xMinScroll;
					si.nMax = (page_size_width*resolution*zoom_factor+2*minimum_margin);
					si.nPos = xCurrentScroll;
					SetScrollInfo(hwndDVIView, SB_HORZ, &si, TRUE);

					InvalidateRect(hwndDVIView, NULL, TRUE);
				}
					break;

				case ID_FILE_ZOOMOUT:
				{
					RECT rc;
					GetClientRect(hwndDVIView, &rc);
					zoom_factor /= 2;
					SCROLLINFO si = {0};

					yMaxScroll = max(page_size_height*resolution*zoom_factor+2*minimum_margin - rc.bottom, 0);
					yCurrentScroll = min(yCurrentScroll, yMaxScroll);
					xMaxScroll = max(page_size_width*resolution*zoom_factor+2*minimum_margin - rc.right, 0);
					xCurrentScroll = min(xCurrentScroll, xMaxScroll);

					si.cbSize = sizeof si;
					si.fMask = SIF_RANGE|SIF_POS;
					si.nMin = yMinScroll;
					si.nMax = (page_size_height*resolution*zoom_factor+2*minimum_margin);
					si.nPos = yCurrentScroll;
					SetScrollInfo(hwndDVIView, SB_VERT, &si, TRUE);


					si.cbSize = sizeof si;
					si.fMask = SIF_RANGE|SIF_POS;
					si.nMin = xMinScroll;
					si.nMax = (page_size_width*resolution*zoom_factor+2*minimum_margin);
					si.nPos = xCurrentScroll;
					SetScrollInfo(hwndDVIView, SB_HORZ, &si, TRUE);

					InvalidateRect(hwndDVIView, NULL, TRUE);
				}
					break;

				case ID_FILE_OPEN:
				{
					static char filter[] = "DVI Files\0*.dvi\0";
					static char the_filename[1024] = {0};
					static OPENFILENAME ofn = {0};
					ZeroMemory( &ofn , sizeof( ofn));
					ofn.lStructSize = sizeof ( ofn );
					ofn.hwndOwner = hwnd  ;
					ofn.lpstrFile = the_filename ;
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof( the_filename );
					ofn.lpstrFilter = "DVI Files\0*.DVI\0";
					ofn.nFilterIndex =1;
					ofn.lpstrFileTitle = NULL ;
					ofn.nMaxFileTitle = 0 ;
					ofn.lpstrInitialDir=NULL ;
					ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
					BOOL ret = GetOpenFileName(&ofn);
					if (ret) 
					{
						strcpy(cur_dvi_filename, ofn.lpstrFile);
						ReadDVIFile(ofn.lpstrFile);
						InvalidateRect(hwnd, NULL, TRUE);
					}

					break;
				}
			}
			break;

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}
