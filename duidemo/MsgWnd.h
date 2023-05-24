#pragma once

#define MSGID_YES	1
#define MSGID_NO	0

class CMsgWnd : public WindowImplBase
{
public:
	static int MessageBox(HWND hParent, LPCTSTR lpstrTitle, LPCTSTR lpstrMsg)
	{
		CMsgWnd* pWnd = new CMsgWnd();
		pWnd->Create(hParent, _T("msgwnd"), WS_POPUP | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW);
		pWnd->CenterWindow();
		pWnd->SetTitle(lpstrTitle);
		pWnd->SetMsg(lpstrMsg);
		return pWnd->ShowModal();
	}

	static void ShowMessageBox(HWND hParent, LPCTSTR lpstrTitle, LPCTSTR lpstrMsg)
	{
		CMsgWnd* pWnd = new CMsgWnd();
		pWnd->Create(hParent, _T("msgwnd"), UI_WNDSTYLE_FRAME, 0);
		pWnd->CenterWindow();
		pWnd->SetTitle(lpstrTitle);
		pWnd->SetMsg(lpstrMsg);
		pWnd->ShowWindow(true);
	}

public:
	CMsgWnd(void);
	~CMsgWnd(void);

	void SetMsg(LPCTSTR lpstrMsg);
	void SetTitle(LPCTSTR lpstrTitle);

public:
	virtual void OnFinalMessage( HWND );
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName( void ) const;
    virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam) override;
	virtual void Notify( TNotifyUI &msg );
	virtual LRESULT OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
private:
};
