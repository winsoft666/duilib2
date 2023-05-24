#pragma once

//////////////////////////////////////////////////////////////////////////
///

#ifdef UILIB_WITH_CEF
class CTranparentCEFWnd : public WindowImplBase
{
public:
	CTranparentCEFWnd(void);
	~CTranparentCEFWnd(void);

public:
	virtual void OnFinalMessage( HWND );
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName( void ) const;
	virtual void InitWindow();

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

	virtual LRESULT OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ResponseDefaultKeyEvent(WPARAM wParam);

private:
	CCefUI* m_pCef;
};
#endif
