#include "StdAfx.h"
#include "UIGroupBox.h"

namespace DuiLib {
    IMPLEMENT_DUICONTROL(CGroupBoxUI)

    //////////////////////////////////////////////////////////////////////////
    //
    CGroupBoxUI::CGroupBoxUI(): 
        m_uTextStyle(DT_SINGLELINE | DT_VCENTER | DT_CENTER), 
        m_dwTextColor(0),
        m_dwDisabledTextColor(0), 
        m_iFont(-1) {
        ::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
        SetInset(CDuiRect(20, 25, 20, 20));
    }

    CGroupBoxUI::~CGroupBoxUI() {
    }

    LPCTSTR CGroupBoxUI::GetClass() const {
        return DUI_CTR_GROUPBOX;
    }

    LPVOID CGroupBoxUI::GetInterface(LPCTSTR pstrName) {
        if( _tcsicmp(pstrName, DUI_CTR_GROUPBOX) == 0 ) 
            return static_cast<CGroupBoxUI *>(this);

        return CVerticalLayoutUI::GetInterface(pstrName);
    }

    void CGroupBoxUI::SetTextColor(DWORD dwTextColor) {
        m_dwTextColor = dwTextColor;
        Invalidate();
    }

    DWORD CGroupBoxUI::GetTextColor() const {
        return m_dwTextColor;
    }

    void CGroupBoxUI::SetDisabledTextColor(DWORD dwTextColor) {
        m_dwDisabledTextColor = dwTextColor;
        Invalidate();
    }

    DWORD CGroupBoxUI::GetDisabledTextColor() const {
        return m_dwDisabledTextColor;
    }
    void CGroupBoxUI::SetFont(int index) {
        m_iFont = index;
        Invalidate();
    }

    int CGroupBoxUI::GetFont() const {
        return m_iFont;
    }

    RECT CGroupBoxUI::GetTextPadding() const {
        return m_rcTextPadding;
    }

    void CGroupBoxUI::SetTextPadding(RECT rc) {
        m_rcTextPadding = rc;
    }

    void CGroupBoxUI::PaintText(HDC hDC) {
        CDuiString sText = GetText();

        if (sText.IsEmpty()) {
            return;
        }

        if (m_dwTextColor == 0)
            m_dwTextColor = m_pManager->GetDefaultFontColor();

        if (m_dwDisabledTextColor == 0)
            m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

        CDuiRect rcItem = m_rcItem;
        rcItem.Deflate(5, 5);

        RECT rcText = CalcTextDrawRect(rcItem);

        DWORD dwTextColor = m_dwTextColor;

        if (!IsEnabled())
            dwTextColor = m_dwDisabledTextColor;

        CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, dwTextColor, m_iFont, m_uTextStyle);
    }

    void CGroupBoxUI::PaintBorder(HDC hDC) {
		SIZE cxyBorderRound = GetBorderRound();
		CDuiRect rcBorderSize = GetBorderSize();
		int nBorderSize = rcBorderSize.left;

        if(nBorderSize > 0 ) {
            CDuiRect rcItem = m_rcItem;
            rcItem.Deflate(5, 5);

            RECT rcText = CalcTextDrawRect(rcItem);

            // Jeffery: 上边框位于文字中间位置
            //
            rcItem.top += ((rcText.bottom - rcText.top) / 2);

            HBRUSH hBrush = NULL;
            HRGN rgn1 = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom, cxyBorderRound.cx, cxyBorderRound.cy);
            HRGN rgn2 = ::CreateRoundRectRgn(rcItem.left + nBorderSize, rcItem.top + nBorderSize, rcItem.right - nBorderSize, rcItem.bottom - nBorderSize, cxyBorderRound.cx, cxyBorderRound.cy);
            ::CombineRgn(rgn1, rgn1, rgn2, RGN_XOR);
            HRGN rgn3 = ::CreateRectRgnIndirect(&rcText);
            ::CombineRgn(rgn2, rgn1, rgn3, RGN_AND);
            ::CombineRgn(rgn1, rgn1, rgn2, RGN_XOR);

            if (IsFocused() && m_dwFocusBorderColor != 0) {
                hBrush = CreateSolidBrush(RGB(GetBValue(m_dwFocusBorderColor), GetGValue(m_dwFocusBorderColor), GetRValue(m_dwFocusBorderColor)));
                ::FillRgn(hDC, rgn1, hBrush);
            }
            else {
                hBrush = CreateSolidBrush(RGB(GetBValue(m_dwBorderColor), GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor)));
                ::FillRgn(hDC, rgn1, hBrush);
            }

            ::DeleteObject(rgn1);
            ::DeleteObject(rgn2);
            ::DeleteObject(rgn3);
            ::DeleteObject(hBrush);
        }
    }

    SIZE CGroupBoxUI::CalcTextSize(SIZE szAvailable) {
        SIZE cxyFixed = GetFixedXY();
        int iTextHeight = 20;
        if (m_pManager) {
            HFONT font = m_pManager->GetFont(m_iFont);
            if (font) {
                LOGFONT lf;
                if (GetObject(font, sizeof(lf), &lf) != 0)
                    iTextHeight = -lf.lfHeight;
            }
        }
        RECT rcText = { 0, 0, MAX(szAvailable.cx, cxyFixed.cx), iTextHeight };

		CDuiString strText = GetText();

        CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, strText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
        SIZE cXY = {rcText.right - rcText.left, rcText.bottom - rcText.top};
        return cXY;
    }

    RECT CGroupBoxUI::CalcTextDrawRect(RECT rcAvaliable) {
        SIZE szAvailable = { rcAvaliable.right - rcAvaliable.left, rcAvaliable.bottom - rcAvaliable.top };
        SIZE sz = CalcTextSize(szAvailable);

        RECT rcText = rcAvaliable;
        rcText.left = rcText.left + 15 + m_rcTextPadding.left;
        rcText.top = rcText.top - m_rcTextPadding.top;
        rcText.right = rcText.left + sz.cx + m_rcTextPadding.right;
        rcText.bottom = rcText.top + sz.cy + m_rcTextPadding.bottom;

        return rcText;
    }

    void CGroupBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
        if( _tcsicmp(pstrName, _T("textcolor")) == 0 ) {
            if( *pstrValue == _T('#'))
                pstrValue = ::CharNext(pstrValue);

            LPTSTR pstr = NULL;
            DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
            SetTextColor(clrColor);
        } 
        else if( _tcsicmp(pstrName, _T("disabledtextcolor")) == 0 ) {
            if( *pstrValue == _T('#')) 
                pstrValue = ::CharNext(pstrValue);

            LPTSTR pstr = NULL;
            DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
            SetDisabledTextColor(clrColor);
        } 
        else if( _tcsicmp(pstrName, _T("font")) == 0 ) {
            SetFont(_ttoi(pstrValue));
        }
        else if (_tcsicmp(pstrName, _T("textpadding")) == 0) {
            RECT rcTextPadding = { 0 };
            LPTSTR pstr = NULL;
            rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);
            ASSERT(pstr);
            rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);
            ASSERT(pstr);
            rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);
            ASSERT(pstr);
            rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10);
            ASSERT(pstr);
            SetTextPadding(rcTextPadding);
        }

        CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
    }
}
