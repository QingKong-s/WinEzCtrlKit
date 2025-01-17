#pragma once
#include "Utility.h"
#include "CRefStr.h"
#include "CMsgBoxHook.h"
#include "GraphicsHelper.h"
#include "CObject.h"
#include "CSrwLock.h"

#pragma push_macro("free")
#pragma push_macro("malloc")
#pragma push_macro("realloc")
#undef free
#undef malloc
#undef realloc
#include "YyJson/yyjson.h"
#pragma pop_macro("free")
#pragma pop_macro("malloc")
#pragma pop_macro("realloc")