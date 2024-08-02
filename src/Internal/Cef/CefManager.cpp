#include "CefManager.h"

namespace DuiLib {
namespace Internal {

CefManager* CefManager::m_pThis = nullptr;
CefManager::CefManager(void) {}

CefManager::~CefManager(void) {}

void CefManager::AddBrowser(CefRefPtr<CefBrowser> browser, HWND hwnd) {
  std::lock_guard<std::recursive_mutex> lg(hooks_mutex_);
  assert(hwnd);
  bool needHook = true;

  if (m_mapHook.find(hwnd) == m_mapHook.end()) {
    HookInfo hi;
    hi.hwnd = hwnd;
    hi.browsers.push_back(browser);
    m_mapHook[hwnd] = hi;
  }
  else {
    m_mapHook[hwnd].browsers.push_back(browser);

    if (m_mapHook[hwnd].prevWndProc)
      needHook = false;
  }

  if(needHook) {
    WNDPROC prevWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)&newWndProc);
    m_mapHook[hwnd].prevWndProc = prevWndProc;
    m_mapHook[hwnd].hwndShown = ::IsWindowVisible(hwnd);
  }
}

void CefManager::RemoveBrowser(CefRefPtr<CefBrowser> browser, HWND hwnd) {
  std::lock_guard<std::recursive_mutex> lg(hooks_mutex_);
  assert(hwnd);
  bool bFound = m_mapHook.find(hwnd) != m_mapHook.end();
  if (bFound) {
    std::list<CefRefPtr<CefBrowser>> list_b = m_mapHook[hwnd].browsers;
    for (std::list<CefRefPtr<CefBrowser>>::iterator i = list_b.begin(); i != list_b.end(); i++) {
      if ((*i)->GetIdentifier() == browser->GetIdentifier()) {
        list_b.erase(i);
        break;
      }
    }
    m_mapHook[hwnd].browsers = list_b;
  }
}

void CefManager::SetBrowsersVisible(HWND hwnd, bool bVisible) {
  std::lock_guard<std::recursive_mutex> lg(hooks_mutex_);
  assert(hwnd);
  bool bFound = m_mapHook.find(hwnd) != m_mapHook.end();
  if (bFound) {
    std::list<CefRefPtr<CefBrowser>> list_b = m_mapHook[hwnd].browsers;
    for (std::list<CefRefPtr<CefBrowser>>::iterator i = list_b.begin(); i != list_b.end(); i++) {
      if ((*i)->GetHost()) {
        if (bVisible) {
          (*i)->GetHost()->WasHidden(false);
          (*i)->GetHost()->SendFocusEvent(true);
        }
        else {
          (*i)->GetHost()->SendFocusEvent(false);
          (*i)->GetHost()->WasHidden(true);
        }
      }
    }
  }
}

void CefManager::SetAutoCloseCefWhenWindowCloseMsg(HWND hwnd, bool b, WPARAM closeMsgWPARAM, LPARAM closeMsgLPARAM) {
  std::lock_guard<std::recursive_mutex> lg(hooks_mutex_);
  assert(hwnd);
  bool bFound = m_mapHook.find(hwnd) != m_mapHook.end();
  if (bFound) {
    m_mapHook[hwnd].autoCloseBrowersWhenWindowCloseMsg = b;
    m_mapHook[hwnd].closeMsgWPARAM = closeMsgWPARAM;
    m_mapHook[hwnd].closeMsgLPARAM = closeMsgLPARAM;
  }
  else {
    HookInfo hi;
    hi.autoCloseBrowersWhenWindowCloseMsg = b;
    hi.closeMsgLPARAM = closeMsgLPARAM;
    hi.closeMsgWPARAM = closeMsgWPARAM;
    hi.hwnd = hwnd;
    m_mapHook[hwnd] = hi;
  }
}

bool CefManager::IsAutoCloseCefWhenWindowCloseMsg(HWND hwnd) {
  std::lock_guard<std::recursive_mutex> lg(hooks_mutex_);
  assert(hwnd);
  if (m_mapHook.find(hwnd) == m_mapHook.end()) {
    return false;
  }
  return m_mapHook[hwnd].autoCloseBrowersWhenWindowCloseMsg;
}

int CefManager::GetBrowserCount(HWND hwnd) {
  std::lock_guard<std::recursive_mutex> lg(hooks_mutex_);
  assert(hwnd);
  bool bFound = m_mapHook.find(hwnd) != m_mapHook.end();
  if (bFound) {
    return m_mapHook[hwnd].browsers.size();
  }
  return 0;
}

bool CefManager::GetCloseMsgParam(HWND hwnd, WPARAM& wparam, LPARAM& lparam) {
  std::lock_guard<std::recursive_mutex> lg(hooks_mutex_);
  assert(hwnd);
  if (m_mapHook.find(hwnd) == m_mapHook.end()) {
    return false;
  }

  wparam = m_mapHook[hwnd].closeMsgWPARAM;
  lparam = m_mapHook[hwnd].closeMsgLPARAM;
  return true;
}

void CefManager::CloseBrowsers(HWND hwnd) {
  std::lock_guard<std::recursive_mutex> lg(hooks_mutex_);
  assert(hwnd);
  bool bFound = m_mapHook.find(hwnd) != m_mapHook.end();

  if (bFound) {
    std::list<CefRefPtr<CefBrowser>> list_b = m_mapHook[hwnd].browsers;
    for (std::list<CefRefPtr<CefBrowser>>::iterator i = list_b.begin(); i != list_b.end(); i++) {
      if ((*i)->GetHost()) {
        (*i)->GetHost()->CloseBrowser(false);
      }
    }
  }
}

LRESULT CALLBACK CefManager::newWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  CefManager* pThis = CefManager::GetInstance()->m_pThis;
  if (!pThis)
    return 0;

  HookInfo hi;
  do {
    std::lock_guard<std::recursive_mutex> lg(pThis->hooks_mutex_);
    if (pThis->m_mapHook.find(hWnd) == pThis->m_mapHook.end())
      return 0;
    hi = pThis->m_mapHook[hWnd];
  } while (false);

  assert(hi.hwnd);
  assert(hi.prevWndProc);
  if (!hi.hwnd || !hi.prevWndProc)
    return 0;

  if (uMsg == WM_CLOSE) {
    if (hi.autoCloseBrowersWhenWindowCloseMsg && hi.closeMsgLPARAM == lParam && hi.closeMsgWPARAM == wParam) {
      SetWindowLongPtr(hWnd, GWL_WNDPROC, (LONG_PTR)hi.prevWndProc);
      pThis->CloseBrowsers(hi.hwnd);

      return 0;
    }
  }
  else if (uMsg == WM_SHOWWINDOW) {
    bool bShow = (wParam == TRUE);
    if (hi.hwndShown != bShow) {
      pThis->SetBrowsersVisible(hWnd, bShow);
      pThis->m_mapHook[hWnd].hwndShown = bShow;
    }
  }

  return ::CallWindowProc(hi.prevWndProc, hWnd, uMsg, wParam, lParam);
}

}  // namespace Internal
}  // namespace DuiLib