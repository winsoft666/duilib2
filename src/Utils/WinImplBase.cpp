#include "StdAfx.h"
#include <algorithm>

namespace DuiLib {
    //////////////////////////////////////////////////////////////////////////
    //
    DUI_BEGIN_MESSAGE_MAP(WindowImplBase, CNotifyPump)
    DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
    DUI_END_MESSAGE_MAP()

    void WindowImplBase::OnFinalMessage( HWND hWnd ) {
        m_PaintManager.RemovePreMessageFilter(this);
        m_PaintManager.RemoveNotifier(this);
        m_PaintManager.ReapObjects(m_PaintManager.GetRoot());
    }

    LRESULT WindowImplBase::ResponseDefaultKeyEvent(WPARAM wParam) {
        if (wParam == VK_RETURN) {
            return FALSE;
        } else if (wParam == VK_ESCAPE) {
            return TRUE;
        }

        return FALSE;
    }

    UINT WindowImplBase::GetClassStyle() const {
        return CS_DBLCLKS;
    }

    CControlUI *WindowImplBase::CreateControl(LPCTSTR pstrClass) {
        return NULL;
    }

    LPCTSTR WindowImplBase::QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType) {
        return NULL;
    }

    LRESULT WindowImplBase::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool & /*bHandled*/) {
        if (uMsg == WM_KEYDOWN) {
            switch (wParam) {
                case VK_RETURN:
                case VK_ESCAPE:
                    return ResponseDefaultKeyEvent(wParam);

                default:
                    break;
            }
        }

        return FALSE;
    }

    LRESULT WindowImplBase::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL &bHandled) {
        if( ::IsIconic(*this) ) 
            bHandled = FALSE;

        return (wParam == 0) ? TRUE : FALSE;
    }

    LRESULT WindowImplBase::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        return 0;
    }

    LRESULT WindowImplBase::OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/) {
        return 0;
    }


    BOOL WindowImplBase::IsInStaticControl(CControlUI *pControl) {
        BOOL bRet = FALSE;

        if (!pControl) {
            return bRet;
        }

        CDuiString strClassName = pControl->GetClass();

        std::vector<CDuiString> vctStaticName;
        vctStaticName.push_back(DUI_CTR_CONTROL);
        vctStaticName.push_back(DUI_CTR_TEXT);
        vctStaticName.push_back(DUI_CTR_LABEL);
        vctStaticName.push_back(DUI_CTR_CONTAINER);
		vctStaticName.push_back(DUI_CTR_CEF);
        vctStaticName.push_back(DUI_CTR_HORIZONTALLAYOUT);
        vctStaticName.push_back(DUI_CTR_VERTICALLAYOUT);
        vctStaticName.push_back(DUI_CTR_TABLAYOUT);
        vctStaticName.push_back(DUI_CTR_CHILDLAYOUT);

        bool bFind = StringIsInVector(vctStaticName, strClassName, false);

        if (bFind) {
            CControlUI *pParent = pControl->GetParent();

            while (pParent) {
                strClassName = pParent->GetClass();

                bFind = StringIsInVector(vctStaticName, strClassName, false);
                if (!bFind) {
                    return bRet;
                }

                pParent = pParent->GetParent();
            }

            bRet = TRUE;
        }

        return bRet;
    }

    LRESULT WindowImplBase::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ::ScreenToClient(*this, &pt);

        RECT rcClient;
        ::GetClientRect(*this, &rcClient);

        if (!::IsZoomed(*this)) {
            RECT rcSizeBox = m_PaintManager.GetSizeBox();

            if (pt.y < rcClient.top + rcSizeBox.top) {
                if (pt.x < rcClient.left + rcSizeBox.left)
                    return HTTOPLEFT;

                if (pt.x > rcClient.right - rcSizeBox.right)
                    return HTTOPRIGHT;

                return HTTOP;
            } else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
                if (pt.x < rcClient.left + rcSizeBox.left)
                    return HTBOTTOMLEFT;

                if (pt.x > rcClient.right - rcSizeBox.right)
                    return HTBOTTOMRIGHT;

                return HTBOTTOM;
            }

            if (pt.x < rcClient.left + rcSizeBox.left)
                return HTLEFT;

            if (pt.x > rcClient.right - rcSizeBox.right)
                return HTRIGHT;
        }

        RECT rcCaption = m_PaintManager.GetCaptionRect();
        
        // Jeffery: Caption 属性适应DPI缩放
        //
        rcCaption = m_PaintManager.GetDPIObj()->Scale(rcCaption);

        if (-1 == rcCaption.bottom) {
            rcCaption.bottom = rcClient.bottom;
        }

        if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right
                && pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
            CControlUI *pControl = m_PaintManager.FindControl(pt);

            if (IsInStaticControl(pControl)) {
                return HTCAPTION;
            }
        }

        return HTCLIENT;
    }

    LRESULT WindowImplBase::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        MONITORINFO Monitor = {};
        Monitor.cbSize = sizeof(Monitor);
        ::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &Monitor);
        RECT rcWork = Monitor.rcWork;

        if( Monitor.dwFlags != MONITORINFOF_PRIMARY ) {
            ::OffsetRect(&rcWork, -rcWork.left, -rcWork.top);
        }

        LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
        lpMMI->ptMaxPosition.x  = rcWork.left;
        lpMMI->ptMaxPosition.y  = rcWork.top;
        lpMMI->ptMaxSize.x = rcWork.right - rcWork.left;
        lpMMI->ptMaxSize.y = rcWork.bottom - rcWork.top;
        lpMMI->ptMaxTrackSize.x = m_PaintManager.GetMaxInfo().cx == 0 ? rcWork.right - rcWork.left : m_PaintManager.GetMaxInfo().cx;
        lpMMI->ptMaxTrackSize.y = m_PaintManager.GetMaxInfo().cy == 0 ? rcWork.bottom - rcWork.top : m_PaintManager.GetMaxInfo().cy;
        lpMMI->ptMinTrackSize.x = m_PaintManager.GetMinInfo().cx;
        lpMMI->ptMinTrackSize.y = m_PaintManager.GetMinInfo().cy;

        bHandled = TRUE;
        return 0;
    }

    LRESULT WindowImplBase::OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        SIZE szRoundCorner = m_PaintManager.GetRoundCorner();

        if( !::IsIconic(*this) ) {
            CDuiRect rcWnd;
            ::GetWindowRect(*this, &rcWnd);
            rcWnd.Offset(-rcWnd.left, -rcWnd.top);
            rcWnd.right++;
            rcWnd.bottom++;
            HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
            ::SetWindowRgn(*this, hRgn, TRUE);
            ::DeleteObject(hRgn);
        }

        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        if (wParam == SC_CLOSE) {
            bHandled = TRUE;
            SendMessage(WM_CLOSE);
            return 0;
        }

        LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
        return lRes;
    }

    LRESULT WindowImplBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        // Update Window Style
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

        m_PaintManager.Init(m_hWnd, GetManagerName());
        m_PaintManager.AddPreMessageFilter(this);

        // Create Root Window
        //
        CControlUI *pRoot = NULL;
        CDialogBuilder builder;
        CDuiString sSkinType = GetSkinType();

        if (sSkinType.IsEmpty()) {
            pRoot = builder.Create(GetSkinFile().GetData(), (UINT)0, this, &m_PaintManager);
        } else {
            STRINGorID xml(_ttoi(GetSkinFile()));
            pRoot = builder.Create(xml, sSkinType, this, &m_PaintManager);
        }

        if (pRoot == NULL) {
            CDuiString sError = _T("Load Resource XML Failed：");
            sError += GetSkinFile();
            MessageBox(NULL, sError, _T("DuiLib"), MB_OK | MB_ICONERROR);
            return 0;
        }

        m_PaintManager.AttachDialog(pRoot);
        m_PaintManager.AddNotifier(this);

        InitWindow();
        return 0;
    }

    LRESULT WindowImplBase::OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LRESULT WindowImplBase::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        LRESULT lRes = 0;
        BOOL bHandled = TRUE;

        switch (uMsg) {
            case WM_CREATE:
                lRes = OnCreate(uMsg, wParam, lParam, bHandled);
                break;
            case WM_CLOSE:
                lRes = OnClose(uMsg, wParam, lParam, bHandled);
                break;
            case WM_DESTROY:
                lRes = OnDestroy(uMsg, wParam, lParam, bHandled);
                break;
            case WM_NCACTIVATE:
                lRes = OnNcActivate(uMsg, wParam, lParam, bHandled);
                break;

            case WM_NCCALCSIZE:
                lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled);
                break;

            case WM_NCPAINT:
                lRes = OnNcPaint(uMsg, wParam, lParam, bHandled);
                break;

            case WM_NCHITTEST:
                lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled);
                break;

            case WM_GETMINMAXINFO:
                lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled);
                break;

            case WM_MOUSEWHEEL:
                lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled);
                break;

            case WM_SIZE:
                lRes = OnSize(uMsg, wParam, lParam, bHandled);
                break;

            case WM_CHAR:
                lRes = OnChar(uMsg, wParam, lParam, bHandled);
                break;

            case WM_SYSCOMMAND:
                lRes = OnSysCommand(uMsg, wParam, lParam, bHandled);
                break;

            case WM_KEYDOWN:
                lRes = OnKeyDown(uMsg, wParam, lParam, bHandled);
                break;

            case WM_KILLFOCUS:
                lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
                break;

            case WM_SETFOCUS:
                lRes = OnSetFocus(uMsg, wParam, lParam, bHandled);
                break;

            case WM_LBUTTONUP:
                lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled);
                break;

            case WM_LBUTTONDOWN:
                lRes = OnLButtonDown(uMsg, wParam, lParam, bHandled);
                break;

            case WM_MOUSEMOVE:
                lRes = OnMouseMove(uMsg, wParam, lParam, bHandled);
                break;

            case WM_MOUSEHOVER:
                lRes = OnMouseHover(uMsg, wParam, lParam, bHandled);
                break;

            default:
                bHandled = FALSE;
                break;
        }

        if (bHandled)
            return lRes;

        lRes = HandleCustomMessage(uMsg, wParam, lParam, bHandled);

        if (bHandled)
			return lRes;

        if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
            return lRes;

        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

    LRESULT WindowImplBase::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        bHandled = FALSE;
        return 0;
    }

    LONG WindowImplBase::GetStyle() {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        return styleValue;
    }

    void WindowImplBase::OnClick(TNotifyUI &msg) {
    }

    void WindowImplBase::Notify(TNotifyUI &msg) {
        return CNotifyPump::NotifyPump(msg);
    }
}