#include "StdAfx.h"
#include "Utils/Task.h"
#ifdef UILIB_WITH_CEF
#include "Internal/Cef/CefManager.h"
#endif

#ifdef _DEBUG
    #include <shlwapi.h>
    #pragma comment(lib, "shlwapi.lib")
#endif

namespace DuiLib {

    //////////////////////////////////////////////////////////////////////////
    //
    DUI_BASE_BEGIN_MESSAGE_MAP(CNotifyPump)
    DUI_END_MESSAGE_MAP()

    static const DUI_MSGMAP_ENTRY *DuiFindMessageEntry(const DUI_MSGMAP_ENTRY *lpEntry, TNotifyUI &msg ) {
        CDuiString sMsgType = msg.sType;
        CDuiString sCtrlName = msg.pSender->GetName();
        const DUI_MSGMAP_ENTRY *pMsgTypeEntry = NULL;

        while (lpEntry->nSig != DuiSig_end) {
            if(lpEntry->sMsgType == sMsgType) {
                if(!lpEntry->sCtrlName.IsEmpty()) {
                    if(lpEntry->sCtrlName == sCtrlName) {
                        return lpEntry;
                    }
                } else {
                    pMsgTypeEntry = lpEntry;
                }
            }

            lpEntry++;
        }

        return pMsgTypeEntry;
    }

    bool CNotifyPump::AddVirtualWnd(CDuiString strName, CNotifyPump *pObject) {
        if( m_VirtualWndMap.Find(strName) == NULL ) {
            m_VirtualWndMap.Insert(strName, (LPVOID)pObject);
            return true;
        }

        return false;
    }

    bool CNotifyPump::RemoveVirtualWnd(CDuiString strName) {
        if( m_VirtualWndMap.Find(strName) != NULL ) {
            m_VirtualWndMap.Remove(strName);
            return true;
        }

        return false;
    }

    bool CNotifyPump::LoopDispatch(TNotifyUI &msg) {
        const DUI_MSGMAP_ENTRY *lpEntry = NULL;
        const DUI_MSGMAP *pMessageMap = NULL;

#ifndef UILIB_STATIC

        for(pMessageMap = GetMessageMap(); pMessageMap != NULL; pMessageMap = (*pMessageMap->pfnGetBaseMap)())
#else
        for(pMessageMap = GetMessageMap(); pMessageMap != NULL; pMessageMap = pMessageMap->pBaseMap)
#endif
        {
#ifndef UILIB_STATIC
            ASSERT(pMessageMap != (*pMessageMap->pfnGetBaseMap)());
#else
            ASSERT(pMessageMap != pMessageMap->pBaseMap);
#endif

            if ((lpEntry = DuiFindMessageEntry(pMessageMap->lpEntries, msg)) != NULL) {
                goto LDispatch;
            }
        }

        return false;

    LDispatch:
        union DuiMessageMapFunctions mmf;
        mmf.pfn = lpEntry->pfn;

        bool bRet = false;
        int nSig;
        nSig = lpEntry->nSig;

        switch (nSig) {
            default:
                ASSERT(FALSE);
                break;

            case DuiSig_lwl:
                (this->*mmf.pfn_Notify_lwl)(msg.wParam, msg.lParam);
                bRet = true;
                break;

            case DuiSig_vn:
                (this->*mmf.pfn_Notify_vn)(msg);
                bRet = true;
                break;
        }

        return bRet;
    }

    void CNotifyPump::NotifyPump(TNotifyUI &msg) {
        ///遍历虚拟窗口
        if( !msg.sVirtualWnd.IsEmpty() ) {
            for (std::map<CDuiString, LPVOID>::iterator it = m_VirtualWndMap.Begin(); it != m_VirtualWndMap.End(); it++) {
                if (_tcsicmp(it->first, msg.sVirtualWnd) == 0) {
                    CNotifyPump *pObject = static_cast<CNotifyPump *>(it->second);

                    if (pObject && pObject->LoopDispatch(msg))
                        return;
                }
            }
        }

        ///
        //遍历主窗口
        LoopDispatch( msg );
    }

    //////////////////////////////////////////////////////////////////////////
    ///
    CWindowWnd::CWindowWnd() : m_hWnd(NULL), m_OldWndProc(::DefWindowProc), m_bSubclassed(false) {
    }

    HWND CWindowWnd::GetHWND() const {
        return m_hWnd;
    }

    UINT CWindowWnd::GetClassStyle() const {
        return 0;
    }

    LPCTSTR CWindowWnd::GetSuperClassName() const {
        return NULL;
    }

    CWindowWnd::operator HWND() const {
        return m_hWnd;
    }

    HWND CWindowWnd::CreateDuiWindow( HWND hwndParent, LPCTSTR pstrWindowName, DWORD dwStyle /*=0*/, DWORD dwExStyle /*=0*/ ) {
        return Create(hwndParent, pstrWindowName, dwStyle, dwExStyle, 0, 0, 0, 0, NULL);
    }

    HWND CWindowWnd::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu) {
        return Create(hwndParent, pstrName, dwStyle, dwExStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hMenu);
    }

    HWND CWindowWnd::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HMENU hMenu) {
        if (GetSuperClassName()) {
            if (!RegisterSuperclass()) {
                return NULL;
            }
        } else {
            if (!RegisterWindowClass()) {
                return NULL;
            }
        }

        m_hWnd = ::CreateWindowEx(dwExStyle, GetWindowClassName(), pstrName, dwStyle, x, y, cx, cy, hwndParent, hMenu, CPaintManagerUI::GetInstance(), this);
        ASSERT(m_hWnd != NULL);
        return m_hWnd;
    }

    HWND CWindowWnd::Subclass(HWND hWnd) {
        ASSERT(::IsWindow(hWnd));
        ASSERT(m_hWnd == NULL);

        if (!::IsWindow(hWnd))
            return NULL;

        m_OldWndProc = SubclassWindow(hWnd, __WndProc);

        if( m_OldWndProc == NULL )
            return NULL;

        m_bSubclassed = true;
        m_hWnd = hWnd;
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
        return m_hWnd;
    }

    void CWindowWnd::Unsubclass() {
        if( !::IsWindow(m_hWnd) )
            return;

        if( !m_bSubclassed )
            return;

        SubclassWindow(m_hWnd, m_OldWndProc);
        m_OldWndProc = ::DefWindowProc;
        m_bSubclassed = false;
    }

    void CWindowWnd::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/) {
        if( !::IsWindow(m_hWnd) )
            return;

        ::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
    }

    UINT CWindowWnd::ShowModal() {
        UINT nRet = 0;
        HWND hWndParent = GetWindowOwner(m_hWnd);
        ::ShowWindow(m_hWnd, SW_SHOWNORMAL);
        ::EnableWindow(hWndParent, FALSE);
        MSG msg = { 0 };

        while( ::IsWindow(m_hWnd) && ::GetMessage(&msg, NULL, 0, 0) ) {
            {
                std::unique_lock<std::mutex> lock(CPaintManagerUI::UIWorksMutex);
                while (!CPaintManagerUI::UIWorks.empty()) {
                    std::function<void()> task = std::move(CPaintManagerUI::UIWorks.front());
                    CPaintManagerUI::UIWorks.pop();
                    task();
                }
            }

            if(msg.message == WM_USER + 1234 && msg.wParam == WM_USER + 1234 && msg.lParam == WM_USER + 1234)
              continue;

            if (msg.message == WM_CLOSE && msg.hwnd == m_hWnd) {
                nRet = msg.wParam;
                ::EnableWindow(hWndParent, TRUE);
                ::SetFocus(hWndParent);
            }

            if (!CPaintManagerUI::TranslateMessage(&msg)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }

            if (msg.message == WM_QUIT)
                break;
        }

        ::EnableWindow(hWndParent, TRUE);
        ::SetFocus(hWndParent);

        if( msg.message == WM_QUIT )
            ::PostQuitMessage(msg.wParam);

        return nRet;
    }

    void CWindowWnd::Close(UINT nRet) {
        ASSERT(::IsWindow(m_hWnd));

        if( !::IsWindow(m_hWnd) )
            return;

        PostMessage(WM_CLOSE, (WPARAM)nRet, 0L);
    }

    void CWindowWnd::CenterWindow() {
        ASSERT(::IsWindow(m_hWnd));
        ASSERT((GetWindowStyle(m_hWnd)&WS_CHILD) == 0);
        RECT rcDlg = { 0 };
        ::GetWindowRect(m_hWnd, &rcDlg);
        RECT rcArea = { 0 };
        RECT rcCenter = { 0 };
        HWND hWnd = *this;
        HWND hWndParent = ::GetParent(m_hWnd);
        HWND hWndCenter = ::GetWindowOwner(m_hWnd);

        if (hWndCenter != NULL)
            hWnd = hWndCenter;

        // 处理多显示器模式下屏幕居中
        MONITORINFO oMonitor = {};
        oMonitor.cbSize = sizeof(oMonitor);
        ::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
        rcArea = oMonitor.rcWork;

        if( hWndCenter == NULL )
            rcCenter = rcArea;
        else
            ::GetWindowRect(hWndCenter, &rcCenter);

        int DlgWidth = rcDlg.right - rcDlg.left;
        int DlgHeight = rcDlg.bottom - rcDlg.top;

        // Find dialog's upper left based on rcCenter
        int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
        int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

        // The dialog is outside the screen, move it inside
        if( xLeft < rcArea.left ) xLeft = rcArea.left;
        else if( xLeft + DlgWidth > rcArea.right ) xLeft = rcArea.right - DlgWidth;

        if( yTop < rcArea.top ) yTop = rcArea.top;
        else if( yTop + DlgHeight > rcArea.bottom ) yTop = rcArea.bottom - DlgHeight;

        ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void CWindowWnd::SetIcon(UINT nRes) {
        HICON hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                                         (::GetSystemMetrics(SM_CXICON) + 15) & ~15, (::GetSystemMetrics(SM_CYICON) + 15) & ~15, // 防止高DPI下图标模糊
                                         LR_DEFAULTCOLOR);
        ASSERT(hIcon);
        ::SendMessage(m_hWnd, WM_SETICON, (WPARAM) TRUE, (LPARAM) hIcon);

        hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                                   (::GetSystemMetrics(SM_CXICON) + 15) & ~15, (::GetSystemMetrics(SM_CYICON) + 15) & ~15, // 防止高DPI下图标模糊
                                   LR_DEFAULTCOLOR);
        ASSERT(hIcon);
        ::SendMessage(m_hWnd, WM_SETICON, (WPARAM) FALSE, (LPARAM) hIcon);
    }

    bool CWindowWnd::RegisterWindowClass() {
        WNDCLASS wc = { 0 };
        wc.style = GetClassStyle();
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hIcon = NULL;
        wc.lpfnWndProc = CWindowWnd::__WndProc;
        wc.hInstance = CPaintManagerUI::GetInstance();
        wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = GetWindowClassName();
        ATOM ret = ::RegisterClass(&wc);
        ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
        return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    bool CWindowWnd::RegisterSuperclass() {
        // Get the class information from an existing
        // window so we can subclass it later on...
        WNDCLASSEX wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEX);

        if( !::GetClassInfoEx(NULL, GetSuperClassName(), &wc) ) {
            if( !::GetClassInfoEx(CPaintManagerUI::GetInstance(), GetSuperClassName(), &wc) ) {
                ASSERT(!"Unable to locate window class");
                return NULL;
            }
        }

        m_OldWndProc = wc.lpfnWndProc;
        wc.lpfnWndProc = CWindowWnd::__ControlProc;
        wc.hInstance = CPaintManagerUI::GetInstance();
        wc.lpszClassName = GetWindowClassName();
        ATOM ret = ::RegisterClassEx(&wc);
        ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
        return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    LRESULT CALLBACK CWindowWnd::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        CWindowWnd *pThis = NULL;

        if( uMsg == WM_NCCREATE ) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pThis = static_cast<CWindowWnd *>(lpcs->lpCreateParams);
            pThis->m_hWnd = hWnd;
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
        } else {
            pThis = reinterpret_cast<CWindowWnd *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

            if( uMsg == WM_NCDESTROY && pThis != NULL ) {
                LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
                ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);

                if( pThis->m_bSubclassed )
                    pThis->Unsubclass();

                pThis->m_hWnd = NULL;
                pThis->OnFinalMessage(hWnd);
                return lRes;
            }
        }

        if(pThis) {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }

        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    LRESULT CALLBACK CWindowWnd::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        CWindowWnd *pThis = NULL;

        if( uMsg == WM_NCCREATE ) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pThis = static_cast<CWindowWnd *>(lpcs->lpCreateParams);
            ::SetProp(hWnd, _T("WndX"), (HANDLE) pThis);
            pThis->m_hWnd = hWnd;
        } else {
            pThis = reinterpret_cast<CWindowWnd *>(::GetProp(hWnd, _T("WndX")));

            if( uMsg == WM_NCDESTROY && pThis != NULL ) {
                LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);

                if( pThis->m_bSubclassed ) pThis->Unsubclass();

                ::SetProp(hWnd, _T("WndX"), NULL);
                pThis->m_hWnd = NULL;
                pThis->OnFinalMessage(hWnd);
                return lRes;
            }
        }

        if( pThis != NULL ) {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        } else {
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    LRESULT CWindowWnd::SendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) {
        ASSERT(::IsWindow(m_hWnd));
        return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
    }

    LRESULT CWindowWnd::PostMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) {
        ASSERT(::IsWindow(m_hWnd));
        return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
    }

    void CWindowWnd::ResizeClient(int cx /*= -1*/, int cy /*= -1*/) {
        ASSERT(::IsWindow(m_hWnd));
        RECT rc = { 0 };

        if( !::GetClientRect(m_hWnd, &rc) )
            return;

        if( cx != -1 ) rc.right = cx;

        if( cy != -1 ) rc.bottom = cy;

        if( !::AdjustWindowRectEx(&rc, GetWindowStyle(m_hWnd), (!(GetWindowStyle(m_hWnd) & WS_CHILD) && (::GetMenu(m_hWnd) != NULL)), GetWindowExStyle(m_hWnd)) )
            return;

        ::SetWindowPos(m_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }

#ifdef UILIB_WITH_CEF
void CWindowWnd::SetAutoCloseCefWhenWindowCloseMsg(bool b, WPARAM closeMsgWPARAM, LPARAM closeMsgLPARAM) {
  Internal::CefManager::GetInstance()->SetAutoCloseCefWhenWindowCloseMsg(m_hWnd, b, closeMsgWPARAM, closeMsgLPARAM);
}

bool CWindowWnd::IsAutoCloseCefWhenWindowCloseMsg() {
  return Internal::CefManager::GetInstance()->IsAutoCloseCefWhenWindowCloseMsg(m_hWnd);
}
#endif

LRESULT CWindowWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
    }

    void CWindowWnd::OnFinalMessage(HWND /*hWnd*/) {
    }

} // namespace DuiLib
