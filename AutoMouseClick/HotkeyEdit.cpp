// HotkeyEdit.cpp : implementation file
//

#include "stdafx.h"
#include "AutoMouseClick.h"
#include "AutoMouseClickDlg.h"
#include "HotkeyEdit.h"

// CHotkeyEdit

IMPLEMENT_DYNAMIC(CHotkeyEdit, CEdit)

CHotkeyEdit::CHotkeyEdit()
{

}

CHotkeyEdit::~CHotkeyEdit()
{
}

BOOL CHotkeyEdit::PreTranslateMessage(MSG* pMsg)
{
	TCHAR sz[100], key[16];
	bool ctrl = false, alt = false, shift = false;
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) {
		if (pMsg->wParam != VK_CONTROL && pMsg->wParam != VK_SHIFT && pMsg->wParam != VK_MENU) {
			ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
			shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			if (this->m_Flag == HOTKEY_CLICK) {
				if (m_Parent->m_HotKeyExit.m_HotkeyCtrl == ctrl &&
					m_Parent->m_HotKeyExit.m_HotkeyAlt == alt &&
					m_Parent->m_HotKeyExit.m_HotkeyShift == shift &&
					m_Parent->m_HotKeyExit.m_HotkeyCode == pMsg->wParam) {
						MessageBox(_T("Already Registered!"));
						return TRUE;
				}
				UnregisterHotKey(this->GetParent()->GetSafeHwnd(), HOTKEY_CLICK);
				RegisterHotKey(this->GetParent()->GetSafeHwnd(), HOTKEY_CLICK,
					((ctrl)?MOD_CONTROL:0) | ((alt)?MOD_ALT:0) | ((shift)?MOD_SHIFT:0),
					pMsg->wParam);
			} else if (this->m_Flag == HOTKEY_EXIT) {
				if (m_Parent->m_HotKeyEdit.m_HotkeyCtrl == ctrl &&
					m_Parent->m_HotKeyEdit.m_HotkeyAlt == alt &&
					m_Parent->m_HotKeyEdit.m_HotkeyShift == shift &&
					m_Parent->m_HotKeyEdit.m_HotkeyCode == pMsg->wParam) {
						MessageBox(_T("Already Registered!"));
						return TRUE;
				}
				UnregisterHotKey(this->GetParent()->GetSafeHwnd(), HOTKEY_EXIT);
				RegisterHotKey(this->GetParent()->GetSafeHwnd(), HOTKEY_EXIT,
					((ctrl)?MOD_CONTROL:0) | ((alt)?MOD_ALT:0) | ((shift)?MOD_SHIFT:0),
					pMsg->wParam);
			}
			m_HotkeyCtrl = ctrl;
			m_HotkeyAlt = alt;
			m_HotkeyShift = shift;
			m_HotkeyCode = pMsg->wParam;

			GetKeyNameText(pMsg->lParam, key, 16);
			_stprintf_s(sz, _countof(sz), _T("%s%s%s%s"),
				(ctrl)?_T("Ctrl + "):_T(""),
				(alt)?_T("Alt + "):_T(""),
				(shift)?_T("Shift + "):_T(""),
				key);
			this->SetWindowText(sz);

			return TRUE;
		}
	}

	return FALSE;
}

void CHotkeyEdit::SetFlag(int flag)
{
	this->m_Flag = flag;
}

void CHotkeyEdit::SetParent(CAutoMouseClickDlg *p)
{
	m_Parent = p;
}

BEGIN_MESSAGE_MAP(CHotkeyEdit, CEdit)
END_MESSAGE_MAP()



// CHotkeyEdit message handlers
