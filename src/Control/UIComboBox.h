#ifndef __UICOMBOBOX_H__
#define __UICOMBOBOX_H__

#pragma once

namespace DuiLib {
    /////////////////////////////////////////////////////////////////////////////////////
    //

    class CComboBoxWnd;

    class UILIB_API CComboBoxUI : public CContainerUI, public IListOwnerUI {
        DECLARE_DUICONTROL(CComboBoxUI)
        friend class CComboBoxWnd;
      public:
        CComboBoxUI();

        LPCTSTR GetClass() const;
        LPVOID GetInterface(LPCTSTR pstrName);

        void DoInit();
        UINT GetControlFlags() const;

        CDuiString GetText() const;
        void SetEnabled(bool bEnable = true);

        void SetTextStyle(UINT uStyle);
        UINT GetTextStyle() const;
        void SetTextColor(DWORD dwTextColor);
        DWORD GetTextColor() const;
        void SetDisabledTextColor(DWORD dwTextColor);
        DWORD GetDisabledTextColor() const;
        void SetFont(int index);
        int GetFont() const;
        RECT GetTextPadding() const;
        void SetTextPadding(RECT rc);
        bool IsShowHtml();
        void SetShowHtml(bool bShowHtml = true);
        bool IsShowShadow();
        void SetShowShadow(bool bShow = true);

        CDuiString GetDropBoxAttributeList();
        void SetDropBoxAttributeList(LPCTSTR pstrList);
        SIZE GetDropBoxSize() const;
        void SetDropBoxSize(SIZE szDropBox);

        ListType GetListType();
        TListInfoUI *GetListInfo();
        int GetCurSel() const;
        bool SelectItem(int iIndex, bool bTakeFocus = false);
        bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
        bool UnSelectItem(int iIndex, bool bOthers = false);
        bool SetItemIndex(CControlUI *pControl, int iIndex);

        int GetCount() const;
        bool Add(CControlUI *pControl);
        bool AddAt(CControlUI *pControl, int iIndex);
        bool Remove(CControlUI *pControl);
        bool RemoveAt(int iIndex);
        void RemoveAll();

        bool Activate();

        LPCTSTR GetNormalImage() const;
        void SetNormalImage(LPCTSTR pStrImage);
        LPCTSTR GetHotImage() const;
        void SetHotImage(LPCTSTR pStrImage);
        LPCTSTR GetPushedImage() const;
        void SetPushedImage(LPCTSTR pStrImage);
        LPCTSTR GetFocusedImage() const;
        void SetFocusedImage(LPCTSTR pStrImage);
        LPCTSTR GetDisabledImage() const;
        void SetDisabledImage(LPCTSTR pStrImage);

        LPCTSTR GetArrowNormalImage() const;
        void SetArrowNormalImage(LPCTSTR pStrImage);
        LPCTSTR GetArrowHotImage() const;
        void SetArrowHotImage(LPCTSTR pStrImage);
        LPCTSTR GetArrowPushedImage() const;
        void SetArrowPushedImage(LPCTSTR pStrImage);
        LPCTSTR GetArrowFocusedImage() const;
        void SetArrowFocusedImage(LPCTSTR pStrImage);
        LPCTSTR GetArrowDisabledImage() const;
        void SetArrowDisabledImage(LPCTSTR pStrImage);

        SIZE EstimateSize(SIZE szAvailable);
        void SetPos(RECT rc, bool bNeedInvalidate = true);
        void Move(SIZE szOffset, bool bNeedInvalidate = true);
        void DoEvent(TEventUI &event);
        void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

        bool DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl);
        void PaintText(HDC hDC);
        void PaintStatusImage(HDC hDC);

      protected:
        CComboBoxWnd *m_pWindow;

        DWORD	m_dwTextColor;
        DWORD	m_dwDisabledTextColor;
        int		m_iFont;
        UINT	m_uTextStyle;
        RECT	m_rcTextPadding;
        bool	m_bShowHtml;
        bool	m_bShowShadow;
        UINT    m_uButtonState;

        CDuiString m_sNormalImage;
        CDuiString m_sHotImage;
        CDuiString m_sPushedImage;
        CDuiString m_sFocusedImage;
        CDuiString m_sDisabledImage;


        SIZE m_szDropBox;
        CDuiString m_sDropBoxAttributes;

        CDuiString m_sArrowNormalImage;
        CDuiString m_sArrowHotImage;
        CDuiString m_sArrowPushedImage;
        CDuiString m_sArrowFocusedImage;
        CDuiString m_sArrowDisabledImage;
    };

} // namespace DuiLib

#endif // __UICOMBO_H__
