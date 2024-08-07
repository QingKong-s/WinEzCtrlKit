﻿/*
* WinEzCtrlKit Library
*
* Utility2.h ： 实用函数
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CRefBin.h"
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
inline constexpr BYTE c_Base64DecodeTable[]
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f,

	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
	0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
	0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
	0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

constexpr inline char c_Base64EncodeTable[]{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };

inline CRefBin Base64Decode(PCSTR psz, int cch)
{
	int i{}, j{};
	size_t idxBin{}, idxText{};
	BYTE temp[4];

	CRefBin rb(cch / 4 * 3 + 3);
	while (cch-- && (psz[idxText] != '='))
	{
		temp[i++] = psz[idxText]; idxText++;
		if (i == 4)
		{
			for (i = 0; i < 4; i++)
				temp[i] = c_Base64DecodeTable[temp[i]];

			rb[idxBin++] = (temp[0] << 2) + ((temp[1] & 0x30) >> 4);
			rb[idxBin++] = ((temp[1] & 0xf) << 4) + ((temp[2] & 0x3c) >> 2);
			rb[idxBin++] = ((temp[2] & 0x3) << 6) + temp[3];

			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 4; j++)
			temp[j] = 0;

		for (j = 0; j < 4; j++)
			temp[j] = c_Base64DecodeTable[temp[j]];

		rb[idxBin++] = (temp[0] << 2) + ((temp[1] & 0x30) >> 4);
		rb[idxBin++] = ((temp[1] & 0xf) << 4) + ((temp[2] & 0x3c) >> 2);
		rb[idxBin++] = ((temp[2] & 0x3) << 6) + temp[3];
	}
	rb.ReSize(idxBin);
	return rb;
}

inline CRefStrA Base64Encode(PCVOID p_, size_t cb)
{
	PCBYTE p = (PCBYTE)p_;
	CRefStrA rs(int(4 * ((cb + 2) / 3)));
	int i = 0;
	int j = 0;
	BYTE t3[3];
	BYTE t4[4];
	int pos = 0;

	while (cb--)
	{
		t3[i++] = *(p++);
		if (i == 3)
		{
			t4[0] = (t3[0] & 0xfc) >> 2;
			t4[1] = ((t3[0] & 0x03) << 4) + ((t3[1] & 0xf0) >> 4);
			t4[2] = ((t3[1] & 0x0f) << 2) + ((t3[2] & 0xc0) >> 6);
			t4[3] = t3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				rs[pos++] = c_Base64EncodeTable[t4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			t3[j] = '\0';

		t4[0] = (t3[0] & 0xfc) >> 2;
		t4[1] = ((t3[0] & 0x03) << 4) + ((t3[1] & 0xf0) >> 4);
		t4[2] = ((t3[1] & 0x0f) << 2) + ((t3[2] & 0xc0) >> 6);
		t4[3] = t3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			rs[pos++] = c_Base64EncodeTable[t4[j]];

		while ((i++ < 3))
			rs[pos++] = '=';
	}

	return rs;
}

EckInline CRefStrA Base64Encode(const CRefBin& rb)
{
	return Base64Encode(rb.Data(), rb.Size());
}

constexpr inline UINT CrcTable[]
{
	0x00000000u,0x77073096u,0xEE0E612Cu,0x990951BAu,0x076DC419u,0x706AF48Fu,0xE963A535u,0x9E6495A3u,
	0x0EDB8832u,0x79DCB8A4u,0xE0D5E91Eu,0x97D2D988u,0x09B64C2Bu,0x7EB17CBDu,0xE7B82D07u,0x90BF1D91u,
	0x1DB71064u,0x6AB020F2u,0xF3B97148u,0x84BE41DEu,0x1ADAD47Du,0x6DDDE4EBu,0xF4D4B551u,0x83D385C7u,
	0x136C9856u,0x646BA8C0u,0xFD62F97Au,0x8A65C9ECu,0x14015C4Fu,0x63066CD9u,0xFA0F3D63u,0x8D080DF5u,
	0x3B6E20C8u,0x4C69105Eu,0xD56041E4u,0xA2677172u,0x3C03E4D1u,0x4B04D447u,0xD20D85FDu,0xA50AB56Bu,
	0x35B5A8FAu,0x42B2986Cu,0xDBBBC9D6u,0xACBCF940u,0x32D86CE3u,0x45DF5C75u,0xDCD60DCFu,0xABD13D59u,
	0x26D930ACu,0x51DE003Au,0xC8D75180u,0xBFD06116u,0x21B4F4B5u,0x56B3C423u,0xCFBA9599u,0xB8BDA50Fu,
	0x2802B89Eu,0x5F058808u,0xC60CD9B2u,0xB10BE924u,0x2F6F7C87u,0x58684C11u,0xC1611DABu,0xB6662D3Du,
	0x76DC4190u,0x01DB7106u,0x98D220BCu,0xEFD5102Au,0x71B18589u,0x06B6B51Fu,0x9FBFE4A5u,0xE8B8D433u,
	0x7807C9A2u,0x0F00F934u,0x9609A88Eu,0xE10E9818u,0x7F6A0DBBu,0x086D3D2Du,0x91646C97u,0xE6635C01u,
	0x6B6B51F4u,0x1C6C6162u,0x856530D8u,0xF262004Eu,0x6C0695EDu,0x1B01A57Bu,0x8208F4C1u,0xF50FC457u,
	0x65B0D9C6u,0x12B7E950u,0x8BBEB8EAu,0xFCB9887Cu,0x62DD1DDFu,0x15DA2D49u,0x8CD37CF3u,0xFBD44C65u,
	0x4DB26158u,0x3AB551CEu,0xA3BC0074u,0xD4BB30E2u,0x4ADFA541u,0x3DD895D7u,0xA4D1C46Du,0xD3D6F4FBu,
	0x4369E96Au,0x346ED9FCu,0xAD678846u,0xDA60B8D0u,0x44042D73u,0x33031DE5u,0xAA0A4C5Fu,0xDD0D7CC9u,
	0x5005713Cu,0x270241AAu,0xBE0B1010u,0xC90C2086u,0x5768B525u,0x206F85B3u,0xB966D409u,0xCE61E49Fu,
	0x5EDEF90Eu,0x29D9C998u,0xB0D09822u,0xC7D7A8B4u,0x59B33D17u,0x2EB40D81u,0xB7BD5C3Bu,0xC0BA6CADu,
	0xEDB88320u,0x9ABFB3B6u,0x03B6E20Cu,0x74B1D29Au,0xEAD54739u,0x9DD277AFu,0x04DB2615u,0x73DC1683u,
	0xE3630B12u,0x94643B84u,0x0D6D6A3Eu,0x7A6A5AA8u,0xE40ECF0Bu,0x9309FF9Du,0x0A00AE27u,0x7D079EB1u,
	0xF00F9344u,0x8708A3D2u,0x1E01F268u,0x6906C2FEu,0xF762575Du,0x806567CBu,0x196C3671u,0x6E6B06E7u,
	0xFED41B76u,0x89D32BE0u,0x10DA7A5Au,0x67DD4ACCu,0xF9B9DF6Fu,0x8EBEEFF9u,0x17B7BE43u,0x60B08ED5u,
	0xD6D6A3E8u,0xA1D1937Eu,0x38D8C2C4u,0x4FDFF252u,0xD1BB67F1u,0xA6BC5767u,0x3FB506DDu,0x48B2364Bu,
	0xD80D2BDAu,0xAF0A1B4Cu,0x36034AF6u,0x41047A60u,0xDF60EFC3u,0xA867DF55u,0x316E8EEFu,0x4669BE79u,
	0xCB61B38Cu,0xBC66831Au,0x256FD2A0u,0x5268E236u,0xCC0C7795u,0xBB0B4703u,0x220216B9u,0x5505262Fu,
	0xC5BA3BBEu,0xB2BD0B28u,0x2BB45A92u,0x5CB36A04u,0xC2D7FFA7u,0xB5D0CF31u,0x2CD99E8Bu,0x5BDEAE1Du,
	0x9B64C2B0u,0xEC63F226u,0x756AA39Cu,0x026D930Au,0x9C0906A9u,0xEB0E363Fu,0x72076785u,0x05005713u,
	0x95BF4A82u,0xE2B87A14u,0x7BB12BAEu,0x0CB61B38u,0x92D28E9Bu,0xE5D5BE0Du,0x7CDCEFB7u,0x0BDBDF21u,
	0x86D3D2D4u,0xF1D4E242u,0x68DDB3F8u,0x1FDA836Eu,0x81BE16CDu,0xF6B9265Bu,0x6FB077E1u,0x18B74777u,
	0x88085AE6u,0xFF0F6A70u,0x66063BCAu,0x11010B5Cu,0x8F659EFFu,0xF862AE69u,0x616BFFD3u,0x166CCF45u,
	0xA00AE278u,0xD70DD2EEu,0x4E048354u,0x3903B3C2u,0xA7672661u,0xD06016F7u,0x4969474Du,0x3E6E77DBu,
	0xAED16A4Au,0xD9D65ADCu,0x40DF0B66u,0x37D83BF0u,0xA9BCAE53u,0xDEBB9EC5u,0x47B2CF7Fu,0x30B5FFE9u,
	0xBDBDF21Cu,0xCABAC28Au,0x53B39330u,0x24B4A3A6u,0xBAD03605u,0xCDD70693u,0x54DE5729u,0x23D967BFu,
	0xB3667A2Eu,0xC4614AB8u,0x5D681B02u,0x2A6F2B94u,0xB40BBE37u,0xC30C8EA1u,0x5A05DF1Bu,0x2D02EF8Du,
};

inline constexpr UINT CalcCrc32(PCVOID pData, size_t cbData)
{
	const BYTE* p = (const BYTE*)pData;
	UINT crc{ 0xFFFFFFFF };
	while (cbData--)
	{
		crc = ((crc >> 8u) & 0x00FFFFFFu) ^ CrcTable[(crc ^ (*p)) & 0xFFu];
		++p;
	}
	return crc ^ 0xFFFFFFFF;
}

EckInline CRefStrW ToStr(const CRefBin& rb)
{
	CRefStrW rs((int)rb.Size() / 2);
	memcpy(rs.Data(), rb.Data(), rs.Size() * sizeof(WCHAR));
	return rs;
}

template<class TAlloc, class TChar, class TTraits, class TAlloc1>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const CRefStrT<TChar, TTraits, TAlloc1>& rs)
{
	rb.PushBack(rs.Data(), rs.ByteSize());
	return rb;
}

template<class TAlloc, class TAlloc1>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const CRefBinT<TAlloc1>& rb1)
{
	rb.PushBack(rb1.Data(), rb1.Size());
	return rb;
}

template<class TAlloc, class T>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const T& t)
{
	rb.PushBack(&t, sizeof(T));
	return rb;
}

[[nodiscard]] inline CRefStrW Utf16RevByte(PCWSTR pszText, int cchText = -1)
{
	CRefStrW rs{};
	int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
		pszText, cchText, NULL, 0, NULL, NULL, 0);
	if (!cchResult)
		return rs;
	rs.ReSize(cchResult);
	LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
		pszText, cchText, rs.Data(), cchResult, NULL, NULL, 0);
	return rs;
}

EckInline BOOL IsFILETIMEZero(const FILETIME& ft)
{
	return ft.dwLowDateTime == 0 && ft.dwHighDateTime == 0;
}

EckInline bool operator==(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == 0;
}

EckInline bool operator>(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == 1;
}

EckInline bool operator<(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == -1;
}

inline int CalcDbcsStringCharCount(PCSTR pszText, int cchText, UINT uCp = CP_ACP)
{
	int c = 0;
	for (auto p = pszText; p < pszText + cchText; ++c)
	{
		if (IsDBCSLeadByteEx(uCp, *p))
			p += 2;
		else
			p += 1;
	}
	return c;
}

EckInline void FreeSTRRET(const STRRET& strret)
{
	if (strret.uType == STRRET_WSTR)
		CoTaskMemFree(strret.pOleStr);
}

EckInline const auto* UserSharedData() { return USER_SHARED_DATA; }
ECK_NAMESPACE_END