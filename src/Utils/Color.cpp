#include "StdAfx.h"
#include "Utils/Color.h"

namespace DuiLib {
    void ColorConvert::RGBtoHSL(BYTE red, BYTE green, BYTE blue, float &hue, float &saturation, float &lightness) {
        float mn = 0.0f;
        float mx = 0.0f;
        int	major = Red;

        if (red < green) {
            mn = red;
            mx = green;
            major = Green;
        }
        else {
            mn = green;
            mx = red;
            major = Red;
        }

        if (blue < mn) {
            mn = blue;
        }
        else if (blue > mx) {
            mx = blue;
            major = Blue;
        }

        if (mn == mx) {
            lightness = mn / 255.0f;
            saturation = 0;
            hue = 0;
        }
        else {
            lightness = (mn + mx) / 510.0f;

            if (lightness <= 0.5f) {
                saturation = (mx - mn) / (mn + mx);
            }
            else {
                saturation = (mx - mn) / (510.0f - mn - mx);
            }

            switch (major) {
            case Red:
                hue = (green - blue) * 60 / (mx - mn) + 360;
                break;

            case Green:
                hue = (blue - red) * 60 / (mx - mn) + 120;
                break;

            case Blue: hue = (red - green) * 60 / (mx - mn) + 240;
                break;
            }

            if (hue >= 360.0f) {
                hue = hue - 360.0f;
            }
        }
    }

    unsigned char Value_Hgy(float m1, float m2, float h) {
        while (h >= 360.0f) {
            h -= 360.0f;
        }
        while (h < 0) {
            h += 360.0f;
        }

        if (h < 60.0f) {
            m1 = m1 + (m2 - m1) * h / 60;
        }
        else if (h < 180.0f) {
            m1 = m2;
        }
        else if (h < 240.0f) {
            m1 = m1 + (m2 - m1) * (240 - h) / 60;
        }

        return (unsigned char)(m1 * 255);
    }


    void ColorConvert::HSLtoRGB(float hue, float saturation, float lightness, BYTE &red, BYTE &green, BYTE &blue) {
        lightness = min(1.0f, lightness);
        saturation = min(1.0f, saturation);

        if (saturation == 0) {
            red = green = blue = (unsigned char)(lightness * 255);
        }
        else {
            float m1, m2;

            if (lightness <= 0.5f) {
                m2 = lightness + lightness * saturation;
            }
            else {
                m2 = lightness + saturation - lightness * saturation;
            }

            m1 = 2 * lightness - m2;

            red = Value_Hgy(m1, m2, hue + 120);
            green = Value_Hgy(m1, m2, hue);
            blue = Value_Hgy(m1, m2, hue - 120);
        }
    }
}