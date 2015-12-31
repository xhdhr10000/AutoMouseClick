
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


#define HOTKEY_CLICK	1
#define HOTKEY_EXIT		2
#define HOTKEY1			3
#define HOTKEY2			4

// history.ini
#define HISTORY_SECTION		_T("history")
#define HISTORY_COUNT		_T("count")

#define HISTORY_HOTKEY			_T("hotkey")
#define HISTORY_HOTKEY_CTRL		_T("hotkey_ctrl")
#define HISTORY_HOTKEY_ALT		_T("hotkey_alt")
#define HISTORY_HOTKEY_SHIFT	_T("hotkey_shift")
#define HISTORY_HOTKEY_CODE		_T("hotkey_code")

#define HISTORY_HOTKEY_EXIT			_T("hotkey_exit")
#define HISTORY_HOTKEY_EXIT_CTRL	_T("hotkey_exit_ctrl")
#define HISTORY_HOTKEY_EXIT_ALT		_T("hotkey_exit_alt")
#define HISTORY_HOTKEY_EXIT_SHIFT	_T("hotkey_exit_shift")
#define HISTORY_HOTKEY_EXIT_CODE	_T("hotkey_exit_code")

#define HISTORY_HOTKEY1			_T("hotkey1")
#define HISTORY_HOTKEY1_CTRL	_T("hotkey1_ctrl")
#define HISTORY_HOTKEY1_ALT		_T("hotkey1_alt")
#define HISTORY_HOTKEY1_SHIFT	_T("hotkey1_shift")
#define HISTORY_HOTKEY1_CODE	_T("hotkey1_code")

#define HISTORY_HOTKEY2			_T("hotkey2")
#define HISTORY_HOTKEY2_CTRL	_T("hotkey2_ctrl")
#define HISTORY_HOTKEY2_ALT		_T("hotkey2_alt")
#define HISTORY_HOTKEY2_SHIFT	_T("hotkey2_shift")
#define HISTORY_HOTKEY2_CODE	_T("hotkey2_code")

#define HISTORY_POINT_COUNT		_T("point_count")
#define HISTORY_POINT			_T("point")