#include "CFile.h"

ECK_NAMESPACE_BEGIN




void CMappingFile::Close()
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

void* CMappingFile::Open(PCWSTR pszFile, DWORD dwMap, DWORD dwProtect, DWORD dwMode, DWORD dwAccess, DWORD dwShareMode, DWORD dwAttr)
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


void CMappingFile2::Close()
{
	if (m_hMapping && m_pFile)
	{
		UnmapViewOfFile(m_pFile);
		CloseHandle(m_hMapping);
		m_pFile = NULL;
		m_hMapping = NULL;
	}
}

void* CMappingFile2::Create(DWORD dwMap, DWORD dwProtect)
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
ECK_NAMESPACE_END