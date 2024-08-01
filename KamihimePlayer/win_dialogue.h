﻿#ifndef WIN_DIALOGUE_H_
#define WIN_DIALOGUE_H_

#include <string>

namespace win_dialogue
{
    std::wstring SelectWorkFolder(void *hParentWnd);
    std::wstring SelectOpenFile(const wchar_t* pwzFileType, const wchar_t* pwzSpec, const wchar_t*pwzTitle, void* hParentWnd);
    std::wstring SelectSaveFile(const wchar_t* pwzFileType, const wchar_t* pwzSpec, const wchar_t* pwzDefaultFileName, void* hParentWnd);
    void ShowMessageBox(const char* pzTitle, const char* pzMessage);
    void ShowMessageBox(const wchar_t* pwzTitle, const wchar_t* pwzMessage);
}
#endif // WIN_DIALOGUE_H_