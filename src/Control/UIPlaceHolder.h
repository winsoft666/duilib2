#pragma once

namespace DuiLib {
    class CPlaceHolderWnd : public CWindowWnd {
        friend class CPlaceHolderUI;
      public:
        CPlaceHolderWnd(CPaintManagerUI *pParentPM);
        ~CPlaceHolderWnd();

        void SetBkColor(DWORD dwBackColor);

        void Init(CPlaceHolderUI *pOwner);
        LPCTSTR GetWindowClassName() const;
        void OnFinalMessage(HWND hWnd);

        LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
        LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

      protected:
        CPlaceHolderUI *m_pOwner;
        CPaintManagerUI *m_pParentPM;
        bool		  m_bInit;
        DWORD m_dwBkColor;
        bool m_bBkColorSet;
    };

    class CPlaceHolderUI : public CControlUI {
        friend class CPlaceHolderWnd;
        DECLARE_DUICONTROL(CPlaceHolderUI)
      public:
        class Delegate {
          public:
            virtual void OnPlaceHolderSizeChanged(const CDuiString &strName) = 0;
        };
        CPlaceHolderUI(void);
        ~CPlaceHolderUI(void);

        LPCTSTR	GetClass() const override;
        LPVOID	GetInterface(LPCTSTR pstrName) override;

        void RegisterDelegate(Delegate *delegate);

        void DoInit() override;
        void DoEvent(TEventUI &event) override;
        void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
        void SetPos(RECT rc, bool bNeedInvalidate = true);
        void SetBkColor(DWORD dwBackColor);
        HWND GetHWND();
      private:
        void OnSize(WPARAM wParam, LPARAM lParam);
      protected:
        CPlaceHolderWnd *m_pWindow;
        DWORD m_dwBkColor;
        Delegate *m_pDelegate;
    };


}