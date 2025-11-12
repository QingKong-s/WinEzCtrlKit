#include "pch.h"
#include "CProject.h"

std::shared_ptr<DsForm> CProject::RsAddForm(std::wstring_view svName)
{
	auto& p = m_vForm.emplace_back(std::make_shared<DsForm>());
	p->rsName = svName;
	return p;
}