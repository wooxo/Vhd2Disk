// urlctrl - altered static text ctrl - bittmann 2004
//
//  based upon "urlctrl" by jbrown
//  please visit: http://www.catch22.net/tuts/

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <shellapi.h> //shellexecute
#include "urlctrl.h"

#define this_malloc(size) (void*)GlobalAlloc(GMEM_FIXED,(size))
#define this_free(ptr)    GlobalFree(ptr)

typedef struct
{	TCHAR szURL[MAX_PATH];//url!=display
	WNDPROC oldproc;//old window proc
	HCURSOR hcur;//cursor
	HFONT hfont;//underlined font
	COLORREF crUnvisited;//color unvisited
	COLORREF crVisited;//color visited
	BOOL fClicking;//internal state
	DWORD dwFlags;//combination of UCF_xxx values
} URLCTRL;


BOOL util_url_draw(HWND,HDC,RECT*);//draw (LPRECT==NULL) OR calc (LPRECT!=NULL)
BOOL util_url_fit(HWND,BOOL fRedraw);//resize window
BOOL util_url_open(HWND);//open url
LRESULT CALLBACK urlctrl_proc(HWND,UINT,WPARAM,LPARAM);


BYTE curAND[128]=
{	0xF9,0xFF,0xFF,0xFF, 0xF0,0xFF,0xFF,0xFF, 0xF0,0xFF,0xFF,0xFF, 0xF0,0xFF,0xFF,0xFF,
	0xF0,0xFF,0xFF,0xFF, 0xF0,0x3F,0xFF,0xFF, 0xF0,0x07,0xFF,0xFF, 0xF0,0x01,0xFF,0xFF,
	0xF0,0x00,0xFF,0xFF, 0x10,0x00,0x7F,0xFF, 0x00,0x00,0x7F,0xFF, 0x00,0x00,0x7F,0xFF,
	0x80,0x00,0x7F,0xFF, 0xC0,0x00,0x7F,0xFF, 0xC0,0x00,0x7F,0xFF, 0xE0,0x00,0x7F,0xFF,
	0xE0,0x00,0xFF,0xFF, 0xF0,0x00,0xFF,0xFF, 0xF0,0x00,0xFF,0xFF, 0xF8,0x01,0xFF,0xFF,
	0xF8,0x01,0xFF,0xFF, 0xF8,0x01,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF
};//hand cursor mask

BYTE curXOR[128]=
{	0x00,0x00,0x00,0x00, 0x06,0x00,0x00,0x00, 0x06,0x00,0x00,0x00, 0x06,0x00,0x00,0x00,
	0x06,0x00,0x00,0x00, 0x06,0x00,0x00,0x00, 0x06,0xC0,0x00,0x00, 0x06,0xD8,0x00,0x00,
	0x06,0xDA,0x00,0x00, 0x06,0xDB,0x00,0x00, 0x67,0xFB,0x00,0x00, 0x77,0xFF,0x00,0x00,
	0x37,0xFF,0x00,0x00, 0x17,0xFF,0x00,0x00, 0x1F,0xFF,0x00,0x00, 0x0F,0xFF,0x00,0x00,
	0x0F,0xFE,0x00,0x00, 0x07,0xFE,0x00,0x00, 0x07,0xFE,0x00,0x00, 0x03,0xFC,0x00,0x00,
	0x03,0xFC,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};//hand cursor mask


BOOL util_url_draw(HWND hwnd,HDC hdc,RECT *calcrect) //calc OR draw
{	BOOL fResult=FALSE;
	if(GetWindowLongPtr(hwnd,GWLP_WNDPROC)==(ULONG_PTR)urlctrl_proc && hdc) //is urlctrl and hdc?
	{	URLCTRL *url=(URLCTRL*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
		if(url)
		{	HANDLE hOld;
			DWORD style=DT_SINGLELINE|DT_NOPREFIX;
			TCHAR szText[MAX_PATH];
			RECT rc;
			SendMessage(hwnd,WM_GETTEXT,(WPARAM)MAX_PATH,(LPARAM)szText);
			if(url->dwFlags&UCF_TXT_RIGHT) style|=DT_RIGHT;
			else if(url->dwFlags&UCF_TXT_HCENTER) style|=DT_CENTER;
			if(url->dwFlags&UCF_TXT_BOTTOM) style|=DT_BOTTOM;
			else if(url->dwFlags&UCF_TXT_VCENTER) style|=DT_VCENTER;
			if(calcrect==NULL) //draw!
			{	COLORREF crOldText;
				int iOldBkMode;
				GetClientRect(hwnd,&rc);
				FillRect(hdc,&rc,(HBRUSH)(COLOR_3DFACE+1));
				if(url->dwFlags&UCF_KBD)
				{	if(GetFocus()==hwnd) DrawFocusRect(hdc,&rc);
					++rc.left; --rc.right; ++rc.top; --rc.bottom;//protect focus rect
				}
				crOldText=SetTextColor(hdc,
					(url->dwFlags&UCF_LNK_VISITED)?(url->crVisited):(url->crUnvisited));
				iOldBkMode=SetBkMode(hdc,TRANSPARENT);
				hOld=SelectObject(hdc,url->hfont);
				DrawText(hdc,szText,-1,&rc,style);
				SelectObject(hdc,hOld);
				SetBkMode(hdc,iOldBkMode);
				SetTextColor(hdc,crOldText);
			}
			else //just calc
			{	calcrect->left=calcrect->top=0;
				calcrect->right=calcrect->bottom=0x00007FFF;//some big value
				hOld=SelectObject(hdc,url->hfont);
				DrawText(hdc,szText,-1,calcrect,style|DT_CALCRECT);
				SelectObject(hdc,hOld);
				if(url->dwFlags&UCF_KBD)
				{	calcrect->right+=2; calcrect->bottom+=2;//for focus rect
				}
			}
			fResult=TRUE;
		}
	}
	return fResult;
}

BOOL util_url_fit(HWND hwnd,BOOL fRedraw)
{	BOOL fResult=FALSE;
	if(GetWindowLongPtr(hwnd,GWLP_WNDPROC)==(ULONG_PTR)urlctrl_proc) //is urlctrl?
	{	HDC hdc=GetDC(hwnd);
		if(hdc)
		{	RECT rc;
			fResult=util_url_draw(hwnd,hdc,&rc);//calc rect (does NOT draw)
			ReleaseDC(hwnd,hdc);
			if(fResult) //apply rect
			{	UINT flags=SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE;
				if(fRedraw==FALSE) flags|=SWP_NOREDRAW;
				fResult=SetWindowPos(hwnd,NULL,0,0,rc.right,rc.bottom,flags);
			}
		}
	}
	return fResult;
}

BOOL util_url_open(HWND hwnd)
{	BOOL fResult=FALSE;
	if(GetWindowLongPtr(hwnd,GWLP_WNDPROC)==(ULONG_PTR)urlctrl_proc) //is urlctrl?
	{	URLCTRL *url=(URLCTRL*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
		if(url)
		{	TCHAR *p,szText[MAX_PATH];
			if(url->szURL[0]) //use url
			{	p=url->szURL;
			}
			else //use window text
			{	p=szText;
				SendMessage(hwnd,WM_GETTEXT,(WPARAM)MAX_PATH,(LPARAM)szText);
			}
			// open browser
			if((int)ShellExecute(NULL,_T("open"),p,NULL,NULL,SW_SHOWNORMAL)>32)
			{	url->dwFlags|=UCF_LNK_VISITED;//set visited
				InvalidateRect(hwnd,NULL,TRUE);
				UpdateWindow(hwnd);
				fResult=TRUE;
			}
		}
	}
	return fResult;
}

LRESULT CALLBACK urlctrl_proc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	URLCTRL *url=(URLCTRL*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	WNDPROC oldproc=url->oldproc;
	switch(uMsg)
	{	case WM_NCDESTROY:
		{	SetWindowLongPtr(hwnd,GWLP_WNDPROC,(ULONG_PTR)oldproc);
			if(url->hcur) DestroyCursor(url->hcur);
			if(url->hfont) DeleteObject(url->hfont);
			this_free(url);
			break;
		}
		case WM_PAINT:
		{	PAINTSTRUCT ps;
			HDC hdc=(HDC)wParam;
			if(wParam==0) hdc=BeginPaint(hwnd,&ps);
			if(hdc)
			{	util_url_draw(hwnd,hdc,NULL);
				if(wParam==0) EndPaint(hwnd,&ps);
			}
			return 0;
		}
		case WM_SETTEXT:
		{	LRESULT result=CallWindowProc(oldproc,hwnd,uMsg,wParam,lParam);
			if(url->dwFlags&UCF_FIT) util_url_fit(hwnd,FALSE);
			InvalidateRect(hwnd,NULL,TRUE);
			UpdateWindow(hwnd);
			return result;
		}
		case WM_SETFONT: //we always modify the font!
		{	LOGFONT lf;
			HFONT hfont=(HFONT)wParam;
			if(hfont==NULL) hfont=(HFONT)GetStockObject(SYSTEM_FONT);
			GetObject(hfont,sizeof(LOGFONT),&lf);
			lf.lfUnderline=TRUE;//add underline
			if(url->hfont) DeleteObject(url->hfont);//delete old font
			url->hfont=CreateFontIndirect(&lf);//create new font
			CallWindowProc(oldproc,hwnd,uMsg,wParam,0);//block redraw
			if(url->dwFlags&UCF_FIT) util_url_fit(hwnd,FALSE);
			if(LOWORD(lParam)) //redraw?
			{	InvalidateRect(hwnd,NULL,TRUE);
				UpdateWindow(hwnd);
			}
			return 0;
		}
		case WM_SETCURSOR:
		{	HCURSOR hcur=url->hcur;
			if(hcur==NULL) hcur=LoadCursor(NULL,IDC_ARROW);//fall back
			SetCursor(hcur);
			return 1;
		}
		case WM_NCHITTEST:
		{	// static returns HTTRANSPARENT (prevents receiving mouse msgs)
			return HTCLIENT;
		}
		case WM_LBUTTONDOWN:
		{	if(url->dwFlags&UCF_KBD) SetFocus(hwnd);
			SetCapture(hwnd);
			url->fClicking=TRUE;
			break;
		}
		case WM_LBUTTONUP:
		{	ReleaseCapture();
			if(url->fClicking)
			{	POINT pt;
				RECT rc;
				url->fClicking=FALSE;
				pt.x=(short)LOWORD(lParam);
				pt.y=(short)HIWORD(lParam);
				ClientToScreen(hwnd,&pt);
				GetWindowRect(hwnd,&rc);
				if(PtInRect(&rc,pt)) util_url_open(hwnd);//inside?
			}
			break;
		}
		case WM_KEYDOWN:
		{	if(url->dwFlags&UCF_KBD)
			{	if(wParam==VK_SPACE)
				{	util_url_open(hwnd);
					return 0;
				}
			}
			break;
		}
		case WM_KEYUP:
		{	if(url->dwFlags&UCF_KBD)
			{	if(wParam==VK_SPACE)
				{	return 0;
				}
			}
			break;
		}
		case WM_GETDLGCODE:
		{	if(url->dwFlags&UCF_KBD) return DLGC_WANTCHARS;
			break;
		}
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		{	if(url->dwFlags&UCF_KBD)
			{	InvalidateRect(hwnd,NULL,TRUE);
				UpdateWindow(hwnd);
				return 0;
			}
			break;
		}
	}
	return CallWindowProc(oldproc,hwnd,uMsg,wParam,lParam);
}

BOOL urlctrl_set(HWND hwnd,TCHAR *szURL,COLORREF *unvisited,COLORREF *visited,DWORD dwFlags)
{	BOOL fResult=FALSE;
	if(IsWindow(hwnd))
	{	URLCTRL *url;
		if(GetWindowLongPtr(hwnd,GWLP_WNDPROC)==(ULONG_PTR)urlctrl_proc) //update old
		{	url=(URLCTRL*)GetWindowLong(hwnd,GWLP_USERDATA);
		}
		else //create new
		{	url=(URLCTRL*)this_malloc(sizeof(URLCTRL));
			if(url) //init once
			{	HFONT hfont = 0;
				hfont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0,0);
				url->oldproc =( WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (ULONG_PTR)urlctrl_proc);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (ULONG_PTR)url);//store in control
				url->fClicking = FALSE;
				url->hcur = CreateCursor(GetModuleHandle(NULL),5,0,32,32,curAND,curXOR);

				SendMessage(hwnd, WM_SETFONT,(WPARAM)hfont,0);//modify current font
			}
		}
		if(url) //init always
		{	LONG style=GetWindowLong(hwnd,GWL_STYLE);
			if(szURL) lstrcpyn(url->szURL,szURL,MAX_PATH);
			else url->szURL[0]=0;
			url->crUnvisited=(unvisited)?(*unvisited):(RGB(0,0,255));
			url->crVisited=(visited)?(*visited):(RGB(128,0,128));
			//if(dwFlags&UCF_TXT_RIGHT) dwFlags&=~UCF_TXT_HCENTER;//patch
			//if(dwFlags&UCF_TXT_BOTTOM) dwFlags&=~UCF_TXT_VCENTER;//patch
			url->dwFlags=dwFlags;
			style&=~(WS_BORDER|WS_TABSTOP);//remove
			style|=SS_NOTIFY;//add
			if(url->dwFlags&UCF_KBD) style|=WS_TABSTOP;//add?
			SetWindowLongPtr(hwnd,GWL_STYLE,style);
			if(url->dwFlags&UCF_FIT) util_url_fit(hwnd,FALSE);
			InvalidateRect(hwnd,NULL,TRUE);
			UpdateWindow(hwnd);
			fResult=TRUE;
		}
	}
	return fResult;
}

BOOL urlctrl_fit(HWND hwnd)
{	return util_url_fit(hwnd,TRUE);
}
