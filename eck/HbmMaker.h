#pragma once
#include "ECK.h"
#include "ImageHelper.h"
#include "CSubclassMgr.h"

ECK_NAMESPACE_BEGIN
using CHbmRecorder = CObjRecorderRef<HBITMAP, ObjRecorderRefPlaceholder>;
extern CHbmRecorder g_HbmRecorder;

CHbmRecorder::TRecord g_HbmRecord;
CHbmRecorder g_HbmRecorder(g_HbmRecord, ObjRecordRefStdDeleter);


class CHbm
{
public:
	HBITMAP operator()(CRefBin& rb)
	{
		HBITMAP hbm = CreateHBITMAP(rb, rb.m_cb);
		if (!hbm)
			return NULL;
		g_HbmRecorder.AddRef(hbm, ObjRecorderRefPlaceholderVal);
		return hbm;
	}

	HBITMAP operator()(PCWSTR pszFile)
	{
		HBITMAP hbm = CreateHBITMAP(pszFile);
		if (!hbm)
			return NULL;
		g_HbmRecorder.AddRef(hbm, ObjRecorderRefPlaceholderVal);
		return hbm;
	}
};


ECK_NAMESPACE_END