#pragma once
#include "MediaTag.h"
#include "SystemHelper.h"

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
		std::variant<std::vector<CRefStrW>, CRefBin> Value;
		CRefStrA rsU8Cache;

		void EnsureStrList()
		{
			if (eType != ItemType::U8String)
				Value.emplace<0>();
		}
		void EnsureBin()
		{
			if (eType != ItemType::Binary)
				Value.emplace<CRefBin>();
		}
		EckInlineNdCe auto& GetStrList() { return std::get<0>(Value); }
		EckInlineNdCe auto& GetStrList() const { return std::get<0>(Value); }
		EckInlineNdCe auto& GetBin() { return std::get<CRefBin>(Value); }
		EckInlineNdCe auto& GetBin() const { return std::get<CRefBin>(Value); }
	};
	struct IAMGE
	{
		PicType eType;
		eck::CRefStrA rsKey;
		eck::CRefStrW rsDesc;
		eck::CRefBin rbData;
		CRefStrA rsU8Cache;
	};

	SIZE_T m_cbTag{};
	APE_Header m_Hdr{};

	std::vector<ITEM> m_vItem{};
	std::vector<IAMGE> m_vImage{};

	static constexpr ItemType GetItemType(UINT uFlags) { return (ItemType)GetLowNBits(uFlags >> 1, 2); }
public:
	CApe(CMediaFile& mf) : CTag{ mf } {}

	Result ReadTag(UINT uFlags) override
	{
		m_Stream.MoveTo(m_File.m_Loc.posApeHdr) >> m_Hdr;
		if (!ThIsLegalApeHeader(m_Hdr))
			return Result::TagErr;
		BYTE* pBuf = (BYTE*)VAlloc(m_Hdr.cbBody);
		if (!pBuf)
			return Result::OutOfMemory;
		UniquePtr<DelVA<BYTE>> _{ pBuf };
		m_Stream.MoveTo(m_File.m_Loc.posApe).Read(pBuf, m_Hdr.cbBody);
		CMemWalker w(pBuf, m_Hdr.cbBody);
		UINT cbVal, uItemFlags;

		EckCounterNV(m_Hdr.cItems)
		{
			ITEM e{};
			w >> cbVal >> uItemFlags;
			e.eType = GetItemType(uItemFlags);
			w >> e.rsKey;
			if (e.rsKey.IsStartWithI(EckStrAndLen("Cover Art")))
			{
				PicType eType = PicType::Invalid;
				const auto posBracket0 = e.rsKey.FindChar('(');
				if (posBracket0 >= 0)
				{
					const auto cchType =
						e.rsKey.FindChar(')', posBracket0 + 1) - posBracket0 - 1;
					if (cchType > 0)
					{
						EckCounter(ARRAYSIZE(ApePicType), i)
						{
							const auto& sv = ApePicType[i];
							if (TcsEqualLen2I(sv.data(), sv.size(),
								e.rsKey.Data() + posBracket0 + 1, cchType))
								eType = (PicType)i;
						}
					}
				}
				const auto pBaseU8 = (PCCH)w.Data();
				const auto cchDesc = (int)TcsLen(pBaseU8);
				auto& f = m_vImage.emplace_back(eType, std::move(e.rsKey),
					StrU82W(pBaseU8, cchDesc), CRefBin(cbVal - cchDesc - 1));
				memcpy(f.rbData.Data(), w.Data() + cchDesc + 1, cbVal - cchDesc - 1);
				w += cbVal;
			}
			else if (e.eType == ItemType::Link ||
				e.eType == ItemType::U8String ||
				e.rsKey.CompareI(EckStrAndLen("TITLE")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("ARTIST")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("ALBUM")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("LYRICS")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("DESCRIPTION")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("GENRE")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("TRACKNUMBER")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("TRACK")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("TRACKTOTAL")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("DISCNUMBER")) == 0 ||
				e.rsKey.CompareI(EckStrAndLen("DISCTOTAL")) == 0)
			{
				const auto pBaseU8 = (PCCH)w.Data();
				w += cbVal;
				int pos0{}, pos1{};
				for (; pos1 < (int)cbVal; ++pos1)
				{
					if (pBaseU8[pos1] == '\0')
					{
						if (pos1 > pos0)
							e.GetStrList().emplace_back(
								StrU82W(pBaseU8 + pos0, pos1 - pos0));
						pos0 = pos1 + 1;
					}
				}
				if (pos1 > pos0)
					e.GetStrList().emplace_back(
						StrU82W(pBaseU8 + pos0, pos1 - pos0));
				m_vItem.emplace_back(std::move(e));
			}
			else
			{
				e.EnsureBin();
				e.GetBin().ReSize(cbVal);
				memcpy(e.GetBin().Data(), w.Data(), cbVal);
				w += cbVal;
				m_vItem.emplace_back(std::move(e));
			}
		}
		return Result::Ok;
	}

	Result WriteTag(UINT uFlags = MIF_APPEND_TAG) override
	{
		// 提前计算固定8字节+1字节键结束符大小
		SIZE_T cbBody{ (m_vItem.size() + m_vImage.size()) * (4 + 4 + 1) };
		for (auto& e : m_vItem)
		{
			cbBody += e.rsKey.Size();
			switch (e.eType)
			{
			case ItemType::Link:
			case ItemType::U8String:
			{
				e.rsU8Cache.Clear();
				BOOL bAppendNull{};
				for (auto& s : e.GetStrList())
				{
					if (bAppendNull)
						e.rsU8Cache.PushBackChar('\0');
					StrW2U8(s.Data(), s.Size(), e.rsU8Cache);
					bAppendNull = TRUE;
				}
				cbBody += e.rsU8Cache.Size();
			}
			break;
			case ItemType::Reserved:
			case ItemType::Binary:
				cbBody += e.GetBin().Size();
				break;
			default:
				ECK_UNREACHABLE;
			}
		}
		for (auto& e : m_vImage)
		{
			e.rsU8Cache.Clear();
			StrW2U8(e.rsDesc.Data(), e.rsDesc.Size(), e.rsU8Cache);
			cbBody += (e.rsKey.Size() + e.rsU8Cache.Size() + 1 + e.rbData.Size());
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
					if ((uFlags & MIF_ALLOW_PADDING) &&
						cbPadding > 4 + 4 + 5/*Dummy*/ + 1 &&
						cbPadding <= 1024)
						cbPadding -= (4 + 4 + 5/*Dummy*/ + 1);
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
			{
				m_Stream << (DWORD)e.rsU8Cache.Size() << dwFlag << e.rsKey;
				m_Stream.Write(e.rsU8Cache.Data(), e.rsU8Cache.Size());
			}
			break;
			case ItemType::Reserved:
			case ItemType::Binary:
				m_Stream << (DWORD)e.GetBin().Size() << dwFlag << e.rsKey << e.GetBin();
				break;
			}
		}
		for (const auto& e : m_vImage)
		{
			const auto dwFlag = (DWORD)ItemType::Binary << 1;
			m_Stream << DWORD(e.rbData.Size() + e.rsU8Cache.Size() + 1)
				<< dwFlag << e.rsKey
				<< e.rsU8Cache << e.rbData;
		}

		if (cbPadding)
		{
			UniquePtr<DelVA<void>> p{ VAlloc(cbPadding) };
			m_Stream << (DWORD)cbPadding
				<< ((DWORD)ItemType::Binary << 1)
				<< "Dummy"sv;
			m_Stream.Write(p.get(), cbPadding);
		}

		m_Stream << Hdr;
		m_Stream->Commit(STGC_DEFAULT);
		return Result::Ok;
	}

	Result SimpleGetSet(MUSICINFO& mi, const SIMPLE_OPT& Opt) override
	{
#undef EckPriv_GetSetVal
#define EckPriv_GetSetVal(MiField)		\
		if (bSet)						\
		{								\
			e.GetStrList().resize(1);	\
			if (bMove){					\
				e.GetStrList().emplace_back(std::move(MiField));\
				MiField.Clear();		\
			}else						\
				e.GetStrList().emplace_back(MiField);	\
		}								\
		else if (!e.GetStrList().empty())				\
			if (bMove)					\
				MiField = std::move(e.GetStrList()[0]);	\
			else						\
				MiField = e.GetStrList()[0];

		mi.Clear();
		if (m_File.m_Loc.posApeHdr == SIZETMax)
			return Result::NoTag;
		const auto bSet = Opt.uFlags & SMOF_SET;
		const auto bMove = Opt.uFlags & SMOF_MOVE;
		size_t idxCurrImg{};
		for (size_t i{}; auto& e : m_vItem)
		{
			if ((mi.uMask & MIM_TITLE) && e.rsKey.CompareI(EckStrAndLen("TITLE")) == 0)
			{
				EckPriv_GetSetVal(mi.rsTitle);
				mi.uMaskChecked |= MIM_TITLE;
			}
			else if ((mi.uMask & MIM_ARTIST) && e.rsKey.CompareI(EckStrAndLen("ARTIST")) == 0)
			{
				if (bSet)
				{
					e.GetStrList().clear();
					for (const auto f : mi.slArtist)
						e.GetStrList().emplace_back(f);
				}
				else
					for (const auto& f : e.GetStrList())
						mi.slArtist.PushBackString(f, Opt.svArtistDiv);
				mi.uMaskChecked |= MIM_ARTIST;
			}
			else if ((mi.uMask & MIM_ALBUM) && e.rsKey.CompareI(EckStrAndLen("ALBUM")) == 0)
			{
				EckPriv_GetSetVal(mi.rsAlbum);
				mi.uMaskChecked |= MIM_ALBUM;
			}
			else if ((mi.uMask & MIM_LRC) && e.rsKey.CompareI(EckStrAndLen("LYRICS")) == 0)
			{
				EckPriv_GetSetVal(mi.rsLrc);
				mi.uMaskChecked |= MIM_LRC;
			}
			else if ((mi.uMask & MIM_COMMENT) && e.rsKey.CompareI(EckStrAndLen("DESCRIPTION")) == 0)
			{
				if (bSet)
				{
					e.GetStrList().clear();
					for (const auto f : mi.slComment)
						e.GetStrList().emplace_back(f);
				}
				else
					for (const auto& f : e.GetStrList())
						mi.slComment.PushBackString(f, Opt.svCommDiv);
				mi.uMaskChecked |= MIM_COMMENT;
			}
			else if ((mi.uMask & MIM_GENRE) && e.rsKey.CompareI(EckStrAndLen("GENRE")) == 0)
			{
				EckPriv_GetSetVal(mi.rsGenre);
				mi.uMaskChecked |= MIM_GENRE;
			}
			else if (mi.uMask & MIM_TRACK)
			{
				if (e.rsKey.CompareI("TRACKNUMBER") || e.rsKey.CompareI("TRACK"))
				{
					if (bSet)
						e.GetStrList().resize(1);
					ThGetSetNumAndTotalNum(bSet, mi.uFlag & MIF_WRITE_TRACK_TOTAL,
						mi.nTrack, mi.cTotalTrack, e.GetStrList().front());
					mi.uMaskChecked |= MIM_TRACK;
				}
				else if (e.rsKey.CompareI("TRACKTOTAL"))
				{
					if (bSet)
					{
						e.GetStrList().resize(1);
						e.GetStrList().front().Format(L"%d", mi.cTotalTrack);
					}
					else
					{
						const auto& rs = e.GetStrList().front();
						TcsToInt(rs.Data(), rs.Size(), mi.cTotalTrack);
					}
					mi.uMaskChecked |= MIM_TRACK;
				}
			}
			else if (mi.uMask & MIM_DISC)
			{
				if (e.rsKey.CompareI("DISCNUMBER"))
				{
					if (bSet)
						e.GetStrList().resize(1);
					ThGetSetNumAndTotalNum(bSet, mi.uFlag & MIF_WRITE_DISC_TOTAL,
						mi.nDisc, mi.cTotalDisc, e.GetStrList().front());
					mi.uMaskChecked |= MIM_DISC;
				}
				else if (e.rsKey.CompareI("DISCTOTAL"))
				{
					if (bSet)
					{
						e.GetStrList().resize(1);
						e.GetStrList().front().Format(L"%d", mi.cTotalDisc);
					}
					else
					{
						const auto& rs = e.GetStrList().front();
						TcsToInt(rs.Data(), rs.Size(), mi.cTotalDisc);
					}
					mi.uMaskChecked |= MIM_DISC;
				}
			}
			++i;
		}
		if (mi.uMask & MIM_COVER)
			if (bSet)
			{
				m_vImage.clear();
				for (auto& e : mi.vImage)
				{
					const auto eType = ((e.eType == PicType::Invalid) &&
						(mi.uFlag & MIF_APE_INVALID_COVER_AS_FRONT)) ?
						PicType::CoverFront : e.eType;
					CRefBin rbData{};
					if (e.bLink)
					{
						rbData = ReadInFile(e.GetPicPath().Data());
						if (rbData.IsEmpty())
							continue;
					}
					else
					{
						if (bMove)
							rbData = std::move(e.GetPicData());
						else
						{
							rbData.ReSize(e.GetPicData().Size());
							memcpy(rbData.Data(), e.GetPicData().Data(), e.GetPicData().Size());
						}
					}
					m_vImage.emplace_back(eType, PicTypeToApeString(eType),
						e.rsDesc, std::move(rbData));
				}
				mi.uMaskChecked |= MIM_COVER;
			}
			else
			{
				for (auto& e : m_vImage)
				{
					if (bMove)
						mi.vImage.emplace_back(e.eType, FALSE,
							std::move(e.rsDesc), CRefStrA{}, std::move(e.rbData));
					else
					{
						auto& f = mi.vImage.emplace_back(e.eType, FALSE,
							e.rsDesc, CRefStrA{}, CRefBin(e.rbData.Size()));
						memcpy(f.GetPicData().Data(),
							e.rbData.Data(), e.rbData.Size());
					}
				}
				mi.uMaskChecked |= MIM_COVER;
			}
		return Result::Ok;
	}

	void Reset() override { m_vItem.clear(); }

	EckInline constexpr auto& GetItemList() { return m_vItem; }
	EckInline constexpr const auto& GetItemList() const { return m_vItem; }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END