/*
* WinEzCtrlKit Library
*
* MediaTagApe.h £º Ape¶ÁÐ´
*
* Copyright(C) 2024 QingKong
*/
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
		CRefStrA rsU8Cache;

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

	Result WriteTag(UINT uFlags = MIF_APPEND_TAG) override
	{
		SIZE_T cbBody{ m_vItem.size() * (4 + 4 + 1) };
		for (auto& e : m_vItem)
		{
			cbBody += e.rsKey.Size();
			switch (e.eType)
			{
			case ItemType::Link:
			case ItemType::U8String:
				e.rsU8Cache = StrW2U8(e.GetStr());
				cbBody += e.rsU8Cache.Size();
				break;
			case ItemType::Reserved:
			case ItemType::Binary:
				cbBody += e.GetBin().Size();
				break;
			default:
				ECK_UNREACHABLE;
			}
		}

		APE_Header Hdr;
		memcpy(Hdr.byPreamble, "APETAGEX", 8);
		Hdr.dwVer = 2000;
		Hdr.cbBody = DWORD(cbBody + 32);
		Hdr.cItems = (DWORD)m_vItem.size();
		Hdr.dwFlags = APE_HAS_HEADER | APE_HAS_FOOTER;
		ZeroMemory(Hdr.byReserved, sizeof(Hdr.byReserved));

		const SIZE_T cbTotal = Hdr.cbBody + 32;

		SIZE_T cbPadding{};
		if (m_File.m_Loc.posApeTag != SIZETMax)
			if (m_File.m_Loc.bPrependApe != !(uFlags & MIF_APPEND_TAG))
			{
				m_Stream.Erase(m_File.m_Loc.posApeTag, m_File.m_Loc.cbApeTag);
				goto NewTag;
			}
			else
			{
				if (m_File.m_Loc.cbApeTag < cbTotal)
					m_Stream.Insert(m_File.m_Loc.posApeTag + m_File.m_Loc.cbApeTag,
						cbTotal - m_File.m_Loc.cbApeTag);
				else
				{
					cbPadding = m_File.m_Loc.cbApeTag - cbTotal;
					if ((uFlags & MIF_ALLOW_PADDING) && cbPadding > 4 + 4 + 5/*dummy*/ + 1 && cbPadding <= 1024)
						cbPadding -= (4 + 4 + 5/*dummy*/ + 1);
					else
					{
						m_Stream.Erase(m_File.m_Loc.posApeTag + m_File.m_Loc.cbApeTag, cbPadding);
						cbPadding = 0;
					}
				}
				m_Stream.MoveTo(m_File.m_Loc.posApeTag);
			}
		else
		{
		NewTag:
			if (uFlags & MIF_APPEND_TAG)
			{
				SSIZE_T pos{};
				if (m_File.m_Loc.posV1Ext != SIZETMax)
					pos = -(SSIZE_T)m_File.m_Loc.posV1Ext;
				else if (m_File.m_Loc.posV1 != SIZETMax)
					pos = -(SSIZE_T)m_File.m_Loc.posV1;
				m_Stream.Insert(pos, cbTotal);
				m_Stream.MoveTo(pos);
			}
			else
			{
				m_Stream.Insert(0, cbTotal);
				m_Stream.MoveToBegin();
			}
		}

		Hdr.dwFlags |= APE_HEADER;
		m_Stream << Hdr;
		Hdr.dwFlags &= ~APE_HEADER;

		for (const auto& e : m_vItem)
		{
			const auto dwFlag = (DWORD)e.eType << 1;
			switch (e.eType)
			{
			case ItemType::Link:
			case ItemType::U8String:
				m_Stream << (DWORD)e.GetStr().Size() << dwFlag << e.rsKey;
				m_Stream.Write(e.rsU8Cache.Data(), e.rsU8Cache.Size());
				break;
			case ItemType::Reserved:
			case ItemType::Binary:
				m_Stream << (DWORD)e.GetBin().Size() << dwFlag << e.rsKey << e.GetBin();
				break;
			}
		}

		if (cbPadding)
		{
			UniquePtrVA<void> p(VAlloc(cbPadding));
			m_Stream << (DWORD)cbPadding << ((DWORD)ItemType::Binary << 1) << "Dummy";
			m_Stream.Write(p.get(), cbPadding);
		}

		m_Stream << Hdr;
		m_Stream->Commit(STGC_DEFAULT);
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

	EckInline constexpr auto& GetItems() { return m_vItem; }

	EckInline constexpr const auto& GetItems() const { return m_vItem; }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END