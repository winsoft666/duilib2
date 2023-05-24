#ifndef GifAnimUI_h__
#define GifAnimUI_h__

#pragma once

namespace DuiLib {
    class UILIB_API CGifAnimUI : public CButtonUI {
        DECLARE_DUICONTROL(CGifAnimUI)
      public:
        CGifAnimUI(void);
        ~CGifAnimUI(void);

        LPCTSTR	GetClass() const;
        LPVOID	GetInterface(LPCTSTR pstrName);
        void	DoInit();
        bool	DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl);
        void	SetVisible(bool bVisible = true );
        void	SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
        void	SetGifImage(LPCTSTR pStrImage);
        LPCTSTR GetGifImage();

        void	SetAutoPlay(bool bIsAuto = true );
        bool	IsAutoPlay() const;

        void	SetAutoSize(bool bIsAuto = true );
        bool	IsAutoSize() const;

        void  SetAdaptDpiScale(bool b = true);
        bool  IsAdaptDpiScale() const;

        void	PlayGif();
        void	PauseGif();
        void	StopGif();

      private:
        void	InitGifImage();
        void	DeleteGif();
        void	DrawFrame( HDC hDC );		// 绘制GIF每帧
      private:
        Gdiplus::Image	*m_pGifImage;
        UINT			m_nFrameCount;				// gif图片总帧数
        UINT			m_nFramePosition;			// 当前放到第几帧
        Gdiplus::PropertyItem	*m_pPropertyItem;	// 帧与帧之间间隔时间

        CDuiString		m_sGifImage;
        bool			m_bIsAutoPlay;				// 是否自动播放gif
        bool			m_bIsAutoSize;				// 是否自动根据图片设置大小
        bool			m_bIsPlaying;
        bool      m_bAdaptDpiScale;
        class Impl;
        Impl *m_pImpl;
    };
}

#endif // GifAnimUI_h__
