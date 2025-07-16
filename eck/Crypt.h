#pragma once
#include "BaseN.h"
#include "CMemWalker.h"
#include "AutoPtrDef.h"

ECK_NAMESPACE_BEGIN
constexpr inline PCWSTR CryptChainName[]
{
	BCRYPT_CHAIN_MODE_NA,
	BCRYPT_CHAIN_MODE_CBC,
	BCRYPT_CHAIN_MODE_ECB,
	BCRYPT_CHAIN_MODE_CFB,
	BCRYPT_CHAIN_MODE_CCM,
	BCRYPT_CHAIN_MODE_GCM,
};

constexpr inline ULONG CryptChainBytes[]
{
	sizeof(BCRYPT_CHAIN_MODE_NA),
	sizeof(BCRYPT_CHAIN_MODE_CBC),
	sizeof(BCRYPT_CHAIN_MODE_ECB),
	sizeof(BCRYPT_CHAIN_MODE_CFB),
	sizeof(BCRYPT_CHAIN_MODE_CCM),
	sizeof(BCRYPT_CHAIN_MODE_GCM),
};

enum class CryptChain
{
	NA,
	CBC,
	ECB,
	CFB,
	CCM,
	GCM,
};


inline NTSTATUS AesEncrypt(PCVOID pKey, SIZE_T cbKey, PCVOID pIv, SIZE_T cbIv,
	PCVOID pOrg, SIZE_T cbOrg, CRefBin& rbResult,
	CryptChain eChain = CryptChain::CBC, ULONG cbBlock = 0)
{
	NTSTATUS nts;
	BCRYPT_ALG_HANDLE hAlg;
	BCRYPT_KEY_HANDLE hKey{};
	ULONG cbRet, cbKeyObject;
	UniquePtr<DelMA<UCHAR>> pBuf{};
	if (!NT_SUCCESS(nts = BCryptOpenAlgorithmProvider(&hAlg,
		BCRYPT_AES_ALGORITHM, nullptr, 0)))
		return nts;
	if (!NT_SUCCESS(nts = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
		(UCHAR*)CryptChainName[(int)eChain], CryptChainBytes[(int)eChain], 0)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH,
		(UCHAR*)&cbKeyObject, sizeof(cbKeyObject), &cbRet, 0)))
		goto Tidyup;
	if (cbBlock)
	{
		if (!NT_SUCCESS(nts = BCryptSetProperty(hAlg, BCRYPT_BLOCK_LENGTH,
			(UCHAR*)&cbKey, sizeof(cbKey), 0)))
			goto Tidyup;
	}
	else
	{
		if (!NT_SUCCESS(nts = BCryptGetProperty(hAlg, BCRYPT_BLOCK_LENGTH,
			(UCHAR*)&cbBlock, sizeof(cbBlock), &cbRet, 0)))
			goto Tidyup;
		if (cbBlock != cbIv)
		{
			nts = STATUS_INVALID_PARAMETER;
			goto Tidyup;
		}
	}
	pBuf.reset((UCHAR*)malloc(cbKeyObject + cbBlock));
	memcpy(pBuf.get() + cbKeyObject, pIv, cbBlock);

	if (!NT_SUCCESS(nts = BCryptGenerateSymmetricKey(hAlg, &hKey, pBuf.get(), cbKeyObject,
		(UCHAR*)pKey, (ULONG)cbKey, 0)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptEncrypt(hKey, (UCHAR*)pOrg, (ULONG)cbOrg, nullptr,
		(UCHAR*)pBuf.get() + cbKeyObject, cbBlock, nullptr, 0, &cbRet, BCRYPT_BLOCK_PADDING)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptEncrypt(hKey, (UCHAR*)pOrg, (ULONG)cbOrg, nullptr,
		(UCHAR*)pBuf.get() + cbKeyObject, cbBlock, rbResult.PushBackNoExtra(cbRet),
		cbRet, &cbRet, BCRYPT_BLOCK_PADDING)))
		goto Tidyup;
Tidyup:;
	if (hKey)
		BCryptDestroyKey(hKey);
	if (hAlg)
		BCryptCloseAlgorithmProvider(hAlg, 0);
	return nts;
}

inline NTSTATUS DesDecrypt(PCVOID pKey, SIZE_T cbKey, PCVOID pIv, SIZE_T cbIv,
	PCVOID pOrg, SIZE_T cbOrg, CRefBin& rbResult, CryptChain eChain = CryptChain::CBC,
	ULONG ulPadding = BCRYPT_BLOCK_PADDING)
{
	NTSTATUS nts;
	BCRYPT_ALG_HANDLE hAlg;
	BCRYPT_KEY_HANDLE hKey{};
	ULONG cbRet, cbIvBuf{};
	UCHAR* pIvBuf{};
	if (!NT_SUCCESS(nts = BCryptOpenAlgorithmProvider(&hAlg,
		BCRYPT_DES_ALGORITHM, nullptr, 0)))
		return nts;
	if (!NT_SUCCESS(nts = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
		(UCHAR*)CryptChainName[(int)eChain], CryptChainBytes[(int)eChain], 0)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0,
		(UCHAR*)pKey, (ULONG)cbKey, 0)))
		goto Tidyup;
	if (pIv)
	{
		if (!NT_SUCCESS(nts = BCryptGetProperty(hAlg, BCRYPT_BLOCK_LENGTH,
			(UCHAR*)&cbIvBuf, sizeof(cbIvBuf), &cbRet, 0)))
			goto Tidyup;
		if (cbIvBuf > cbIv)
		{
			nts = STATUS_INVALID_PARAMETER;
			goto Tidyup;
		}
		pIvBuf = (UCHAR*)_malloca(cbIvBuf);
		memcpy(pIvBuf, pIv, cbIvBuf);
	}
	if (!NT_SUCCESS(nts = BCryptDecrypt(hKey, (UCHAR*)pOrg, (ULONG)cbOrg, nullptr,
		pIvBuf, cbIvBuf, nullptr, 0, &cbRet, ulPadding)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptDecrypt(hKey, (UCHAR*)pOrg, (ULONG)cbOrg, nullptr,
		pIvBuf, cbIvBuf, rbResult.PushBackNoExtra(cbRet), cbRet, &cbRet, ulPadding)))
		goto Tidyup;
Tidyup:;
	if (hKey)
		BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	_freea(pIvBuf);
	return nts;
}

inline NTSTATUS DesEncrypt(PCVOID pKey, SIZE_T cbKey, PCVOID pIv, SIZE_T cbIv,
	PCVOID pOrg, SIZE_T cbOrg, CRefBin& rbResult, CryptChain eChain = CryptChain::CBC,
	ULONG ulPadding = BCRYPT_BLOCK_PADDING)
{
	NTSTATUS nts;
	BCRYPT_ALG_HANDLE hAlg;
	BCRYPT_KEY_HANDLE hKey{};
	ULONG cbRet;
	if (!NT_SUCCESS(nts = BCryptOpenAlgorithmProvider(&hAlg,
		BCRYPT_DES_ALGORITHM, nullptr, 0)))
		return nts;
	if (!NT_SUCCESS(nts = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
		(UCHAR*)CryptChainName[(int)eChain], CryptChainBytes[(int)eChain], 0)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0,
		(UCHAR*)pKey, (ULONG)cbKey, 0)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptEncrypt(hKey, (UCHAR*)pOrg, (ULONG)cbOrg, nullptr,
		(UCHAR*)pIv, (ULONG)cbIv, nullptr, 0, &cbRet, ulPadding)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptEncrypt(hKey, (UCHAR*)pOrg, (ULONG)cbOrg, nullptr,
		(UCHAR*)pIv, (ULONG)cbIv, rbResult.PushBackNoExtra(cbRet), cbRet, &cbRet, ulPadding)))
		goto Tidyup;
Tidyup:;
	if (hKey)
		BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	return nts;
}

inline BOOL ImportPublicKeyFromPEM(PCSTR pszKey, int cchKey, BCRYPT_KEY_HANDLE& hKey)
{
	hKey = 0;
	const auto rb = Base64Decode(pszKey, cchKey);

	DWORD Dummy{};
	CERT_PUBLIC_KEY_INFO* ppki;
	if (CryptDecodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO,
		rb.Data(), (DWORD)rb.Size(),
		CRYPT_DECODE_ALLOC_FLAG,
		0, &ppki, &Dummy))
	{
		const auto b = CryptImportPublicKeyInfoEx2(X509_ASN_ENCODING, ppki, 0, 0, &hKey);
		LocalFree(ppki);
		return b;
	}
	return FALSE;
}

inline BOOL GetExponentAndModulusFromPEM(PCSTR pszKey, int cchKey,
	CRefBin& rbExponent, CRefBin& rbModulus)
{
	const auto rb = Base64Decode(pszKey, cchKey);
	CMemReader r(rb.Data(), rb.Size());
	USHORT us;
	BYTE by[15];
	constexpr BYTE ByMagic[]{ 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86,
		0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00 };
	r >> us;
	if (us == 0x8130)
		r += 1;
	else if (us == 0x8230)
		r += 2;
	else
		return FALSE;
	r >> by;
	if (memcmp(by, ByMagic, sizeof(ByMagic)) != 0)
		return FALSE;
	r >> us;
	if (us == 0x8103)
		r += 1;
	else if (us == 0x8203)
		r += 2;
	else
		return FALSE;
	r >> by[0];
	if (by[0] != 0)
		return FALSE;
	r >> us;
	if (us == 0x8130)
		r += 1;
	else if (us == 0x8230)
		r += 2;
	else
		return FALSE;
	r >> us;
	USHORT cbModulus;
	if (us == 0x8102)
	{
		cbModulus = 0;
		r.Read(&cbModulus, 1);
	}
	else if (us == 0x8202)
	{
		r >> cbModulus;
		cbModulus = ReverseInteger(cbModulus);
	}
	else
		return FALSE;

	if (*(r.Data() + 1) == 0)
	{
		r += 1;
		--cbModulus;
	}
	rbModulus.PushBackNoExtra(cbModulus);
	r.Read(rbModulus.Data(), cbModulus);
	r >> by[0];
	if (by[0] != 2)
		return FALSE;
	BYTE cbExponent;
	r >> cbExponent;
	rbExponent.PushBackNoExtra(cbExponent);
	r.Read(rbExponent.Data(), cbExponent);
	return TRUE;
}

inline BOOL MakeBCryptRsaKeyBlob(PCVOID pKey, SIZE_T cbKey, UCHAR*& pBlob, ULONG& cbBlob)
{
	BYTE* pKeyBlob;
	CRefBin rbExp{}, rbMod{};
	if (!GetExponentAndModulusFromPEM((PCSTR)pKey, (int)cbKey, rbExp, rbMod))
	{
		pBlob = nullptr;
		cbBlob = 0;
		return FALSE;
	}
	cbBlob = ULONG(sizeof(BCRYPT_RSAKEY_BLOB) + rbExp.Size() + rbMod.Size());
	pKeyBlob = (BYTE*)malloc(cbBlob);
	EckCheckMem(pKeyBlob);
	auto phdr = (BCRYPT_RSAKEY_BLOB*)pKeyBlob;
	phdr->Magic = BCRYPT_RSAPUBLIC_MAGIC;
	phdr->BitLength = (ULONG)rbMod.Size() * 8;
	phdr->cbPublicExp = (ULONG)rbExp.Size();
	phdr->cbModulus = (ULONG)rbMod.Size();
	phdr->cbPrime1 = phdr->cbPrime2 = 0;
	memcpy(pKeyBlob + sizeof(BCRYPT_RSAKEY_BLOB), rbExp.Data(), rbExp.Size());
	memcpy(pKeyBlob + sizeof(BCRYPT_RSAKEY_BLOB) + rbExp.Size(), rbMod.Data(), rbMod.Size());
	pBlob = pKeyBlob;
	return TRUE;
}

inline NTSTATUS RsaEncrypt(PCVOID pKey, SIZE_T cbKey,
	PCVOID pOrg, SIZE_T cbOrg, CRefBin& rbResult, ULONG ulPadding = BCRYPT_PAD_PKCS1)
{
	NTSTATUS nts;
	BCRYPT_ALG_HANDLE hAlg;
	BCRYPT_KEY_HANDLE hKey{};
	ULONG cbRet;

	if (!NT_SUCCESS(nts = BCryptOpenAlgorithmProvider(&hAlg,
		BCRYPT_RSA_ALGORITHM, nullptr, 0)))
		return nts;
	if (!ImportPublicKeyFromPEM((PCSTR)pKey, (int)cbKey, hKey))
	{
		nts = STATUS_INVALID_PARAMETER;
		goto Tidyup;
	}
	if (!NT_SUCCESS(nts = BCryptEncrypt(hKey, (UCHAR*)pOrg, (ULONG)cbOrg, nullptr,
		nullptr, 0, nullptr, 0, &cbRet, ulPadding)))
		goto Tidyup;
	if (!NT_SUCCESS(nts = BCryptEncrypt(hKey, (UCHAR*)pOrg, (ULONG)cbOrg, nullptr,
		nullptr, 0, rbResult.PushBackNoExtra(cbRet), cbRet, &cbRet, ulPadding)))
		goto Tidyup;
Tidyup:;
	if (hKey)
		BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	return nts;
}
ECK_NAMESPACE_END