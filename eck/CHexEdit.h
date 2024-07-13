#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CHexEdit : public CWnd
{
private:
	struct CHAR_COL
	{
		UINT uCp;
		COLORREF crCol;
	};
	int m_cLinePerRow{ 8 };
	int m_cColPerGroup{ 4 };

	SIZE_T m_cbData{};
	SIZE_T m_posFirstVisible{};

	BITBOOL m_bReadOnly : 1 = FALSE;
	BITBOOL m_bShowAddress : 1 = TRUE;
	BITBOOL m_bShowChar : 1 = TRUE;
	BITBOOL m_bShowHeader : 1 = TRUE;

	std::vector<CHAR_COL> m_vCharCol{};

	COLORREF m_crOddCol{};
	COLORREF m_crEvenCol{};
	COLORREF m_crAddressCol{};
public:
	
};
ECK_NAMESPACE_END