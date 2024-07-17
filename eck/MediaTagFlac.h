/*
* WinEzCtrlKit Library
*
* MediaTagFlac.h ： Flac注释读写
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

	static void ParseImageBlock(CRefBin& rb, MUSICPIC& Pic)
	{
		CMemWalker r(rb.Data(), rb.Size());
		DWORD t;
		DWORD dwType;
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

		r += 16;// 跳过宽度、高度、色深、索引图颜色数

		r.ReadRev(t);// 图片数据长度

		Pic.bLink = (Pic.rsMime == "-->");

		if (Pic.bLink)
		{
			CRefStrA u8(t);
			r.Read(u8.Data(), t);
			Pic.varPic = StrX2W(u8, CP_UTF8);
		}
		else
		{
			rb.Erase(0, r.Data() - rb.Data());
			Pic.varPic = std::move(rb);
		}
	}
public:
	ECK_DISABLE_COPY_MOVE(CFlac)
public:
	CFlac(CMediaFile& File) :CTag(File) {}

	Result SimpleExtract(MUSICINFO& mi)
	{
		for (const auto& e : m_vItem)
		{
			if ((mi.uFlag & MIM_TITLE) && e.rsKey == "TITLE")
			{
				mi.rsTitle = e.rsValue;
				mi.uMaskRead |= MIM_TITLE;
			}
			else if ((mi.uFlag & MIM_ARTIST) && e.rsKey == "ARTIST")
			{
				mi.AppendArtist(e.rsValue);
				mi.uMaskRead |= MIM_ARTIST;
			}
			else if ((mi.uFlag & MIM_ALBUM) && e.rsKey == "ALBUM")
			{
				mi.rsAlbum = e.rsValue;
				mi.uMaskRead |= MIM_ALBUM;
			}
			else if ((mi.uFlag & MIM_LRC) && e.rsKey == "LYRICS")
			{
				mi.rsLrc = e.rsValue;
				mi.uMaskRead |= MIM_LRC;
			}
			else if ((mi.uFlag & MIM_COMMENT) && e.rsKey == "DESCRIPTION")
			{
				mi.AppendComment(e.rsValue);
				mi.uMaskRead |= MIM_COMMENT;
			}
			else if ((mi.uFlag & MIM_GENRE) && e.rsKey == "GENRE")
			{
				mi.rsGenre = e.rsValue;
				mi.uMaskRead |= MIM_GENRE;
			}
			else if ((mi.uFlag & MIM_DATE) && e.rsKey == "DATE")
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
				mi.vImage.emplace_back(e);
			mi.uMaskRead |= MIM_COVER;
		}
	}

	Result ReadTag(UINT uFlags)
	{
		if (m_File.m_Id3Loc.posFlac == SIZETMax)
			return Result::NoTag;
		m_Stream.MoveTo(m_File.m_Id3Loc.posFlac);
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
				m_Stream >> m_si.cBlockSampleMin >> m_si.cBlockSampleMax;
				m_Stream.ReadRev(&m_si.cbMinFrame, 3).Read(&m_si.cbMaxFrame, 3);
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
					if (u8.IsStartOf("METADATA_BLOCK_PICTURE"))
					{
						auto rb = Base64Decode(u8.Data() + iPos, cchActual);
						MUSICPIC Pic{};
						ParseImageBlock(rb, Pic);
						m_vPic.emplace_back(std::move(Pic));
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
				MUSICPIC Pic{};
				ParseImageBlock(rb, Pic);
				m_vPic.emplace_back(std::move(Pic));
			}
			break;
			default:
			{
				CRefBin rb(cbBlock);
				m_Stream.Read(rb.Data(), cbBlock);
				m_vBlock.emplace_back((BlockType)(Header.by & 0x7F), std::move(rb));
			}
			break;
			}
		} while (!(Header.by & 0x80));
		m_posFlacTagEnd = m_Stream.GetPos();
		return Result::Ok;
	}

	Result WriteTag(UINT uFlags)
	{
		if (m_File.m_Id3Loc.posFlac == SIZETMax)
			return Result::NoTag;

		CRefBin rbVorbis{}, rbImage{};
		// 序列化Vorbis注释
		rbVorbis.PushBack(4u);// 悬而未决
		if (!m_rsVendor.IsEmpty())
		{
			const int cchVendor = WideCharToMultiByte(CP_UTF8, 0, m_rsVendor.Data(), m_rsVendor.Size(),
				NULL, 0, NULL, NULL);
			WideCharToMultiByte(CP_UTF8, 0, m_rsVendor.Data(), m_rsVendor.Size(),
				(CHAR*)rbVorbis.PushBack(cchVendor), cchVendor, NULL, NULL);
		}
		rbVorbis << (UINT)m_vItem.size();
		for (const auto& e : m_vItem)
		{
			rbVorbis << e.rsKey;
			rbVorbis.Back() = '=';
			const int cchValue = WideCharToMultiByte(CP_UTF8, 0, e.rsValue.Data(), e.rsValue.Size(),
				NULL, 0, NULL, NULL);
			WideCharToMultiByte(CP_UTF8, 0, e.rsValue.Data(), e.rsValue.Size(),
				(CHAR*)rbVorbis.PushBack(cchValue), cchValue, NULL, NULL);
		}
		const auto cbData = (UINT)rbVorbis.Size();
		const auto phdr = (FLAC_BlockHeader*)rbVorbis.Data();
		phdr->bySize[0] = GetIntegerByte<2>(cbData);
		phdr->bySize[1] = GetIntegerByte<1>(cbData);
		phdr->bySize[2] = GetIntegerByte<0>(cbData);
		phdr->by = 0_by;
		// 序列化图片
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
			const auto phdr = (FLAC_BlockHeader*)(rbImage.Data() - cbData - 4);
			phdr->bySize[0] = GetIntegerByte<2>(cbData);
			phdr->bySize[1] = GetIntegerByte<1>(cbData);
			phdr->bySize[2] = GetIntegerByte<0>(cbData);
			phdr->by = 0_by;
		}
		auto cb = rbVorbis.Size() + rbImage.Size();
		for (const auto& e : m_vBlock)
			cb += (e.rbData.Size() + 4);


	}

	Result Shrink(UINT uFlags)
	{

	}
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END