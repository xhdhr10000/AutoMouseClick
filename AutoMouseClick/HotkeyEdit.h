#pragma once


// CHotkeyEdit

class CHotkeyEdit : public CEdit
{
	DECLARE_DYNAMIC(CHotkeyEdit)

public:
	CHotkeyEdit();
	virtual ~CHotkeyEdit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SetFlag(int flag);

protected:
	DECLARE_MESSAGE_MAP()

public:
	bool m_HotkeyCtrl, m_HotkeyShift, m_HotkeyAlt;
	WPARAM m_HotkeyCode;
private:
	int m_Flag;
};


