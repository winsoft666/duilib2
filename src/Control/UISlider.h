#ifndef __UISLIDER_H__
#define __UISLIDER_H__

#pragma once

namespace DuiLib {
    class UILIB_API CSliderUI : public CProgressUI {
        DECLARE_DUICONTROL(CSliderUI)
      public:
        CSliderUI();

        LPCTSTR GetClass() const;
        UINT GetControlFlags() const;
        LPVOID GetInterface(LPCTSTR pstrName);

        void SetEnabled(bool bEnable = true);

        int GetChangeStep();
        void SetChangeStep(int step);
        void SetThumbSize(SIZE szXY);

        RECT CalcThumbRect() const;
        LPCTSTR GetThumbImage() const;

        void SetThumbImage(LPCTSTR pStrImage);
        LPCTSTR GetThumbHotImage() const;
        void SetThumbHotImage(LPCTSTR pStrImage);

        LPCTSTR GetThumbPushedImage() const;
        void SetThumbPushedImage(LPCTSTR pStrImage);

        LPCTSTR GetThumbFocusedImage() const;
        void SetThumbFocusedImage(LPCTSTR pStrImage);

        void SetBkColorWidth(int v);
        int GetBkColorWidth() const;

        void SetBkColorHeight(int v);
        int GetBkColorHeight() const;

        void SetThumbColor(DWORD dwColor);
        DWORD GetThumbColor() const;

        void SetHotThumbColor(DWORD dwColor);
        DWORD GetHotThumbColor() const;

        void SetDisabledThumbColor(DWORD dwColor);
        DWORD GetDisabledThumbColor() const;

        void SetFocusedThumbColor(DWORD dwColor);
        DWORD GetFocusedThumbColor() const;

        void DoEvent(TEventUI &event);
        void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
        void PaintForeColor(HDC hDC);
        void PaintForeImage(HDC hDC);
        void PaintBkColor(HDC hDC);
        void SetValue(int nValue);
        void SetCanSendMove(bool bCanSend);
        bool GetCanSendMove() const;
      protected:
        SIZE m_szThumb;
        UINT m_uButtonState;
        int m_nStep;
        int m_nBkColorWidth;
        int m_nBkColorHeight;

        DWORD m_dwThumbColor;
        DWORD m_dwThumbHotColor;
        DWORD m_dwThumbPushedColor;
        DWORD m_dwThumbFocusedColor;
        DWORD m_dwThumbDisabledColor;

        CDuiString m_sThumbImage;
        CDuiString m_sThumbHotImage;
        CDuiString m_sThumbPushedImage;
        CDuiString m_sThumbFocusedImage;

        CDuiString m_sImageModify;
        bool	   m_bSendMove;
    };
}

#endif // __UISLIDER_H__