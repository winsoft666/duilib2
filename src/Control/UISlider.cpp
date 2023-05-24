#include "StdAfx.h"
#include "UISlider.h"

namespace DuiLib {
    IMPLEMENT_DUICONTROL(CSliderUI)
    CSliderUI::CSliderUI() :
		m_uButtonState(0), 
		m_nStep(1),
		m_nBkColorWidth(-1), 
		m_nBkColorHeight(-1), 
		m_dwThumbColor(0),
		m_dwThumbHotColor(0),
		m_dwThumbPushedColor(0),
		m_dwThumbFocusedColor(0),
		m_dwThumbDisabledColor(0),
		m_bSendMove(false) {
        m_uTextStyle = DT_SINGLELINE | DT_CENTER;
        m_szThumb.cx = m_szThumb.cy = 10;
    }

    LPCTSTR CSliderUI::GetClass() const {
        return DUI_CTR_SLIDER;
    }

    UINT CSliderUI::GetControlFlags() const {
        if( IsEnabled() ) 
            return UIFLAG_SETCURSOR;
        else 
            return 0;
    }

    LPVOID CSliderUI::GetInterface(LPCTSTR pstrName) {
        if( _tcsicmp(pstrName, DUI_CTR_SLIDER) == 0 ) 
            return static_cast<CSliderUI *>(this);

        return CProgressUI::GetInterface(pstrName);
    }

    void CSliderUI::SetEnabled(bool bEnable) {
        CControlUI::SetEnabled(bEnable);

        if( !IsEnabled() ) {
            m_uButtonState = 0;
        }
    }

    int CSliderUI::GetChangeStep() {
        return m_nStep;
    }

    void CSliderUI::SetChangeStep(int step) {
        m_nStep = step;
    }

    void CSliderUI::SetThumbSize(SIZE szXY) {
        m_szThumb = szXY;
    }

    RECT CSliderUI::CalcThumbRect() const {
        RECT rcThumb = {0};
        SIZE szThumb = m_szThumb;

        if (m_pManager) {
            m_pManager->GetDPIObj()->Scale(&szThumb);
        }

        if( m_bHorizontal ) {
            int left = m_rcItem.left + (m_rcItem.right - m_rcItem.left - szThumb.cx) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
            int top = (m_rcItem.bottom + m_rcItem.top - szThumb.cy) / 2;
            rcThumb = CDuiRect(left, top, left + szThumb.cx, top + szThumb.cy);
        } else {
            int left = (m_rcItem.right + m_rcItem.left - szThumb.cx) / 2;
            int top = m_rcItem.bottom - szThumb.cy - (m_rcItem.bottom - m_rcItem.top - szThumb.cy) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
            rcThumb = CDuiRect(left, top, left + szThumb.cx, top + szThumb.cy);
        }

        return rcThumb;
    }

    LPCTSTR CSliderUI::GetThumbImage() const {
        return m_sThumbImage;
    }

    void CSliderUI::SetThumbImage(LPCTSTR pStrImage) {
        m_sThumbImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CSliderUI::GetThumbHotImage() const {
        return m_sThumbHotImage;
    }

    void CSliderUI::SetThumbHotImage(LPCTSTR pStrImage) {
        m_sThumbHotImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CSliderUI::GetThumbPushedImage() const {
        return m_sThumbPushedImage;
    }

    void CSliderUI::SetThumbPushedImage(LPCTSTR pStrImage) {
        m_sThumbPushedImage = pStrImage;
        Invalidate();
    }

	LPCTSTR CSliderUI::GetThumbFocusedImage() const {
		return m_sThumbFocusedImage;
	}

	void CSliderUI::SetThumbFocusedImage(LPCTSTR pStrImage) {
		m_sThumbFocusedImage = pStrImage;
		Invalidate();
	}

	void CSliderUI::SetBkColorWidth(int v) {
		m_nBkColorWidth = v;
		Invalidate();
	}

	int CSliderUI::GetBkColorWidth() const {
		return m_nBkColorWidth;
	}

	void CSliderUI::SetBkColorHeight(int v) {
		m_nBkColorHeight = v;
		Invalidate();
	}

	int CSliderUI::GetBkColorHeight() const {
		return m_nBkColorHeight;
	}

	void CSliderUI::SetThumbColor(DWORD dwColor) {
		m_dwThumbColor = dwColor;
		Invalidate();
	}

	DWORD CSliderUI::GetThumbColor() const {
		return m_dwThumbColor;
	}

	void CSliderUI::SetHotThumbColor(DWORD dwColor) {
		m_dwThumbHotColor = dwColor;
		Invalidate();
	}

	DWORD CSliderUI::GetHotThumbColor() const {
		return m_dwThumbHotColor;
	}

	void CSliderUI::SetDisabledThumbColor(DWORD dwColor) {
		m_dwThumbDisabledColor = dwColor;
		Invalidate();
	}

	DWORD CSliderUI::GetDisabledThumbColor() const {
		return m_dwThumbDisabledColor;
	}

	void CSliderUI::SetFocusedThumbColor(DWORD dwColor) {
		m_dwThumbFocusedColor = dwColor;
		Invalidate();
	}

	DWORD CSliderUI::GetFocusedThumbColor() const {
		return m_dwThumbFocusedColor;
	}

	void CSliderUI::SetValue(int nValue) {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) 
            return;

        CProgressUI::SetValue(nValue);
    }

    void CSliderUI::DoEvent(TEventUI &event) {
        if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
            if( m_pParent != NULL ) 
                m_pParent->DoEvent(event);
            else 
                CProgressUI::DoEvent(event);

            return;
        }

        if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
            if( IsEnabled() ) {
                m_uButtonState |= UISTATE_CAPTURED;

                int nValue;

                if( m_bHorizontal ) {
                    if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) 
                        nValue = m_nMax;
                    else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) 
                        nValue = m_nMin;
                    else 
                        nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
                } else {
                    if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) 
                        nValue = m_nMin;
                    else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) 
                        nValue = m_nMax;
                    else 
                        nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
                }

                if(m_nValue != nValue && nValue >= m_nMin && nValue <= m_nMax) {
                    m_nValue = nValue;
                    Invalidate();
                }

                UpdateText();
            }

            return;
        }

        if( event.Type == UIEVENT_BUTTONUP || event.Type == UIEVENT_RBUTTONUP) {
            if( IsEnabled() ) {
                int nValue = 0;

                if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
                    m_uButtonState &= ~UISTATE_CAPTURED;
                }

                if( m_bHorizontal ) {
                    if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) nValue = m_nMax;
                    else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) nValue = m_nMin;
                    else nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
                } else {
                    if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) nValue = m_nMin;
                    else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) nValue = m_nMax;
                    else nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
                }

                if(nValue >= m_nMin && nValue <= m_nMax) {
                    m_nValue = nValue;
                    m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
                    Invalidate();
                }

                UpdateText();
                return;
            }
        }

        if( event.Type == UIEVENT_CONTEXTMENU ) {
            return;
        }

        if( event.Type == UIEVENT_SCROLLWHEEL ) {
            if( IsEnabled() ) {
				int zDelta = (int)(short)HIWORD(event.wParam);
				if (zDelta < 0) {
					SetValue(GetValue() - GetChangeStep());
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
					return;
				}
				else if(zDelta > 0) {
					SetValue(GetValue() + GetChangeStep());
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
					return;
				}
            }
        }

        if( event.Type == UIEVENT_MOUSEMOVE ) {
            if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
                if( m_bHorizontal ) {
                    if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) m_nValue = m_nMax;
                    else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) m_nValue = m_nMin;
                    else m_nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
                } else {
                    if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) m_nValue = m_nMin;
                    else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) m_nValue = m_nMax;
                    else m_nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
                }

                if (m_bSendMove) {
                    UpdateText();
                    m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED_MOVE);
                }

                Invalidate();
            }

            POINT pt = event.ptMouse;
            RECT rcThumb = CalcThumbRect();

            if( IsEnabled() && ::PtInRect(&rcThumb, event.ptMouse) ) {
                m_uButtonState |= UISTATE_HOT;
                Invalidate();
            } else {
                m_uButtonState &= ~UISTATE_HOT;
                Invalidate();
            }

            return;
        }

        if( event.Type == UIEVENT_SETCURSOR ) {
            RECT rcThumb = CalcThumbRect();

            if( IsEnabled()) {
                ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
                return;
            }
        }

        if( event.Type == UIEVENT_MOUSELEAVE ) {
            if( IsEnabled() ) {
                m_uButtonState &= ~UISTATE_HOT;
                Invalidate();
            }

            return;
        }

        CControlUI::DoEvent(event);
    }

    void CSliderUI::SetCanSendMove(bool bCanSend) {
        m_bSendMove = bCanSend;
    }

    bool CSliderUI::GetCanSendMove() const {
        return m_bSendMove;
    }

    void CSliderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
        if( _tcsicmp(pstrName, _T("thumbimage")) == 0 ) 
			SetThumbImage(pstrValue);
        else if( _tcsicmp(pstrName, _T("thumbhotimage")) == 0 ) 
			SetThumbHotImage(pstrValue);
        else if( _tcsicmp(pstrName, _T("thumbpushedimage")) == 0 ) 
			SetThumbPushedImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("thumbfocusedimage")) == 0)
			SetThumbFocusedImage(pstrValue);
        else if( _tcsicmp(pstrName, _T("thumbsize")) == 0 ) {
            SIZE szXY = {0};
            LPTSTR pstr = NULL;
            szXY.cx = _tcstol(pstrValue, &pstr, 10);
            ASSERT(pstr);
            szXY.cy = _tcstol(pstr + 1, &pstr, 10);
            ASSERT(pstr);
            SetThumbSize(szXY);
        } 
		else if( _tcsicmp(pstrName, _T("step")) == 0 ) {
            SetChangeStep(_ttoi(pstrValue));
        } 
		else if( _tcsicmp(pstrName, _T("sendmove")) == 0 ) {
            SetCanSendMove(_tcsicmp(pstrValue, _T("true")) == 0);
        }
		else if (_tcsicmp(pstrName, _T("bkcolorwidth")) == 0) {
			SetBkColorWidth(_ttoi(pstrValue));
		}
		else if (_tcsicmp(pstrName, _T("bkcolorheight")) == 0) {
			SetBkColorHeight(_ttoi(pstrValue));
		}
		else if (_tcsicmp(pstrName, _T("thumbcolor")) == 0) {
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);

			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetThumbColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("disabledthumbcolor")) == 0) {
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);

			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledThumbColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("hotthumbcolor")) == 0) {
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);

			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotThumbColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("focusedthumbcolor")) == 0) {
			if (*pstrValue == _T('#'))
				pstrValue = ::CharNext(pstrValue);

			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetFocusedThumbColor(clrColor);
		}
		else
			CProgressUI::SetAttribute(pstrName, pstrValue);
    }

	void CSliderUI::PaintForeColor(HDC hDC) {
		RECT rcThumb = CalcThumbRect();

		if (IsFocused())
			m_uButtonState |= UISTATE_FOCUSED;
		else
			m_uButtonState &= ~UISTATE_FOCUSED;

		if (!IsEnabled())
			m_uButtonState |= UISTATE_DISABLED;
		else
			m_uButtonState &= ~UISTATE_DISABLED;

		if ((m_uButtonState & UISTATE_DISABLED) != 0) {
			if (m_dwThumbDisabledColor != 0) {
				CRenderEngine::DrawColor(hDC, rcThumb, GetAdjustColor(m_dwThumbDisabledColor));
				return;
			}
		}
		else if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
			if (m_dwThumbPushedColor != 0) {
				CRenderEngine::DrawColor(hDC, rcThumb, GetAdjustColor(m_dwThumbPushedColor));
				return;
			}
		}
		else if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (m_dwThumbHotColor != 0) {
				CRenderEngine::DrawColor(hDC, rcThumb, GetAdjustColor(m_dwThumbHotColor));
				return;
			}
		}
		else if ((m_uButtonState & UISTATE_FOCUSED) != 0) {
			if (m_dwThumbFocusedColor != 0) {
				CRenderEngine::DrawColor(hDC, rcThumb, GetAdjustColor(m_dwThumbFocusedColor));
				return;
			}
		}

		if(m_dwThumbColor != 0)
			CRenderEngine::DrawColor(hDC, rcThumb, GetAdjustColor(m_dwThumbColor));
	}

	void CSliderUI::PaintForeImage(HDC hDC) {
        RECT rcThumb = CalcThumbRect();
        rcThumb.left -= m_rcItem.left;
        rcThumb.top -= m_rcItem.top;
        rcThumb.right -= m_rcItem.left;
        rcThumb.bottom -= m_rcItem.top;

        m_pManager->GetDPIObj()->ScaleBack(&rcThumb);

		if (IsFocused())
			m_uButtonState |= UISTATE_FOCUSED;
		else
			m_uButtonState &= ~UISTATE_FOCUSED;

        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            if( !m_sThumbPushedImage.IsEmpty() ) {
                m_sImageModify.Empty();
                m_sImageModify.Format(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);

                if( DrawImage(hDC, (LPCTSTR)m_sThumbPushedImage, (LPCTSTR)m_sImageModify) )
					return;
            }
        } 
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
            if( !m_sThumbHotImage.IsEmpty() ) {
                m_sImageModify.Empty();
                m_sImageModify.Format(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);

                if(DrawImage(hDC, (LPCTSTR)m_sThumbHotImage, (LPCTSTR)m_sImageModify) )
					return;
            }
        }
		else if ((m_uButtonState & UISTATE_FOCUSED) != 0) {
			if (!m_sThumbFocusedImage.IsEmpty()) {
				m_sImageModify.Empty();
				m_sImageModify.Format(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);

				if (DrawImage(hDC, (LPCTSTR)m_sThumbFocusedImage, (LPCTSTR)m_sImageModify))
					return;
			}
		}

        if( !m_sThumbImage.IsEmpty() ) {
            m_sImageModify.Empty();
            m_sImageModify.Format(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);

            if(DrawImage(hDC, (LPCTSTR)m_sThumbImage, (LPCTSTR)m_sImageModify))
				return;
        }
    }

	void CSliderUI::PaintBkColor(HDC hDC) {
		RECT rcItem = m_rcItem;

		if (IsHorizontal()) {
			if (m_nBkColorHeight != -1) {
				rcItem.top += (rcItem.bottom - rcItem.top - m_nBkColorHeight) / 2;
				rcItem.bottom -= (rcItem.bottom - rcItem.top - m_nBkColorHeight) / 2;
			}
		}
		else {
			if (m_nBkColorWidth != -1) {
				rcItem.left += (rcItem.right - rcItem.left - m_nBkColorWidth) / 2;
				rcItem.right -= (rcItem.right - rcItem.left - m_nBkColorWidth) / 2;
			}
		}

		if (m_dwBackColor != 0) {
			bool bVer = (m_sGradient.CompareNoCase(_T("hor")) != 0);

			if (m_dwBackColor2 != 0) {
				if (m_dwBackColor3 != 0) {
					RECT rc = rcItem;
					rc.bottom = (rc.bottom + rc.top) / 2;
					CRenderEngine::DrawGradient(hDC, rc, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 8);
					rc.top = rc.bottom;
					rc.bottom = rcItem.bottom;
					CRenderEngine::DrawGradient(hDC, rc, GetAdjustColor(m_dwBackColor2), GetAdjustColor(m_dwBackColor3), bVer, 8);
				}
				else {
					CRenderEngine::DrawGradient(hDC, rcItem, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 16);
				}
			}
			else
				CRenderEngine::DrawColor(hDC, rcItem, GetAdjustColor(m_dwBackColor));
		}
	}
}
