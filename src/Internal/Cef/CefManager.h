#pragma once
#include "Internal/Cef/CefHandler.h"
#include "Internal/Cef/CefUtil.h"
#include "include/base/cef_build.h"
#include <mutex>

namespace DuiLib {
  namespace Internal {
    struct HookInfo {
      HWND hwnd;
      std::list<CefRefPtr<CefBrowser>> browsers;
      WNDPROC prevWndProc;
      LPARAM closeMsgLPARAM;
      WPARAM closeMsgWPARAM;
      bool autoCloseBrowersWhenWindowCloseMsg;
      bool hwndShown;

      HookInfo() {
        hwnd = nullptr;
        prevWndProc = nullptr;
        closeMsgLPARAM = 0;
        closeMsgWPARAM = 0;
        autoCloseBrowersWhenWindowCloseMsg = true;
		    hwndShown = true;
      }
    };
    class CefManager {
    private:
      CefManager(void);
      ~CefManager(void);

      static LRESULT CALLBACK newWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
      void CloseBrowsers(HWND hwnd);

    public:
      static CefManager* GetInstance() {
        if (!m_pThis) {
          if (!m_pThis) {
            m_pThis = new CefManager();
          }
        }
        return m_pThis;
      };

      static void Release(void) {
        if (m_pThis) {
          delete m_pThis;
          m_pThis = nullptr;
        }
      }
      void AddBrowser(CefRefPtr<CefBrowser> browser, HWND hwnd);
      void RemoveBrowser(CefRefPtr<CefBrowser> browser, HWND hwnd);
      void SetBrowsersVisible(HWND hwnd, bool bVisible);
      void SetAutoCloseCefWhenWindowCloseMsg(HWND hwnd, bool b, WPARAM closeMsgWPARAM, LPARAM closeMsgLPARAM);
      bool IsAutoCloseCefWhenWindowCloseMsg(HWND hwnd);
      int GetBrowserCount(HWND hwnd);

      bool GetCloseMsgParam(HWND hwnd, WPARAM& wparam, LPARAM& lparam);
    private:
      static CefManager * m_pThis;

      std::recursive_mutex hooks_mutex_;
      std::map<HWND, HookInfo> m_mapHook;
    };
  }
}