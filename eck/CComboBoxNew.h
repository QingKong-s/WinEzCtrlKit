#pragma once
#include "CListBoxNew.h"
#include "CEditExt.h"

ECK_NAMESPACE_BEGIN
class CComboBoxNew :public CWnd
{
private:
	CListBoxNew m_LB{};
	CEditExt m_ED{};
public:

};
ECK_NAMESPACE_END