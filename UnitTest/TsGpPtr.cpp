#include "pch.h"
#include "../eck/GpPtr.h"

#pragma comment(lib, "Gdiplus.lib")

using namespace eck;
using namespace Gdiplus;

TS_NS_BEGIN
TEST_CLASS(TsGpPtr)
{
public:
    ULONG_PTR uGpToken{};

    TEST_METHOD_INITIALIZE(Init)
    {
        GdiplusStartupInput gpsi{};
        GdiplusStartup(&uGpToken, &gpsi, nullptr);
    }
    TEST_METHOD_CLEANUP(Cleanup)
    {
        GdiplusShutdown(uGpToken);
    }

    TEST_METHOD(Ts)
    {
        // ===== Brush =====
        {
            GpPtr<GpSolidFill> brush;
            GdipCreateSolidFill(Color::MakeARGB(255, 255, 0, 0), &brush);

            GpPtr<GpSolidFill> brushClone;
            auto s = brush.Clone(brushClone);
            Assert::IsTrue(s == Ok && brushClone.Get());
        }

        // ===== Pen =====
        {
            GpPtr<GpPen> pen;
            GdipCreatePen1(Color::MakeARGB(255, 0, 0, 255), 3.f, UnitPixel, &pen);

            GpPtr<GpPen> penClone;
            auto s = pen.Clone(penClone);
            Assert::IsTrue(s == Ok && penClone.Get());
        }

        // ===== GraphicsPath =====
        {
            GpPtr<GpPath> path;
            GdipCreatePath(FillModeAlternate, &path);
            GdipAddPathEllipse(path.Get(), 0, 0, 100, 100);

            GpPtr<GpPath> pathClone;
            auto s = path.Clone(pathClone);
            Assert::IsTrue(s == Ok && pathClone.Get());
        }

        // ===== Region =====
        {
            GpPtr<GpRegion> region;
            GdipCreateRegion(&region);

            GpPtr<GpRegion> regionClone;
            auto s = region.Clone(regionClone);
            Assert::IsTrue(s == Ok && regionClone.Get());
        }

        // ===== Matrix =====
        {
            GpPtr<GpMatrix> matrix;
            GdipCreateMatrix(&matrix);
            GdipTranslateMatrix(matrix.Get(), 10, 20, MatrixOrderAppend);

            GpPtr<GpMatrix> matrixClone;
            auto s = matrix.Clone(matrixClone);
            Assert::IsTrue(s == Ok && matrixClone.Get());
        }

        // ===== FontFamily =====
        {
            GpPtr<GpFontFamily> family;
            GdipCreateFontFamilyFromName(L"Arial", nullptr, &family);

            GpPtr<GpFontFamily> familyClone;
            auto s = family.Clone(familyClone);
            Assert::IsTrue(s == Ok && familyClone.Get());
        }

        // ===== Font =====
        {
            GpPtr<GpFontFamily> family;
            GdipCreateFontFamilyFromName(L"Arial", nullptr, &family);

            GpPtr<GpFont> font;
            GdipCreateFont(family.Get(), 16.f, FontStyleRegular, UnitPixel, &font);

            GpPtr<GpFont> fontClone;
            auto s = font.Clone(fontClone);
            Assert::IsTrue(s == Ok && fontClone.Get());
        }

        // ===== StringFormat =====
        {
            GpPtr<GpStringFormat> fmt;
            GdipCreateStringFormat(0, LANG_NEUTRAL, &fmt);

            GpPtr<GpStringFormat> fmtClone;
            auto s = fmt.Clone(fmtClone);
            Assert::IsTrue(s == Ok && fmtClone.Get());
        }

        // ===== ImageAttributes =====
        {
            GpPtr<GpImageAttributes> attr;
            GdipCreateImageAttributes(&attr);

            GpPtr<GpImageAttributes> attrClone;
            auto s = attr.Clone(attrClone);
            Assert::IsTrue(s == Ok && attrClone.Get());
        }

        // ===== Bitmap (via GpImage base) =====
        {
            GpPtr<GpBitmap> bmp;
            GdipCreateBitmapFromScan0(
                64, 64, 0,
                PixelFormat32bppARGB,
                nullptr,
                &bmp);

            GpPtr<GpBitmap> bmpClone;
            auto s = bmp.Clone(bmpClone);
            Assert::IsTrue(s == Ok && bmpClone.Get());
        }

        // ===== AdjustableArrowCap (CustomLineCap) =====
        {
            GpPtr<GpAdjustableArrowCap> cap;
            GdipCreateAdjustableArrowCap(10, 5, TRUE, &cap);

            GpPtr<GpAdjustableArrowCap> capClone;
            auto s = cap.Clone(capClone);
            Assert::IsTrue(s == Ok && capClone.Get());
        }

        // ===== Graphics =====
        {
            GpPtr<GpGraphics> graphics;
            HDC hdc = GetDC(0);
            GdipCreateFromHDC(hdc, &graphics);
            ReleaseDC(0, hdc);

            Assert::IsTrue(graphics.Get() != nullptr);
        }

        // ===== PathIterator =====
        {
            GpPtr<GpPath> path;
            GdipCreatePath(FillModeAlternate, &path);
            GdipAddPathEllipse(path.Get(), 0, 0, 100, 100);

            GpPtr<GpPathIterator> iter;
            GdipCreatePathIter(&iter, path.Get());

            Assert::IsTrue(iter.Get() != nullptr);
        }

        // ===== CachedBitmap =====
        {
            HDC hdc = GetDC(0);

            GpPtr<GpGraphics> graphics;
            GdipCreateFromHDC(hdc, &graphics);

            GpPtr<GpBitmap> bmp;
            GdipCreateBitmapFromScan0(
                64, 64, 0,
                PixelFormat32bppARGB,
                nullptr,
                &bmp);

            GpPtr<GpCachedBitmap> cached;
            GdipCreateCachedBitmap(bmp.Get(), graphics.Get(), &cached);

            ReleaseDC(0, hdc);

            Assert::IsTrue(cached.Get() != nullptr);
        }

        // ===== Effect =====
        {
            GpPtr<GpEffect> effect;

            GUID blurGuid = BlurEffectGuid; // 需要 <gdipluseffects.h>
            GdipCreateEffect(blurGuid, &effect);

            Assert::IsTrue(effect.Get() != nullptr);
        }
    }
};
TS_NS_END