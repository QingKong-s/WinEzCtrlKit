#pragma once
#include "MediaTag.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
class CApe :public CTag
{
private:
	enum class ItemType
	{
		U8String,
		Binary,
		Link,
		Reserved
	};

	struct ITEM
	{
		ItemType eType;
		CRefStrA rsKey;
		std::variant<CRefStrW, CRefBin> Value;

		EckInline constexpr auto& GetStr() { return std::get<CRefStrW>(Value); }
		EckInline constexpr auto& GetBin() { return std::get<CRefBin>(Value); }

		EckInline constexpr const auto& GetStr() const { return std::get<CRefStrW>(Value); }
		EckInline constexpr const auto& GetBin() const { return std::get<CRefBin>(Value); }
	};

	SIZE_T m_cbTag{};
	APE_Header m_Hdr{};

	std::vector<ITEM> m_vItem{};

	ItemType GetItemType(UINT uFlags)
	{
		return (ItemType)GetLowNBits(uFlags >> 1, 2);
	}
public:
	CApe(CMediaFile& File) :CTag(File) {}

	Result ReadTag(UINT uFlags) override
	{
		m_Stream.MoveTo(m_File.m_Loc.posApeHdr) >> m_Hdr;
		if (!IsLegalApeHeader(m_Hdr))
			return Result::TagErr;
		BYTE* pBuf = (BYTE*)VAlloc(m_Hdr.cbBody);
		if (!pBuf)
			return Result::OutOfMemory;
		UniquePtrVA<BYTE> _(pBuf);
		m_Stream.MoveTo(m_File.m_Loc.posApe).Read(pBuf, m_Hdr.cbBody);
		CMemWalker w(pBuf, m_Hdr.cbBody);
		UINT cbVal, uItemFlags;

		CRefStrA u8{};
		EckCounterNV(m_Hdr.cItems)
		{
			ITEM e{};
			w >> cbVal >> uItemFlags;
			e.eType = GetItemType(uItemFlags);
			w >> e.rsKey;
			switch (e.eType)
			{
			case ItemType::Link:
			case ItemType::U8String:
				u8.ReSize(cbVal);
				w.Read(u8.Data(), cbVal);
				e.Value = StrU82W(u8);
				break;
			case ItemType::Reserved:
			case ItemType::Binary:
				e.Value = CRefBin(w.Data(), cbVal);
				w += cbVal;
				break;
			}
			m_vItem.emplace_back(std::move(e));
		}
	}

	Result WriteTag(UINT uFlags) override
	{
		return Result::Ok;
	}

	Result SimpleExtract(MUSICINFO& mi) override
	{
		return Result::Ok;
	}

	void Reset() override
	{
		m_vItem.clear();
	}
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END