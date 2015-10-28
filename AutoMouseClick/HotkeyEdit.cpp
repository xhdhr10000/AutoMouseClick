// HotkeyEdit.cpp : implementation file
//

#include "stdafx.h"
#include "AutoMouseClick.h"
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
			GetKeyNameText(pMsg->lParam, key, 16);
			_stprintf_s(sz, 100, _T("%s%s%s%s"),
				(ctrl)?_T("Ctrl + "):_T(""),
				(alt)?_T("Alt + "):_T(""),
				(shift)?_T("Shift + "):_T(""),
				key);
			this->SetWindowText(sz);
			if (this->m_Flag == HOTKEY_CLICK) {
				UnregisterHotKey(this->GetParent()->GetSafeHwnd(), HOTKEY_CLICK);
				RegisterHotKey(this->GetParent()->GetSafeHwnd(), HOTKEY_CLICK,
					((ctrl)?MOD_CONTROL:0) | ((alt)?MOD_ALT:0) | ((shift)?MOD_SHIFT:0),
					pMsg->wParam);
			} else if (this->m_Flag == HOTKEY_EXIT) {
				UnregisterHotKey(this->GetParent()->GetSafeHwnd(), HOTKEY_EXIT);
				RegisterHotKey(this->GetParent()->GetSafeHwnd(), HOTKEY_EXIT,
					((ctrl)?MOD_CONTROL:0) | ((alt)?MOD_ALT:0) | ((shift)?MOD_SHIFT:0),
					pMsg->wParam);
			}
			m_HotkeyCtrl = ctrl;
			m_HotkeyAlt = alt;
			m_HotkeyShift = shift;
			m_HotkeyCode = pMsg->wParam;

			return TRUE;
		}
	}

	return FALSE;
}

void CHotkeyEdit::SetFlag(int flag)
{
	this->m_Flag = flag;
}

BEGIN_MESSAGE_MAP(CHotkeyEdit, CEdit)
END_MESSAGE_MAP()



// CHotkeyEdit message handlers
