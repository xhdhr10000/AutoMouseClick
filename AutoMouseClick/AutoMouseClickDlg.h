
// AutoMouseClickDlg.h : ͷ�ļ�
//

#pragma once

#include <iostream>
#include <vector>
#include "HotkeyEdit.h"

using namespace std;

// CAutoMouseClickDlg �Ի���
class CAutoMouseClickDlg : public CDialogEx
{
// ����
public:
	CAutoMouseClickDlg(CWnd* pParent = NULL);	// ��׼���캯��
	void MouseSetup(INPUT *buffer, POINT p);
	void MouseMoveAbsolute(INPUT *buffer, int x, int y);
	void MouseClick(INPUT *buffer);
	void RefreshList(int index);
	int SendHotkey(CHotkeyEdit *hk);
	void initWnds();

// �Ի�������
	enum { IDD = IDD_AUTOMOUSECLICK_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
public:
	INPUT mBufMouse;
	bool mStopThread, mStopPaintThread, mPaintUpdate;
	int mClickCounter;
	int mClickCount;
	int mClickInterval;
	CWinThread *mClickThread, *mPaintThread;
	CHotkeyEdit m_HotKeyEdit, m_HotKeyExit;
#ifdef TWO_HOTKEY
	CHotkeyEdit m_HotKey1, m_HotKey2;
#endif
	CListBox *m_List;
	vector<CPoint> mPoints;
	BOOL mAddingPoint;
	vector<CWnd*> mWnds;
	CDialog *m_Dialog;
	int mScrX, mScrY;

protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnClose();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnEnChangeCount();
	afx_msg void OnBnClickedExit();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDel();
};
