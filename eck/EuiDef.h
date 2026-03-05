#pragma once
#include "UiElement.h"

#define ECK_EUI_NAMESPACE_BEGIN namespace Eui {
#define ECK_EUI_NAMESPACE_END   }

ECK_NAMESPACE_BEGIN
ECK_EUI_NAMESPACE_BEGIN
using namespace UiElement::Declaration;
enum : UINT
{
    DES_BACK_BUFFER = 1u << 31,
};

enum : UINT
{
    RDF_GDI = 1u << 0,
    RDF_GDIX = 1u << 1,
    RDF_TEXT_GDI = 1u << 2,
    RDF_TEXT_GDIX = 1u << 3,
    RDF_TEXT_DWRITE = 1u << 4,
    RDF_LAYERED = 1u << 5,
};

struct PAINTINFO
{
    RECT rcOldClip;
    RECT rcClipInClient;
};
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END