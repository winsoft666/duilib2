#include "StdAfx.h"
#include "TranparentCefWnd.h"
#ifdef UILIB_WITH_CEF

DUI_BEGIN_MESSAGE_MAP(CTranparentCEFWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
DUI_END_MESSAGE_MAP()

CTranparentCEFWnd::CTranparentCEFWnd(void) : 
	m_pCef(NULL)
{
}

CTranparentCEFWnd::~CTranparentCEFWnd(void)
{
}

void CTranparentCEFWnd::OnFinalMessage( HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CTranparentCEFWnd::GetSkinFile()
{
	return _T("transparent_cef.xml");
}

LPCTSTR CTranparentCEFWnd::GetWindowClassName( void ) const
{
	return _T("TranparentWnd");
}

void CTranparentCEFWnd::OnClick( TNotifyUI &msg )
{
	CDuiString sName = msg.pSender->GetName();
	sName.MakeLower();
}

LRESULT CTranparentCEFWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 {
	 bHandled = FALSE;
	 return 0;
 }

LRESULT CTranparentCEFWnd::ResponseDefaultKeyEvent(WPARAM wParam) {
	if (wParam == VK_ESCAPE) {
		Close();
		return TRUE;
	}
	else if (wParam == VK_RETURN) {
		return TRUE;
	}
	return FALSE;
}

LRESULT CTranparentCEFWnd::OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	bHandled = FALSE;
	return 0L;
}

void CTranparentCEFWnd::InitWindow()
{
	m_pCef = static_cast<CCefUI*>(m_PaintManager.FindControl(TEXT("cef")));

	if (m_pCef) {
		m_pCef->SetUrl((GetCurrentProcessDirectoryW() +  TEXT("../../../test-resource/lrc.html")).c_str());
	}
}
#endif
