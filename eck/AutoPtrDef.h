#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<class TDeleter>
using UniquePtr = std::unique_ptr<typename TDeleter::T, TDeleter>;

#define ECK_DECL_HANDLE_DELETER(Name, Type, DelFunc)	\
	struct Del##Name {									\
		using T = std::remove_pointer_t<Type>;			\
		void operator()(Type h) { DelFunc(h); }			\
	};													\
	using UnqPtr##Name = UniquePtr<Del##Name>;			\

ECK_DECL_HANDLE_DELETER(HImgList, HIMAGELIST, ImageList_Destroy);
ECK_DECL_HANDLE_DELETER(HIcon, HICON, DestroyIcon);
ECK_DECL_HANDLE_DELETER(HCursor, HCURSOR, DestroyCursor);
ECK_DECL_HANDLE_DELETER(HMenu, HMENU, DestroyMenu);
ECK_DECL_HANDLE_DELETER(HGdiObj, HGDIOBJ, DeleteObject);
ECK_DECL_HANDLE_DELETER(HNtObj, HANDLE, CloseHandle);// 不用NtClose，CloseHandle进行必要的无效检查
ECK_DECL_HANDLE_DELETER(HTheme, HTHEME, CloseThemeData);
ECK_DECL_HANDLE_DELETER(GpGraphics, GpGraphics*, GdipDeleteGraphics);
ECK_DECL_HANDLE_DELETER(GpBrush, GpBrush*, GdipDeleteBrush);
ECK_DECL_HANDLE_DELETER(GpPen, GpPen*, GdipDeletePen);
ECK_DECL_HANDLE_DELETER(GpPath, GpPath*, GdipDeletePath);
ECK_DECL_HANDLE_DELETER(GpFont, GpFont*, GdipDeleteFont);
ECK_DECL_HANDLE_DELETER(GpFontFamily, GpFontFamily*, GdipDeleteFontFamily);
ECK_DECL_HANDLE_DELETER(GpImage, GpImage*, GdipDisposeImage);
ECK_DECL_HANDLE_DELETER(GpBitmap, GpBitmap*, GdipDisposeImage);
ECK_DECL_HANDLE_DELETER(GpMetafile, GpMetafile*, GdipDisposeImage);
ECK_DECL_HANDLE_DELETER(GpRegion, GpRegion*, GdipDeleteRegion);
ECK_DECL_HANDLE_DELETER(GpStringFormat, GpStringFormat*, GdipDeleteStringFormat);
ECK_DECL_HANDLE_DELETER(GpPathIterator, GpPathIterator*, GdipDeletePathIter);
ECK_DECL_HANDLE_DELETER(GpCachedBitmap, GpCachedBitmap*, GdipDeleteCachedBitmap);
ECK_DECL_HANDLE_DELETER(GpImageAttributes, GpImageAttributes*, GdipDisposeImageAttributes);
ECK_DECL_HANDLE_DELETER(GpEffect, GpEffect*, GdipDeleteEffect);

template<class T_ = void>
struct DelMA
{
	using T = T_;
	void operator()(T* p) { free(p); }
};

template<class T = void>
using UnqPtrMA = UniquePtr<DelMA<T>>;
ECK_NAMESPACE_END