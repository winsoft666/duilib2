#include "StdAfx.h"
#include "MsgWnd.h"

CMsgWnd::CMsgWnd(void) {
}

CMsgWnd::~CMsgWnd(void) {
}

void CMsgWnd::SetTitle(LPCTSTR lpstrTitle) {
    if(lstrlen(lpstrTitle) <= 0)
        return;

    CControlUI *pControl = static_cast<CControlUI *>(m_PaintManager.FindControl(_T("lblTitle")));

    if( pControl )
        pControl->SetText(lpstrTitle);
}

void CMsgWnd::SetMsg(LPCTSTR lpstrMsg) {
    if(lstrlen(lpstrMsg) <= 0)
        return;

    CControlUI *pControl = static_cast<CControlUI *>(m_PaintManager.FindControl(_T("lblMsg")));
    if( pControl )
        pControl->SetText(lpstrMsg);
}

void CMsgWnd::OnFinalMessage( HWND hWnd) {
    __super::OnFinalMessage(hWnd);
    delete this;
}

CDuiString CMsgWnd::GetSkinFile() {
    return _T("msg.xml");
}

LPCTSTR CMsgWnd::GetWindowClassName( void ) const {
    return _T("MsgWnd");
}

LRESULT CMsgWnd::ResponseDefaultKeyEvent(WPARAM wParam) {
    if (wParam == VK_ESCAPE) {
        Close(MSGID_NO);
        return TRUE;
    }
    return FALSE;
}

void CMsgWnd::Notify( TNotifyUI &msg ) {
    if (msg.sType == DUI_MSGTYPE_CLICK) {
        CDuiString sName = msg.pSender->GetName();

        if (sName.CompareNoCase(_T("btnClose")) == 0) {
            Close(MSGID_NO);
            return;
        } else if (sName.CompareNoCase(_T("btnYes")) == 0) {
            Close(MSGID_YES);
        } else if (sName.CompareNoCase(_T("btnNo")) == 0) {
            Close(MSGID_NO);
        }
    }

    return WindowImplBase::Notify(msg);
}

LRESULT CMsgWnd::OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled ) {
    bHandled = FALSE;
    return 0L;
}
