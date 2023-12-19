#pragma once
#include "Utility.h"

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
		:m_hFile{ CreateFileW(pszFile, dwAccess, dwShareMode, NULL, dwMode, dwAttr, NULL) }
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
		return (m_hFile = CreateFileW(pszFile, dwAccess, dwShareMode, NULL, dwMode, dwAttr, NULL));
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
		return GetFileSize(m_hFile, NULL);
	}

	EckInline BOOL Read(void* pBuf, DWORD dwSize, DWORD* pdwRead = NULL)
	{
		DWORD dw;
		if (!pdwRead)
			pdwRead = &dw;
		return ReadFile(m_hFile, pBuf, dwSize, pdwRead, NULL);
	}

	EckInline BOOL Write(PCVOID pBuf, DWORD dwSize, DWORD* pdwWritten = NULL)
	{
		DWORD dw;
		if (!pdwWritten)
			pdwWritten = &dw;
		return WriteFile(m_hFile, pBuf, dwSize, pdwWritten, NULL);
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
	EckInline CFile& operator<<(T& Buf)
	{
		Write(&Buf, sizeof(T));
		return *this;
	}

	EckInline CFile& operator+=(LONG l)
	{
		SetFilePointer(m_hFile, l, NULL, FILE_CURRENT);
		return *this;
	}

	EckInline CFile& operator-=(LONG l)
	{
		SetFilePointer(m_hFile, -l, NULL, FILE_CURRENT);
		return *this;
	}

	EckInline CFile& MoveToBegin()
	{
		SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
		return *this;
	}

	EckInline CFile& MoveToEnd()
	{
		SetFilePointer(m_hFile, 0, NULL, FILE_END);
		return *this;
	}

	EckInline CFile& MoveTo(LONG l)
	{
		SetFilePointer(m_hFile, l, NULL, FILE_BEGIN);
		return *this;
	}

	EckInline DWORD GetCurrPos()
	{
		return SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
	}

	EckInline void Flush()
	{
		FlushFileBuffers(m_hFile);
	}
};

class CMappingFile
{
private:
	CFile m_File{};
	HANDLE m_hMapping = NULL;
	void* m_pFile = NULL;
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
			m_pFile = NULL;
			m_hMapping = NULL;
		}
	}

	void* Open(PCWSTR pszFile, DWORD dwMap = FILE_MAP_READ, DWORD dwProtect = PAGE_READONLY,
		DWORD dwMode = OPEN_EXISTING, DWORD dwAccess = GENERIC_READ,
		DWORD dwShareMode = 0, DWORD dwAttr = FILE_ATTRIBUTE_NORMAL)
	{
		if (m_File.Open(pszFile, dwMode, dwAccess, dwShareMode, dwAttr) == INVALID_HANDLE_VALUE)
			return NULL;
		DWORD dwSize = m_File.GetSize32();
		m_hMapping = CreateFileMappingW(m_File.GetHandle(), NULL, dwProtect, 0, dwSize, NULL);
		if (!m_hMapping)
		{
			m_File.Close();
			return NULL;
		}
		m_pFile = MapViewOfFile(m_hMapping, dwMap, 0, 0, dwSize);
		if (!m_hMapping)
		{
			CloseHandle(m_hMapping);
			m_File.Close();
			return NULL;
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
	HANDLE m_hMapping = NULL;
	void* m_pFile = NULL;
public:
	CMappingFile2() = delete;
	CMappingFile2(CFile& File) : m_File(File) {}

	~CMappingFile2()
	{
#ifdef _DEBUG
		if (m_hMapping)
		{
			EckDbgPrintWithPos(L"CMappingFile2没有调用Close()");
			EckDbgBreak();
		}
#endif
	}

	void Close()
	{
		if (m_hMapping && m_pFile)
		{
			UnmapViewOfFile(m_pFile);
			CloseHandle(m_hMapping);
			m_pFile = NULL;
			m_hMapping = NULL;
		}
	}

	void* Create(DWORD dwMap = FILE_MAP_READ, DWORD dwProtect = PAGE_READONLY)
	{
		DWORD dwSize = m_File.GetSize32();
		m_hMapping = CreateFileMappingW(m_File.GetHandle(), NULL, dwProtect, 0, dwSize, NULL);
		if (!m_hMapping)
		{
			m_File.Close();
			return NULL;
		}
		m_pFile = MapViewOfFile(m_hMapping, dwMap, 0, 0, dwSize);
		if (!m_hMapping)
		{
			CloseHandle(m_hMapping);
			m_File.Close();
			return NULL;
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