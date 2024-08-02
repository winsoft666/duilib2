#include "Stdafx.h"
#include <assert.h>
#include "Utils/SystemTrayIcon.h"
using namespace std;

namespace DuiLib {
    UINT CSystemTrayIcon::m_nMaxTooltipLength = 128;

    CSystemTrayIcon::CSystemTrayIcon() {
        memset(&m_tnd, 0, sizeof(m_tnd));
        m_bHidden = TRUE;
        m_bRemoved = TRUE;
        m_hInstance = NULL;
        m_hIcon = NULL;
        m_uCreationFlags = 0;
    }

    BOOL CSystemTrayIcon::Create(HINSTANCE hInst,
                                 HWND hParent,
                                 UINT uCallbackMessage,
                                 LPCTSTR szToolTip,
                                 HICON icon,
                                 UINT uID,
                                 BOOL bHidden /*=FALSE*/,
                                 LPCTSTR szBalloonTip /*=NULL*/,
                                 LPCTSTR szBalloonTitle /*=NULL*/,
                                 DWORD dwBalloonIcon /*=NIIF_NONE*/,
                                 UINT uBalloonTimeout /*=10*/) {

        m_nMaxTooltipLength = _countof(m_tnd.szTip);

        m_hInstance = hInst;
        m_hIcon = icon;

        m_tnd.cbSize = sizeof(NOTIFYICONDATA);
        m_tnd.hWnd = hParent;
        m_tnd.uID = uID;
        m_tnd.hIcon = icon;
        m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        m_tnd.uCallbackMessage = uCallbackMessage;

        wcsncpy_s(m_tnd.szTip, szToolTip, m_nMaxTooltipLength);

        if (szBalloonTip) {
            ASSERT(NIIF_NONE == dwBalloonIcon || NIIF_INFO == dwBalloonIcon ||
                   NIIF_WARNING == dwBalloonIcon || NIIF_ERROR == dwBalloonIcon);

            // The timeout must be between 10 and 30 seconds.
            ASSERT(uBalloonTimeout >= 10 && uBalloonTimeout <= 30);

            m_tnd.uFlags |= NIF_INFO;

            _tcsncpy(m_tnd.szInfo, szBalloonTip, 255);
            if (szBalloonTitle)
                _tcsncpy(m_tnd.szInfoTitle, szBalloonTitle, 63);
            else
                m_tnd.szInfoTitle[0] = _T('\0');
            m_tnd.uTimeout = uBalloonTimeout * 1000;
            m_tnd.dwInfoFlags = dwBalloonIcon;
        }

        m_bHidden = bHidden;

        if (m_bHidden) {
            m_tnd.uFlags = NIF_STATE;
            m_tnd.dwState = NIS_HIDDEN;
            m_tnd.dwStateMask = NIS_HIDDEN;
        }

        m_uCreationFlags = m_tnd.uFlags;

        BOOL bResult = TRUE;
        if (!m_bHidden) {
            bResult = Shell_NotifyIcon(NIM_ADD, &m_tnd);
            if (!bResult) {
                TraceMsgW(L"DuiLib Shell_NotifyIcon failed with %lu\n", GetLastError());
            }
            m_bHidden = m_bRemoved = !bResult;
        }

        if (szBalloonTip) {
            // Zero out the balloon text string so that later operations won't redisplay the balloon.
            m_tnd.szInfo[0] = _T('\0');
        }

        return bResult;
    }

    CSystemTrayIcon::~CSystemTrayIcon() {
        RemoveIcon();
    }

    UINT CSystemTrayIcon::RegisterTaskbarCreatedMessage(HWND h) {
        UINT msg = ::RegisterWindowMessage(_T("TaskbarCreated"));
        UIPIMsgFilter(h, msg, TRUE);
        return msg;
    }

    BOOL CSystemTrayIcon::IsVisible() {
        return !m_bHidden;
    }

    void CSystemTrayIcon::SetFocus() {
        Shell_NotifyIcon(NIM_SETFOCUS, &m_tnd);
    }

    BOOL CSystemTrayIcon::AddIcon() {
        if (!m_bRemoved)
            RemoveIcon();

        m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        if (Shell_NotifyIcon(NIM_ADD, &m_tnd))
            m_bRemoved = m_bHidden = FALSE;

        return (m_bRemoved == FALSE);
    }

    BOOL CSystemTrayIcon::RemoveIcon() {
        if (m_bRemoved)
            return TRUE;

        m_tnd.uFlags = 0;
        if (Shell_NotifyIcon(NIM_DELETE, &m_tnd))
            m_bRemoved = m_bHidden = TRUE;

        return (m_bRemoved == TRUE);
    }

    BOOL CSystemTrayIcon::HideIcon() {
        if (m_bRemoved || m_bHidden)
            return TRUE;

        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = NIS_HIDDEN;
        m_tnd.dwStateMask = NIS_HIDDEN;

        m_bHidden = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);

        return (m_bHidden == TRUE);
    }

    BOOL CSystemTrayIcon::ShowIcon() {
        if (m_bRemoved)
            return AddIcon();

        if (!m_bHidden)
            return TRUE;

        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = 0;
        m_tnd.dwStateMask = NIS_HIDDEN;
        Shell_NotifyIcon(NIM_MODIFY, &m_tnd);

        return (m_bHidden == FALSE);
    }

    BOOL CSystemTrayIcon::SetIcon(HICON hIcon) {
        m_tnd.uFlags = NIF_ICON;
        m_tnd.hIcon = hIcon;

        if (m_bHidden)
            return TRUE;
        else
            return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
    }

    BOOL CSystemTrayIcon::SetIcon(LPCTSTR lpszIconName) {
        HICON hIcon = (HICON) ::LoadImage(m_hInstance,
                                          lpszIconName,
                                          IMAGE_ICON,
                                          0, 0,
                                          LR_LOADFROMFILE);

        if (!hIcon)
            return FALSE;
        BOOL returnCode = SetIcon(hIcon);
        ::DestroyIcon(hIcon);
        return returnCode;
    }

    BOOL CSystemTrayIcon::SetIcon(UINT nIDResource) {
        HICON hIcon = (HICON) ::LoadImage(m_hInstance,
                                          MAKEINTRESOURCE(nIDResource),
                                          IMAGE_ICON,
                                          0, 0,
                                          LR_DEFAULTCOLOR);

        BOOL returnCode = SetIcon(hIcon);
        ::DestroyIcon(hIcon);
        return returnCode;
    }

    BOOL CSystemTrayIcon::SetStandardIcon(LPCTSTR lpIconName) {
        HICON hIcon = ::LoadIcon(NULL, lpIconName);

        return SetIcon(hIcon);
    }

    BOOL CSystemTrayIcon::SetStandardIcon(UINT nIDResource) {
        HICON hIcon = ::LoadIcon(NULL, MAKEINTRESOURCE(nIDResource));

        return SetIcon(hIcon);
    }

    HICON CSystemTrayIcon::GetIcon() const {
        return m_tnd.hIcon;
    }

    BOOL CSystemTrayIcon::SetTooltipText(LPCTSTR pszTip) {
        ASSERT(_tcslen(pszTip) < m_nMaxTooltipLength);

        m_tnd.uFlags = NIF_TIP;
        _tcsncpy(m_tnd.szTip, pszTip, m_nMaxTooltipLength - 1);

        if (m_bHidden)
            return TRUE;
        else
            return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
    }

    BOOL CSystemTrayIcon::SetTooltipText(UINT nID) {
        TCHAR strBuffer[1024];
        ASSERT(1024 >= m_nMaxTooltipLength);

        if (!LoadString(m_hInstance, nID, strBuffer, m_nMaxTooltipLength - 1))
            return FALSE;

        return SetTooltipText(strBuffer);
    }

    LPTSTR CSystemTrayIcon::GetTooltipText() const {
        static TCHAR strBuffer[1024];
        ASSERT(1024 >= m_nMaxTooltipLength);

        wcsncpy_s(strBuffer, 1024, m_tnd.szTip, m_nMaxTooltipLength - 1);

        return strBuffer;
    }


    BOOL CSystemTrayIcon::ShowBalloon(LPCTSTR szText,
                                      LPCTSTR szTitle  /*=NULL*/,
                                      DWORD   dwIcon   /*=NIIF_NONE*/,
                                      UINT    uTimeout /*=10*/) {

        // The balloon tooltip text can be up to 255 chars long.
        ASSERT(lstrlen(szText) < 256);

        // The balloon title text can be up to 63 chars long.
        if (szTitle) {
            ASSERT(lstrlen(szTitle) < 64);
        }

        // dwBalloonIcon must be valid.
        ASSERT(NIIF_NONE == dwIcon || NIIF_INFO == dwIcon ||
               NIIF_WARNING == dwIcon || NIIF_ERROR == dwIcon);

        m_tnd.uFlags = NIF_INFO;
        _tcsncpy(m_tnd.szInfo, szText, 256);
        if (szTitle)
            _tcsncpy(m_tnd.szInfoTitle, szTitle, 64);
        else
            m_tnd.szInfoTitle[0] = _T('\0');
        m_tnd.dwInfoFlags = dwIcon;
        m_tnd.uTimeout = uTimeout * 1000;   // convert time to ms

        BOOL bSuccess = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);

        // Zero out the balloon text string so that later operations won't redisplay the balloon.
        m_tnd.szInfo[0] = _T('\0');

        return bSuccess;
    }

    void CSystemTrayIcon::RemoveBalloon() {
        m_tnd.uFlags = NIF_INFO;
        m_tnd.szInfo[0] = 0;
        m_tnd.szInfoTitle[0] = 0;

        m_tnd.dwInfoFlags = NIIF_NONE;

        Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
    }

    BOOL CSystemTrayIcon::SetNotificationWnd(HWND hNotifyWnd) {
        if (!hNotifyWnd || !::IsWindow(hNotifyWnd)) {
            ASSERT(FALSE);
            return FALSE;
        }

        m_tnd.hWnd = hNotifyWnd;
        m_tnd.uFlags = 0;

        if (m_bHidden)
            return TRUE;
        else
            return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
    }

    HWND CSystemTrayIcon::GetNotificationWnd() const {
        return m_tnd.hWnd;
    }

    BOOL CSystemTrayIcon::SetCallbackMessage(UINT uCallbackMessage) {
        ASSERT(uCallbackMessage >= WM_APP);

        m_tnd.uCallbackMessage = uCallbackMessage;
        m_tnd.uFlags = NIF_MESSAGE;

        if (m_bHidden)
            return TRUE;
        else
            return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
    }

    UINT CSystemTrayIcon::GetCallbackMessage() const {
        return m_tnd.uCallbackMessage;
    }

    bool CSystemTrayIcon::ReCreate() {
        RemoveIcon();

        m_tnd.uFlags = m_uCreationFlags;
        BOOL ret = Shell_NotifyIcon(NIM_ADD, &m_tnd);
        m_bRemoved = m_bHidden = !ret;

        return ret;
    }
}