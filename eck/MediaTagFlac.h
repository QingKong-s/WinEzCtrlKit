/*
* WinEzCtrlKit Library
*
* MediaTagFlac.h ： Flac元数据读写
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "MediaTag.h"
#include "StrBinInterop.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
class CFlac :public CTag
{
public:
	struct ITEM
	{
		CRefStrA rsKey;
		CRefStrW rsValue;
	};

	enum class BlockType : BYTE
	{
		StreamInfo = 0,
		Padding = 1,
		Application = 2,
		SeekTable = 3,
		VorbisComment = 4,
		CueSheet = 5,
		Picture = 6,
		Invalid = 127,
	};

	struct BLOCK
	{
		BlockType eType;
		CRefBin rbData;
	};

	struct STREAMINFO
	{
		USHORT cBlockSampleMin;
		USHORT cBlockSampleMax;
		DWORD cbMinFrame;
		DWORD cbMaxFrame;
		DWORD cSampleRate;
		BYTE cChannels;
		BYTE cBitsPerSample;
		ULONGLONG ullTotalSamples;
		BYTE Md5[16];
	};

	struct IMAGE
	{
		PicType eType{};
		CRefStrA rsMime{};
		CRefStrW rsDesc{};
		CRefBin rbData{};
		UINT cx;
		UINT cy;
		UINT bpp;
		UINT cColor;
	};
private:
	std::vector<ITEM> m_vItem{};	// 所有Vorbis注释
	std::vector<IMAGE> m_vPic{};	// 所有图片
	std::vector<BLOCK> m_vBlock{};	// 其他块，STREAMINFO、Vorbis注释、图片和填充除外
	STREAMINFO m_si{};				// Flac流信息

	CRefStrW m_rsVendor{};

	SIZE_T m_posStreamInfoEnd{ SIZETMax };
	SIZE_T m_posFlacTagEnd{ SIZETMax };

	static void ParseImageBlock(CRefBin& rb, IMAGE& Pic)
	{
		CMemWalker r(rb.Data(), rb.Size());
		UINT t;
		UINT dwType;
		r >> dwType;// 图片类型
		if (dwType < (BYTE)PicType::Begin___ || dwType >= (BYTE)PicType::End___)
			Pic.eType = PicType::Invalid;
		else
			Pic.eType = (PicType)dwType;

		r.ReadRev(t);// 长度
		Pic.rsMime.ReSize(t);
		r.Read(Pic.rsMime.Data(), t);// MIME类型字符串

		r.ReadRev(t);// 描述字符串长度
		CRefStrA u8Desc(t);
		r.Read(u8Desc.Data(), t);// MIME类型字符串
		Pic.rsDesc = StrX2W(u8Desc, CP_UTF8);

		r.ReadRev(Pic.cx).ReadRev(Pic.cy).ReadRev(Pic.bpp).ReadRev(Pic.cColor);

		r.ReadRev(t);// 图片数据长度

		rb.Erase(0, r.Data() - rb.Data());
		Pic.rbData = std::move(rb);
	}

	Result InitForWriteTag()
	{
		if (m_File.m_Id3Loc.posFlac == SIZETMax)
			return Result::NoTag;
		m_Stream.MoveTo(m_File.m_Id3Loc.posFlac) += 4;
		FLAC_BlockHeader Header;
		DWORD cbBlock;
		UINT t;
		do
		{
			m_Stream >> Header;
			cbBlock = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
			if (!cbBlock)
				return Result::LenErr;
			switch (Header.by & 0x7F)
			{
			case 0:// 流信息
				m_Stream += cbBlock;
				m_posStreamInfoEnd = m_Stream.GetPos();
			break;
			default:
				m_Stream += cbBlock;
			break;
			}
		} while (!(Header.by & 0x80));
		m_posFlacTagEnd = m_Stream.GetPos();
		return Result::Ok;
	}
public:
	ECK_DISABLE_COPY_MOVE(CFlac)
public:
	CFlac(CMediaFile& File) :CTag(File) {}

	Result SimpleExtract(MUSICINFO& mi) override
	{
		for (const auto& e : m_vItem)
		{
			if ((mi.uFlag & MIM_TITLE) && e.rsKey.CompareI( "TITLE"))
			{
				mi.rsTitle = e.rsValue;
				mi.uMaskRead |= MIM_TITLE;
			}
			else if ((mi.uFlag & MIM_ARTIST) && e.rsKey.CompareI("ARTIST"))
			{
				mi.AppendArtist(e.rsValue);
				mi.uMaskRead |= MIM_ARTIST;
			}
			else if ((mi.uFlag & MIM_ALBUM) && e.rsKey.CompareI("ALBUM"))
			{
				mi.rsAlbum = e.rsValue;
				mi.uMaskRead |= MIM_ALBUM;
			}
			else if ((mi.uFlag & MIM_LRC) && e.rsKey.CompareI("LYRICS"))
			{
				mi.rsLrc = e.rsValue;
				mi.uMaskRead |= MIM_LRC;
			}
			else if ((mi.uFlag & MIM_COMMENT) && e.rsKey.CompareI("DESCRIPTION"))
			{
				mi.AppendComment(e.rsValue);
				mi.uMaskRead |= MIM_COMMENT;
			}
			else if ((mi.uFlag & MIM_GENRE) && e.rsKey.CompareI("GENRE"))
			{
				mi.rsGenre = e.rsValue;
				mi.uMaskRead |= MIM_GENRE;
			}
			else if ((mi.uFlag & MIM_DATE) && e.rsKey.CompareI("DATE"))
			{
				WORD y, m{}, d{};
				if (swscanf(e.rsValue.Data(), L"%hd-%hd-%hd", &y, &m, &d) >= 1)
				{
					mi.Date = SYSTEMTIME{ .wYear = y,.wMonth = m,.wDay = d };
					mi.uMaskRead |= MIM_DATE;
				}
			}
		}
		if ((mi.uFlag & MIM_COVER) && !m_vPic.empty())
		{
			for (const auto& e : m_vPic)
			{
				const BOOL bLink = (e.rsMime == "-->");
				if (bLink)
					mi.vImage.emplace_back(e.eType, TRUE, e.rsDesc, e.rsMime,
						StrX2W((PCSTR)e.rbData.Data(), (int)e.rbData.Size(), CP_UTF8));
				else ECKLIKELY
					mi.vImage.emplace_back(e.eType, FALSE, e.rsDesc, e.rsMime,
						e.rbData);
			}
			mi.uMaskRead |= MIM_COVER;
		}
		return Result::Ok;
	}

	Result ReadTag(UINT uFlags) override
	{
		if (m_File.m_Id3Loc.posFlac == SIZETMax)
			return Result::NoTag;
		m_Stream.MoveTo(m_File.m_Id3Loc.posFlac) += 4;
		FLAC_BlockHeader Header;
		DWORD cbBlock;
		UINT t;
		do
		{
			m_Stream >> Header;
			cbBlock = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
			if (!cbBlock)
				return Result::LenErr;
			switch (Header.by & 0x7F)
			{
			case 0:// 流信息
			{
				m_Stream.ReadRev(&m_si.cBlockSampleMin, 2).ReadRev(&m_si.cBlockSampleMax, 2)
					.ReadRev(&m_si.cbMinFrame, 3).Read(&m_si.cbMaxFrame, 3);
				ULONGLONG ull;
				m_Stream >> ull;
				m_si.cSampleRate = (UINT)GetLowNBits(ull, 20);
				m_si.cChannels = (BYTE)GetLowNBits(ull >> 20, 3) + 1;
				m_si.cBitsPerSample = (BYTE)GetLowNBits(ull >> 23, 5) + 1;
				m_si.ullTotalSamples = GetHighNBits(ull, 36);
				m_Stream.Read(m_si.Md5, 16);
				m_posStreamInfoEnd = m_Stream.GetPos();
			}
			break;
			case 4:// Vorbis注释（小端）
			{
				m_Stream >> t;// 编码器信息大小
				CRefStrA u8(t);
				m_Stream.Read(u8.Data(), t);

				m_rsVendor = StrX2W(u8, CP_UTF8);

				UINT cItem;
				m_Stream >> cItem;// 标签数量

				EckCounterNV(cItem)
				{
					m_Stream >> t;// 标签大小
					u8.ReSize(t);
					m_Stream.Read(u8.Data(), t);// 读标签
					int iPos = u8.Find("=");// 找等号
					if (iPos == StrNPos || iPos == 0 || iPos == u8.Size() - 1)
						continue;
					++iPos;
					const int cchActual = u8.Size() - iPos;
					if (u8.IsStartOfI("METADATA_BLOCK_PICTURE"))
					{
						auto rb = Base64Decode(u8.Data() + iPos, cchActual);
						ParseImageBlock(rb, m_vPic.emplace_back());
					}
					else ECKLIKELY
					{
						m_vItem.emplace_back(
							u8.SubStr(0, iPos - 1),
							StrX2W(u8.Data() + iPos, cchActual, CP_UTF8));
					}
				}
			}
			break;
			case 6:// 图片
			{
				CRefBin rb(cbBlock);
				m_Stream.Read(rb.Data(), cbBlock);
				ParseImageBlock(rb, m_vPic.emplace_back());
			}
			break;
			default:
			{
				if ((Header.by & 0x7F) == (BYTE)BlockType::Padding)
					m_Stream += cbBlock;
				else
				{
					CRefBin rb(cbBlock);
					m_Stream.Read(rb.Data(), cbBlock);
					m_vBlock.emplace_back((BlockType)(Header.by & 0x7F), std::move(rb));
				}
			}
			break;
			}
		} while (!(Header.by & 0x80));
		m_posFlacTagEnd = m_Stream.GetPos();
		return Result::Ok;
	}

	Result WriteTag(UINT uFlags) override
	{
		if (m_File.m_Id3Loc.posFlac == SIZETMax)
			return Result::NoTag;
		if (m_posFlacTagEnd == SIZETMax || m_posStreamInfoEnd == SIZETMax)
			InitForWriteTag();
		if (m_posFlacTagEnd == SIZETMax || m_posStreamInfoEnd == SIZETMax)
			return Result::TagErr;
		CRefBin rbVorbis{}, rbImage{};
		// 序列化Vorbis注释
		rbVorbis.PushBack(4u);// 悬而未决
		if (m_rsVendor.IsEmpty())
			rbVorbis << 0u;
		else
		{
			const int cchVendor = WideCharToMultiByte(CP_UTF8, 0, m_rsVendor.Data(), m_rsVendor.Size(),
				NULL, 0, NULL, NULL);
			rbVorbis << cchVendor;
			WideCharToMultiByte(CP_UTF8, 0, m_rsVendor.Data(), m_rsVendor.Size(),
				(CHAR*)rbVorbis.PushBack(cchVendor), cchVendor, NULL, NULL);
		}
		rbVorbis << (UINT)m_vItem.size();
		for (const auto& e : m_vItem)
		{
			if (e.rsKey.IsEmpty())
				continue;
			const int cchValue = WideCharToMultiByte(CP_UTF8, 0, e.rsValue.Data(), e.rsValue.Size(),
				NULL, 0, NULL, NULL);
			rbVorbis << UINT(cchValue + 1 + e.rsKey.Size())
				<< e.rsKey;
			rbVorbis.Back() = '=';
			WideCharToMultiByte(CP_UTF8, 0, e.rsValue.Data(), e.rsValue.Size(),
				(CHAR*)rbVorbis.PushBack(cchValue), cchValue, NULL, NULL);
		}
		const auto cbData = (UINT)rbVorbis.Size() - 4;
		auto phdr = (FLAC_BlockHeader*)rbVorbis.Data();
		phdr->bySize[0] = GetIntegerByte<2>(cbData);
		phdr->bySize[1] = GetIntegerByte<1>(cbData);
		phdr->bySize[2] = GetIntegerByte<0>(cbData);
		phdr->by = (BYTE)BlockType::VorbisComment;
		// 序列化图片
		size_t cbImageGuess{};
		for (const auto& e : m_vPic)
			cbImageGuess += (e.rsMime.Size() + e.rsDesc.Size() * 2 + e.rbData.Size() + 40);
		rbImage.Reserve(cbImageGuess);
		for (const auto& e : m_vPic)
		{
			const auto cbCurr = rbImage.Size();
			rbImage.PushBack(4u);// 悬而未决
			rbImage << ReverseInteger((UINT)e.eType)
				<< ReverseInteger(e.rsMime.Size())
				<< e.rsMime;
			rbImage.PopBack(1);
			const int cchDesc = WideCharToMultiByte(CP_UTF8, 0, e.rsDesc.Data(), e.rsDesc.Size(),
				NULL, 0, NULL, NULL);
			rbImage << ReverseInteger(cchDesc);
			WideCharToMultiByte(CP_UTF8, 0, e.rsDesc.Data(), e.rsDesc.Size(),
				(CHAR*)rbImage.PushBack(cchDesc), cchDesc, NULL, NULL);
			rbImage << ReverseInteger(e.cx)
				<< ReverseInteger(e.cy)
				<< ReverseInteger(e.bpp)
				<< ReverseInteger(e.cColor)
				<< ReverseInteger((UINT)e.rbData.Size())
				<< e.rbData;
			const auto cbData = (UINT)(rbImage.Size() - cbCurr - 4);
			const auto phdr = (FLAC_BlockHeader*)(rbImage.Data() + cbCurr);
			phdr->bySize[0] = GetIntegerByte<2>(cbData);
			phdr->bySize[1] = GetIntegerByte<1>(cbData);
			phdr->bySize[2] = GetIntegerByte<0>(cbData);
			phdr->by = (BYTE)BlockType::Picture;
		}
		auto cb = rbVorbis.Size() + rbImage.Size();
		for (const auto& e : m_vBlock)
			cb += (e.rbData.Size() + 4);
		// 写入
		const auto cbAvailable = m_posFlacTagEnd - m_posStreamInfoEnd;
		BOOL bHasEnd{};
		if (cbAvailable > cb)
		{
			UINT cbPadding = (UINT)(cbAvailable - cb);
			if ((uFlags & MIF_REMOVE_PADDING) || cbPadding > 1024)
				m_Stream.Erase(m_posStreamInfoEnd + cb, cbPadding);
			else
			{
				bHasEnd = TRUE;
				cbPadding -= 4;
				m_Stream.MoveTo(m_posStreamInfoEnd + cb)
					<< BYTE(0b1000'0000 | (BYTE)BlockType::Padding)// 这种情况下填充肯定为最后一个块
					<< GetIntegerByte<2>(cbPadding)
					<< GetIntegerByte<1>(cbPadding)
					<< GetIntegerByte<0>(cbPadding);
				void* p = VirtualAlloc(NULL, cbPadding, MEM_COMMIT, PAGE_READWRITE);
				EckCheckMem(p);
				m_Stream.Write(p, cbPadding);
				VirtualFree(p, 0, MEM_RELEASE);
			}
		}
		else if (cbAvailable < cb)
			m_Stream.Insert(m_posFlacTagEnd, cb - cbAvailable);
		m_Stream.MoveTo(m_posStreamInfoEnd);
		m_Stream << rbVorbis << rbImage;
		EckCounter(m_vBlock.size(), i)
		{
			const auto& e = m_vBlock[i];
			if (!bHasEnd && i == m_vBlock.size() - 1)
				m_Stream << BYTE(0b1000'0000_by | (BYTE)e.eType);
			else
				m_Stream << (BYTE)e.eType;
			const UINT cbData = (UINT)e.rbData.Size();
			m_Stream << GetIntegerByte<2>(cbData)
				<< GetIntegerByte<1>(cbData)
				<< GetIntegerByte<0>(cbData)
				<< e.rbData;
		}
		m_Stream.GetStream()->Commit(STGC_DEFAULT);
		return Result::Ok;
	}

	auto& GetVorbisComments() { return m_vItem; }

	const auto& GetVorbisComments() const { return m_vItem; }

	auto& GetImages() { return m_vPic; }

	const auto& GetImages() const { return m_vPic; }

	auto& GetBlocks() { return m_vBlock; }

	const auto& GetBlocks() const { return m_vBlock; }

	const auto& GetStreamInfo() const { return m_si; }

	auto& GetVendor() { return m_rsVendor; }

	const auto& GetVendor() const { return m_rsVendor; }

	size_t GetVorbisComment(PCSTR pszKey) const
	{
		EckCounter(m_vItem.size(), i)
		{
			if (m_vItem[i].rsKey.CompareI(pszKey) == 0)
				return i;
		}
		return SizeTMax;
	}

	auto& GetOrCreateVorbisComment(PCSTR pszKey)
	{
		const auto i = GetVorbisComment(pszKey);
		if (i == SizeTMax)
			return m_vItem.emplace_back(pszKey);
		else
			return m_vItem[i];
	}
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END