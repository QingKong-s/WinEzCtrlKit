#pragma once
#define ECK_OPT_NO_DARKMODE 1
#include "eck\PchInc.h"
#include "eck\ConsoleIo.h"

using eck::PCVOID;
using eck::PCBYTE;

namespace Cio = eck::Cio;

extern Cio::CReader<char> ConIn;
extern Cio::CWriter<char> ConOut;
extern Cio::CWriter<char> ConErr;