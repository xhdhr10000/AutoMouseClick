
// AutoMouseClickDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <GdiPlus.h>
#include "AutoMouseClick.h"
#include "AutoMouseClickDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAutoMouseClickDlg 对话框
CAutoMouseClickDlg *g_Dialog;

ULONG_PTR token;
HHOOK hhookMs = NULL, hhookKb = NULL;
LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(INT nCode, WPARAM wParam, LPARAM lParam);
BOOL InstallKbHook();
BOOL UninstallKbHook();
BOOL InstallMsHook();
BOOL UninstallMsHook();

CAutoMouseClickDlg::CAutoMouseClickDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAutoMouseClickDlg::IDD, pParent)
{
	g_Dialog = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoMouseClickDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HOTKEY, m_HotKeyEdit);
	DDX_Control(pDX, IDC_HOTKEYEXIT, m_HotKeyExit);
#ifdef TWO_HOTKEY
	DDX_Control(pDX, IDC_HOTKEY1, m_HotKey1);
	DDX_Control(pDX, IDC_HOTKEY2, m_HotKey2);
#endif
	m_HotKeyEdit.SetFlag(HOTKEY_CLICK);
	m_HotKeyExit.SetFlag(HOTKEY_EXIT);
#ifdef TWO_HOTKEY
	m_HotKey1.SetFlag(HOTKEY1);
	m_HotKey2.SetFlag(HOTKEY2);
#endif
	m_HotKeyEdit.SetParent(this);
	m_HotKeyExit.SetParent(this);
#ifdef TWO_HOTKEY
	m_HotKey1.SetParent(this);
	m_HotKey2.SetParent(this);
#endif
}

BEGIN_MESSAGE_MAP(CAutoMouseClickDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HOTKEY()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_EN_CHANGE(IDC_COUNT, &CAutoMouseClickDlg::OnEnChangeCount)
	ON_BN_CLICKED(IDC_EXIT, &CAutoMouseClickDlg::OnBnClickedExit)
	ON_WM_DESTROY()
	ON_BN_CLICKED(BT_ADD, &CAutoMouseClickDlg::OnBnClickedAdd)
	ON_BN_CLICKED(BT_DEL, &CAutoMouseClickDlg::OnBnClickedDel)
END_MESSAGE_MAP()


// CAutoMouseClickDlg 消息处理程序

BOOL CAutoMouseClickDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	mStopThread = false;
	mClickThread = NULL;
	mClickCounter = 0;
	mScrX = GetSystemMetrics(SM_CXSCREEN);
	mScrY = GetSystemMetrics(SM_CYSCREEN);
	if (mScrX == 0 || mScrY == 0) {
		MessageBox(_T("获取屏幕坐标失败"));
		OnCancel();
	}

	m_List = (CListBox*)this->GetDlgItem(IDC_LIST);
	
	TCHAR *mem = new TCHAR[MAX_PATH + 64*4];
	TCHAR *szPath = mem, 
		  *szCount = szPath + MAX_PATH, 
		  *szHotkey = szCount + 64, 
		  *szHotkeyExit = szHotkey + 64, 
#ifdef TWO_HOTKEY
		  *szHotkey1 = szHotkeyExit + 64, 
		  *szHotkey2 = szHotkey1 + 64, 
#endif
		  *szValue = szHotkeyExit + 64;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath))) {
		if (!PathAppend(szPath, _T("AutoMouseClick"))) {
			MessageBox(_T("PathAppend:73 error"));
			OnCancel();
		}
		if (!PathAppend(szPath, _T("\\history.ini"))) {
			MessageBox(_T("PathAppend:77 error"));
			OnCancel();
		}
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_COUNT, _T("50"), szCount, 64, szPath);
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY, _T("K"), szHotkey, 64, szPath);
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_CTRL, _T("0"), szValue, 64, szPath);
		m_HotKeyEdit.m_HotkeyCtrl = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_ALT, _T("0"), szValue, 64, szPath);
		m_HotKeyEdit.m_HotkeyAlt = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_SHIFT, _T("0"), szValue, 64, szPath);
		m_HotKeyEdit.m_HotkeyShift = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_CODE, _T("75"), szValue, 64, szPath);
		_stscanf_s(szValue, _T("%u"), &m_HotKeyEdit.m_HotkeyCode);

		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT, _T("J"), szHotkeyExit, 64, szPath);
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_CTRL, _T("0"), szValue, 64, szPath);
		m_HotKeyExit.m_HotkeyCtrl = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_ALT, _T("0"), szValue, 64, szPath);
		m_HotKeyExit.m_HotkeyAlt = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_SHIFT, _T("0"), szValue, 64, szPath);
		m_HotKeyExit.m_HotkeyShift = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_CODE, _T("74"), szValue, 64, szPath);
		_stscanf_s(szValue, _T("%u"), &m_HotKeyExit.m_HotkeyCode);

#ifdef TWO_HOTKEY
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1, _T("D"), szHotkey1, 64, szPath);
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1_CTRL, _T("0"), szValue, 64, szPath);
		m_HotKey1.m_HotkeyCtrl = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1_ALT, _T("0"), szValue, 64, szPath);
		m_HotKey1.m_HotkeyAlt = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1_SHIFT, _T("0"), szValue, 64, szPath);
		m_HotKey1.m_HotkeyShift = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1_CODE, _T("68"), szValue, 64, szPath);
		_stscanf_s(szValue, _T("%u"), &m_HotKey1.m_HotkeyCode);

		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2, _T("G"), szHotkey2, 64, szPath);
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2_CTRL, _T("0"), szValue, 64, szPath);
		m_HotKey2.m_HotkeyCtrl = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2_ALT, _T("0"), szValue, 64, szPath);
		m_HotKey2.m_HotkeyAlt = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2_SHIFT, _T("0"), szValue, 64, szPath);
		m_HotKey2.m_HotkeyShift = _tcscmp(szValue, _T("0")) != 0;
		GetPrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2_CODE, _T("71"), szValue, 64, szPath);
		_stscanf_s(szValue, _T("%u"), &m_HotKey2.m_HotkeyCode);
#endif

		GetPrivateProfileString(HISTORY_SECTION, HISTORY_POINT_COUNT, _T("0"), szValue, 64, szPath);
		unsigned int count = 0;
		_stscanf_s(szValue, _T("%u"), &count);
		for (unsigned int i=0; i<count; i++) {
			_stprintf_s(szValue, 64, _T("%s%u"), HISTORY_POINT, i);
			GetPrivateProfileString(HISTORY_SECTION, szValue, _T(""), szValue, 64, szPath);
			if (szValue && _tcslen(szValue)) {
				int x, y;
				_stscanf_s(szValue, _T("%d,%d"), &x, &y);
				mPoints.push_back(CPoint(x, y));
				this->RefreshList(i);
			}
		}
	}
	GetDlgItem(IDC_COUNT)->SetWindowText(szCount);
	GetDlgItem(IDC_HOTKEY)->SetWindowText(szHotkey);
	GetDlgItem(IDC_HOTKEYEXIT)->SetWindowText(szHotkeyExit);
#ifdef TWO_HOTKEY
	GetDlgItem(IDC_HOTKEY1)->SetWindowText(szHotkey1);
	GetDlgItem(IDC_HOTKEY2)->SetWindowText(szHotkey2);
#endif

	BOOL bRet = TRUE;
	bRet = RegisterHotKey(m_hWnd, HOTKEY_CLICK,
		((m_HotKeyEdit.m_HotkeyCtrl)?MOD_CONTROL:0) |
		((m_HotKeyEdit.m_HotkeyAlt)?MOD_ALT:0) |
		((m_HotKeyEdit.m_HotkeyShift)?MOD_SHIFT:0),
		m_HotKeyEdit.m_HotkeyCode);
	if (!bRet) {
		TCHAR sz[256];
		_stprintf_s(sz, _countof(sz), _T("热键 %s 已被注册\n"), szHotkey);
		MessageBox(sz);
		OnCancel();
	}
	bRet = RegisterHotKey(m_hWnd, HOTKEY_EXIT,
		((m_HotKeyExit.m_HotkeyCtrl)?MOD_CONTROL:0) |
		((m_HotKeyExit.m_HotkeyAlt)?MOD_ALT:0) |
		((m_HotKeyExit.m_HotkeyShift)?MOD_SHIFT:0),
		m_HotKeyExit.m_HotkeyCode);
	if (!bRet) {
		TCHAR sz[256];
		_stprintf_s(sz, _countof(sz), _T("热键 %s 已被注册\n"), szHotkeyExit);
		MessageBox(sz);
		OnCancel();
	}
#ifdef TWO_HOTKEY
	bRet = RegisterHotKey(m_hWnd, HOTKEY1,
		((m_HotKey1.m_HotkeyCtrl)?MOD_CONTROL:0) |
		((m_HotKey1.m_HotkeyAlt)?MOD_ALT:0) |
		((m_HotKey1.m_HotkeyShift)?MOD_SHIFT:0),
		m_HotKey1.m_HotkeyCode);
	if (!bRet) {
		TCHAR sz[256];
		_stprintf_s(sz, _countof(sz), _T("热键 %s 已被注册\n"), szHotkey1);
		MessageBox(sz);
		OnCancel();
	}
	bRet = RegisterHotKey(m_hWnd, HOTKEY2,
		((m_HotKey2.m_HotkeyCtrl)?MOD_CONTROL:0) |
		((m_HotKey2.m_HotkeyAlt)?MOD_ALT:0) |
		((m_HotKey2.m_HotkeyShift)?MOD_SHIFT:0),
		m_HotKey2.m_HotkeyCode);
	if (!bRet) {
		TCHAR sz[256];
		_stprintf_s(sz, _countof(sz), _T("热键 %s 已被注册\n"), szHotkey2);
		MessageBox(sz);
		OnCancel();
	}
#endif

	delete [] mem;

	InstallKbHook();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAutoMouseClickDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAutoMouseClickDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


UINT _cdecl MouseClickThread(LPVOID pParam)
{
	CAutoMouseClickDlg *pThis = (class CAutoMouseClickDlg*)pParam;
	LARGE_INTEGER freq, now, last, elapsed;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&last);
	QueryPerformanceCounter(&now);
	while (!pThis->mStopThread/* && pThis->mClickCounter < pThis->mClickCount*/) {
		if (pThis->mPoints.size()) {
			for (unsigned int i=0; i<pThis->mPoints.size(); i++) {
				pThis->MouseMoveAbsolute(&pThis->mBufMouse, pThis->mPoints.at(i).x, pThis->mPoints.at(i).y);
				pThis->MouseClick(&pThis->mBufMouse);
			}
		} else
			pThis->MouseClick(&pThis->mBufMouse);

		elapsed.QuadPart = 0;
		while (elapsed.QuadPart < pThis->mClickInterval) {
			QueryPerformanceCounter(&now);
			elapsed.QuadPart = now.QuadPart - last.QuadPart;
			elapsed.QuadPart *= 1000;
			elapsed.QuadPart /= freq.QuadPart;
		}
		last = now;
	}
	pThis->mStopThread = true;
	pThis->mClickThread = NULL;
	return 0;
}

void CAutoMouseClickDlg::initWnds()
{
	for (unsigned int i=0; i<mPoints.size(); i++) {
		mWnds.push_back(WindowFromPoint(mPoints.at(i)));
	}
}

int CAutoMouseClickDlg::SendHotkey(CHotkeyEdit* hk)
{
	if (mWnds.size() == 0 && mPoints.size() != 0) initWnds();

	vector<CWnd*>::iterator it = mWnds.begin();
	CWnd *wnd;
	int altb = 0;

	while (it != mWnds.end()) {
		if (!*it || !::IsWindow((*it)->GetSafeHwnd())) {
			it++;
			continue;
		}
		wnd = *it;
		//while (wnd->GetParent()) wnd = wnd->GetParent();
		if (hk->m_HotkeyCtrl)
			(*it)->PostMessage(WM_SYSKEYDOWN, VK_CONTROL, 1);
		if (hk->m_HotkeyAlt) {
			(*it)->PostMessage(WM_SYSKEYDOWN, VK_MENU, 1 | (1 << 29));
			altb = 1;
		}
		if (hk->m_HotkeyShift)
			(*it)->PostMessage(WM_SYSKEYDOWN, VK_SHIFT, 1);
		if (hk->m_HotkeyCtrl || hk->m_HotkeyAlt || hk->m_HotkeyShift) {
			(*it)->PostMessage(WM_SYSKEYDOWN, hk->m_HotkeyCode, 1 | (altb << 29));
			(*it)->PostMessage(WM_SYSKEYUP, hk->m_HotkeyCode, 1 | (3 << 30) | (altb << 29));
		} else {
			(*it)->PostMessage(WM_KEYDOWN, hk->m_HotkeyCode, 1 | (altb << 29));
			(*it)->PostMessage(WM_KEYUP, hk->m_HotkeyCode, 1 | (3 << 30) | (altb << 29));
		}
		if (hk->m_HotkeyShift)
			(*it)->PostMessage(WM_SYSKEYUP, VK_SHIFT, 1 | (3 << 30));
		if (hk->m_HotkeyAlt) {
			(*it)->PostMessage(WM_SYSKEYUP, VK_MENU, 1 | (3 << 30));
			altb = 0;
		}
		if (hk->m_HotkeyCtrl)
			(*it)->PostMessage(WM_SYSKEYUP, VK_CONTROL, 1 | (3 << 30));
		it++;
	}
	return 0;
}

void CAutoMouseClickDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	POINT mouse;

	switch (nHotKeyId) {
	case HOTKEY_CLICK:
		if (mClickThread) {
			mStopThread = true;
			mClickThread = NULL;
			//KillTimer(1);
		}
		else if (GetCursorPos(&mouse)) {
			//SendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(mouse.x, mouse.y));
			//SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(mouse.x, mouse.y));
			mClickCounter = 0;
			mClickInterval = 1000 / mClickCount;
			MouseSetup(&mBufMouse, mouse);
			LPVOID pParam = this;
			mStopThread = false;
			mClickThread = AfxBeginThread(MouseClickThread, pParam);
			//SetTimer(1, mClickInterval, NULL);
		}
		break;
	case HOTKEY_EXIT:
		//OnCancel();
		mStopThread = true;
		mClickThread = NULL;
		break;
#ifdef TWO_HOTKEY
	case HOTKEY1:
		SendHotkey(&m_HotKey1);
		break;
	case HOTKEY2:
		SendHotkey(&m_HotKey2);
		break;
#endif
	default:
		CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
		break;
	}
}


void CAutoMouseClickDlg::OnClose()
{
	CDialogEx::OnClose();
}

void CAutoMouseClickDlg::MouseSetup(INPUT *buffer, POINT p)
{
    buffer->type = INPUT_MOUSE;
    //buffer->mi.dx = (p.x * (0xFFFF / mScrX));
    //buffer->mi.dy = (p.y * (0xFFFF / mScrY));
    buffer->mi.mouseData = 0;
    buffer->mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
    buffer->mi.time = 0;
    buffer->mi.dwExtraInfo = 0;
}


void CAutoMouseClickDlg::MouseMoveAbsolute(INPUT *buffer, int x, int y)
{
    buffer->mi.dx = (x * 0xFFFF / mScrX);
    buffer->mi.dy = (y * 0xFFFF / mScrY);
    buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK |  MOUSEEVENTF_MOVE);

    SendInput(1, buffer, sizeof(INPUT));
}


void CAutoMouseClickDlg::MouseClick(INPUT *buffer)
{
    buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
    SendInput(1, buffer, sizeof(INPUT));

    //Sleep(10);

    buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP);
    SendInput(1, buffer, sizeof(INPUT));

	mClickCounter++;
}


void CAutoMouseClickDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	//if (mClickThread) MouseMoveAbsolute(&mBufMouse, point.x, point.y);

	//TCHAR sz[256];
	//_stprintf_s(sz, _countof(sz), _T("[%d, %d]"), point.x, point.y);

	CDialogEx::OnMouseMove(nFlags, point);
}



void CAutoMouseClickDlg::OnEnChangeCount()
{
	TCHAR sz[100];
	GetDlgItem(IDC_COUNT)->GetWindowText(sz, 100);
	_stscanf_s(sz, _T("%d"), &mClickCount);
}


void CAutoMouseClickDlg::OnBnClickedExit()
{
	OnCancel();
}


void CAutoMouseClickDlg::OnDestroy()
{
	BOOL ret;
	DWORD error;
	TCHAR *mem = new TCHAR[MAX_PATH + 1024 + 64*3];
	TCHAR *szPath = mem, 
		  *szValue = szPath + MAX_PATH, 
		  *szHotkey = szValue + 1024, 
		  *szHotkeyExit = szHotkey + 64, 
#ifdef TWO_HOTKEY
		  *szHotkey1 = szHotkeyExit + 64, 
		  *szHotkey2 = szHotkey1 + 64,
#endif
		  *szName = szHotkeyExit + 64;
	unsigned int i;

	UninstallKbHook();
	try {
		UnregisterHotKey(m_hWnd, HOTKEY_CLICK);
		UnregisterHotKey(m_hWnd, HOTKEY_EXIT);
#ifdef TWO_HOTKEY
		UnregisterHotKey(m_hWnd, HOTKEY1);
		UnregisterHotKey(m_hWnd, HOTKEY2);
#endif

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath))) {
			ret = PathAppend(szPath, _T("AutoMouseClick"));
			if (!ret) throw _T("PathAppend:275");
			ret = CreateDirectory(szPath, NULL);
			if (!ret) {
				error = GetLastError();
				if (error == ERROR_PATH_NOT_FOUND) throw _T("ERROR_PATH_NOT_FOUND:277");
			}
			ret = PathAppend(szPath, _T("\\history.ini"));
			if (!ret) throw _T("PathAppend:282");
			ret = WritePrivateProfileSection(HISTORY_SECTION, _T(""), szPath);
			if (!ret) throw _T("WritePrivateProfileSection:284");
			m_HotKeyEdit.GetWindowText(szHotkey, 64);
			m_HotKeyExit.GetWindowText(szHotkeyExit, 64);
#ifdef TWO_HOTKEY
			m_HotKey1.GetWindowText(szHotkey1, 64);
			m_HotKey2.GetWindowText(szHotkey2, 64);
#endif
			_stprintf_s(szValue, 1024, _T("%d"), mClickCount);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_COUNT, szValue, szPath);

			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY, szHotkey, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKeyEdit.m_HotkeyCtrl)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_CTRL, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKeyEdit.m_HotkeyAlt)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_ALT, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKeyEdit.m_HotkeyShift)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_SHIFT, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%u"), m_HotKeyEdit.m_HotkeyCode);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_CODE, szValue, szPath);

			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT, szHotkeyExit, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKeyExit.m_HotkeyCtrl)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_CTRL, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKeyExit.m_HotkeyAlt)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_ALT, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKeyExit.m_HotkeyShift)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_SHIFT, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%u"), m_HotKeyExit.m_HotkeyCode);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_CODE, szValue, szPath);

#ifdef TWO_HOTKEY
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1, szHotkey1, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKey1.m_HotkeyCtrl)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1_CTRL, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKey1.m_HotkeyAlt)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1_ALT, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKey1.m_HotkeyShift)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1_SHIFT, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%u"), m_HotKey1.m_HotkeyCode);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY1_CODE, szValue, szPath);

			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2, szHotkey2, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKey2.m_HotkeyCtrl)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2_CTRL, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKey2.m_HotkeyAlt)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2_ALT, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%d"), (m_HotKey2.m_HotkeyShift)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2_SHIFT, szValue, szPath);
			_stprintf_s(szValue, 1024, _T("%u"), m_HotKey2.m_HotkeyCode);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY2_CODE, szValue, szPath);
#endif

			_stprintf_s(szValue, 1024, _T("%d"), mPoints.size());
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_POINT_COUNT, szValue, szPath);
			for (i=0; i<mPoints.size(); i++) {
				_stprintf_s(szName, 64, _T("%s%u"), HISTORY_POINT, i);
				_stprintf_s(szValue, 1024, _T("%d,%d"), mPoints.at(i).x, mPoints.at(i).y);
				WritePrivateProfileString(HISTORY_SECTION, szName, szValue, szPath);
			}
		}
	} catch (TCHAR *e) {
		MessageBox(e);
	}
	delete [] mem;

	CDialogEx::OnDestroy();
}

void CAutoMouseClickDlg::OnOK()
{
}

void CAutoMouseClickDlg::RefreshList(int index)
{
	TCHAR strMsg[100] = {0};
	if (m_List->GetTextLen(index)) m_List->DeleteString(index);
	_stprintf_s(strMsg, _countof(strMsg), _T("[%d, %d]"), mPoints.at(index).x, mPoints.at(index).y);
	m_List->InsertString(index, strMsg);
	//m_List->AddString(strMsg);
}

LRESULT CALLBACK LowLevelKeyboardProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;
	static bool ctrl = false, alt = false, shift = false;

	switch (nCode) {
	case HC_ACTION:
		if (wParam == WM_KEYDOWN || wParam == WM_KEYUP ||
			wParam == WM_SYSKEYDOWN || wParam == WM_SYSKEYUP) {
				if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
					if (pkbhs->vkCode == VK_CONTROL || pkbhs->vkCode == VK_LCONTROL || pkbhs->vkCode == VK_RCONTROL) ctrl = true;
					else if (pkbhs->vkCode == VK_MENU || pkbhs->vkCode == VK_LMENU || pkbhs->vkCode == VK_RMENU) alt = true;
					else if (pkbhs->vkCode == VK_SHIFT || pkbhs->vkCode == VK_LSHIFT || pkbhs->vkCode == VK_RSHIFT) shift = true;
					else if (pkbhs->vkCode == g_Dialog->m_HotKeyEdit.m_HotkeyCode &&
						ctrl == g_Dialog->m_HotKeyEdit.m_HotkeyCtrl &&
						alt == g_Dialog->m_HotKeyEdit.m_HotkeyAlt &&
						shift == g_Dialog->m_HotKeyEdit.m_HotkeyShift) break;
					else if (pkbhs->vkCode == g_Dialog->m_HotKeyExit.m_HotkeyCode &&
						ctrl == g_Dialog->m_HotKeyExit.m_HotkeyCtrl &&
						alt == g_Dialog->m_HotKeyExit.m_HotkeyAlt &&
						shift == g_Dialog->m_HotKeyExit.m_HotkeyShift) break;
				} else {
					if (pkbhs->vkCode == VK_CONTROL || pkbhs->vkCode == VK_LCONTROL || pkbhs->vkCode == VK_RCONTROL) ctrl = false;
					else if (pkbhs->vkCode == VK_MENU || pkbhs->vkCode == VK_LMENU || pkbhs->vkCode == VK_RMENU) alt = false;
					else if (pkbhs->vkCode == VK_SHIFT || pkbhs->vkCode == VK_LSHIFT || pkbhs->vkCode == VK_RSHIFT) shift = false;
					else if (pkbhs->vkCode == g_Dialog->m_HotKeyEdit.m_HotkeyCode &&
						ctrl == g_Dialog->m_HotKeyEdit.m_HotkeyCtrl &&
						alt == g_Dialog->m_HotKeyEdit.m_HotkeyAlt &&
						shift == g_Dialog->m_HotKeyEdit.m_HotkeyShift) break;
					else if (pkbhs->vkCode == g_Dialog->m_HotKeyExit.m_HotkeyCode &&
						ctrl == g_Dialog->m_HotKeyExit.m_HotkeyCtrl &&
						alt == g_Dialog->m_HotKeyExit.m_HotkeyAlt &&
						shift == g_Dialog->m_HotKeyExit.m_HotkeyShift) break;
				}

				if (g_Dialog->mWnds.size() == 0 && g_Dialog->mPoints.size() != 0) g_Dialog->initWnds();
				vector<CWnd*>::iterator it = g_Dialog->mWnds.begin();
				HWND current = ::GetFocus();
				CWnd *wnd;

				if (g_Dialog->GetDlgItem(IDC_HOTKEY)->GetSafeHwnd() == current ||
					g_Dialog->GetDlgItem(IDC_HOTKEYEXIT)->GetSafeHwnd() == current) break;
				current = ::GetForegroundWindow();
				while (it != g_Dialog->mWnds.end()) {
					if (!*it || !::IsWindow((*it)->GetSafeHwnd())) {
						it++;
						continue;
					}
					wnd = *it;
					while (wnd->GetParent()) wnd = wnd->GetParent();
					if (wnd->GetSafeHwnd() == current) {
						it++;
						continue;
					}
					(*it)->PostMessage(wParam, pkbhs->vkCode, 1 | (pkbhs->scanCode << 16) | 
						((pkbhs->flags | LLKHF_EXTENDED) << 24) | ((pkbhs->flags | LLKHF_ALTDOWN) << 29) | 
						(((wParam == WM_KEYUP || wParam == WM_SYSKEYUP)?1:0) << 30) |
						((pkbhs->flags | LLKHF_UP) << 31));
					it++;
				}
		}
		break;
	default:
		break;
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    MSLLHOOKSTRUCT *pkbhs = (MSLLHOOKSTRUCT *)lParam;

    switch (nCode)
    {
		case HC_ACTION:
        {
			//鼠标移动
			if (wParam == WM_MOUSEMOVE) 
			{
				g_Dialog->mPoints.at(g_Dialog->mPoints.size()-1) = pkbhs->pt;
				g_Dialog->RefreshList(g_Dialog->mPoints.size()-1);
			}

			//鼠标左击
			if (wParam == WM_LBUTTONDOWN)
			{
				g_Dialog->mPoints.at(g_Dialog->mPoints.size()-1) = pkbhs->pt;
				g_Dialog->RefreshList(g_Dialog->mPoints.size()-1);
				if (g_Dialog->mWnds.size() == 0)
					g_Dialog->initWnds();
				else
					g_Dialog->mWnds.push_back(g_Dialog->WindowFromPoint(pkbhs->pt));
				UninstallMsHook();
				return 1;
			}

// 			//滚轮事件
// 			if (wParam == WM_MOUSEWHEEL)
// 			{
// 				sprintf(strMsg, "WM_MOUSEWHEEL: %d\n", HIWORD(pkbhs->mouseData));
// 				OutputDebugString(strMsg);
// 			}
        }
	default:
		break;
    }
    return CallNextHookEx (NULL, nCode, wParam, lParam);
}

BOOL InstallKbHook( )
{
	
    if (hhookKb)
        UninstallKbHook();
	
    hhookKb = SetWindowsHookEx(WH_KEYBOARD_LL, 
        (HOOKPROC)LowLevelKeyboardProc, AfxGetApp()->m_hInstance, NULL);
	
    return (hhookKb != NULL);
	
}

BOOL UninstallKbHook()
{
	
    BOOL fOk = FALSE;
    if (hhookKb) {
        fOk = UnhookWindowsHookEx(hhookKb);
        hhookKb = NULL;
    }
	
    return (fOk);
}

BOOL InstallMsHook( )
{
    if (hhookMs)
        UninstallMsHook();
	
    hhookMs = SetWindowsHookEx(WH_MOUSE_LL, 
        (HOOKPROC)LowLevelMouseProc, AfxGetApp()->m_hInstance, NULL);
	
    return (hhookMs != NULL);
	
}

BOOL UninstallMsHook()
{
	
    BOOL fOk = FALSE;
    if (hhookMs) {
        fOk = UnhookWindowsHookEx(hhookMs);
        hhookMs = NULL;
    }
	
    return (fOk);
}

void CAutoMouseClickDlg::OnBnClickedAdd()
{
	//HDC hdc = ::GetDC(NULL);

	//Gdiplus::GdiplusStartupInput input;
	//input.GdiplusVersion = 1;
	//input.DebugEventCallback = NULL;
	//input.SuppressBackgroundThread = input.SuppressExternalCodecs = FALSE;
	//Gdiplus::GdiplusStartup(&token, &input, NULL);

	//Gdiplus::Graphics g( hdc );
	//Gdiplus::Color color( 192, 255, 0, 0 );
	//Gdiplus::Rect rectangle( 0, 0, 400, 400 );
	//Gdiplus::SolidBrush solidBrush( color );
	//g.FillRectangle( &solidBrush, rectangle );
	
	//::Rectangle(hdc, 0, 0, 400, 400);

	mPoints.push_back(CPoint(0, 0));
	InstallMsHook();
}


void CAutoMouseClickDlg::OnBnClickedDel()
{
	//Gdiplus::GdiplusShutdown(token);
	//UninstallKbHook();
	if (m_List->GetCurSel() == -1) return;
	mPoints.erase(mPoints.begin() + m_List->GetCurSel());
	if (mWnds.size() > m_List->GetCurSel())
		mWnds.erase(mWnds.begin() + m_List->GetCurSel());
	m_List->DeleteString(m_List->GetCurSel());
}
