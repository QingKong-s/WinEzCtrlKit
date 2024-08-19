#pragma once
#include "ECK.h"
#include "Utility.h"
#include "CRefStr.h"
#include "CWnd.h"
#include "GraphicsHelper.h"
#include "CCommDlg.h"

#pragma push_macro("free")
#pragma push_macro("malloc")
#pragma push_macro("realloc")
#undef free
#undef malloc
#undef realloc
#include "YyJson\yyjson.h"
#pragma pop_macro("free")
#pragma pop_macro("malloc")
#pragma pop_macro("realloc")