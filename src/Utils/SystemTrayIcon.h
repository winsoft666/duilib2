#ifndef DUILIB2_SYSTEM_TRAY_ICON_H_
#define DUILIB2_SYSTEM_TRAY_ICON_H_
#pragma once
#include <ShellAPI.h>

#pragma warning(disable:4786)
#include <time.h>
#include <vector>

namespace DuiLib {
    typedef std::vector<HICON> ICONVECTOR;

    class UILIB_API CSystemTrayIcon {
      public:
        CSystemTrayIcon();
        virtual ~CSystemTrayIcon();
      public:
        // Register TaskbarCreated message, return Message Id.
        // If receive msg, can call ReCreate to create icon again.
        static UINT RegisterTaskbarCreatedMessage(HWND h);

        BOOL Create(HINSTANCE hInst,
                    HWND hParent,
                    UINT uCallbackMessage,
                    LPCTSTR szTip,
                    HICON icon,
                    UINT uID,
                    BOOL bHidden = FALSE,
                    LPCTSTR szBalloonTip = NULL,
                    LPCTSTR szBalloonTitle = NULL,
                    DWORD dwBalloonIcon = NIIF_NONE,
                    UINT uBalloonTimeout = 10);
        bool ReCreate();

        BOOL   SetTooltipText(LPCTSTR pszTooltipText);
        BOOL   SetTooltipText(UINT nID);
        LPTSTR GetTooltipText() const;

        BOOL  SetIcon(HICON hIcon);
        BOOL  SetIcon(LPCTSTR lpszIconName);
        BOOL  SetIcon(UINT nIDResource);
        BOOL  SetStandardIcon(LPCTSTR lpIconName);
        BOOL  SetStandardIcon(UINT nIDResource);
        HICON GetIcon() const;

        void  SetFocus();
        BOOL  HideIcon();
        BOOL  ShowIcon();
        BOOL  AddIcon();
        BOOL  RemoveIcon();
        BOOL IsVisible();

        BOOL ShowBalloon(LPCTSTR szText,
                         LPCTSTR szTitle = NULL,
                         DWORD dwIcon = NIIF_NONE,
                         UINT uTimeout = 10);
        void RemoveBalloon();

        BOOL  SetNotificationWnd(HWND hNotifyWnd);
        HWND  GetNotificationWnd() const;

        BOOL  SetCallbackMessage(UINT uCallbackMessage);
        UINT  GetCallbackMessage() const;


      protected:
        NOTIFYICONDATA  m_tnd;
        HINSTANCE       m_hInstance;
        BOOL            m_bHidden;
        BOOL            m_bRemoved;

        UINT			m_uCreationFlags;
        HICON           m_hIcon;
        static UINT  m_nMaxTooltipLength;
    };

}
#endif
