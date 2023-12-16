#include "Utility.h"
#include "CRefBin.h"
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
extern CRefStrW g_rsCurrDir;

const CRefStrW& GetRunningPath()
{
	return g_rsCurrDir;
}

RECT MakeRect(POINT pt1, POINT pt2)
{
	RECT rc;
	if (pt1.x >= pt2.x)
	{
		rc.left = pt2.x;
		rc.right = pt1.x;
	}
	else
	{
		rc.left = pt1.x;
		rc.right = pt2.x;
	}

	if (pt1.y >= pt2.y)
	{
		rc.top = pt2.y;
		rc.bottom = pt1.y;
	}
	else
	{
		rc.top = pt1.y;
		rc.bottom = pt2.y;
	}

	return rc;
}
ECK_NAMESPACE_END