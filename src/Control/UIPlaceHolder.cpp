#include "Stdafx.h"
#include <io.h>
#include "Utils/Utils.h"
#include <strsafe.h>
#include <Shlwapi.h>

namespace DuiLib {
    CPlaceHolderWnd::CPlaceHolderWnd(CPaintManagerUI* pParentPM) :
        m_pOwner(NULL),
        m_bInit(false),
        m_pParentPM(pParentPM),
        m_dwBkColor(0xFFFFFFFF),
        m_bBkColorSet(false) {
    }

    CPlaceHolderWnd::~CPlaceHolderWnd() {
    }

    void CPlaceHolderWnd::SetBkColor(DWORD dwBackColor) {
        m_dwBkColor = dwBackColor;
        m_bBkColorSet = true;
    }

    void CPlaceHolderWnd::Init(CPlaceHolderUI* pOwner) {
        m_pOwner = pOwner;
        m_bInit = true;
        if (m_hWnd == NULL) {
            RECT rcPos = m_pOwner->GetPos();
            UINT uStyle = UI_WNDSTYLE_CHILD;

            Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
        }
    }

    LPCTSTR CPlaceHolderWnd::GetWindowClassName() const {
        return _T("PlaceHolderWindowClass");
    }

    void CPlaceHolderWnd::OnFinalMessage(HWND /*hWnd*/) {
        m_pOwner->Invalidate();
       
        m_pOwner->m_pWindow = NULL;
        delete this;
    }

    LRESULT CPlaceHolderWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        LRESULT lRes = 0;
        BOOL bHandled = FALSE;
        switch (uMsg) {
        case WM_PAINT:			lRes = OnPaint(uMsg, wParam, lParam, bHandled); break;
        case WM_SIZE:			lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
        default:				bHandled = FALSE; break;
        }
        // Jeffery [Note]: WM_PAINT消息如果没有被处理BeginPaint/EndPaint，必须掉用CWindowWnd::HandleMessage函数中的::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);来处理掉，
        // 否则消息循环会一直收到WM_PAINT消息，从而导致主线程CPU占用过高，而且会影响系统其他消息的接受。
        //
        if (bHandled) 
            return lRes;

        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

    LRESULT CPlaceHolderWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        if (m_bBkColorSet) {
            PAINTSTRUCT ps = { 0 };
            HDC hDcPaint = ::BeginPaint(m_hWnd, &ps);
            ::SetBkColor(hDcPaint, RGB(GetBValue(m_dwBkColor), GetGValue(m_dwBkColor), GetRValue(m_dwBkColor)));
            ::EndPaint(m_hWnd, &ps);
        }
        return 0;
    }

    LRESULT CPlaceHolderWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        if (m_pOwner) {
            m_pOwner->OnSize(wParam, lParam);
        }
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    IMPLEMENT_DUICONTROL(CPlaceHolderUI)

    CPlaceHolderUI::CPlaceHolderUI(void) :
        m_pWindow(NULL),
        m_pDelegate(NULL),
        m_dwBkColor(0xFFFFFFFF) {
    }

    CPlaceHolderUI::~CPlaceHolderUI(void) {
    }

    void CPlaceHolderUI::DoInit() {
        m_pWindow = new CPlaceHolderWnd(m_pManager);
        if (m_pWindow) {
            m_pWindow->Init(this);
            m_pWindow->ShowWindow();
        }
    }

    LPCTSTR CPlaceHolderUI::GetClass() const {
        return DUI_CTR_PLACE_HOLDER;
    }

    LPVOID CPlaceHolderUI::GetInterface(LPCTSTR pstrName) {
        if (_tcsicmp(pstrName, DUI_CTR_PLACE_HOLDER) == 0) return static_cast<CPlaceHolderUI *>(this);

        return CControlUI::GetInterface(pstrName);
    }

    void CPlaceHolderUI::RegisterDelegate(Delegate* delegate) {
        m_pDelegate = delegate;
    }

    void CPlaceHolderUI::DoEvent(TEventUI &event) {
        CControlUI::DoEvent(event);
    }

    void CPlaceHolderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
         if (_tcscmp(pstrName, _T("bkcolor")) == 0) {
             if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
             LPTSTR pstr = NULL;
             DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
             SetBkColor(clrColor);
         }
         else
             CControlUI::SetAttribute(pstrName, pstrValue);
    }

    void CPlaceHolderUI::SetPos(RECT rc, bool bNeedInvalidate /*= true*/) {
        CControlUI::SetPos(rc, bNeedInvalidate);
        ::SetWindowPos(m_pWindow->GetHWND(), NULL, rc.left, rc.top, rc.right - rc.left,
            rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void CPlaceHolderUI::SetBkColor(DWORD dwBackColor) {
        m_dwBkColor = dwBackColor;
        if (m_pWindow) {
            m_pWindow->SetBkColor(dwBackColor);
        }
    }

    HWND CPlaceHolderUI::GetHWND() {
        if (m_pWindow)
            return m_pWindow->GetHWND();
        return NULL;
    }

    void CPlaceHolderUI::OnSize(WPARAM wParam, LPARAM lParam) {
        if (m_pDelegate) {
            m_pDelegate->OnPlaceHolderSizeChanged(this->GetName());
        }
    }

}