#include "StdAfx.h"
#include "UIGifAnim.h"

///////////////////////////////////////////////////////////////////////////////////////
namespace DuiLib {
class CGifAnimUI::Impl {
 public:
  Impl() {}

  Timer m_Timer;
};
IMPLEMENT_DUICONTROL(CGifAnimUI)

CGifAnimUI::CGifAnimUI(void) {
  m_pGifImage = NULL;
  m_pPropertyItem = NULL;
  m_nFrameCount = 0;
  m_nFramePosition = 0;
  m_bIsAutoPlay = true;
  m_bIsAutoSize = false;
  m_bIsPlaying = false;
  m_bAdaptDpiScale = true;
  m_pImpl = new Impl();
  if (m_pImpl) {
    m_pImpl->m_Timer.SetTimedEvent([this]() {
      this->Invalidate();
      m_nFramePosition = (++m_nFramePosition) % m_nFrameCount;

      long lPause = ((long*)m_pPropertyItem->value)[m_nFramePosition] * 10;
      if (lPause == 0)
        lPause = 100;
      m_pImpl->m_Timer.Stop(false);
      m_pImpl->m_Timer.Start(lPause, FALSE, TRUE);
    });
  }
}

CGifAnimUI::~CGifAnimUI(void) {
  StopGif();
  DeleteGif();

  if (m_pImpl) {
    delete m_pImpl;
    m_pImpl = NULL;
  }
}

LPCTSTR CGifAnimUI::GetClass() const {
  return DUI_CTR_GIFANIM;
}

LPVOID CGifAnimUI::GetInterface(LPCTSTR pstrName) {
  if (_tcsicmp(pstrName, DUI_CTR_GIFANIM) == 0)
    return static_cast<CGifAnimUI*>(this);

  return CControlUI::GetInterface(pstrName);
}

void CGifAnimUI::DoInit() {
  InitGifImage();
}

bool CGifAnimUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl) {
  if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem))
    return true;

  if (NULL == m_pGifImage) {
    InitGifImage();
  }

  DrawFrame(hDC);
  return true;
}

void CGifAnimUI::SetVisible(bool bVisible /* = true */) {
  CControlUI::SetVisible(bVisible);

  if (bVisible)
    PlayGif();
  else
    StopGif();
}

void CGifAnimUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
  if (_tcsicmp(pstrName, _T("image")) == 0) {
    SetGifImage(pstrValue);
  }
  else if (_tcsicmp(pstrName, _T("autoplay")) == 0) {
    SetAutoPlay(_tcsicmp(pstrValue, _T("true")) == 0);
  }
  else if (_tcsicmp(pstrName, _T("autosize")) == 0) {
    SetAutoSize(_tcsicmp(pstrValue, _T("true")) == 0);
  }
  else if (_tcsicmp(pstrName, _T("adaptdpiscale")) == 0) {
    m_bAdaptDpiScale = (_tcsicmp(pstrValue, _T("true")) == 0);
  }
  else
    CButtonUI::SetAttribute(pstrName, pstrValue);
}

void CGifAnimUI::SetGifImage(LPCTSTR pStrImage) {
  if (m_sGifImage == pStrImage || NULL == pStrImage)
    return;

  m_sGifImage = pStrImage;

  StopGif();
  DeleteGif();

  Invalidate();
}

LPCTSTR CGifAnimUI::GetGifImage() {
  return m_sGifImage;
}

void CGifAnimUI::SetAutoPlay(bool bIsAuto) {
  m_bIsAutoPlay = bIsAuto;
}

bool CGifAnimUI::IsAutoPlay() const {
  return m_bIsAutoPlay;
}

void CGifAnimUI::SetAutoSize(bool bIsAuto) {
  m_bIsAutoSize = bIsAuto;
}

bool CGifAnimUI::IsAutoSize() const {
  return m_bIsAutoSize;
}

void CGifAnimUI::SetAdaptDpiScale(bool b /*= true*/) {
  m_bAdaptDpiScale = b;
}

bool CGifAnimUI::IsAdaptDpiScale() const {
  return m_bAdaptDpiScale;
}

void CGifAnimUI::PlayGif() {
  if (m_bIsPlaying || m_pGifImage == NULL || m_nFrameCount <= 1) {
    return;
  }

  long lPause = ((long*)m_pPropertyItem->value)[m_nFramePosition] * 10;

  if (lPause == 0)
    lPause = 100;

  if (m_pImpl) {
    m_pImpl->m_Timer.Start(lPause, FALSE, TRUE);
  }

  m_bIsPlaying = true;
}

void CGifAnimUI::PauseGif() {
  if (!m_bIsPlaying || m_pGifImage == NULL) {
    return;
  }

  this->Invalidate();
  m_bIsPlaying = false;
}

void CGifAnimUI::StopGif() {
  if (!m_bIsPlaying) {
    return;
  }
  if (m_pImpl) {
    m_pImpl->m_Timer.Stop(true);
  }

  m_nFramePosition = 0;
  this->Invalidate();
  m_bIsPlaying = false;
}

void CGifAnimUI::InitGifImage() {
  m_pGifImage = CRenderEngine::GdiplusLoadImage(GetGifImage());

  if (NULL == m_pGifImage)
    return;

  UINT nCount = 0;
  nCount = m_pGifImage->GetFrameDimensionsCount();
  GUID* pDimensionIDs = new GUID[nCount];
  m_pGifImage->GetFrameDimensionsList(pDimensionIDs, nCount);
  m_nFrameCount = m_pGifImage->GetFrameCount(&pDimensionIDs[0]);

  if (m_nFrameCount > 1) {
    int nSize = m_pGifImage->GetPropertyItemSize(PropertyTagFrameDelay);
    m_pPropertyItem = (Gdiplus::PropertyItem*)malloc(nSize);
    m_pGifImage->GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem);
  }

  delete[] pDimensionIDs;
  pDimensionIDs = NULL;

  if (m_bIsAutoSize) {
    SetFixedWidth(m_pGifImage->GetWidth());
    SetFixedHeight(m_pGifImage->GetHeight());
  }

  if (m_bIsAutoPlay) {
    PlayGif();
  }
}

void CGifAnimUI::DeleteGif() {
  if (m_pGifImage != NULL) {
    delete m_pGifImage;
    m_pGifImage = NULL;
  }

  if (m_pPropertyItem != NULL) {
    free(m_pPropertyItem);
    m_pPropertyItem = NULL;
  }

  m_nFrameCount = 0;
  m_nFramePosition = 0;
}

void CGifAnimUI::DrawFrame(HDC hDC) {
  if (NULL == hDC || NULL == m_pGifImage)
    return;

  RECT rc = m_rcItem;

  if (!m_bAdaptDpiScale) {
    int oldWidth = m_rcItem.right - m_rcItem.left;
    int oldHeight = m_rcItem.bottom - m_rcItem.top;
    int newWidth = m_pManager->GetDPIObj()->ScaleBack(oldWidth);
    int newHeight = m_pManager->GetDPIObj()->ScaleBack(oldHeight);

    rc.left = m_rcItem.left + (oldWidth - newWidth) / 2;
    rc.top = m_rcItem.top + (oldHeight - newHeight) / 2;
    rc.right = rc.left + newWidth;
    rc.bottom = rc.top + newHeight;
  }

  GUID pageGuid = Gdiplus::FrameDimensionTime;
  Gdiplus::Graphics graphics(hDC);
  Gdiplus::Status status =
      graphics.DrawImage(m_pGifImage, rc.left, rc.top, rc.right - rc.left,
                         rc.bottom - rc.top);
  ASSERT(status == 0);
  status = m_pGifImage->SelectActiveFrame(&pageGuid, m_nFramePosition);
  ASSERT(status == 0);
}
}  // namespace DuiLib
