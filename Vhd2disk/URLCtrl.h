// urlctrl - altered static text ctrl - bittmann 2004
//
//  based upon "urlctrl" by jbrown
//  please visit: http://www.catch22.net/tuts/
//
// note: is using SetWindowLong(GWL_USERDATA)
// note: include "urlctrl.h" after <windows.h> and <tchar.h>

#ifndef _URLCTRL_H
#define _URLCTRL_H

#ifdef __cplusplus
extern "C" {
#endif


// supported window MESSAGES:
//  WM_SETFONT     - set font (self-modifying: always underlined).
//  WM_SETTEXT     - set display text.
//                   maximum of MAX_PATH TCHARs (including terminator).


// CONVERT STATIC CONTROL TO URLCTRL
// may be called n-times on the same window: first call
// creates the urlcrtl, further calls do updates.
//
//  hwnd           - static control window.
//  url            - url to open (not display text).
//                   if url is NULL, window text is used as url.
//                   maximum of MAX_PATH TCHARs (including terminator).
//  unvisited      - color of unvisited link (NULL=default).
//  visited        - color of visited link (NULL=default).
//  flags          - combination of UCF_xxx values.

#define UCF_TXT_DEFAULT  0 //text orientation...
#define UCF_TXT_LEFT     0
#define UCF_TXT_RIGHT    1
#define UCF_TXT_HCENTER  2
#define UCF_TXT_TOP      0
#define UCF_TXT_VCENTER  4
#define UCF_TXT_BOTTOM   8 //...text orientation
#define UCF_LNK_VISITED 16 //link visited
#define UCF_KBD         32 //keyboard support (tabbing,focus,space key)
#define UCF_FIT         64 //automatic resizing

BOOL urlctrl_set(HWND,TCHAR *url,COLORREF *unvisited,COLORREF *visited,DWORD flags);


// RESIZE URLCTRL TO FIT DISPLAY TEXT
// may be called after urlctrl_set, WM_SETTEXT, WM_SETFONT.
// not needed, if urlctrl has set UCF_FIT flag.

BOOL urlctrl_fit(HWND);


#ifdef __cplusplus
}
#endif

#endif //_URLCTRL_H