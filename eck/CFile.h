﻿#pragma once
#include "CRefBin.h"

ECK_NAMESPACE_BEGIN
enum
{
	FCD_COVER = CREATE_ALWAYS,				// 若文件已存在则清除其数据，若不存在则创建
	FCD_NOCOVER = OPEN_ALWAYS,				// 若文件已存在则打开，若不存在则创建
	FCD_ONLYNEW = CREATE_NEW,				// 若文件已存在则失败，若不存在则创建
	FCD_ONLYEXISTING = OPEN_EXISTING,		// 若文件已存在则打开，若不存在则失败
	FCD_COVEREXISTING = TRUNCATE_EXISTING,	// 若文件已存在则清除其数据，若不存在则失败
};
class CFile
{
private:
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
public:
	CFile() = default;
	CFile(PCWSTR pszFile, DWORD dwMode = OPEN_EXISTING, DWORD dwAccess = GENERIC_READ,
		DWORD dwShareMode = 0, DWORD dwAttr = FILE_ATTRIBUTE_NORMAL)
		:m_hFile{ CreateFileW(pszFile, dwAccess, dwShareMode, nullptr, dwMode, dwAttr, nullptr) }
	{}

	~CFile()
	{
		Close();
	}

	EckInline void Close()
	{
		if (m_hFile != INVALID_HANDLE_VALUE && m_hFile)
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}

	EckInline HANDLE Open(PCWSTR pszFile, DWORD dwMode = OPEN_EXISTING, DWORD dwAccess = GENERIC_READ,
		DWORD dwShareMode = 0, DWORD dwAttr = FILE_ATTRIBUTE_NORMAL)
	{
		Close();
		return (m_hFile = CreateFileW(pszFile, dwAccess, dwShareMode, nullptr, dwMode, dwAttr, nullptr));
	}

	EckInline HANDLE GetHandle()
	{
		return m_hFile;
	}

	EckInline LONGLONG GetSize()
	{
		LARGE_INTEGER lint;
		GetFileSizeEx(m_hFile, &lint);
		return lint.QuadPart;
	}

	EckInline DWORD GetSize32()
	{
		return GetFileSize(m_hFile, nullptr);
	}

	EckInline BOOL Read(void* pBuf, DWORD dwSize, DWORD* pdwRead = nullptr)
	{
		DWORD dw;
		if (!pdwRead)
			pdwRead = &dw;
		return ReadFile(m_hFile, pBuf, dwSize, pdwRead, nullptr);
	}

	EckInline CRefBin ReadBin(DWORD dwSize)
	{
		CRefBin Bin(dwSize);
		Read(Bin.Data(), dwSize);
		return Bin;
	}

	EckInline BOOL Write(PCVOID pBuf, DWORD dwSize, DWORD* pdwWritten = nullptr)
	{
		DWORD dw;
		if (!pdwWritten)
			pdwWritten = &dw;
		return WriteFile(m_hFile, pBuf, dwSize, pdwWritten, nullptr);
	}

	EckInline BOOL End()
	{
		return SetEndOfFile(m_hFile);
	}

	template<class T>
	EckInline CFile& operator>>(T& Buf)
	{
		Read(&Buf, sizeof(T));
		return *this;
	}

	template<class T>
	EckInline CFile& operator<<(const T& Buf)
	{
		Write(&Buf, sizeof(T));
		return *this;
	}

	template<class T, class U, class V>
	EckInline CFile& operator<<(const CRefStrT<T, U, V>& Buf)
	{
		if (Buf.IsEmpty())
			return *this << U::CharTerminatingNull();
		else
		{
			Write(Buf.Data(), (DWORD)Cch2CbW(Buf.Size()));
			return *this;
		}
	}

	EckInline CFile& operator+=(LONG l)
	{
		SetFilePointer(m_hFile, l, nullptr, FILE_CURRENT);
		return *this;
	}

	EckInline CFile& operator-=(LONG l)
	{
		SetFilePointer(m_hFile, -l, nullptr, FILE_CURRENT);
		return *this;
	}

	EckInline CFile& MoveToBegin()
	{
		SetFilePointer(m_hFile, 0, nullptr, FILE_BEGIN);
		return *this;
	}

	EckInline CFile& MoveToEnd()
	{
		SetFilePointer(m_hFile, 0, nullptr, FILE_END);
		return *this;
	}

	EckInline CFile& MoveTo(LONG l)
	{
		SetFilePointer(m_hFile, l, nullptr, FILE_BEGIN);
		return *this;
	}

	EckInline DWORD GetCurrPos()
	{
		return SetFilePointer(m_hFile, 0, nullptr, FILE_CURRENT);
	}

	EckInline void Flush()
	{
		FlushFileBuffers(m_hFile);
	}

	EckInline BOOL IsValid()
	{
		return m_hFile != INVALID_HANDLE_VALUE && m_hFile;
	}
};

class CMappingFile
{
private:
	CFile m_File{};
	HANDLE m_hMapping = nullptr;
	void* m_pFile = nullptr;
public:
	~CMappingFile()
	{
		Close();
	}

	void Close()
	{
		if (m_hMapping && m_pFile)
		{
			UnmapViewOfFile(m_pFile);
			CloseHandle(m_hMapping);
			m_File.Close();
			m_pFile = nullptr;
			m_hMapping = nullptr;
		}
	}

	void* Open(PCWSTR pszFile, DWORD dwMap = FILE_MAP_READ, DWORD dwProtect = PAGE_READONLY,
		DWORD dwMode = OPEN_EXISTING, DWORD dwAccess = GENERIC_READ,
		DWORD dwShareMode = 0, DWORD dwAttr = FILE_ATTRIBUTE_NORMAL)
	{
		if (m_File.Open(pszFile, dwMode, dwAccess, dwShareMode, dwAttr) == INVALID_HANDLE_VALUE)
			return nullptr;
		DWORD dwSize = m_File.GetSize32();
		m_hMapping = CreateFileMappingW(m_File.GetHandle(), nullptr, dwProtect, 0, dwSize, nullptr);
		if (!m_hMapping)
		{
			m_File.Close();
			return nullptr;
		}
		m_pFile = MapViewOfFile(m_hMapping, dwMap, 0, 0, dwSize);
		if (!m_hMapping)
		{
			CloseHandle(m_hMapping);
			m_File.Close();
			return nullptr;
		}
		return m_pFile;
	}

	EckInline CFile& GetFile()
	{
		return m_File;
	}

	EckInline HANDLE GetHMapping()
	{
		return m_hMapping;
	}

	EckInline void* GetPFile()
	{
		return m_pFile;
	}
};

class CMappingFile2
{
private:
	CFile& m_File;
	HANDLE m_hMapping = nullptr;
	void* m_pFile = nullptr;
public:
	CMappingFile2() = delete;
	CMappingFile2(CFile& File) : m_File(File) {}

	~CMappingFile2()
	{
#ifdef _DEBUG
		if (m_hMapping)
		{
			UnmapViewOfFile(m_pFile);
			CloseHandle(m_hMapping);
			EckDbgPrintWithPos(L"CMappingFile2已取消映射，请确保File对象仍然存在");
		}
#endif
	}

	void Close()
	{
		if (m_hMapping && m_pFile)
		{
			UnmapViewOfFile(m_pFile);
			CloseHandle(m_hMapping);
			m_pFile = nullptr;
			m_hMapping = nullptr;
		}
	}

	void* Create(DWORD dwMap = FILE_MAP_READ, DWORD dwProtect = PAGE_READONLY)
	{
		DWORD dwSize = m_File.GetSize32();
		m_hMapping = CreateFileMappingW(m_File.GetHandle(), nullptr, dwProtect, 0, dwSize, nullptr);
		if (!m_hMapping)
		{
			m_File.Close();
			return nullptr;
		}
		m_pFile = MapViewOfFile(m_hMapping, dwMap, 0, 0, dwSize);
		if (!m_hMapping)
		{
			CloseHandle(m_hMapping);
			m_File.Close();
			return nullptr;
		}
		return m_pFile;
	}

	EckInline CFile& GetFile()
	{
		return m_File;
	}

	EckInline void SetFile(CFile& File)
	{
		EckAssert(!m_hMapping);
		m_File = File;
	}

	EckInline HANDLE GetHMapping()
	{
		return m_hMapping;
	}

	EckInline void* GetPFile()
	{
		return m_pFile;
	}
};
ECK_NAMESPACE_END