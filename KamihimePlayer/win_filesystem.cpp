
#include <Windows.h>
#include <shlwapi.h>

#include "win_filesystem.h"

#pragma comment(lib, "Shlwapi.lib")

namespace win_filesystem
{
	/*�t�@�C���̃������W�J*/
	char* LoadExistingFile(const wchar_t* pwzFilePath, unsigned long* ulSize)
	{
		HANDLE hFile = ::CreateFile(pwzFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwSize = ::GetFileSize(hFile, nullptr);
			if (dwSize != INVALID_FILE_SIZE)
			{
				char* pBuffer = static_cast<char*>(malloc(static_cast<size_t>(dwSize + 1ULL)));
				if (pBuffer != nullptr)
				{
					DWORD dwRead = 0;
					BOOL iRet = ::ReadFile(hFile, pBuffer, dwSize, &dwRead, nullptr);
					if (iRet)
					{
						::CloseHandle(hFile);
						*(pBuffer + dwRead) = '\0';
						*ulSize = dwRead;

						return pBuffer;
					}
					else
					{
						free(pBuffer);
					}
				}
			}
			::CloseHandle(hFile);
		}

		return nullptr;
	}

	/*std::string to std::wstring*/
	std::wstring WidenUtf8(const std::string& str)
	{
		if (!str.empty())
		{
			int iLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
			if (iLen > 0)
			{
				std::wstring wstr(iLen, 0);
				::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], iLen);
				return wstr;
			}
		}

		return std::wstring();
	}
}

/*�w��K�w�̃t�@�C���E�t�H���_�ꗗ�쐬*/
bool win_filesystem::CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileExtension, std::vector<std::wstring>& paths)
{
	if (pwzFolderPath == nullptr)return false;

	std::wstring wstrParent = pwzFolderPath;
	wstrParent += L"\\";
	std::wstring wstrPath = wstrParent + L'*';
	if (pwzFileExtension != nullptr)
	{
		wstrPath += pwzFileExtension;
	}

	WIN32_FIND_DATA sFindData;
	std::vector<std::wstring> wstrNames;

	HANDLE hFind = ::FindFirstFile(wstrPath.c_str(), &sFindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (pwzFileExtension != nullptr)
		{
			do
			{
				/*�t�@�C���ꗗ*/
				if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					wstrNames.push_back(sFindData.cFileName);
				}
			} while (::FindNextFile(hFind, &sFindData));
		}
		else
		{
			do
			{
				/*�t�H���_�ꗗ*/
				if ((sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (wcscmp(sFindData.cFileName, L".") != 0 && wcscmp(sFindData.cFileName, L"..") != 0)
					{
						wstrNames.push_back(sFindData.cFileName);
					}
				}
			} while (::FindNextFile(hFind, &sFindData));
		}

		::FindClose(hFind);
	}

	/*���O���ɐ���*/
	for (size_t i = 0; i < wstrNames.size(); ++i)
	{
		size_t nIndex = i;
		for (size_t j = i; j < wstrNames.size(); ++j)
		{
			if (::StrCmpLogicalW(wstrNames.at(nIndex).c_str(), wstrNames.at(j).c_str()) > 0)
			{
				nIndex = j;
			}
		}
		std::swap(wstrNames.at(i), wstrNames.at(nIndex));
	}

	for (const std::wstring& wstr : wstrNames)
	{
		paths.push_back(wstrParent + wstr);
	}

	return paths.size() > 0;
}
/*�w��t�H���_�Ɠ��K�w�̃t�H���_�ꗗ�쐬�E���Έʒu�擾*/
void win_filesystem::GetFolderListAndIndex(const std::wstring& wstrFolderPath, std::vector<std::wstring>& folders, size_t* nIndex)
{
	std::wstring wstrParent;
	std::wstring wstrCurrent;

	size_t nPos = wstrFolderPath.find_last_of(L"\\/");
	if (nPos != std::wstring::npos)
	{
		wstrParent = wstrFolderPath.substr(0, nPos);
		wstrCurrent = wstrFolderPath.substr(nPos + 1);
	}

	win_filesystem::CreateFilePathList(wstrParent.c_str(), nullptr, folders);

	auto iter = std::find(folders.begin(), folders.end(), wstrFolderPath);
	if (iter != folders.end())
	{
		*nIndex = std::distance(folders.begin(), iter);
	}
}
/*������Ƃ��ăt�@�C���ǂݍ���*/
std::wstring win_filesystem::LoadFileAsString(const wchar_t* pwzFilePath)
{
	DWORD dwSize = 0;
	char* pBuffer = LoadExistingFile(pwzFilePath, &dwSize);
	if (pBuffer != nullptr)
	{
		std::string str;
		str.reserve(dwSize);
		/*�I�[�������*/
		for (size_t i = 0; i < dwSize; ++i)
		{
			str.push_back(*(pBuffer + i));
		}

		free(pBuffer);
		std::wstring wstr = WidenUtf8(str);
		return wstr;
	}

	return std::wstring();
}