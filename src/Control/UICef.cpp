#ifdef UILIB_WITH_CEF
#include "StdAfx.h"
#include "UICef.h"
#include <fstream>
#include <cstring>
#include <gl/gl.h>
#include "Internal/Cef/RequestContextHandler.h"
#include "Internal/Cef/CefHandler.h"
#include "Internal/Cef/CefUtil.h"
#include "include/base/cef_build.h"
#include "Internal/Cef/CefDevTools.h"
#include "Utils/Task.h"
#include "Internal/Cef/CefManager.h"
#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif

// DCHECK on gl errors.
#if DCHECK_IS_ON()
#define VERIFY_NO_ERROR                                                      \
  {                                                                          \
    int _gl_error = glGetError();                                            \
    DCHECK(_gl_error == GL_NO_ERROR) << "glGetError returned " << _gl_error; \
  }
#else
#define VERIFY_NO_ERROR
#endif

namespace {

// Helper that calls wglMakeCurrent.
class ScopedGLContext {
 public:
  ScopedGLContext(HDC hdc, HGLRC hglrc, bool swap_buffers)
      : hdc_(hdc), swap_buffers_(swap_buffers) {
    BOOL result = wglMakeCurrent(hdc, hglrc);
    ALLOW_UNUSED_LOCAL(result);
    DCHECK(result);
  }
  ~ScopedGLContext() {
    BOOL result = wglMakeCurrent(NULL, NULL);
    DCHECK(result);
    if (swap_buffers_) {
      result = SwapBuffers(hdc_);
      DCHECK(result);
    }
  }

 private:
  const HDC hdc_;
  const bool swap_buffers_;
};

LPCWSTR kPreWndProc = L"CefPreWndProc";
LPCWSTR kDraggableRegion = L"CefDraggableRegion";
LPCWSTR kTopLevelHwnd = L"CefTopLevelHwnd";

}  // namespace

namespace DuiLib {

class CCefUI::CCefUIImpl : public Internal::ClientHandlerOsr::OsrDelegate {
 public:
  CCefUIImpl(CCefUI* parent)
      : m_pParent(parent)
      , m_pViewBuffer(NULL)
      , m_hViewBitmap(NULL)
      , m_hViewMemoryDC(NULL)
      , m_pPopupBuffer(NULL)
      , m_hPopupBitmap(NULL)
      , m_hPopupMemoryDC(NULL)
      , m_pDevToolsWnd(NULL)
      , m_iViewMemoryBitmapWidth(0)
      , m_iViewMemoryBitmapHeight(0)
      , m_iPopupMemoryBitmapWidth(0)
      , m_iPopupMemoryBitmapHeight(0)
      , last_mouse_pos_()
      , current_mouse_pos_()
      , mouse_pos_()
      , mouse_rotation_(false)
      , mouse_tracking_(false)
      , last_click_x_(0)
      , last_click_y_(0)
      , last_click_button_(MBT_LEFT)
      , last_click_count_(0)
      , last_click_time_(0)
      , last_mouse_down_on_view_(false)
      , m_iFPS(30)
      , m_bBkTransparent(false)
      , m_bOpenGLInit(false)
      , m_iViewWidth(0)
      , m_iViewHeight(0)
      , m_iTextureId(0)
      , m_hDC(nullptr)
      , m_hGLRC(nullptr)
      , m_hOsrWnd(nullptr)
      , m_bUsingOSR(true)
      , m_bDragEnable(false)
      , m_bUsingOpenGL(false)
      , m_bPaintingPopup_(false)
      , m_bClosing(false)
      , m_draggableRegion(NULL)
      , m_dwCefBkColor(0xffffffff) {
    m_hViewMemoryDC = CreateCompatibleDC(NULL);
    m_hPopupMemoryDC = CreateCompatibleDC(NULL);

    srand(time(NULL));
    m_iRandomID = rand();

    m_draggableRegion = ::CreateRectRgn(0, 0, 0, 0);
  }

  ~CCefUIImpl() {
    assert(m_hGLRC == nullptr);
    assert(m_browser == nullptr);

    ::DeleteObject(m_draggableRegion);
    m_draggableRegion = NULL;

    if (m_pDevToolsWnd) {
      TCHAR szName[MAX_PATH];
      StringCchPrintf(szName, MAX_PATH, TEXT("Dev Tools[%lu]"), m_iRandomID);
      HWND h = FindWindow(TEXT("CefDevToolsWnd891$322@"), szName);
      if (h) {
        m_pDevToolsWnd->Close();
        return;
      }
      // Not Delete m_pDevToolsWnd
      m_pDevToolsWnd = NULL;
    }

    if (m_hViewMemoryDC) {
      DeleteDC(m_hViewMemoryDC);
      m_hViewMemoryDC = NULL;
    }

    if (m_hPopupMemoryDC) {
      DeleteDC(m_hPopupMemoryDC);
      m_hPopupMemoryDC = NULL;
    }
  }

 public:
  // static
  static LRESULT CALLBACK OsrWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    CCefUIImpl* self = Internal::GetUserDataPtr<CCefUIImpl*>(hWnd);
    if (!self)
      return DefWindowProc(hWnd, message, wParam, lParam);

    switch (message) {
#if 0
            case WM_IME_SETCONTEXT:
              self->OnIMESetContext(message, wParam, lParam);
              return 0;
            case WM_IME_STARTCOMPOSITION:
              self->OnIMEStartComposition();
              return 0;
            case WM_IME_COMPOSITION:
              self->OnIMEComposition(message, wParam, lParam);
              return 0;
            case WM_IME_ENDCOMPOSITION:
              self->OnIMECancelCompositionEvent();
              // Let WTL call::DefWindowProc() and release its resources.
              break;
#endif
      case WM_LBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP:
      case WM_MOUSEMOVE:
      case WM_MOUSELEAVE:
      case WM_MOUSEWHEEL:
        self->OnMouseEvent(message, wParam, lParam);
        break;

      case WM_SIZE:
        if (self->m_browser)
          self->m_browser->GetHost()->WasResized();
        break;

      case WM_SETFOCUS:
      case WM_KILLFOCUS:
        self->OnFocus(message == WM_SETFOCUS);
        break;

      case WM_CAPTURECHANGED:
      case WM_CANCELMODE:
        if (self->m_browser)
          self->m_browser->GetHost()->SendCaptureLostEvent();
        break;

      case WM_SYSCHAR:
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_CHAR:
        self->OnKeyEvent(message, wParam, lParam);
        break;

      case WM_PAINT:
        PAINTSTRUCT ps;
        BeginPaint(self->m_hOsrWnd, &ps);
        EndPaint(self->m_hOsrWnd, &ps);

        if (self->m_browser)
          self->m_browser->GetHost()->Invalidate(PET_VIEW);
        return 0;

      case WM_ERASEBKGND:
        if (self->m_browser == nullptr)
          break;
        return 0;
      case WM_NCDESTROY:
        Internal::SetUserDataPtr(hWnd, NULL);
        self->m_hOsrWnd = NULL;
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  void CreateWnd(HWND parent_hwnd, const RECT& rect, cef_color_t background_color) {
    DCHECK(parent_hwnd);
    DCHECK(!::IsRectEmpty(&rect));

    HINSTANCE hInst = ::GetModuleHandle(NULL);
    const HBRUSH background_brush =
        CreateSolidBrush(RGB(CefColorGetR(background_color), CefColorGetG(background_color),
                             CefColorGetB(background_color)));

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = OsrWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = background_brush;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = TEXT("OSR");
    wcex.hIconSm = NULL;

    RegisterClassEx(&wcex);

    DWORD ex_style = 0;
    if (GetWindowLongPtr(parent_hwnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE) {
      ex_style |= WS_EX_NOACTIVATE;
    }

    m_hOsrWnd = ::CreateWindowEx(ex_style, TEXT("OSR"), 0,
                                 WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                                 rect.left, rect.top, rect.right - rect.left,
                                 rect.bottom - rect.top, parent_hwnd, 0, hInst, 0);
    CHECK(m_hOsrWnd);
    Internal::SetUserDataPtr(m_hOsrWnd, this);
  }

  bool CreateBrowser() {
    if (!m_bUsingOSR) {
      m_bUsingOpenGL = false;
    }
    cef_color_t bk_color = CefColorSetARGB(m_bBkTransparent ? 0 : 255, GetBValue(m_dwCefBkColor),
                                           GetGValue(m_dwCefBkColor), GetRValue(m_dwCefBkColor));
    if (m_bUsingOpenGL) {
      RECT controlRC = m_pParent->GetPos();
      if (::IsRectEmpty(&controlRC))
        return false;
      CreateWnd(m_pParent->GetManager()->GetPaintWindow(),
                {0, 0, controlRC.right - controlRC.left, controlRC.bottom - controlRC.top},
                bk_color);
      if (m_hOsrWnd) {
        SetWindowPos(m_hOsrWnd, NULL, controlRC.left, controlRC.top, 0, 0, SWP_NOSIZE);
      }
      EnableGL();
      if (!m_bOpenGLInit) {
        DisableGL();
        m_bUsingOpenGL = false;
        Internal::SetUserDataPtr(m_hOsrWnd, nullptr);
        ::ShowWindow(m_hOsrWnd, SW_HIDE);
        ::PostMessage(m_hOsrWnd, WM_CLOSE, 0, 0);
      }
    }

    DCHECK(!m_browser);
    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = m_iFPS;
    browser_settings.background_color = bk_color;

    CefWindowInfo window_info;
    if (m_bUsingOSR) {
      window_info.SetAsWindowless(
        m_bUsingOpenGL ? m_hOsrWnd : m_pParent->m_pManager->GetPaintWindow(), m_bBkTransparent);
      if (GetWindowLongPtr(m_bUsingOpenGL ? m_hOsrWnd : m_pParent->m_pManager->GetPaintWindow(),
        GWL_EXSTYLE) &
        WS_EX_NOACTIVATE) {
        window_info.ex_style |= WS_EX_NOACTIVATE;
      }
    }
    else {
      RECT controlRC = m_pParent->GetPos();
      if (::IsRectEmpty(&controlRC))
        return false;
      window_info.SetAsChild(m_pParent->m_pManager->GetPaintWindow(), controlRC);
      if (GetWindowLongPtr(m_pParent->m_pManager->GetPaintWindow(),
                           GWL_EXSTYLE) & WS_EX_NOACTIVATE) {
        window_info.ex_style |= WS_EX_NOACTIVATE;
      }
    }

    m_ClientHandler = new Internal::ClientHandlerOsr(this);
    m_ClientHandler->SetAllowProtocols(m_vAllowProtocols);

    m_strInitUrl = m_pParent->GetUrl();

    CefRefPtr<CefRequestContext> request_context = CefRequestContext::CreateContext(
        CefRequestContext::GetGlobalContext(), new Internal::RequestContextHandler);
    CefString error;

    CefBrowserHost::CreateBrowser(window_info, m_ClientHandler,
                                  Unicode2Utf8(m_strInitUrl.GetData()), browser_settings,
                                  request_context);
    return true;
  }

  void SetUrl(const CDuiString& url) {
    if (m_browser) {
      if (m_browser->GetMainFrame()) {
        m_browser->GetMainFrame()->LoadURL(Unicode2Utf8(m_pParent->GetUrl().GetData()).c_str());
      }
    }
  }

  void SetUsingOpenGL(bool b) { m_bUsingOpenGL = b; }

  bool IsUsingOpenGL() { return m_bUsingOpenGL; }

  void SetUsingOSR(bool b) {
    m_bUsingOSR = b;
  }

  void SetDragEnable(bool b) {
    m_bDragEnable = b;
  }

  bool IsUsingOSR() const {
    return m_bUsingOSR;
  }


  void GoBack() {
    if (m_browser)
      m_browser->GoBack();
  }

  void GoForward() {
    if (m_browser)
      m_browser->GoForward();
  }

  bool IsLoading() {
    if (m_browser)
      return m_browser->IsLoading();
    return false;
  }

  void StopLoad() {
    if (m_browser)
      m_browser->StopLoad();
  }

  void Reload(bool bIgnoreCache) {
    if (m_browser) {
      if (bIgnoreCache) {
        m_browser->ReloadIgnoreCache();
      }
      else {
        m_browser->Reload();
      }
    }
  }

  void ShowDevTools() {
    if (!m_browser)
      return;
    TCHAR szName[MAX_PATH];
    StringCchPrintf(szName, MAX_PATH, TEXT("Dev Tools[%lu]"), m_iRandomID);

    HWND h = FindWindow(TEXT("CefDevToolsWnd891$322@"), szName);
    if (h) {
      SetForegroundWindow(h);
      return;
    }

    m_pDevToolsWnd = new Internal::CefDevToolsWnd(
        m_browser, m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f);
    m_pDevToolsWnd->Create(NULL, szName, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
    m_pDevToolsWnd->ShowWindow();
  }

  void CloseDevTools() {
    if (m_pDevToolsWnd) {
      m_pDevToolsWnd->Close();
      m_pDevToolsWnd = NULL;  // delete in DevToolsWnd internal.
    }
  }

  bool CallJavascriptFunction(const std::string& strFuncName,
                              const std::vector<CLiteVariant>& args) {
    if (m_bClosing)
      return false;
    bool ret = false;
    if (m_browser) {
      CefRefPtr<CefProcessMessage> message =
          CefProcessMessage::Create(Internal::BROWSER_2_RENDER_CPP_CALL_JS_MSG);
      CefRefPtr<CefListValue> arg_list = message->GetArgumentList();
      arg_list->SetString(0, strFuncName);

      int cefListIndex = 1;
      for (auto item : args) {
        CLiteVariant::DataType dt = item.GetType();
        if (dt == CLiteVariant::DataType::DT_INT) {
          arg_list->SetInt(cefListIndex, item.GetInt());
        }
        else if (dt == CLiteVariant::DataType::DT_DOUBLE) {
          arg_list->SetDouble(cefListIndex, item.GetDouble());
        }
        else if (dt == CLiteVariant::DataType::DT_STRING) {
          arg_list->SetString(cefListIndex, item.GetString());
        }

        cefListIndex++;
      }
      ret = m_browser->SendProcessMessage(PID_RENDERER, message);
    }
    return true;
  }

  // CefContextMenuHandler methods
  void OnBeforeContextMenu(CefRefPtr<CefMenuModel> model) OVERRIDE { model->Clear(); }

  //////////////////////////////////////////////////////////////////////////
  // ClientHandlerOsr::OsrDelegate methods.
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE {
    m_bClosing = false;
    m_browser = browser;
    Internal::CefManager::GetInstance()->AddBrowser(browser,
                                                    m_pParent->GetManager()->GetPaintWindow());
    if (m_pParent && m_browser) {
      if (m_browser->GetHost()) {
        if (m_browser->GetHost()->GetWindowlessFrameRate() != m_iFPS) {
          m_browser->GetHost()->SetWindowlessFrameRate(m_iFPS);
        }
      }

      if (m_pParent->GetUrl() != m_strInitUrl) {
        if (m_browser->GetMainFrame()) {
          m_browser->GetMainFrame()->LoadURL(Unicode2Utf8(m_pParent->GetUrl().GetData()).c_str());
        }
      }
    }
  }

  void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE {
    HWND hwnd = m_pParent->GetManager()->GetPaintWindow();
    Internal::CefManager::GetInstance()->RemoveBrowser(browser, hwnd);
    m_ClientHandler->DetachDelegate();
    m_browser = nullptr;

    DisableGL();

    if (Internal::CefManager::GetInstance()->GetBrowserCount(hwnd) == 0) {
      WPARAM wparam = 0;
      LPARAM lparam = 0;
      Internal::CefManager::GetInstance()->GetCloseMsgParam(hwnd, wparam, lparam);
      ::PostMessage(m_pParent->GetManager()->GetPaintWindow(), WM_CLOSE, wparam, lparam);
    }
  }

  bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE {
    CEF_REQUIRE_UI_THREAD();

    return false;
  }
#if CEFVER == 3626
  void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;
#else
  bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE {
    CEF_REQUIRE_UI_THREAD();
    DCHECK(m_pParent);
    if (!m_pParent)
      return false;
    DCHECK(m_pParent->m_pManager);

    RECT pos = m_pParent->GetPos();
    if (m_bUsingOpenGL)
      GetClientRect(m_hOsrWnd, &pos);

    int width = pos.right - pos.left;
    int height = pos.bottom - pos.top;

    float scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;

    rect.x = 0;
    rect.y = 0;
    rect.width = Internal::DeviceToLogical(width, scale_factor);
    if (rect.width == 0)
      rect.width = 1;

    rect.height = Internal::DeviceToLogical(height, scale_factor);
    if (rect.height == 0)
      rect.height = 1;
    return true;
  }
#endif
  bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
                      int viewX,
                      int viewY,
                      int& screenX,
                      int& screenY) OVERRIDE {
    CEF_REQUIRE_UI_THREAD();
    DCHECK(m_pParent);
    if (!m_pParent)
      return false;
    DCHECK(m_pParent->m_pManager);

    float scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;

    // Convert the point from view coordinates to actual screen coordinates.
    POINT screen_pt = {Internal::LogicalToDevice(viewX, scale_factor),
                       Internal::LogicalToDevice(viewY, scale_factor)};
    ClientToScreen(m_bUsingOpenGL ? m_hOsrWnd : m_pParent->m_pManager->GetPaintWindow(),
                   &screen_pt);
    screenX = screen_pt.x;
    screenY = screen_pt.y;
    return true;
  }

  bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) OVERRIDE {
    CEF_REQUIRE_UI_THREAD();
    DCHECK(m_pParent);
    if (!m_pParent)
      return false;
    DCHECK(m_pParent->m_pManager);

    CefRect view_rect;
    GetViewRect(browser, view_rect);

    screen_info.device_scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;
    screen_info.rect = view_rect;
    screen_info.available_rect = view_rect;
    return true;
  }

  void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE {
    if (!show) {
      m_PopupRect.Reset();
      browser->GetHost()->Invalidate(PET_VIEW);
    }
  }

  void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) OVERRIDE {
    if (!m_pParent)
      return;
    float scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;

    CefRect newRect = Internal::LogicalToDevice(rect, scale_factor);
    if (newRect.width <= 0 || newRect.height <= 0)
      return;
    m_OriginPopupRect = newRect;
    m_PopupRect = GetPopupRectInWebView(newRect);
  }

  CefRect GetPopupRectInWebView(const CefRect& original_rect) {
    CefRect rc(original_rect);
    // if x or y are negative, move them to 0.
    if (rc.x < 0)
      rc.x = 0;
    if (rc.y < 0)
      rc.y = 0;
    // if popup goes outside the view, try to reposition origin
    if (rc.x + rc.width > m_iViewWidth)
      rc.x = m_iViewWidth - rc.width;
    if (rc.y + rc.height > m_iViewHeight)
      rc.y = m_iViewHeight - rc.height;
    // if x or y became negative, move them to 0 again.
    if (rc.x < 0)
      rc.x = 0;
    if (rc.y < 0)
      rc.y = 0;
    return rc;
  }

  void OnPaint(CefRefPtr<CefBrowser> browser,
               CefRenderHandler::PaintElementType type,
               const CefRenderHandler::RectList& dirtyRects,
               const void* buffer,
               int width,
               int height) OVERRIDE {
    if (m_bClosing)
      return;
    if (m_bUsingOpenGL) {
      if (m_bPaintingPopup_) {
        OnOpenGLPaint(browser, type, dirtyRects, buffer, width, height);
        return;
      }

      if (!m_hGLRC) {
        EnableGL();
      }
      assert(m_bOpenGLInit);
      assert(m_hDC);

      ScopedGLContext scoped_gl_context(m_hDC, m_hGLRC, true);
      OnOpenGLPaint(browser, type, dirtyRects, buffer, width, height);
      if (type == PET_VIEW && !m_PopupRect.IsEmpty()) {
        m_bPaintingPopup_ = true;
        browser->GetHost()->Invalidate(PET_POPUP);
        m_bPaintingPopup_ = false;
      }
      OpenGLRender();
      return;
    }
    if (!m_pParent)
      return;
    if (type == CefRenderHandler::PaintElementType::PET_VIEW) {
      m_csViewBuf.Enter();

      m_iViewWidth = width;
      m_iViewHeight = height;

      if (width != m_iViewMemoryBitmapWidth || height != m_iViewMemoryBitmapHeight) {
        BITMAPINFO bi;
        memset(&bi, 0, sizeof(bi));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = int(width);
        bi.bmiHeader.biHeight = -int(height);
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        HBITMAP hbmp = ::CreateDIBSection(0, &bi, DIB_RGB_COLORS, &m_pViewBuffer, NULL, 0);
        ::SelectObject(m_hViewMemoryDC, hbmp);
        if (m_hViewBitmap)
          ::DeleteObject(m_hViewBitmap);

        m_hViewBitmap = hbmp;

        m_iViewMemoryBitmapWidth = width;
        m_iViewMemoryBitmapHeight = height;
      }

      memcpy(m_pViewBuffer, buffer, width * height * 4);
      m_csViewBuf.Leave();

      if (m_pParent)
        m_pParent->Invalidate();
    }
    else if (type == CefRenderHandler::PaintElementType::PET_POPUP) {
      m_csPopupBuf.Enter();

      if (width != m_iPopupMemoryBitmapWidth || height != m_iPopupMemoryBitmapHeight) {
        BITMAPINFO bi;
        memset(&bi, 0, sizeof(bi));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = int(width);
        bi.bmiHeader.biHeight = -int(height);
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        HBITMAP hbmp = ::CreateDIBSection(0, &bi, DIB_RGB_COLORS, &m_pPopupBuffer, NULL, 0);
        ::SelectObject(m_hPopupMemoryDC, hbmp);
        if (m_hPopupBitmap)
          ::DeleteObject(m_hPopupBitmap);

        m_hPopupBitmap = hbmp;

        m_iPopupMemoryBitmapWidth = width;
        m_iPopupMemoryBitmapHeight = height;
      }
      memcpy(m_pPopupBuffer, buffer, width * height * 4);
      m_csPopupBuf.Leave();

      if (m_pParent)
        m_pParent->Invalidate();
    }
  }

  void DoPaint(HDC hdc) {
    if (m_bClosing)
      return;
    if (m_pParent) {
      RECT controlRC = m_pParent->GetPos();
      m_csViewBuf.Enter();
      ASSERT(m_hViewMemoryDC);
      BitBlt(hdc, controlRC.left, controlRC.top, controlRC.right - controlRC.left,
             controlRC.bottom - controlRC.top, m_hViewMemoryDC, 0, 0, SRCCOPY);
      m_csViewBuf.Leave();

      if (!m_PopupRect.IsEmpty()) {
        m_csPopupBuf.Enter();
        BitBlt(hdc, m_PopupRect.x + controlRC.left, m_PopupRect.y + controlRC.top,
               m_PopupRect.width, m_PopupRect.height, m_hPopupMemoryDC, 0, 0, SRCCOPY);
        m_csPopupBuf.Leave();
      }

      if (!m_PopupRect.IsEmpty()) {
        m_browser->GetHost()->Invalidate(PET_POPUP);
      }
    }
  }

  void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                          CefRenderHandler::PaintElementType type,
                          const CefRenderHandler::RectList& dirtyRects,
                          void* share_handle) OVERRIDE {}

  void OnCursorChange(CefRefPtr<CefBrowser> browser,
                      CefCursorHandle cursor,
                      CefRenderHandler::CursorType type,
                      const CefCursorInfo& custom_cursor_info) OVERRIDE {
    CEF_REQUIRE_UI_THREAD();
    if (!m_pParent)
      return;
    HWND hwnd = nullptr;
    if (m_bUsingOpenGL)
      hwnd = m_hOsrWnd;
    else
      hwnd = m_pParent->m_pManager->GetPaintWindow();
    if (!hwnd || !::IsWindow(hwnd))
      return;

    // Change the plugin window's cursor.
    SetClassLongPtr(hwnd, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
    ::SetCursor(cursor);
  }

  bool StartDragging(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefDragData> drag_data,
                     CefRenderHandler::DragOperationsMask allowed_ops,
                     int x,
                     int y) OVERRIDE {
    CEF_REQUIRE_UI_THREAD();
    // Cancel the drag. The dragging implementation requires ATL support.
    return false;
  }

  void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                        CefRenderHandler::DragOperation operation) OVERRIDE {}
#if CEFVER == 3626
  void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
                                    const CefRange& selection_range,
                                    const CefRenderHandler::RectList& character_bounds) OVERRIDE;
#endif
  void UpdateAccessibilityTree(CefRefPtr<CefValue> value) OVERRIDE {}

  void OnBrowserClosing(CefRefPtr<CefBrowser> browser) OVERRIDE { m_bClosing = true; }

  void OnSetAddress(const std::string& url) OVERRIDE {}

  void OnSetTitle(const std::string& title) OVERRIDE {}

  void OnSetFullscreen(bool fullscreen) OVERRIDE {}

  void OnAutoResize(const CefSize& new_size) OVERRIDE {}

  void OnSetLoadingState(bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE {}

  void OnLoadError(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   CefLoadHandler::ErrorCode errorCode,
                   const CefString& errorText,
                   const CefString& failedUrl) {
    if (m_bClosing)
      return;
    if (m_pParent && m_pParent->m_LoadErrorCB) {
      // bool isMainFrame, const std::string &url, int errorCode, const std::string &errorText
      m_pParent->m_LoadErrorCB(frame->IsMain(), failedUrl.ToString(), (int)errorCode,
                               errorText.ToString());
    }
  }

  void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) {
    //if (httpStatusCode < 400)
    //    return;

    //if (frame->IsMain()) {

    //}
  }


  static LRESULT CALLBACK SubclassedWindowProc(HWND hWnd,
                                                        UINT message,
                                                        WPARAM wParam,
                                                        LPARAM lParam) {
    WNDPROC hPreWndProc = reinterpret_cast<WNDPROC>(::GetPropW(hWnd, kPreWndProc));
    HRGN hRegion = reinterpret_cast<HRGN>(::GetPropW(hWnd, kDraggableRegion));
    HWND hTopLevelWnd = reinterpret_cast<HWND>(::GetPropW(hWnd, kTopLevelHwnd));

    if (message == WM_LBUTTONDOWN) {
      POINT point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      if (::PtInRegion(hRegion, point.x, point.y)) {
        ::ClientToScreen(hWnd, &point);
        ::PostMessage(hTopLevelWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
        return 0;
      }
    }

    ASSERT(hPreWndProc);
    return CallWindowProc(hPreWndProc, hWnd, message, wParam, lParam);
  }

  static BOOL CALLBACK SubclassWindowsProc(HWND hwnd, LPARAM lParam) {
    CCefUIImpl* pImpl = (CCefUIImpl*)lParam;
    subclassWindow(hwnd, reinterpret_cast<HRGN>(pImpl->m_draggableRegion),
                   (HWND)pImpl->m_pParent->m_pManager->GetPaintWindow());
    return TRUE;
  }

  static BOOL CALLBACK UnSubclassWindowsProc(HWND hwnd, LPARAM lParam) {
    unSubclassWindow(hwnd);
    return TRUE;
  }

  static void subclassWindow(HWND hWnd, HRGN hRegion, HWND hTopLevelWnd) {
    HANDLE hParentWndProc = ::GetPropW(hWnd, kPreWndProc);
    if (hParentWndProc) {
      ::SetPropW(hWnd, kDraggableRegion, reinterpret_cast<HANDLE>(hRegion));
      return;
    }

    SetLastError(0);
    LONG_PTR hOldWndProc =
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(SubclassedWindowProc));
    if (hOldWndProc == NULL && GetLastError() != ERROR_SUCCESS) {
      return;
    }

    ::SetPropW(hWnd, kPreWndProc, reinterpret_cast<HANDLE>(hOldWndProc));
    ::SetPropW(hWnd, kDraggableRegion, reinterpret_cast<HANDLE>(hRegion));
    ::SetPropW(hWnd, kTopLevelHwnd, reinterpret_cast<HANDLE>(hTopLevelWnd));
  }

  static void unSubclassWindow(HWND hWnd) {
    LONG_PTR hPreWndProc = reinterpret_cast<LONG_PTR>(::GetPropW(hWnd, kPreWndProc));
    if (hPreWndProc) {
      LONG_PTR hPreviousWndProc = SetWindowLongPtr(hWnd, GWLP_WNDPROC, hPreWndProc);
      ALLOW_UNUSED_LOCAL(hPreviousWndProc);
      DCHECK_EQ(hPreviousWndProc, reinterpret_cast<LONG_PTR>(SubclassedWindowProc));
    }

    ::RemovePropW(hWnd, kPreWndProc);
    ::RemovePropW(hWnd, kDraggableRegion);
    ::RemovePropW(hWnd, kTopLevelHwnd);
  }

  void OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser,
                                 const std::vector<CefDraggableRegion>& regions) {
    if (!m_bDragEnable)
      return;
    ::SetRectRgn(m_draggableRegion, 0, 0, 0, 0);

    float dpiScale = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;

    std::vector<CefDraggableRegion>::const_iterator it = regions.begin();
    for (; it != regions.end(); ++it) {
      cef_rect_t rc = it->bounds;
      rc.x = (float)rc.x * dpiScale;
      rc.y = (float)rc.y * dpiScale;
      rc.width = (float)rc.width * dpiScale;
      rc.height = (float)rc.height * dpiScale;
      HRGN region = ::CreateRectRgn(rc.x, rc.y, rc.x + rc.width, rc.y + rc.height);
      ::CombineRgn(m_draggableRegion, m_draggableRegion, region, it->draggable ? RGN_OR : RGN_DIFF);
      ::DeleteObject(region);
    }

    ASSERT(browser && browser->GetHost());
    if (browser && browser->GetHost()) {
      HWND hwnd = browser->GetHost()->GetWindowHandle();
      ASSERT(hwnd);
      if (hwnd) {
        if (m_bUsingOSR) {
          subclassWindow(hwnd, reinterpret_cast<HRGN>(m_draggableRegion),
                         (HWND)m_pParent->m_pManager->GetPaintWindow());
        }
        else {
          WNDENUMPROC proc = !regions.empty() ? SubclassWindowsProc : UnSubclassWindowsProc;
          ::EnumChildWindows(hwnd, proc, reinterpret_cast<LPARAM>(this));
        }
      }
    }
  }

  void OnSetDraggableRegions(const std::vector<CefDraggableRegion>& regions) OVERRIDE {}

  bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) OVERRIDE {
    if (m_bClosing)
      return false;
    if (m_pParent && m_pParent->m_pManager) {
      if (text.length() == 0) {
        m_pParent->m_pManager->HideToolTip();
      }
      else {
        m_pParent->m_pManager->ShowToolTip(text.c_str(), mouse_pos_);
      }
    }

    return true;
  }

  void OnJSNotify(const CefRefPtr<CefListValue>& value_list) OVERRIDE {
    if (m_bClosing)
      return;
    if (value_list->GetSize() < 2)
      return;

    std::string strBusinessName = value_list->GetString(1);
    std::vector<CLiteVariant> liteVars;
    for (size_t i = 2; i < value_list->GetSize(); i++) {
      CLiteVariant liteTmp;
      CefValueType type = value_list->GetType(i);
      switch (type) {
        case VTYPE_BOOL: {
          liteTmp.SetType(CLiteVariant::DataType::DT_INT);
          liteTmp.SetInt(value_list->GetBool(i) ? 1 : 0);
        } break;
        case VTYPE_DOUBLE: {
          liteTmp.SetType(CLiteVariant::DataType::DT_INT);
          liteTmp.SetDouble(value_list->GetDouble(i));
        } break;
        case VTYPE_INT: {
          liteTmp.SetType(CLiteVariant::DataType::DT_INT);
          liteTmp.SetInt(value_list->GetInt(i));
        } break;
        case VTYPE_STRING: {
          liteTmp.SetType(CLiteVariant::DataType::DT_STRING);
          liteTmp.SetString(value_list->GetString(i).ToString());
        } break;
        default:
          break;
      }
      liteVars.push_back(liteTmp);
    }

    if (m_pParent && m_pParent->m_JSCB) {
      m_pParent->m_JSCB(strBusinessName, liteVars);
    }
  }

  void OnResourceResponse(const std::string url, int rsp_status) OVERRIDE {
    if (!m_pParent)
      return;
    if (m_pParent && m_pParent->m_ResourceRspCB) {
      m_pParent->m_ResourceRspCB(url, rsp_status);
    }
  }

  bool OnBeforePopup(const std::string& target_url) OVERRIDE { return true; }

  void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                 CefRequestHandler::TerminationStatus status) {
    if (!m_pParent)
      return;

    if (m_pParent->m_RenderProcessTerminatedCB) {
      std::string url;
      if (browser && browser->GetMainFrame()) {
        url = browser->GetMainFrame()->GetURL().ToString();
      }
      m_pParent->m_RenderProcessTerminatedCB(url, (int)status);
    }
  }

  bool IsOverPopupWidget(int x, int y) const {
    const CefRect& rc = m_PopupRect;
    int popup_right = rc.x + rc.width;
    int popup_bottom = rc.y + rc.height;
    return (x >= rc.x) && (x < popup_right) && (y >= rc.y) && (y < popup_bottom);
  }

  int GetPopupXOffset() const { return m_OriginPopupRect.x - m_PopupRect.x; }

  int GetPopupYOffset() const { return m_OriginPopupRect.y - m_PopupRect.y; }

  void ApplyPopupOffset(int& x, int& y) const {
    if (IsOverPopupWidget(x, y)) {
      x += GetPopupXOffset();
      y += GetPopupYOffset();
    }
  }

  void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {
    DCHECK(m_pParent);
    if (!m_pParent)
      return;
    DCHECK(m_pParent->m_pManager);
    float scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;
    HWND hwnd = m_bUsingOpenGL ? m_hOsrWnd : m_pParent->m_pManager->GetPaintWindow();
    RECT pos = {0, 0, 0, 0};

    if (!m_bUsingOpenGL)
      pos = m_pParent->GetPos();

    CefRefPtr<CefBrowserHost> browser_host;
    if (m_browser)
      browser_host = m_browser->GetHost();

    LONG currentTime = 0;
    bool cancelPreviousClick = false;

    if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN ||
        message == WM_MOUSEMOVE || message == WM_MOUSELEAVE) {
      currentTime = GetMessageTime();
      int x = GET_X_LPARAM(lParam) - pos.left;
      int y = GET_Y_LPARAM(lParam) - pos.top;
      cancelPreviousClick = (abs(last_click_x_ - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2)) ||
                            (abs(last_click_y_ - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2)) ||
                            ((currentTime - last_click_time_) > GetDoubleClickTime());
      if (cancelPreviousClick && (message == WM_MOUSEMOVE || message == WM_MOUSELEAVE)) {
        last_click_count_ = 0;
        last_click_x_ = 0;
        last_click_y_ = 0;
        last_click_time_ = 0;
      }
    }

    switch (message) {
      case WM_LBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_MBUTTONDOWN: {
        ::SetCapture(hwnd);
        ::SetFocus(hwnd);
        int x = GET_X_LPARAM(lParam) - pos.left;
        int y = GET_Y_LPARAM(lParam) - pos.top;
        if (wParam & MK_SHIFT) {
          // Start rotation effect.
          last_mouse_pos_.x = current_mouse_pos_.x = x;
          last_mouse_pos_.y = current_mouse_pos_.y = y;
          mouse_rotation_ = true;
        }
        else {
          CefBrowserHost::MouseButtonType btnType =
              (message == WM_LBUTTONDOWN ? MBT_LEFT
                                         : (message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
          if (!cancelPreviousClick && (btnType == last_click_button_)) {
            ++last_click_count_;
          }
          else {
            last_click_count_ = 1;
            last_click_x_ = x;
            last_click_y_ = y;
          }
          last_click_time_ = currentTime;
          last_click_button_ = btnType;

          if (browser_host) {
            CefMouseEvent mouse_event;
            mouse_event.x = x;
            mouse_event.y = y;
            last_mouse_down_on_view_ = !IsOverPopupWidget(x, y);
            ApplyPopupOffset(mouse_event.x, mouse_event.y);
            Internal::DeviceToLogical(mouse_event, scale_factor);
            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
            browser_host->SendMouseClickEvent(mouse_event, btnType, false, last_click_count_);
          }
        }
      } break;

      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP: {
        if (GetCapture() == hwnd)
          ReleaseCapture();
        if (mouse_rotation_) {
          // End rotation effect.
          mouse_rotation_ = false;
        }
        else {
          int x = GET_X_LPARAM(lParam) - pos.left;
          int y = GET_Y_LPARAM(lParam) - pos.top;
          CefBrowserHost::MouseButtonType btnType =
              (message == WM_LBUTTONUP ? MBT_LEFT
                                       : (message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
          if (browser_host) {
            CefMouseEvent mouse_event;
            mouse_event.x = x;
            mouse_event.y = y;
            if (last_mouse_down_on_view_ && IsOverPopupWidget(x, y) &&
                (GetPopupXOffset() || GetPopupYOffset())) {
              OutputDebugStringA("11\n");
              break;
            }
            ApplyPopupOffset(mouse_event.x, mouse_event.y);
            Internal::DeviceToLogical(mouse_event, scale_factor);
            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
            browser_host->SendMouseClickEvent(mouse_event, btnType, true, last_click_count_);
          }
        }
        break;
      }
      case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam) - pos.left;
        int y = GET_Y_LPARAM(lParam) - pos.top;

        mouse_pos_.x = x;
        mouse_pos_.y = y;

        if (mouse_rotation_) {
          // Apply rotation effect.
          current_mouse_pos_.x = x;
          current_mouse_pos_.y = y;
          last_mouse_pos_.x = current_mouse_pos_.x;
          last_mouse_pos_.y = current_mouse_pos_.y;
        }
        else {
          if (!mouse_tracking_) {
            // Start tracking mouse leave. Required for the WM_MOUSELEAVE event to be generated.
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            mouse_tracking_ = true;
          }

          if (browser_host) {
            CefMouseEvent mouse_event;
            mouse_event.x = x;
            mouse_event.y = y;
            ApplyPopupOffset(mouse_event.x, mouse_event.y);
            Internal::DeviceToLogical(mouse_event, scale_factor);
            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
            browser_host->SendMouseMoveEvent(mouse_event, false);
          }
        }
        break;
      }

      case WM_MOUSELEAVE: {
        if (mouse_tracking_) {
          // Stop tracking mouse leave.
          TRACKMOUSEEVENT tme;
          tme.cbSize = sizeof(TRACKMOUSEEVENT);
          tme.dwFlags = TME_LEAVE & TME_CANCEL;
          tme.hwndTrack = hwnd;
          TrackMouseEvent(&tme);
          mouse_tracking_ = false;
        }

        if (browser_host) {
          // Determine the cursor position in screen coordinates.
          POINT p;
          ::GetCursorPos(&p);
          ::ScreenToClient(hwnd, &p);

          CefMouseEvent mouse_event;
          mouse_event.x = p.x;
          mouse_event.y = p.y;
          Internal::DeviceToLogical(mouse_event, scale_factor);
          mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
          browser_host->SendMouseMoveEvent(mouse_event, true);
        }
      } break;

      case WM_MOUSEWHEEL:
        if (browser_host) {
          POINT screen_point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
          HWND scrolled_wnd = ::WindowFromPoint(screen_point);
          if (scrolled_wnd != hwnd)
            break;

          ScreenToClient(hwnd, &screen_point);
          screen_point.x -= pos.left;
          screen_point.y -= pos.top;
          int delta = GET_WHEEL_DELTA_WPARAM(wParam);

          CefMouseEvent mouse_event;
          mouse_event.x = screen_point.x;
          mouse_event.y = screen_point.y;
          ApplyPopupOffset(mouse_event.x, mouse_event.y);
          Internal::DeviceToLogical(mouse_event, scale_factor);
          mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
          browser_host->SendMouseWheelEvent(mouse_event, Internal::IsKeyDown(VK_SHIFT) ? delta : 0,
                                            !Internal::IsKeyDown(VK_SHIFT) ? delta : 0);
        }
        break;
    }
  }

  void OnKeyEvent(UINT message, WPARAM wParam, LPARAM lParam) {
    if (!m_browser)
      return;

    CefKeyEvent event;
    event.windows_key_code = wParam;
    event.native_key_code = lParam;
    event.is_system_key =
        (message == WM_SYSCHAR || message == WM_SYSKEYDOWN || message == WM_SYSKEYUP);

    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
      event.type = KEYEVENT_RAWKEYDOWN;
    else if (message == WM_KEYUP || message == WM_SYSKEYUP)
      event.type = KEYEVENT_KEYUP;
    else
      event.type = KEYEVENT_CHAR;
    event.modifiers = Internal::GetCefKeyboardModifiers(wParam, lParam);

    m_browser->GetHost()->SendKeyEvent(event);
  }

  void OnFocus(bool setFocus) {
    if (m_browser)
      m_browser->GetHost()->SendFocusEvent(setFocus);
  }

  void OnSize() {
    if (m_bUsingOSR) {
      if (m_bUsingOpenGL && m_hOsrWnd) {
        RECT controlRC = m_pParent->GetPos();
        SetWindowPos(m_hOsrWnd, NULL, controlRC.left, controlRC.top, controlRC.right - controlRC.left,
          controlRC.bottom - controlRC.top, SWP_NOZORDER);
      } else {
        if (m_browser)
          m_browser->GetHost()->WasResized();
      }
    }
    else {
      RECT controlRC = m_pParent->GetPos();
      HWND hwnd = NULL;
      if (m_browser)
        hwnd = m_browser->GetHost()->GetWindowHandle();
      if (hwnd) {
        SetWindowPos(hwnd, NULL, controlRC.left, controlRC.top, controlRC.right - controlRC.left,
          controlRC.bottom - controlRC.top, SWP_NOZORDER);
      }
    }
  }

  void SetAllowProtocols(const std::vector<std::string> vAllowProtocols) {
    m_vAllowProtocols = vAllowProtocols;
    if (m_ClientHandler) {
      m_ClientHandler->SetAllowProtocols(vAllowProtocols);
    }
  }

  std::vector<std::string> GetAllowProtocols() const { return m_vAllowProtocols; }

  void SetFPS(int fps) {
    if (m_iFPS != fps) {
      m_iFPS = fps;
      if (m_browser && m_browser->GetHost())
        m_browser->GetHost()->SetWindowlessFrameRate(fps);
    }
  }

  int GetFPS() const { return m_iFPS; }

  void SetCefBkColor(DWORD dwBackColor) { m_dwCefBkColor = dwBackColor; }

  DWORD GetCefBkColor() const { return m_dwCefBkColor; }

  void SetBkTransparent(bool b) { m_bBkTransparent = b; }

  bool GetBkTransparent() const { return m_bBkTransparent; }

  bool HDCToFile(const char* FilePath, HDC Context, RECT Area, uint16_t BitsPerPixel) {
    uint32_t Width = Area.right - Area.left;
    uint32_t Height = Area.bottom - Area.top;
    BITMAPINFO Info;
    BITMAPFILEHEADER Header;
    memset(&Info, 0, sizeof(Info));
    memset(&Header, 0, sizeof(Header));
    Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    Info.bmiHeader.biWidth = Width;
    Info.bmiHeader.biHeight = Height;
    Info.bmiHeader.biPlanes = 1;
    Info.bmiHeader.biBitCount = BitsPerPixel;
    Info.bmiHeader.biCompression = BI_RGB;
    Info.bmiHeader.biSizeImage = Width * Height * (BitsPerPixel > 24 ? 4 : 3);
    Header.bfType = 0x4D42;
    Header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    char* Pixels = NULL;
    HDC MemDC = CreateCompatibleDC(Context);
    HBITMAP Section = CreateDIBSection(Context, &Info, DIB_RGB_COLORS, (void**)&Pixels, 0, 0);
    DeleteObject(SelectObject(MemDC, Section));
    BitBlt(MemDC, 0, 0, Width, Height, Context, Area.left, Area.top, SRCCOPY);
    DeleteDC(MemDC);
    std::fstream hFile(FilePath, std::ios::out | std::ios::binary);
    if (hFile.is_open()) {
      hFile.write((char*)&Header, sizeof(Header));
      hFile.write((char*)&Info.bmiHeader, sizeof(Info.bmiHeader));
      hFile.write(Pixels, (((BitsPerPixel * Width + 31) & ~31) / 8) * Height);
      hFile.close();
      DeleteObject(Section);
      return true;
    }
    DeleteObject(Section);
    return false;
  }

  void EnableGL() {
    PIXELFORMATDESCRIPTOR pfd;
    int format;

    // Get the device context.
    m_hDC = GetDC(m_hOsrWnd);

    // Set the pixel format for the DC.
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    format = ChoosePixelFormat(m_hDC, &pfd);
    SetPixelFormat(m_hDC, format, &pfd);

    // Create and enable the render context.
    m_hGLRC = wglCreateContext(m_hDC);

    ScopedGLContext scoped_gl_context(m_hDC, m_hGLRC, false);
    InitOpenGL();
  }

  void DisableGL() {
    if (!m_hGLRC) {
      return;
    }

    {
      ScopedGLContext scoped_gl_context(m_hDC, m_hGLRC, false);
      CleanupOpenGL();
    }

    if (IsWindow(m_hOsrWnd)) {
      // wglDeleteContext will make the context not current before
      // deleting it.
      BOOL result = wglDeleteContext(m_hGLRC);
      ALLOW_UNUSED_LOCAL(result);
      DCHECK(result);
      ReleaseDC(m_hOsrWnd, m_hDC);
    }

    m_hGLRC = NULL;
  }

  void InitOpenGL() {
    if (m_bOpenGLInit)
      return;

    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    VERIFY_NO_ERROR;

    if (m_bBkTransparent) {
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      VERIFY_NO_ERROR;
    }
    else {
      cef_color_t background_color =
          CefColorSetARGB(m_bBkTransparent ? 0 : 255, GetBValue(m_dwCefBkColor),
                          GetGValue(m_dwCefBkColor), GetRValue(m_dwCefBkColor));
      glClearColor(float(CefColorGetR(background_color)) / 255.0f,
                   float(CefColorGetG(background_color)) / 255.0f,
                   float(CefColorGetB(background_color)) / 255.0f, 1.0f);
      VERIFY_NO_ERROR;
    }

    // Necessary for non-power-of-2 textures to render correctly.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    VERIFY_NO_ERROR;

    // Create the texture.
    glGenTextures(1, &m_iTextureId);
    VERIFY_NO_ERROR;
    DCHECK_NE(m_iTextureId, 0U);
    VERIFY_NO_ERROR;

    glBindTexture(GL_TEXTURE_2D, m_iTextureId);
    VERIFY_NO_ERROR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    VERIFY_NO_ERROR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    VERIFY_NO_ERROR;
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    VERIFY_NO_ERROR;

    m_bOpenGLInit = true;
  }

  void CleanupOpenGL() {
    if (m_iTextureId != 0)
      glDeleteTextures(1, &m_iTextureId);
  }

  void OpenGLRender() {
    if (m_iViewWidth == 0 || m_iViewHeight == 0)
      return;

    DCHECK(m_bOpenGLInit);

    struct {
      float tu, tv;
      float x, y, z;
    } static vertices[] = {{0.0f, 1.0f, -1.0f, -1.0f, 0.0f},
                           {1.0f, 1.0f, 1.0f, -1.0f, 0.0f},
                           {1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
                           {0.0f, 0.0f, -1.0f, 1.0f, 0.0f}};

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    VERIFY_NO_ERROR;

    glMatrixMode(GL_MODELVIEW);
    VERIFY_NO_ERROR;
    glLoadIdentity();
    VERIFY_NO_ERROR;

    // Match GL units to screen coordinates.
    glViewport(0, 0, m_iViewWidth, m_iViewHeight);
    VERIFY_NO_ERROR;
    glMatrixMode(GL_PROJECTION);
    VERIFY_NO_ERROR;
    glLoadIdentity();
    VERIFY_NO_ERROR;

    // Draw the background gradient.
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    VERIFY_NO_ERROR;
    // Don't check for errors until glEnd().
    glBegin(GL_QUADS);
    glColor4f(1.0, 0.0, 0.0, 1.0);  // red
    glVertex2f(-1.0, -1.0);
    glVertex2f(1.0, -1.0);
    glColor4f(0.0, 0.0, 1.0, 1.0);  // blue
    glVertex2f(1.0, 1.0);
    glVertex2f(-1.0, 1.0);
    glEnd();
    VERIFY_NO_ERROR;
    glPopAttrib();
    VERIFY_NO_ERROR;

    // Rotate the view based on the mouse spin.
#if 0
          if (spin_x_ != 0) {
            glRotatef(-spin_x_, 1.0f, 0.0f, 0.0f);
            VERIFY_NO_ERROR;
          }
          if (spin_y_ != 0) {
            glRotatef(-spin_y_, 0.0f, 1.0f, 0.0f);
            VERIFY_NO_ERROR;
          }

#endif

    if (m_bBkTransparent) {
      // Alpha blending style. Texture values have premultiplied alpha.
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      VERIFY_NO_ERROR;

      // Enable alpha blending.
      glEnable(GL_BLEND);
      VERIFY_NO_ERROR;
    }

    // Enable 2D textures.
    glEnable(GL_TEXTURE_2D);
    VERIFY_NO_ERROR;

    // Draw the facets with the texture.
    DCHECK_NE(m_iTextureId, 0U);
    VERIFY_NO_ERROR;
    glBindTexture(GL_TEXTURE_2D, m_iTextureId);
    VERIFY_NO_ERROR;
    glInterleavedArrays(GL_T2F_V3F, 0, vertices);
    VERIFY_NO_ERROR;
    glDrawArrays(GL_QUADS, 0, 4);
    VERIFY_NO_ERROR;

    // Disable 2D textures.
    glDisable(GL_TEXTURE_2D);
    VERIFY_NO_ERROR;

    if (m_bBkTransparent) {
      // Disable alpha blending.
      glDisable(GL_BLEND);
      VERIFY_NO_ERROR;
    }
  }

  void OnOpenGLPaint(CefRefPtr<CefBrowser> browser,
                     CefRenderHandler::PaintElementType type,
                     const CefRenderHandler::RectList& dirtyRects,
                     const void* buffer,
                     int width,
                     int height) {
    if (m_bBkTransparent) {
      // Enable alpha blending.
      glEnable(GL_BLEND);
      VERIFY_NO_ERROR;
    }

    // Enable 2D textures.
    glEnable(GL_TEXTURE_2D);
    VERIFY_NO_ERROR;

    DCHECK_NE(m_iTextureId, 0U);
    glBindTexture(GL_TEXTURE_2D, m_iTextureId);
    VERIFY_NO_ERROR;

    if (type == PET_VIEW) {
      int old_width = m_iViewWidth;
      int old_height = m_iViewHeight;

      m_iViewWidth = width;
      m_iViewHeight = height;

      glPixelStorei(GL_UNPACK_ROW_LENGTH, m_iViewWidth);
      VERIFY_NO_ERROR;

      if (old_width != m_iViewWidth || old_height != m_iViewHeight ||
          (dirtyRects.size() == 1 && dirtyRects[0] == CefRect(0, 0, m_iViewWidth, m_iViewHeight))) {
        // Update/resize the whole texture.
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        VERIFY_NO_ERROR;
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        VERIFY_NO_ERROR;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iViewWidth, m_iViewHeight, 0, GL_BGRA,
                     GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
        VERIFY_NO_ERROR;
      }
      else {
        // Update just the dirty rectangles.
        CefRenderHandler::RectList::const_iterator i = dirtyRects.begin();
        for (; i != dirtyRects.end(); ++i) {
          const CefRect& rect = *i;
          DCHECK(rect.x + rect.width <= m_iViewWidth);
          DCHECK(rect.y + rect.height <= m_iViewHeight);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x);
          VERIFY_NO_ERROR;
          glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y);
          VERIFY_NO_ERROR;
          glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height, GL_BGRA,
                          GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
          VERIFY_NO_ERROR;
        }
      }
    }
    else if (type == PET_POPUP && m_PopupRect.width > 0 && m_PopupRect.height > 0) {
      int skip_pixels = 0, x = m_PopupRect.x;
      int skip_rows = 0, y = m_PopupRect.y;
      int w = width;
      int h = height;

      // Adjust the popup to fit inside the view.
      if (x < 0) {
        skip_pixels = -x;
        x = 0;
      }
      if (y < 0) {
        skip_rows = -y;
        y = 0;
      }
      if (x + w > m_iViewWidth)
        w -= x + w - m_iViewWidth;
      if (y + h > m_iViewHeight)
        h -= y + h - m_iViewHeight;

      // Update the popup rectangle.
      glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
      VERIFY_NO_ERROR;
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, skip_pixels);
      VERIFY_NO_ERROR;
      glPixelStorei(GL_UNPACK_SKIP_ROWS, skip_rows);
      VERIFY_NO_ERROR;
      glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
      VERIFY_NO_ERROR;
    }

    // Disable 2D textures.
    glDisable(GL_TEXTURE_2D);
    VERIFY_NO_ERROR;

    if (m_bBkTransparent) {
      // Disable alpha blending.
      glDisable(GL_BLEND);
      VERIFY_NO_ERROR;
    }
  }
  
  void SetVisible(bool b) {
    //if (!CefCurrentlyOn(TID_UI)) {
    //  CefPostTask(TID_UI, base::Bind(&CCefUIImpl::SetVisible, this, b));
    //  return;
    //}

    if (!m_browser)
      return;

    if (b) {
      m_browser->GetHost()->WasHidden(false);
      m_browser->GetHost()->SendFocusEvent(true);
    }
    else {
      m_browser->GetHost()->SendFocusEvent(false);
      m_browser->GetHost()->WasHidden(true);
    }
  }
 public:
  CCefUI* m_pParent;

  // View
  HDC m_hViewMemoryDC;
  HBITMAP m_hViewBitmap;
  void* m_pViewBuffer;
  int m_iViewMemoryBitmapWidth;
  int m_iViewMemoryBitmapHeight;
  int m_iViewWidth;
  int m_iViewHeight;
  CriticalSection m_csPopupBuf;

  // Popup
  HDC m_hPopupMemoryDC;
  HBITMAP m_hPopupBitmap;
  void* m_pPopupBuffer;
  int m_iPopupMemoryBitmapWidth;
  int m_iPopupMemoryBitmapHeight;
  CefRect m_OriginPopupRect;
  CefRect m_PopupRect;
  CriticalSection m_csViewBuf;

  CDuiString m_strInitUrl;
  uint32_t m_iRandomID;

  std::vector<std::string> m_vAllowProtocols;
  int m_iFPS;
  bool m_bBkTransparent;
  DWORD m_dwCefBkColor;

  // Mouse state tracking.
  POINT last_mouse_pos_;
  POINT current_mouse_pos_;
  POINT mouse_pos_;
  bool mouse_rotation_;
  bool mouse_tracking_;
  int last_click_x_;
  int last_click_y_;
  CefBrowserHost::MouseButtonType last_click_button_;
  int last_click_count_;
  double last_click_time_;
  bool last_mouse_down_on_view_;

  CefRefPtr<Internal::ClientHandlerOsr> m_ClientHandler;
  CefRefPtr<CefBrowser> m_browser;
  Internal::CefDevToolsWnd* m_pDevToolsWnd;

  bool m_bUsingOSR;
  bool m_bUsingOpenGL;
  bool m_bOpenGLInit;
  bool m_bDragEnable;
  unsigned int m_iTextureId;
  HDC m_hDC;
  HGLRC m_hGLRC;
  HWND m_hOsrWnd;
  bool m_bPaintingPopup_;

  bool m_bClosing;

  HRGN m_draggableRegion;
};

IMPLEMENT_DUICONTROL(CCefUI)

CCefUI::CCefUI()
    : m_ResourceRspCB(nullptr)
    , m_RenderProcessTerminatedCB(nullptr)
    , m_JSCB(nullptr)
    , m_LoadErrorCB(nullptr)
    , m_hCreated(false) {
  m_pImpl = new CCefUIImpl(this);
}

CCefUI::~CCefUI() {
  if (m_pImpl) {
    delete m_pImpl;
    m_pImpl = nullptr;
  }
}

LPCTSTR CCefUI::GetClass() const {
  return DUI_CTR_CEF;
}

LPVOID CCefUI::GetInterface(LPCTSTR pstrName) {
  if (_tcsicmp(pstrName, DUI_CTR_CEF) == 0)
    return static_cast<CCefUI*>(this);

  return CContainerUI::GetInterface(pstrName);
}

void CCefUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
  if (_tcsicmp(pstrName, _T("url")) == 0)
    SetUrl(pstrValue);
  else if (_tcsicmp(pstrName, _T("transparent")) == 0)
    m_pImpl->SetBkTransparent(_tcsicmp(pstrValue, _T("true")) == 0);
  else if (_tcsicmp(pstrName, _T("opengl")) == 0)
    m_pImpl->SetUsingOpenGL(_tcsicmp(pstrValue, _T("true")) == 0);
  else if (_tcsicmp(pstrName, _T("osr")) == 0)
    m_pImpl->SetUsingOSR(_tcsicmp(pstrValue, _T("true")) == 0);
  else if (_tcsicmp(pstrName, _T("drag")) == 0)
    m_pImpl->SetDragEnable(_tcsicmp(pstrValue, _T("true")) == 0);
  else if (_tcsicmp(pstrName, _T("fps")) == 0)
    m_pImpl->SetFPS(_ttoi(pstrValue));
  else if (_tcsicmp(pstrName, _T("cefbkcolor")) == 0) {
    while (*pstrValue > _T('\0') && *pstrValue <= _T(' '))
      pstrValue = ::CharNext(pstrValue);

    if (*pstrValue == _T('#'))
      pstrValue = ::CharNext(pstrValue);

    LPTSTR pstr = NULL;
    DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
    m_pImpl->SetCefBkColor(clrColor);
  }
  else
    CControlUI::SetAttribute(pstrName, pstrValue);
}

void CCefUI::SetPos(RECT rc, bool bNeedInvalidate /* = true */) {
  CContainerUI::SetPos(rc, bNeedInvalidate);
  if (!m_hCreated) {
    m_hCreated = m_pImpl->CreateBrowser();
  }
  else {
    m_pImpl->OnSize();
  }
}

void CCefUI::DoInit() {
  if (!m_hCreated) {
    m_hCreated = m_pImpl->CreateBrowser();
  }
}

bool CCefUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl) {
  bool bRet = CContainerUI::DoPaint(hDC, rcPaint, pStopControl);
  m_pImpl->DoPaint(hDC);
  return bRet;
}

void CCefUI::DoEvent(TEventUI& event) {
  if (event.Type == UIEVENT_MOUSEMOVE || event.Type == UIEVENT_MOUSELEAVE ||
      event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_BUTTONUP ||
      event.Type == UIEVENT_RBUTTONDOWN || event.Type == UIEVENT_RBUTTONUP ||
      event.Type == UIEVENT_MBUTTONDOWN || event.Type == UIEVENT_MBUTTONUP ||
      event.Type == UIEVENT_SCROLLWHEEL) {
    int msg;
    switch (event.Type) {
      case UIEVENT_MOUSEMOVE:
        msg = WM_MOUSEMOVE;
        break;
      case UIEVENT_MOUSELEAVE:
        msg = WM_MOUSELEAVE;
        break;
      case UIEVENT_BUTTONDOWN:
        msg = WM_LBUTTONDOWN;
        break;
      case UIEVENT_BUTTONUP:
        msg = WM_LBUTTONUP;
        break;
      case UIEVENT_RBUTTONDOWN:
        msg = WM_RBUTTONDOWN;
        break;
      case UIEVENT_RBUTTONUP:
        msg = WM_RBUTTONUP;
        break;
      case UIEVENT_MBUTTONDOWN:
        msg = WM_MBUTTONDOWN;
        break;
      case UIEVENT_MBUTTONUP:
        msg = WM_MBUTTONUP;
        break;
      case UIEVENT_SCROLLWHEEL:
        msg = WM_MOUSEWHEEL;
        break;
      default:
        assert(0);
        break;
    }

    m_pImpl->OnMouseEvent(msg, event.wParam, event.lParam);
  }
  else if (event.Type == UIEVENT_KEYDOWN || event.Type == UIEVENT_KEYUP ||
           event.Type == UIEVENT_CHAR || event.Type == UIEVENT_SYSCHAR ||
           event.Type == UIEVENT_SYSKEYDOWN || event.Type == UIEVENT_SYSKEYUP) {
    int msg;
    switch (event.Type) {
      case UIEVENT_KEYDOWN:
        msg = WM_KEYDOWN;
        break;
      case UIEVENT_KEYUP:
        msg = WM_KEYUP;
        break;
      case UIEVENT_CHAR:
        msg = WM_CHAR;
        break;
      case UIEVENT_SYSCHAR:
        msg = WM_SYSCHAR;
        break;
      case UIEVENT_SYSKEYDOWN:
        msg = WM_SYSKEYDOWN;
        break;
      case UIEVENT_SYSKEYUP:
        msg = WM_SYSKEYUP;
        break;
      default:
        assert(0);
        break;
    }
    m_pImpl->OnKeyEvent(msg, event.wParam, event.lParam);
  }
  else if (event.Type == UIEVENT_SETFOCUS || event.Type == UIEVENT_KILLFOCUS) {
    m_pImpl->OnFocus(event.Type == UIEVENT_SETFOCUS);
  }
  CContainerUI::DoEvent(event);
}

void CCefUI::SetVisible(bool bVisible /*= true*/) {
  m_pImpl->SetVisible(bVisible);
  CControlUI::SetVisible(bVisible);
}

void CCefUI::SetInternVisible(bool bVisible /*= true*/) {
  m_pImpl->SetVisible(bVisible);
  CControlUI::SetInternVisible(bVisible);
}

bool CCefUI::GetBkTransparent() const {
  return m_pImpl->GetBkTransparent();
}

void CCefUI::SetResourceResponseCallback(ResourceResponseCallback cb) {
  m_ResourceRspCB = cb;
}

CCefUI::ResourceResponseCallback CCefUI::GetResourceResponseCallback() const {
  return m_ResourceRspCB;
}

void CCefUI::SetRenderProcessTerminatedCallback(RenderProcessTerminatedCallback cb) {
  m_RenderProcessTerminatedCB = cb;
}

CCefUI::RenderProcessTerminatedCallback CCefUI::GetRenderProcessTerminatedCallback() const {
  return m_RenderProcessTerminatedCB;
}

void CCefUI::SetJSCallback(JSCallback cb) {
  m_JSCB = cb;
}

CCefUI::JSCallback CCefUI::GetJSCallback() const {
  return m_JSCB;
}

void CCefUI::SetLoadErrorCallback(LoadErrorCallback cb) {
  m_LoadErrorCB = cb;
}

CCefUI::LoadErrorCallback CCefUI::GetLoadErrorCallback() const {
  return m_LoadErrorCB;
}

void CCefUI::SetUrl(const CDuiString& url) {
  m_strUrl = url;
  m_pImpl->SetUrl(url);
}

DuiLib::CDuiString CCefUI::GetUrl() const {
  return m_strUrl;
}

int CCefUI::GetFPS() const {
  return m_pImpl->GetFPS();
}

void CCefUI::SetFPS(int fps) {
  m_pImpl->SetFPS(fps);
}

DWORD CCefUI::GetCefBkColor() const {
  return m_pImpl->GetCefBkColor();
}

void CCefUI::SetUsingOpenGL(bool b) {
  m_pImpl->SetUsingOpenGL(b);
}

bool CCefUI::IsUsingOpenGL() const {
  return m_pImpl->IsUsingOpenGL();
}

void CCefUI::SetUsingOSR(bool b) {
  m_pImpl->SetUsingOSR(b);
}

bool CCefUI::IsUsingOSR() const {
  return m_pImpl->IsUsingOSR();
}

void CCefUI::GoBack() {
  m_pImpl->GoBack();
}

void CCefUI::GoForward() {
  m_pImpl->GoForward();
}

void CCefUI::Reload(bool bIgnoreCache) {
  m_pImpl->Reload(bIgnoreCache);
}

bool CCefUI::IsLoading() {
  return m_pImpl->IsLoading();
}

void CCefUI::StopLoad() {
  m_pImpl->StopLoad();
}

void CCefUI::ShowDevTools() {
  m_pImpl->ShowDevTools();
}

void CCefUI::CloseDevTools() {
  m_pImpl->CloseDevTools();
}

bool CCefUI::CallJavascriptFunction(const std::string& strFuncName,
                                    const std::vector<CLiteVariant>& args) {
  return m_pImpl->CallJavascriptFunction(strFuncName, args);
}

void CCefUI::SetAllowProtocols(const std::vector<std::string> vAllowProtocols) {
  m_pImpl->SetAllowProtocols(vAllowProtocols);
}

std::vector<std::string> CCefUI::GetAllowProtocols() const {
  return m_pImpl->GetAllowProtocols();
}
}  // namespace DuiLib
#endif