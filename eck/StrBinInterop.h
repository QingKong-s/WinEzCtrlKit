#pragma once
#include "CRefBin.h"
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
EckInline CRefStrW ToStr(const CRefBin& rb)
{
	CRefStrW rs((int)rb.Size() / 2);
	memcpy(rs.Data(), rb.Data(), rs.Size() * sizeof(WCHAR));
	return rs;
}
ECK_NAMESPACE_END