#pragma once
#define _CRTDBG_MAP_ALLOC	1

#define ECK_OPT_NO_DARKMODE 1
#define ECK_OPT_NO_DX       1

#include "eck\PchInc.h"
#include "eck\ConsoleIo.h"

using eck::PCVOID;
using eck::PCBYTE;

namespace Cio = eck::Cio;

extern Cio::CReader<char> ConIn;
extern Cio::CWriter<char> ConOut;
extern Cio::CWriter<char> ConErr;