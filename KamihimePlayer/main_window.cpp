
#include <Windows.h>
#include <CommCtrl.h>


#include "main_window.h"
#include "file_operation.h"
#include "Resource.h"

#include "audio_volume_dialogue.h"

#pragma comment(lib, "Comctl32.lib")

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_DAGON));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_ICON_DAGON);
    wcex.lpszClassName = m_class_name.c_str();
    wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_DAGON));

    if (::RegisterClassExW(&wcex))
    {
        m_hInstance = hInstance;

        m_hWnd = ::CreateWindowW(m_class_name.c_str(), m_window_name.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, nullptr, nullptr, hInstance, this);
        if (m_hWnd != nullptr)
        {
            return true;
        }
        else
        {
            std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
        }
    }
    else
    {
        std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

	return false;
}

int CMainWindow::MessageLoop()
{
    MSG msg;

    for (;;)
    {
        BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
        if (bRet > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        else if (bRet == 0)
        {
            /*ループ終了*/
            return static_cast<int>(msg.wParam);
        }
        else
        {
            /*ループ異常*/
            std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
            return -1;
        }
    }
    return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pThis != nullptr)
    {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate(hWnd);
    case WM_DESTROY:
        return OnDestroy();
    case WM_CLOSE:
        return OnClose();
    case WM_PAINT:
        return OnPaint();
    case WM_ERASEBKGND:
        return 0;
    case WM_COMMAND:
        return OnCommand(wParam);
    case WM_MOUSEWHEEL:
        return OnMouseWheel(wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(wParam, lParam);
    case WM_MBUTTONUP:
        return OnMButtonUp(wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    InitialiseMenuBar();

    m_pKamihimeScenePlayer = new CKamihimeScenePlayer(m_hWnd);

    m_pKamihimeMediaPlayer = new CKamihimeMediaPlayer();

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    ::PostQuitMessage(0);

    if (m_pKamihimeScenePlayer != nullptr)
    {
        delete m_pKamihimeScenePlayer;
        m_pKamihimeScenePlayer = nullptr;
    }

    if (m_pKamihimeMediaPlayer != nullptr)
    {
        delete m_pKamihimeMediaPlayer;
        m_pKamihimeMediaPlayer = nullptr;
    }
    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_class_name.c_str(), m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    if (m_pKamihimeScenePlayer != nullptr)
    {
        m_pKamihimeScenePlayer->DisplayImage();
    }

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam)
{
    int wmKind = HIWORD(wParam);
    int wmId = LOWORD(wParam);
    if (wmKind == 0)
    {
        /*Menus*/
        switch (wmId)
        {
        case Menu::kOpen:
            MenuOnOpen();
            break;
        case Menu::kNextFolder:
            MenuOnNextFolder();
            break;
        case Menu::kForeFolder:
            MenuOnForeFolder();
            break;
        case Menu::kNextAudio:
            MenuOnNextAudio();
            break;
        case Menu::kBack:
            MenuOnBack();
            break;
        case Menu::kPlay:
            MenuOnPlay();
            break;
        case Menu::kLoop:
            MenuOnLoop();
            break;
        case Menu::kVolume:
            MenuOnVolume();
            break;
        case Menu::kPauseImage:
            MenuOnPauseImage();
            break;
        }
    }
    if (wmKind > 1)
    {
        /*Controls*/
    }

    return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
    WORD wKey = LOWORD(wParam);

    if (wKey == 0 && m_pKamihimeScenePlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pKamihimeScenePlayer->UpScale();
        }
        else
        {
            m_pKamihimeScenePlayer->DownScale();
        }
    }

    if (wKey == MK_RBUTTON && m_pKamihimeMediaPlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pKamihimeMediaPlayer->Next();
        }
        else
        {
            m_pKamihimeMediaPlayer->Back();
        }
    }

    if (wKey == MK_LBUTTON && m_pKamihimeScenePlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pKamihimeScenePlayer->SpeedUp();
        }
        else
        {
            m_pKamihimeScenePlayer->SpeedDown();
        }
        m_bSpeedSet = true;
    }

    return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    ::GetCursorPos(&m_CursorPos);

    return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (m_bSpeedSet == true)
    {
        m_bSpeedSet = false;
        return 0;
    }

    WORD usKey = LOWORD(wParam);

    if (usKey == MK_RBUTTON && m_bHideBar)
    {
        ::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_DOWN;
        ::SendInput(1, &input, sizeof(input));
    }

    if (usKey == 0 && m_pKamihimeScenePlayer != nullptr)
    {
        POINT pt{};
        ::GetCursorPos(&pt);
        int iX = m_CursorPos.x - pt.x;
        int iY = m_CursorPos.y - pt.y;

        if (iX == 0 && iY == 0)
        {
            m_pKamihimeScenePlayer->Next();
        }
        else
        {
            m_pKamihimeScenePlayer->SetOffset(iX, iY);
        }

    }
    return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);
    if (usKey == 0 && m_pKamihimeScenePlayer != nullptr)
    {
        m_pKamihimeScenePlayer->ResetScale();
    }

    if (usKey == MK_RBUTTON && m_pKamihimeScenePlayer != nullptr)
    {
        SwitchWindowMode();
    }

    return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
    HMENU hManuFolder = nullptr;
    HMENU hMenuAudio = nullptr;
    HMENU kMenuImage = nullptr;
    HMENU hMenuBar = nullptr;
    BOOL iRet = FALSE;

    if (m_hMenuBar != nullptr)return;

    hManuFolder = ::CreateMenu();
    if (hManuFolder == nullptr)goto failed;

    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kOpen, "Open");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kNextFolder, "Next");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kForeFolder, "Back");
    if (iRet == 0)goto failed;

    hMenuAudio = ::CreateMenu();
    if (hMenuAudio == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kNextAudio, "Next");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kBack, "Back");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kPlay, "Play");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kLoop, "Loop");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kVolume, "Setting");
    if (iRet == 0)goto failed;

    kMenuImage = ::CreateMenu();
    iRet = ::AppendMenuA(kMenuImage, MF_STRING, Menu::kPauseImage, "Pause");
    if (iRet == 0)goto failed;

    hMenuBar = ::CreateMenu();
    if (hMenuBar == nullptr) goto failed;

    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hManuFolder), "Folder");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuAudio), "Audio");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(kMenuImage), "Image");
    if (iRet == 0)goto failed;

    iRet = ::SetMenu(m_hWnd, hMenuBar);
    if (iRet == 0)goto failed;

    m_hMenuBar = hMenuBar;

    /*正常終了*/
    return;

failed:
    std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
    ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    /*SetMenu成功後はウィンドウ破棄時に破棄されるが、今は紐づけ前なのでここで破棄する。*/
    if (hManuFolder != nullptr)
    {
        ::DestroyMenu(hManuFolder);
    }
    if (hMenuAudio != nullptr)
    {
        ::DestroyMenu(hMenuAudio);
    }
    if (kMenuImage != nullptr)
    {
        ::DestroyMenu(kMenuImage);
    }
    if (hMenuBar != nullptr)
    {
        ::DestroyMenu(hMenuBar);
    }

}
/*フォルダ選択*/
void CMainWindow::MenuOnOpen()
{
    wchar_t* buffer = SelectWorkingFolder();
    if (buffer != nullptr)
    {
        SetPlayerFolder(buffer);
        CreateFolderList(buffer);

        ::CoTaskMemFree(buffer);
    }
}
/*次フォルダに移動*/
void CMainWindow::MenuOnNextFolder()
{
    if (m_folders.empty())return;

    ++m_nIndex;
    if (m_nIndex >= m_folders.size())m_nIndex = 0;
    SetPlayerFolder(m_folders.at(m_nIndex).c_str());
}
/*前フォルダに移動*/
void CMainWindow::MenuOnForeFolder()
{
    if (m_folders.empty())return;

    --m_nIndex;
    if (m_nIndex >= m_folders.size())m_nIndex = m_folders.size() - 1;
    SetPlayerFolder(m_folders.at(m_nIndex).c_str());
}
/*次音声再生*/
void CMainWindow::MenuOnNextAudio()
{
    if (m_pKamihimeMediaPlayer != nullptr)
    {
        m_pKamihimeMediaPlayer->Next();
    }
}
/*前音声再生*/
void CMainWindow::MenuOnBack()
{
    if (m_pKamihimeMediaPlayer != nullptr)
    {
        m_pKamihimeMediaPlayer->Back();
    }
}
/*現行音声再再生*/
void CMainWindow::MenuOnPlay()
{
    if (m_pKamihimeMediaPlayer != nullptr)
    {
        m_pKamihimeMediaPlayer->Play();
    }
}
/*音声ループ設定変更*/
void CMainWindow::MenuOnLoop()
{
    if (m_pKamihimeMediaPlayer != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kAudio);
            if (hMenu != nullptr)
            {
                BOOL iRet = m_pKamihimeMediaPlayer->SwitchLoop();
                ::CheckMenuItem(hMenu, Menu::kLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*音量・再生速度変更*/
void CMainWindow::MenuOnVolume()
{
    CAudioVolumeDialogue* pAudioVolumeDialogue = new CAudioVolumeDialogue();
    if (pAudioVolumeDialogue != nullptr)
    {
        pAudioVolumeDialogue->Open(m_hInstance, m_hWnd, m_pKamihimeMediaPlayer);

        delete pAudioVolumeDialogue;
    }
}
/*一時停止*/
void CMainWindow::MenuOnPauseImage()
{
    if (m_pKamihimeScenePlayer != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kImage);
            if (hMenu != nullptr)
            {
                bool bRet = m_pKamihimeScenePlayer->SwitchPause();
                ::CheckMenuItem(hMenu, Menu::kPauseImage, bRet ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*標題変更*/
void CMainWindow::ChangeWindowTitle(const wchar_t* pzTitle)
{
    std::wstring wstr;
    if (pzTitle != nullptr)
    {
        std::wstring wstrTitle = pzTitle;
        size_t pos = wstrTitle.find_last_of(L"\\/");
        wstr = pos == std::wstring::npos ? wstrTitle : wstrTitle.substr(pos + 1);
    }

    ::SetWindowTextW(m_hWnd, wstr.empty() ? m_window_name.c_str() : wstr.c_str());
}
/*表示形式変更*/
void CMainWindow::SwitchWindowMode()
{
    if (!m_bPlayReady)return;

    RECT rect;
    ::GetWindowRect(m_hWnd, &rect);
    LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    m_bHideBar ^= true;

    if (m_bHideBar)
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
        ::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
        ::SetMenu(m_hWnd, nullptr);
    }
    else
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
        ::SetMenu(m_hWnd, m_hMenuBar);
    }

    if (m_pKamihimeScenePlayer != nullptr)
    {
        m_pKamihimeScenePlayer->SwitchSizeLore(m_bHideBar);
    }

}
/*フォルダ一覧表作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return false;

    m_folders.clear();
    m_nIndex = 0;

    std::wstring wstrFolder = pwzFolderPath;
    std::wstring wstrParent;
    std::wstring wstrCurrent;

    size_t nPos = wstrFolder.find_last_of(L"\\/");
    if (nPos != std::wstring::npos)
    {
        wstrParent = wstrFolder.substr(0, nPos);
        wstrCurrent = wstrFolder.substr(nPos + 1);
    }

    if (wstrParent.empty())return false;

    wstrParent += L"\\/";
    std::wstring wstrPath = wstrParent + L'*';

    WIN32_FIND_DATA sFindData{};
    std::vector<std::wstring> wstrFolders;

    HANDLE hFind = ::FindFirstFile(wstrPath.c_str(), &sFindData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                if (wcscmp(sFindData.cFileName, L".") != 0 && wcscmp(sFindData.cFileName, L"..") != 0)
                {
                    wstrFolders.push_back(sFindData.cFileName);
                }
            }
        } while (::FindNextFile(hFind, &sFindData));
        ::FindClose(hFind);
    }

    for (size_t i = 0; i < wstrFolders.size(); ++i)
    {
        size_t nIndex = i;
        for (size_t j = i + 1; j < wstrFolders.size() - 1; ++j)
        {
            if (::StrCmpLogicalW(wstrFolders.at(nIndex).c_str(), wstrFolders.at(j).c_str()) > 0)
            {
                nIndex = j;
            }
        }
        std::swap(wstrFolders.at(i), wstrFolders.at(nIndex));
    }

    auto iter = std::find(wstrFolders.begin(), wstrFolders.end(), wstrCurrent);
    if (iter != wstrFolders.end())
    {
        m_nIndex = std::distance(wstrFolders.begin(), iter);
    }

    for (std::wstring wstr : wstrFolders)
    {
        m_folders.push_back(wstrParent + wstr);
    }

    return m_folders.size() > 0;
}
/*再生フォルダ設定*/
void CMainWindow::SetPlayerFolder(const wchar_t* pwzFolderPath)
{
    if (m_pKamihimeMediaPlayer != nullptr)
    {
        m_pKamihimeMediaPlayer->SetFolder(pwzFolderPath);
    }

    if (m_pKamihimeScenePlayer != nullptr)
    {
        m_bPlayReady = m_pKamihimeScenePlayer->SetFolder(pwzFolderPath);
    }

    ChangeWindowTitle(m_bPlayReady ? pwzFolderPath : nullptr);
}
