/*
* WinEzCtrlKit Library
*
* GdiplusFlatDef.h ： GDI+Flat定义
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include <Windows.h>

#define WINGDIPAPI WINAPI
#define GDIPCONST const

typedef void* DrawImageAbort;
typedef void EncoderParameters;
typedef float REAL;
typedef DWORD ARGB;

enum GpStatus
{
	GpOk,
	GpGenericError,
	GpInvalidParameter,
	GpOutOfMemory,
	GpObjectBusy,
	GpInsufficientBuffer,
	GpNotImplemented,
	GpWin32Error,
	GpWrongState,
	GpAborted,
	GpFileNotFound,
	GpValueOverflow,
	GpAccessDenied,
	GpUnknownImageFormat,
	GpFontFamilyNotFound,
	GpFontStyleNotFound,
	GpNotTrueTypeFont,
	GpUnsupportedGdiplusVersion,
	GpGdiplusNotInitialized,
	GpPropertyNotFound,
	GpPropertyNotSupported,
	GpProfileNotFound
};
enum GpSmoothingMode
{
	SmoothingModeDefault,
	SmoothingModeHighSpeed,
	SmoothingModeHighQuality,
	SmoothingModeNone,
};
enum GpTextRenderingHint
{
	TextRenderingHintSystemDefault,
	TextRenderingHintSingleBitPerPixelGridFit,
	TextRenderingHintSingleBitPerPixel,
	TextRenderingHintAntiAliasGridFit,
	TextRenderingHintAntiAlias,
	TextRenderingHintClearTypeGridFit
};
enum GpFillMode
{
	FillModeAlternate,
	FillModeWinding
};
enum GpWrapMode
{
	WrapModeTile,
	WrapModeTileFlipX,
	WrapModeTileFlipY,
	WrapModeTileFlipXY,
	WrapModeClamp
};
enum GpFontStyle {
	FontStyleRegular = 0,
	FontStyleBold = 1,
	FontStyleItalic = 2,
	FontStyleBoldItalic = 3,
	FontStyleUnderline = 4,
	FontStyleStrikeout = 8
};
enum GpStringAlignment
{
	StringAlignmentNear,
	StringAlignmentCenter,
	StringAlignmentFar
};
enum class GpUnit
{
	World,
	Display,
	Pixel,
	Point,
	Inch,
	Document,
	Millimeter
};
enum GpStringFormatFlags
{
	StringFormatFlagsDirectionRightToLeft  = 0x00000001,
	StringFormatFlagsDirectionVertical     = 0x00000002,
	StringFormatFlagsNoFitBlackBox         = 0x00000004,
	StringFormatFlagsDisplayFormatControl  = 0x00000020,
	StringFormatFlagsNoFontFallback        = 0x00000400,
	StringFormatFlagsMeasureTrailingSpaces = 0x00000800,
	StringFormatFlagsNoWrap                = 0x00001000,
	StringFormatFlagsLineLimit             = 0x00002000,
	StringFormatFlagsNoClip                = 0x00004000,
	StringFormatFlagsBypassGDI             = 0x80000000
};
enum class GpPixelFormat
{
	Indexed        = 0x00010000,
	GDI            = 0x00020000,
	Alpha          = 0x00040000,
	PAlpha         = 0x00080000,
	Extended       = 0x00100000,
	Canonical      = 0x00200000,

	Undefined      = 0,
	DontCare       = 0,

	PF1bppIndexed    = (1 | (1 << 8) | Indexed | GDI),
	PF4bppIndexed    = (2 | (4 << 8) | Indexed | GDI),
	PF8bppIndexed    = (3 | (8 << 8) | Indexed | GDI),
	PF16bppGrayScale = (4 | (16 << 8) | Extended),
	PF16bppRGB555    = (5 | (16 << 8) | GDI),
	PF16bppRGB565    = (6 | (16 << 8) | GDI),
	PF16bppARGB1555  = (7 | (16 << 8) | Alpha | GDI),
	PF24bppRGB       = (8 | (24 << 8) | GDI),
	PF32bppRGB       = (9 | (32 << 8) | GDI),
	PF32bppARGB      = (10 | (32 << 8) | Alpha | GDI | Canonical),
	PF32bppPARGB     = (11 | (32 << 8) | Alpha | PAlpha | GDI),
	PF48bppRGB       = (12 | (48 << 8) | Extended),
	PF64bppARGB      = (13 | (64 << 8) | Alpha | Canonical | Extended),
	PF64bppPARGB     = (14 | (64 << 8) | Alpha | PAlpha | Extended),
	PF32bppCMYK      = (15 | (32 << 8)),
	Max            = 16
};
enum GpPathPointType {
	PathPointTypeStart = 0,
	PathPointTypeLine = 1,
	PathPointTypeBezier = 3,
	PathPointTypePathTypeMask = 0x07,
	PathPointTypeDashMode = 0x10,
	PathPointTypePathMarker = 0x20,
	PathPointTypeCloseSubpath = 0x80,
	PathPointTypeBezier3 = 3
};
enum GpMatrixOrder {
	MatrixOrderPrepend = 0,
	MatrixOrderAppend = 1
};
enum GpDebugEventLevel 
{
	DebugEventLevelFatal,
	DebugEventLevelWarning
};
enum GpImageLockMode
{
	ImageLockModeRead = 0x0001,
	ImageLockModeWrite = 0x0002,
	ImageLockModeUserInputBuf = 0x0004
};
enum GpInterpolationMode
{
	Invalid,
	Default,
	LowQuality,
	HighQuality,
	Bilinear,
	Bicubic,
	NearestNeighbor,
	HighQualityBilinear,
	HighQualityBicubic
};
enum class GpColorMatrixFlags
{
	Default = 0,
	SkipGrays = 1,
	AltGray = 2
};
enum class GpColorAdjustType {
	Default,
	Bitmap,
	Brush,
	Pen,
	Text,
	Count,
	Any
};


typedef VOID(WINAPI* GpDebugEventProc)(GpDebugEventLevel level, CHAR* message);

struct GdiplusStartupInput
{
	UINT32 GdiplusVersion;
	GpDebugEventProc DebugEventCallback;
	BOOL SuppressBackgroundThread;
	BOOL SuppressExternalCodecs;
};
struct GpRectF
{
	REAL Left;
	REAL Top;
	REAL Width;
	REAL Height;
};
struct GpRect
{
	INT Left;
	INT Top;
	INT Width;
	INT Height;
};
struct GpPointF
{
	REAL x;
	REAL y;
};
struct GpPoint
{
	int x;
	int y;
};
struct GpBlurParams
{
	REAL radius;
	BOOL expandEdge;
};
struct GpImageCodecInfo
{
	CLSID Clsid;
	GUID  FormatID;
	const WCHAR* CodecName;
	const WCHAR* DllName;
	const WCHAR* FormatDescription;
	const WCHAR* FilenameExtension;
	const WCHAR* MimeType;
	DWORD Flags;
	DWORD Version;
	DWORD SigCount;
	DWORD SigSize;
	const BYTE* SigPattern;
	const BYTE* SigMask;
};
struct GpBitmapData
{
	UINT Width;// Number of pixels in one scan line of the bitmap.
	UINT Height;// Number of scan lines in the bitmap.
	INT Stride;// Offset, in bytes, between consecutive scan lines of the bitmap.If the stride is positive, the bitmap is top - down.If the stride is negative, the bitmap is bottom - up.
	GpPixelFormat PixelFormat;// Integer that specifies the pixel format of the bitmap.
	void* Scan0;// Pointer to the first(index 0) scan line of the bitmap.
	UINT_PTR Reserved;// Reserved for future use.
};
struct GpColorMatrix {
	REAL m[5][5];
};



#define DECLFAKEGDIPOBJ(type) struct type{int unused;};
#define DECLFAKEGDIPOBJINHERIT(type,base) struct type:public base{int unused;};
DECLFAKEGDIPOBJ(GpGraphics)
DECLFAKEGDIPOBJ(GpFontCollection)
DECLFAKEGDIPOBJ(GpFontFamily)
DECLFAKEGDIPOBJ(GpFont)
DECLFAKEGDIPOBJ(GpStringFormat)
DECLFAKEGDIPOBJ(GpPen)
DECLFAKEGDIPOBJ(GpPath)
DECLFAKEGDIPOBJ(GpBrush)
DECLFAKEGDIPOBJINHERIT(GpSolidFill, GpBrush)
DECLFAKEGDIPOBJINHERIT(GpLineGradient, GpBrush)
DECLFAKEGDIPOBJ(GpImage)
DECLFAKEGDIPOBJ(GpImageAttributes)
DECLFAKEGDIPOBJ(GpEffect)
DECLFAKEGDIPOBJINHERIT(GpBitmap, GpImage)
DECLFAKEGDIPOBJ(GpRegion)
DECLFAKEGDIPOBJ(GpMatrix);
#undef DECLFAKEGDIPOBJ
#undef DECLFAKEGDIPOBJINHERIT

EXTERN_C_START
GpStatus WINGDIPAPI GdiplusStartup(ULONG_PTR* token, const GdiplusStartupInput* input, void* output);
void WINGDIPAPI GdiplusShutdown(ULONG_PTR token);
GpStatus WINGDIPAPI GdipCreateFromHDC(HDC hdc, GpGraphics** graphics);
GpStatus WINGDIPAPI GdipSetSmoothingMode(GpGraphics* graphics, GpSmoothingMode smoothingMode);
GpStatus WINGDIPAPI GdipSetTextRenderingHint(GpGraphics* graphics, GpTextRenderingHint mode);
GpStatus WINGDIPAPI GdipCreateFontFamilyFromName(GDIPCONST WCHAR* name, GpFontCollection* fontCollection, GpFontFamily** FontFamily);
GpStatus WINGDIPAPI GdipCreateFont(GDIPCONST GpFontFamily* fontFamily, REAL emSize, GpFontStyle style, GpUnit unit, GpFont** font);
GpStatus WINGDIPAPI GdipCreateStringFormat(int formatAttributes, LANGID language, GpStringFormat** format);
GpStatus WINGDIPAPI GdipSetStringFormatAlign(GpStringFormat* format, GpStringAlignment align);
GpStatus WINGDIPAPI GdipCreatePen1(ARGB color, REAL width, GpUnit unit, GpPen** pen);
GpStatus WINGDIPAPI GdipMeasureString(GpGraphics* graphics, GDIPCONST WCHAR* string, INT length, GDIPCONST GpFont* font,
	GDIPCONST GpRectF* layoutRect, GDIPCONST GpStringFormat* stringFormat, GpRectF* boundingBox, INT* codepointsFitted, INT* linesFilled);
GpStatus WINGDIPAPI GdipCreatePath(GpFillMode brushMode, GpPath** path);
GpStatus WINGDIPAPI GdipAddPathString(GpPath* path, GDIPCONST WCHAR* string, INT length, GDIPCONST GpFontFamily* family,
	GpFontStyle style, REAL emSize, GDIPCONST GpRectF* layoutRect, GDIPCONST GpStringFormat* format);
GpStatus WINGDIPAPI GdipCreateSolidFill(ARGB color, GpSolidFill** brush);
GpStatus WINGDIPAPI GdipFillPath(GpGraphics* graphics, GpBrush* brush, GpPath* path);
GpStatus WINGDIPAPI GdipDrawPath(GpGraphics* graphics, GpPen* pen, GpPath* path);
GpStatus WINGDIPAPI GdipCreateLineBrush(GDIPCONST GpPointF* point1, GDIPCONST GpPointF* point2, ARGB color1, ARGB color2,
	GpWrapMode wrapMode, GpLineGradient** lineGradient);
GpStatus WINGDIPAPI GdipFillRectangle(GpGraphics* graphics, GpBrush* brush, REAL x, REAL y, REAL width, REAL height);
GpStatus WINGDIPAPI GdipFillRectangleI(GpGraphics* graphics, GpBrush* brush, INT x, INT y, INT width, INT height);
GpStatus WINGDIPAPI GdipGraphicsClear(GpGraphics* graphics, ARGB color);
GpStatus WINGDIPAPI GdipDrawRectangle(GpGraphics* graphics, GpPen* pen, REAL x, REAL y, REAL width, REAL height);
GpStatus WINGDIPAPI GdipDrawRectangleI(GpGraphics* graphics, GpPen* pen, INT x, INT y, INT width, INT height);
GpStatus WINGDIPAPI GdipResetPath(GpPath* path);
GpStatus WINGDIPAPI GdipLoadImageFromStream(IStream* stream, GpImage** image);
GpStatus WINGDIPAPI GdipGetImageHeight(GpImage* image, UINT* height);
GpStatus WINGDIPAPI GdipGetImageWidth(GpImage* image, UINT* width);
GpStatus WINGDIPAPI GdipDrawImageRectRect(GpGraphics* graphics, GpImage* image, REAL dstx, REAL dsty, REAL dstwidth, REAL dstheight,
	REAL srcx, REAL srcy, REAL srcwidth, REAL srcheight, GpUnit srcUnit, GDIPCONST GpImageAttributes* imageAttributes,
	DrawImageAbort callback, VOID* callbackData);
GpStatus WINGDIPAPI GdipDisposeImage(GpImage* image);
GpStatus WINGDIPAPI GdipDeletePath(GpPath* path);
GpStatus WINGDIPAPI GdipDeletePen(GpPen* pen);
GpStatus WINGDIPAPI GdipDeleteBrush(GpBrush* brush);
GpStatus WINGDIPAPI GdipDeleteGraphics(GpGraphics* graphics);
GpStatus WINGDIPAPI GdipDeleteFont(GpFont* font);
GpStatus WINGDIPAPI GdipDeleteStringFormat(GpStringFormat* format);
GpStatus WINGDIPAPI GdipDeleteFontFamily(GpFontFamily* fontFamily);
GpStatus WINGDIPAPI GdipCreateEffect(const GUID guid, GpEffect** effect);
GpStatus WINGDIPAPI GdipDeleteEffect(GpEffect* effect);
GpStatus WINGDIPAPI GdipCreateBitmapFromHBITMAP(HBITMAP hbm, HPALETTE hpal, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipSetEffectParameters(GpEffect* effect, const VOID* params, const UINT size);
GpStatus WINGDIPAPI GdipCreateBitmapFromGraphics(INT width, INT height, GpGraphics* target, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipBitmapApplyEffect(GpBitmap* bitmap, GpEffect* effect, RECT* roi, BOOL useAuxData, VOID** auxData, INT* auxDataSize);
GpStatus WINGDIPAPI GdipCreateBitmapFromStream(IStream* stream, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipCloneBitmapArea(REAL x, REAL y, REAL width, REAL height, GpPixelFormat format, GpBitmap* srcBitmap, GpBitmap** dstBitmap);
GpStatus WINGDIPAPI GdipGetImageGraphicsContext(GpImage* image, GpGraphics** graphics);
GpStatus WINGDIPAPI GdipCreateBitmapFromScan0(INT width, INT height, INT stride, GpPixelFormat format, BYTE* scan0, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipLoadImageFromFile(GDIPCONST WCHAR* filename, GpImage** image);
GpStatus WINGDIPAPI GdipCreateHBITMAPFromBitmap(GpBitmap* bitmap, HBITMAP* hbmReturn, ARGB background);
GpStatus WINGDIPAPI GdipDrawImage(GpGraphics* graphics, GpImage* image, REAL x, REAL y);
GpStatus WINGDIPAPI GdipCreateHICONFromBitmap(GpBitmap* bitmap, HICON* hbmReturn);
GpStatus WINGDIPAPI GdipCreateBitmapFromHICON(HICON hicon, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipStringFormatGetGenericDefault(GpStringFormat** format);
GpStatus WINGDIPAPI GdipDrawString(GpGraphics* graphics, GDIPCONST WCHAR* string, INT length, GDIPCONST GpFont* font,
	GDIPCONST GpRectF* layoutRect, GDIPCONST GpStringFormat* stringFormat, GDIPCONST GpBrush* brush);
GpStatus WINGDIPAPI GdipSetStringFormatLineAlign(GpStringFormat* format, GpStringAlignment align);
GpStatus WINGDIPAPI GdipFillEllipse(GpGraphics* graphics, GpBrush* brush, REAL x, REAL y, REAL width, REAL height);
GpStatus WINGDIPAPI GdipFillPie(GpGraphics* graphics, GpBrush* brush, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
GpStatus WINGDIPAPI GdipCreateBitmapFromFile(GDIPCONST WCHAR* filename, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipDrawLine(GpGraphics* graphics, GpPen* pen, REAL x1, REAL y1, REAL x2, REAL y2);
GpStatus WINGDIPAPI GdipDrawLineI(GpGraphics* graphics, GpPen* pen, INT x1, INT y1, INT x2, INT y2);
GpStatus WINGDIPAPI GdipSaveImageToStream(GpImage* image, IStream* stream, GDIPCONST CLSID* clsidEncoder, GDIPCONST EncoderParameters* encoderParams);
GpStatus WINGDIPAPI GdipGetImageEncodersSize(UINT* numEncoders, UINT* size);
GpStatus WINGDIPAPI GdipGetImageEncoders(UINT numEncoders, UINT size, GpImageCodecInfo* encoders);
GpStatus WINGDIPAPI GdipDrawLinesI(GpGraphics* graphics, GpPen* pen, GDIPCONST GpPoint* points, INT count);
GpStatus WINGDIPAPI GdipDrawLines(GpGraphics* graphics, GpPen* pen, GDIPCONST GpPointF* points, INT count);
GpStatus WINGDIPAPI GdipDrawArc(GpGraphics* graphics, GpPen* pen, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
GpStatus WINGDIPAPI GdipDrawArcI(GpGraphics* graphics, GpPen* pen, INT x, INT y, INT width, INT height, REAL startAngle, REAL sweepAngle);
GpStatus WINGDIPAPI GdipDrawEllipseI(GpGraphics* graphics, GpPen* pen, INT x, INT y, INT width, INT height);
GpStatus WINGDIPAPI GdipDrawEllipse(GpGraphics* graphics, GpPen* pen, REAL x, REAL y, REAL width, REAL height);
GpStatus WINGDIPAPI GdipDrawPie(GpGraphics* graphics, GpPen* pen, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
GpStatus WINGDIPAPI GdipDrawPieI(GpGraphics* graphics, GpPen* pen, INT x, INT y, INT width, INT height, REAL startAngle, REAL sweepAngle);
GpStatus WINGDIPAPI GdipDrawPolygonI(GpGraphics* graphics, GpPen* pen, GDIPCONST GpPoint* points, INT count);
GpStatus WINGDIPAPI GdipDrawPolygon(GpGraphics* graphics, GpPen* pen, GDIPCONST GpPointF* points, INT count);
GpStatus WINGDIPAPI GdipFillEllipseI(GpGraphics* graphics, GpBrush* brush, INT x, INT y, INT width, INT height);
GpStatus WINGDIPAPI GdipFillEllipse(GpGraphics* graphics, GpBrush* brush, REAL x, REAL y, REAL width, REAL height);
GpStatus WINGDIPAPI GdipFillPie(GpGraphics* graphics, GpBrush* brush, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
GpStatus WINGDIPAPI GdipFillPieI(GpGraphics* graphics, GpBrush* brush, INT x, INT y, INT width, INT height, REAL startAngle, REAL sweepAngle);
GpStatus WINGDIPAPI GdipFillRegion(GpGraphics* graphics, GpBrush* brush, GpRegion* region);
GpStatus WINGDIPAPI GdipCreatePath2(GDIPCONST GpPointF* points, GDIPCONST BYTE* types, INT count, GpFillMode fillMode, GpPath** path);
GpStatus WINGDIPAPI GdipCreatePath2I(GDIPCONST GpPoint* points, GDIPCONST BYTE* types, INT count, GpFillMode fillMode, GpPath** path);
GpStatus WINGDIPAPI GdipBitmapSetResolution(GpBitmap* bitmap, REAL xdpi, REAL ydpi);
GpStatus WINGDIPAPI GdipGetDpiX(GpGraphics* graphics, REAL* dpi);
GpStatus WINGDIPAPI GdipGetDpiY(GpGraphics* graphics, REAL* dpi);
GpStatus WINGDIPAPI GdipDrawImageI(GpGraphics* graphics, GpImage* image, INT x, INT y);
GpStatus WINGDIPAPI GdipDrawImageRectI(GpGraphics* graphics, GpImage* image, INT x, INT y, INT width, INT height);
GpStatus WINGDIPAPI GdipDrawImageRect(GpGraphics* graphics, GpImage* image, REAL x, REAL y, REAL width, REAL height);
GpStatus WINGDIPAPI GdipDrawImageRectRectI(GpGraphics* graphics, GpImage* image, INT dstx, INT dsty, INT dstwidth, INT dstheight, INT srcx, INT srcy, INT srcwidth, INT srcheight, GpUnit srcUnit, 
	GDIPCONST GpImageAttributes* imageAttributes, DrawImageAbort callback, VOID* callbackData);
GpStatus WINGDIPAPI GdipGetPenWidth(GpPen* pen, REAL* width);
GpStatus WINGDIPAPI GdipCreateMatrix2(REAL m11, REAL m12, REAL m21, REAL m22, REAL dx, REAL dy, GpMatrix** matrix);
GpStatus WINGDIPAPI GdipCreateMatrix(GpMatrix** matrix);
GpStatus WINGDIPAPI GdipDeleteMatrix(GpMatrix* matrix);
GpStatus WINGDIPAPI GdipSetWorldTransform(GpGraphics* graphics, GpMatrix* matrix);
GpStatus WINGDIPAPI GdipResetWorldTransform(GpGraphics* graphics);
GpStatus WINGDIPAPI GdipMultiplyWorldTransform(GpGraphics* graphics, GDIPCONST GpMatrix* matrix, GpMatrixOrder order);
GpStatus WINGDIPAPI GdipCreateLineBrushI(GDIPCONST GpPoint* point1, GDIPCONST GpPoint* point2, ARGB color1, ARGB color2, GpWrapMode wrapMode, GpLineGradient** lineGradient);
GpStatus WINGDIPAPI GdipGetFontSize(GpFont* font, REAL* size);
GpStatus WINGDIPAPI GdipTranslateMatrix(GpMatrix* matrix, REAL offsetX, REAL offsetY, GpMatrixOrder order);
GpStatus WINGDIPAPI GdipScaleMatrix(GpMatrix* matrix, REAL scaleX, REAL scaleY, GpMatrixOrder order);
GpStatus WINGDIPAPI GdipRotateMatrix(GpMatrix* matrix, REAL angle, GpMatrixOrder order);
GpStatus WINGDIPAPI GdipShearMatrix(GpMatrix* matrix, REAL shearX, REAL shearY, GpMatrixOrder order);
GpStatus WINGDIPAPI GdipInvertMatrix(GpMatrix* matrix);
GpStatus WINGDIPAPI GdipGetMatrixElements(GDIPCONST GpMatrix* matrix, REAL* matrixOut);
GpStatus WINGDIPAPI GdipGetFontHeight(GDIPCONST GpFont* font, GDIPCONST GpGraphics* graphics, REAL* height);
GpStatus WINGDIPAPI GdipStringFormatGetGenericTypographic(GpStringFormat** format);
GpStatus WINGDIPAPI GdipBitmapLockBits(GpBitmap* bitmap, GDIPCONST GpRect* rect, UINT flags, GpPixelFormat format, GpBitmapData* lockedBitmapData);
GpStatus WINGDIPAPI GdipBitmapUnlockBits(GpBitmap* bitmap, GpBitmapData* lockedBitmapData);
GpStatus WINGDIPAPI GdipSaveImageToFile(GpImage* image, GDIPCONST WCHAR* filename, GDIPCONST CLSID* clsidEncoder, GDIPCONST EncoderParameters* encoderParams);
GpStatus WINGDIPAPI GdipGetImageDecoders(UINT numDecoders, UINT size, GpImageCodecInfo* decoders);
GpStatus WINGDIPAPI GdipGetImageDecodersSize(UINT* numDecoders, UINT* size);
GpStatus WINGDIPAPI GdipCreateFromHWND(HWND hwnd, GpGraphics** graphics);
GpStatus WINGDIPAPI GdipSetSolidFillColor(GpSolidFill* brush, ARGB color);
GpStatus WINGDIPAPI GdipSetPenColor(GpPen* pen, ARGB argb);
GpStatus WINGDIPAPI GdipSetInterpolationMode(GpGraphics* graphics, GpInterpolationMode interpolationMode);
GpStatus WINGDIPAPI GdipGetInterpolationMode(GpGraphics* graphics, GpInterpolationMode* interpolationMode);
GpStatus WINGDIPAPI
GdipCreateImageAttributes(GpImageAttributes** imageattr);

GpStatus WINGDIPAPI
GdipCloneImageAttributes(GDIPCONST GpImageAttributes* imageattr,
	GpImageAttributes** cloneImageattr);

GpStatus WINGDIPAPI
GdipDisposeImageAttributes(GpImageAttributes* imageattr);

GpStatus WINGDIPAPI
GdipSetImageAttributesToIdentity(GpImageAttributes* imageattr,
	GpColorAdjustType type);
GpStatus WINGDIPAPI
GdipResetImageAttributes(GpImageAttributes* imageattr,
	GpColorAdjustType type);

GpStatus WINGDIPAPI
GdipSetImageAttributesColorMatrix(GpImageAttributes* imageattr,
	GpColorAdjustType type,
	BOOL enableFlag,
	GDIPCONST GpColorMatrix* colorMatrix,
	GDIPCONST GpColorMatrix* grayMatrix,
	GpColorMatrixFlags flags);

GpStatus WINGDIPAPI
GdipSetImageAttributesThreshold(GpImageAttributes* imageattr,
	GpColorAdjustType type,
	BOOL enableFlag,
	REAL threshold);

GpStatus WINGDIPAPI
GdipSetImageAttributesGamma(GpImageAttributes* imageattr,
	GpColorAdjustType type,
	BOOL enableFlag,
	REAL gamma);

GpStatus WINGDIPAPI
GdipSetImageAttributesNoOp(GpImageAttributes* imageattr,
	GpColorAdjustType type,
	BOOL enableFlag);

GpStatus WINGDIPAPI
GdipSetImageAttributesColorKeys(GpImageAttributes* imageattr,
	GpColorAdjustType type,
	BOOL enableFlag,
	ARGB colorLow,
	ARGB colorHigh);

GpStatus WINGDIPAPI
GdipSetImageAttributesOutputChannelColorProfile(GpImageAttributes* imageattr,
	GpColorAdjustType type,
	BOOL enableFlag,
	GDIPCONST
	WCHAR* colorProfileFilename);

GpStatus WINGDIPAPI
GdipSetImageAttributesWrapMode(
	GpImageAttributes* imageAttr,
	GpWrapMode wrap,
	ARGB argb,
	BOOL clamp
);

GpStatus WINGDIPAPI
GdipSetImageAttributesICMMode(
	GpImageAttributes* imageAttr,
	BOOL on
);
EXTERN_C_END