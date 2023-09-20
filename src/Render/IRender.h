#pragma once

namespace DuiLib {
    typedef struct UILIB_API tagTFontInfo {
        HFONT hFont;
        int iSize;
        bool bBold;
        bool bUnderline;
        bool bItalic;
        TEXTMETRIC tm;
        CDuiString sFontName;

        tagTFontInfo() {
            hFont = NULL;
            iSize = 0;
            bBold = bUnderline = bItalic = false;
            ZeroMemory(&tm, sizeof(TEXTMETRIC));
        }
    } TFontInfo;

    typedef struct UILIB_API tagTImageInfo {
        HBITMAP hBitmap;
        LPBYTE pBits;
        LPBYTE pSrcBits;
        int nX;
        int nY;
        bool bAlpha;
        bool bUseHSL;
        CDuiString sResType;
        DWORD dwMask;
    } TImageInfo;

    typedef struct UILIB_API tagTDrawInfo {
        tagTDrawInfo();
        void Parse(LPCTSTR pStrImage, LPCTSTR pStrModify, CPaintManagerUI *pManager);
        void Clear();
        bool IsImageExist(const CDuiString &newImageName);

        CDuiString sDrawString;
        CDuiString sDrawModify;
        CDuiString sImageName;
        CDuiString sResType;
        RECT rcDest;
        RECT rcSource;
        RECT rcCorner;
        DWORD dwMask;
        BYTE uFade;
        bool bHole;
        bool bTiledX;
        bool bTiledY;
        bool bHSL;
        bool bForceOriginImage; // Jeffery: 是否因为没有找到对应DPI的图片，而强制使用原图
        bool bAdaptDpiScale;    // Jeffery: 是否适应DPI缩放
    } TDrawInfo;

    class IRender {

    };
}