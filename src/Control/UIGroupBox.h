#ifndef __UIGROUPBOX_H__
#define __UIGROUPBOX_H__

#pragma once

namespace DuiLib {

    class UILIB_API CGroupBoxUI : public CVerticalLayoutUI {
        DECLARE_DUICONTROL(CGroupBoxUI)
      public:
        CGroupBoxUI();
        ~CGroupBoxUI();
        LPCTSTR GetClass() const;
        LPVOID GetInterface(LPCTSTR pstrName);
        void SetTextColor(DWORD dwTextColor);
        DWORD GetTextColor() const;
        void SetDisabledTextColor(DWORD dwTextColor);
        DWORD GetDisabledTextColor() const;
        void SetFont(int index);
        int GetFont() const;

        RECT GetTextPadding() const;
        void SetTextPadding(RECT rc);
      protected:
        //Paint
        virtual void PaintText(HDC hDC) override;
        virtual void PaintBorder(HDC hDC);
        virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
      private:
        SIZE CalcTextSize(SIZE szAvailable);
        RECT CalcTextDrawRect(RECT rcAvaliable);
      protected:
        DWORD m_dwTextColor;
        DWORD m_dwDisabledTextColor;
        int m_iFont;
        UINT m_uTextStyle;
        RECT m_rcTextPadding;
    };
}
#endif // __UIGROUPBOX_H__