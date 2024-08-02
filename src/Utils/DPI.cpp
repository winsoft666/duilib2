#include "StdAfx.h"
#include "DPI.h"
#include "VersionHelpers.h"


namespace DuiLib {
    //96 DPI = 100% scaling
    //120 DPI = 125% scaling
    //144 DPI = 150% scaling
    //168 DPI = 175% scaling
    //192 DPI = 200% scaling

    typedef HRESULT (WINAPI *LPSetProcessDpiAwareness)(
        _In_ PROCESS_DPI_AWARENESS value
    );

    typedef HRESULT (WINAPI *LPGetProcessDpiAwareness)(
        _In_  HANDLE                hprocess,
        _Out_ PROCESS_DPI_AWARENESS *value
    );


    typedef HRESULT (WINAPI *LPGetDpiForMonitor)(
        _In_  HMONITOR         hmonitor,
        _In_  MONITOR_DPI_TYPE dpiType,
        _Out_ UINT             *dpiX,
        _Out_ UINT             *dpiY
    );


    CDPI::CDPI() {
        m_nScaleFactor = 100;
        
        PROCESS_DPI_AWARENESS dpiAwareness;
        if (GetDPIAwareness(&dpiAwareness)) {
            if (dpiAwareness == PROCESS_DPI_UNAWARE) {
                SetDPIAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
            }
            else if (dpiAwareness == PROCESS_SYSTEM_DPI_AWARE) {
                m_nScaleFactor = MulDiv(GetMainMonitorDPI(), 100, 96);
            }
        }
    }

    int CDPI::GetDPIOfMonitor(HMONITOR hMonitor) {
        UINT dpix = 96, dpiy = 96;
        HRESULT  hr = E_FAIL;
        HMODULE hModule = ::LoadLibrary(_T("Shcore.dll"));
        if (hModule != NULL) {
            LPGetDpiForMonitor GetDpiForMonitor = (LPGetDpiForMonitor)GetProcAddress(hModule, "GetDpiForMonitor");

            if (GetDpiForMonitor != NULL && GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy) != S_OK) {
                return 96;
            }
        }
        else {
            HDC screen = GetDC(0);
            dpix = GetDeviceCaps(screen, LOGPIXELSX);
            ReleaseDC(0, screen);
        }

        return dpix;
    }

    int CDPI::GetDPIOfMonitorNearestToPoint(POINT pt) {
        HMONITOR hMonitor;
        hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        return GetDPIOfMonitor(hMonitor);
    }

    int CDPI::GetMainMonitorDPI() {
        POINT    pt;
        // Get the DPI for the main monitor
        pt.x = 1;
        pt.y = 1;
        return GetDPIOfMonitorNearestToPoint(pt);
    }

    BOOL CDPI::GetDPIAwareness(PROCESS_DPI_AWARENESS* pAwareness) {
        BOOL bRet = FALSE;
        HMODULE hModule = ::LoadLibrary(_T("Shcore.dll"));

        if (hModule != NULL) {
            LPGetProcessDpiAwareness GetProcessDpiAwareness = (LPGetProcessDpiAwareness)GetProcAddress(hModule, "GetProcessDpiAwareness");

            if (GetProcessDpiAwareness != NULL) {
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());

                if (GetProcessDpiAwareness(hProcess, pAwareness) == S_OK) {
                    bRet = TRUE;
                }
            }
        }

        return bRet;
    }

    BOOL CDPI::SetDPIAwareness(PROCESS_DPI_AWARENESS Awareness) {
        BOOL bRet = FALSE;
        HMODULE hModule = ::LoadLibrary(_T("Shcore.dll"));

        if (hModule != NULL) {
            LPSetProcessDpiAwareness SetProcessDpiAwareness = (LPSetProcessDpiAwareness)GetProcAddress(hModule, "SetProcessDpiAwareness");

            if (SetProcessDpiAwareness != NULL) {
                HRESULT hr = SetProcessDpiAwareness(Awareness);
                if (hr == S_OK) {
                    bRet = TRUE;
                }
            }
        }

        return bRet;
    }

    UINT DuiLib::CDPI::GetDPI() {
        return MulDiv(m_nScaleFactor, 96, 100);
    }

    UINT CDPI::GetScale() {
        return m_nScaleFactor;
    }

    void CDPI::SetScale(UINT uDPI) {
        m_nScaleFactor = MulDiv(uDPI, 100, 96);
    }

    int  CDPI::Scale(int iValue) {
        return MulDiv(iValue, m_nScaleFactor, 100);
    }

    int  CDPI::ScaleBack(int iValue) {
        return MulDiv(iValue, 100, m_nScaleFactor);
    }

    bool CDPI::StripDPIInfo(const CDuiString& name, CDuiString& newName) {
        int lastPotPos = name.ReverseFind(TEXT('.'));

        if (lastPotPos == -1)
            return false;

        int lastSplitPos = name.ReverseFind(TEXT('@'));

        if (lastSplitPos == -1 || lastSplitPos > lastPotPos)
            return false;

        CDuiString str = name.Mid(lastSplitPos + 1, lastPotPos - lastSplitPos - 1);
        int len = str.GetLength();
        bool isAllDigit = true;

        for (int i = 0; i < len; i++) {
#ifdef UNICODE

            if (iswdigit(str[i]) == 0) {
#else

            if (isdigit(str[i]) == 0) {
#endif
                isAllDigit = false;
                break;
            }
        }

        if (!isAllDigit)
            return false;

        newName = name.Left(lastSplitPos) + name.Right(name.GetLength() - lastPotPos);

        return true;
    }


    bool CDPI::AddDPIInfo(const CDuiString& name, int dpi, CDuiString& newName) {
        int lastPotPos = name.ReverseFind(TEXT('.'));

        if (lastPotPos == -1)
            return false;

        CDuiString sScale;
        sScale.Format(_T("@%d"), dpi);

        newName = name.Left(lastPotPos) + sScale + name.Right(name.GetLength() - lastPotPos);

        return true;
    }


    RECT CDPI::Scale(RECT rcRect) {
        RECT rcScale = rcRect;
        int sw = Scale(rcRect.right - rcRect.left);
        int sh = Scale(rcRect.bottom - rcRect.top);
        rcScale.left = Scale(rcRect.left);
        rcScale.top = Scale(rcRect.top);
        rcScale.right = rcScale.left + sw;
        rcScale.bottom = rcScale.top + sh;
        return rcScale;
    }

    void CDPI::Scale(RECT *pRect) {
        int sw = Scale(pRect->right - pRect->left);
        int sh = Scale(pRect->bottom - pRect->top);
        pRect->left = Scale(pRect->left);
        pRect->top = Scale(pRect->top);
        pRect->right = pRect->left + sw;
        pRect->bottom = pRect->top + sh;
    }

    void CDPI::ScaleBack(RECT *pRect) {
        int sw = ScaleBack(pRect->right - pRect->left);
        int sh = ScaleBack(pRect->bottom - pRect->top);
        pRect->left = ScaleBack(pRect->left);
        pRect->top = ScaleBack(pRect->top);
        pRect->right = pRect->left + sw;
        pRect->bottom = pRect->top + sh;
    }

    void CDPI::Scale(POINT *pPoint) {
        pPoint->x = Scale(pPoint->x);
        pPoint->y = Scale(pPoint->y);
    }

    POINT CDPI::Scale(POINT ptPoint) {
        POINT ptScale = ptPoint;
        ptScale.x = Scale(ptPoint.x);
        ptScale.y = Scale(ptPoint.y);
        return ptScale;
    }

    void CDPI::Scale(SIZE *pSize) {
        pSize->cx = Scale(pSize->cx);
        pSize->cy = Scale(pSize->cy);
    }

    SIZE CDPI::Scale(SIZE szSize) {
        SIZE szScale = szSize;
        szScale.cx = Scale(szSize.cx);
        szScale.cy = Scale(szSize.cy);
        return szScale;
    }
}
