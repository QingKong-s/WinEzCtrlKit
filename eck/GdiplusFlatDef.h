/*
* WinEzCtrlKit Library
*
* GdiplusFlatDef.h £∫ GDI+Flat∂®“Â
*
* Copyright(C) 2023 QingKong
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
	Ok,
	GenericError,
	InvalidParameter,
	OutOfMemory,
	ObjectBusy,
	InsufficientBuffer,
	NotImplemented,
	Win32Error,
	WrongState,
	Aborted,
	FileNotFound,
	ValueOverflow,
	AccessDenied,
	UnknownImageFormat,
	FontFamilyNotFound,
	FontStyleNotFound,
	NotTrueTypeFont,
	UnsupportedGdiplusVersion,
	GdiplusNotInitialized,
	PropertyNotFound,
	PropertyNotSupported,
	ProfileNotFound
};
enum SmoothingMode
{
	SmoothingModeDefault,
	SmoothingModeHighSpeed,
	SmoothingModeHighQuality,
	SmoothingModeNone,
};
enum TextRenderingHint
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
enum StringAlignment
{
	StringAlignmentNear,
	StringAlignmentCenter,
	StringAlignmentFar
};
enum GpUnit
{
	UnitWorld,
	UnitDisplay,
	UnitPixel,
	UnitPoint,
	UnitInch,
	UnitDocument,
	UnitMillimeter
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
enum PixelFormat
{
	PixelFormatIndexed        = 0x00010000,
	PixelFormatGDI            = 0x00020000,
	PixelFormatAlpha          = 0x00040000,
	PixelFormatPAlpha         = 0x00080000,
	PixelFormatExtended       = 0x00100000,
	PixelFormatCanonical      = 0x00200000,

	PixelFormatUndefined      = 0,
	PixelFormatDontCare       = 0,

	PixelFormat1bppIndexed    = (1 | (1 << 8) | PixelFormatIndexed | PixelFormatGDI),
	PixelFormat4bppIndexed    = (2 | (4 << 8) | PixelFormatIndexed | PixelFormatGDI),
	PixelFormat8bppIndexed    = (3 | (8 << 8) | PixelFormatIndexed | PixelFormatGDI),
	PixelFormat16bppGrayScale = (4 | (16 << 8) | PixelFormatExtended),
	PixelFormat16bppRGB555    = (5 | (16 << 8) | PixelFormatGDI),
	PixelFormat16bppRGB565    = (6 | (16 << 8) | PixelFormatGDI),
	PixelFormat16bppARGB1555  = (7 | (16 << 8) | PixelFormatAlpha | PixelFormatGDI),
	PixelFormat24bppRGB       = (8 | (24 << 8) | PixelFormatGDI),
	PixelFormat32bppRGB       = (9 | (32 << 8) | PixelFormatGDI),
	PixelFormat32bppARGB      = (10 | (32 << 8) | PixelFormatAlpha | PixelFormatGDI | PixelFormatCanonical),
	PixelFormat32bppPARGB     = (11 | (32 << 8) | PixelFormatAlpha | PixelFormatPAlpha | PixelFormatGDI),
	PixelFormat48bppRGB       = (12 | (48 << 8) | PixelFormatExtended),
	PixelFormat64bppARGB      = (13 | (64 << 8) | PixelFormatAlpha | PixelFormatCanonical | PixelFormatExtended),
	PixelFormat64bppPARGB     = (14 | (64 << 8) | PixelFormatAlpha | PixelFormatPAlpha | PixelFormatExtended),
	PixelFormat32bppCMYK      = (15 | (32 << 8)),
	PixelFormatMax            = 16
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

struct GdiplusStartupInput
{
	UINT32 GdiplusVersion;
	void* DebugEventCallback;
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
struct BlurParams
{
	REAL radius;
	BOOL expandEdge;
};
struct ImageCodecInfo
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
GpStatus WINGDIPAPI GdipSetSmoothingMode(GpGraphics* graphics, SmoothingMode smoothingMode);
GpStatus WINGDIPAPI GdipSetTextRenderingHint(GpGraphics* graphics, TextRenderingHint mode);
GpStatus WINGDIPAPI GdipCreateFontFamilyFromName(GDIPCONST WCHAR* name, GpFontCollection* fontCollection, GpFontFamily** FontFamily);
GpStatus WINGDIPAPI GdipCreateFont(GDIPCONST GpFontFamily* fontFamily, REAL emSize, GpFontStyle style, GpUnit unit, GpFont** font);
GpStatus WINGDIPAPI GdipCreateStringFormat(int formatAttributes, LANGID language, GpStringFormat** format);
GpStatus WINGDIPAPI GdipSetStringFormatAlign(GpStringFormat* format, StringAlignment align);
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
GpStatus WINGDIPAPI GdipCloneBitmapArea(REAL x, REAL y, REAL width, REAL height, PixelFormat format, GpBitmap* srcBitmap, GpBitmap** dstBitmap);
GpStatus WINGDIPAPI GdipGetImageGraphicsContext(GpImage* image, GpGraphics** graphics);
GpStatus WINGDIPAPI GdipCreateBitmapFromScan0(INT width, INT height, INT stride, PixelFormat format, BYTE* scan0, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipLoadImageFromFile(GDIPCONST WCHAR* filename, GpImage** image);
GpStatus WINGDIPAPI GdipCreateHBITMAPFromBitmap(GpBitmap* bitmap, HBITMAP* hbmReturn, ARGB background);
GpStatus WINGDIPAPI GdipDrawImage(GpGraphics* graphics, GpImage* image, REAL x, REAL y);
GpStatus WINGDIPAPI GdipCreateHICONFromBitmap(GpBitmap* bitmap, HICON* hbmReturn);
GpStatus WINGDIPAPI GdipCreateBitmapFromHICON(HICON hicon, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipStringFormatGetGenericDefault(GpStringFormat** format);
GpStatus WINGDIPAPI GdipDrawString(GpGraphics* graphics, GDIPCONST WCHAR* string, INT length, GDIPCONST GpFont* font,
	GDIPCONST GpRectF* layoutRect, GDIPCONST GpStringFormat* stringFormat, GDIPCONST GpBrush* brush);
GpStatus WINGDIPAPI GdipSetStringFormatLineAlign(GpStringFormat* format, StringAlignment align);
GpStatus WINGDIPAPI GdipFillEllipse(GpGraphics* graphics, GpBrush* brush, REAL x, REAL y, REAL width, REAL height);
GpStatus WINGDIPAPI GdipFillPie(GpGraphics* graphics, GpBrush* brush, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
GpStatus WINGDIPAPI GdipCreateBitmapFromFile(GDIPCONST WCHAR* filename, GpBitmap** bitmap);
GpStatus WINGDIPAPI GdipDrawLine(GpGraphics* graphics, GpPen* pen, REAL x1, REAL y1, REAL x2, REAL y2);
GpStatus WINGDIPAPI GdipDrawLineI(GpGraphics* graphics, GpPen* pen, INT x1, INT y1, INT x2, INT y2);
GpStatus WINGDIPAPI GdipSaveImageToStream(GpImage* image, IStream* stream, GDIPCONST CLSID* clsidEncoder, GDIPCONST EncoderParameters* encoderParams);
GpStatus WINGDIPAPI GdipGetImageEncodersSize(UINT* numEncoders, UINT* size);
GpStatus WINGDIPAPI GdipGetImageEncoders(UINT numEncoders, UINT size, ImageCodecInfo* encoders);
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
EXTERN_C_END