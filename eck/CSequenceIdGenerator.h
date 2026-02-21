#pragma once
#include "CSelectionRange.h"

ECK_NAMESPACE_BEGIN
template<class T>
class CSequenceIdGenerator
{
private:
	CSelectionRangeT<T> m_RgDeleted{};
	T m_Curr{};
public:
	constexpr T Generate() noexcept
	{
		if (m_RgDeleted.IsEmpty())
			return m_Curr++;
		else
		{
			const auto Ret = m_RgDeleted.GetList().front().idxBegin;
			m_RgDeleted.ExcludeItem(Ret);
			return Ret;
		}
	}

	constexpr void Free(T Id) noexcept
	{
		m_RgDeleted.IncludeItem(Id);
	}

	constexpr void Reset() noexcept
	{
		m_RgDeleted.Clear();
		m_Curr = T{};
	}
};
ECK_NAMESPACE_END