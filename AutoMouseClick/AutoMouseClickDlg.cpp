
// AutoMouseClickDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AutoMouseClick.h"
#include "AutoMouseClickDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAutoMouseClickDlg 对话框




CAutoMouseClickDlg::CAutoMouseClickDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAutoMouseClickDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoMouseClickDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HOTKEY, m_HotKeyEdit);
	DDX_Control(pDX, IDC_HOTKEYEXIT, m_HotKeyExit);
	m_HotKeyEdit.SetFlag(HOTKEY_CLICK);
	m_HotKeyExit.SetFlag(HOTKEY_EXIT);
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
	
	TCHAR szPath[MAX_PATH], szCount[64], szHotkey[64], szHotkeyExit[64], szValue[64];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath))) {
		PathAppend(szPath, _T("AutoMouseClick"));
		PathAppend(szPath, _T("\\history.ini"));
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
	}
	GetDlgItem(IDC_COUNT)->SetWindowText(szCount);
	GetDlgItem(IDC_HOTKEY)->SetWindowText(szHotkey);
	GetDlgItem(IDC_HOTKEYEXIT)->SetWindowText(szHotkeyExit);
	RegisterHotKey(m_hWnd, HOTKEY_CLICK,
		((m_HotKeyEdit.m_HotkeyCtrl)?MOD_CONTROL:0) |
		((m_HotKeyEdit.m_HotkeyAlt)?MOD_ALT:0) |
		((m_HotKeyEdit.m_HotkeyShift)?MOD_SHIFT:0),
		m_HotKeyEdit.m_HotkeyCode);
	RegisterHotKey(m_hWnd, HOTKEY_EXIT,
		((m_HotKeyExit.m_HotkeyCtrl)?MOD_CONTROL:0) |
		((m_HotKeyExit.m_HotkeyAlt)?MOD_ALT:0) |
		((m_HotKeyExit.m_HotkeyShift)?MOD_SHIFT:0),
		m_HotKeyExit.m_HotkeyCode);

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
		OnCancel();
		break;
	}

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}


void CAutoMouseClickDlg::OnClose()
{
	CDialogEx::OnClose();
}

void CAutoMouseClickDlg::MouseSetup(INPUT *buffer, POINT p)
{
    buffer->type = INPUT_MOUSE;
    buffer->mi.dx = (p.x * (0xFFFF / mScrX));
    buffer->mi.dy = (p.y * (0xFFFF / mScrY));
    buffer->mi.mouseData = 0;
    buffer->mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
    buffer->mi.time = 0;
    buffer->mi.dwExtraInfo = 0;
}


void CAutoMouseClickDlg::MouseMoveAbsolute(INPUT *buffer, int x, int y)
{
    buffer->mi.dx = (x * (0xFFFF / mScrX));
    buffer->mi.dy = (y * (0xFFFF / mScrY));
    //buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);

    //SendInput(1, buffer, sizeof(INPUT));
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
	if (mClickThread) MouseMoveAbsolute(&mBufMouse, point.x, point.y);

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

	try {
		UnregisterHotKey(m_hWnd, HOTKEY_CLICK);
		UnregisterHotKey(m_hWnd, HOTKEY_EXIT);

		TCHAR szPath[MAX_PATH*4], szValue[1024], szHotkey[64], szHotkeyExit[64];
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
			memset(szValue, 0, sizeof(szValue));
			_stprintf_s(szValue, 1023, _T("%d"), mClickCount);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_COUNT, szValue, szPath);

			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY, szHotkey, szPath);
			_stprintf_s(szValue, 1023, _T("%d"), (m_HotKeyEdit.m_HotkeyCtrl)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_CTRL, szValue, szPath);
			_stprintf_s(szValue, 1023, _T("%d"), (m_HotKeyEdit.m_HotkeyAlt)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_ALT, szValue, szPath);
			_stprintf_s(szValue, 1023, _T("%d"), (m_HotKeyEdit.m_HotkeyShift)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_SHIFT, szValue, szPath);
			_stprintf_s(szValue, 1023, _T("%u"), m_HotKeyEdit.m_HotkeyCode);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_CODE, szValue, szPath);

			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT, szHotkeyExit, szPath);
			_stprintf_s(szValue, 1023, _T("%d"), (m_HotKeyExit.m_HotkeyCtrl)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_CTRL, szValue, szPath);
			_stprintf_s(szValue, 1023, _T("%d"), (m_HotKeyExit.m_HotkeyAlt)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_ALT, szValue, szPath);
			_stprintf_s(szValue, 1023, _T("%d"), (m_HotKeyExit.m_HotkeyShift)?1:0);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_SHIFT, szValue, szPath);
			_stprintf_s(szValue, 1023, _T("%u"), m_HotKeyExit.m_HotkeyCode);
			WritePrivateProfileString(HISTORY_SECTION, HISTORY_HOTKEY_EXIT_CODE, szValue, szPath);
		}
	} catch (TCHAR *e) {
		MessageBox(e);
	}

	CDialogEx::OnDestroy();
}

void CAutoMouseClickDlg::OnOK()
{
}