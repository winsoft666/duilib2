#ifndef __COLOR_H__
#define __COLOR_H__
#pragma once


// Jeffery: 微软定义的RGB宏，从高位到低位依次为B,G,R，DUI_RGB宏从高位到低位依次为R,G,B
//          传入DuiLib的颜色值，要使用DUI_RGB宏代替RGB
//
#define DUI_RGB(r,g,b)          ((COLORREF)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))
#define DUI_ARGB(a,r,g,b)       ((COLORREF)( (BYTE)(b) | ( ((WORD)(g)) << 8) | ( ((DWORD)(r)) << 16 ) | ( ((DWORD)(a)) << 24 ) ))

namespace DuiLib {
    class UILIB_API ColorConvert {
      public:
        enum { Red = 0, Green, Blue };
        static void RGBtoHSL(BYTE R, BYTE G, BYTE B, float &H, float &S, float &L);
        static void HSLtoRGB(float H, float S, float L, BYTE &R, BYTE &G, BYTE &B);
    };
}

#endif // !__COLOR_H__