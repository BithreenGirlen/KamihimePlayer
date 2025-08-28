
#include <Windows.h>
#include <CommCtrl.h>

#include "main_window.h"

#include "Resource.h"
#include "win_dialogue.h"
#include "win_filesystem.h"
#include "media_setting_dialogue.h"


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
	wcex.lpszClassName = m_swzClassName;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_DAGON));

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		m_hWnd = ::CreateWindowW(m_swzClassName, m_swzDefaultWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
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
		BOOL iRet = ::GetMessageW(&msg, nullptr, 0, 0);
		if (iRet > 0)
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else if (iRet == 0)
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
		return 1;
	case WM_KEYDOWN:
		return OnKeyDown(wParam, lParam);
	case WM_KEYUP:
		return OnKeyUp(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_TIMER:
		return OnTimer(wParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_RBUTTONUP:
		return OnRButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return OnMButtonUp(wParam, lParam);
	default:
		break;
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	InitialiseMenuBar();

	m_pD2ImageDrawer = new CD2ImageDrawer(m_hWnd);

	m_pAudioPlayer = new CMfMediaPlayer();

	m_pD2TextWriter = new CD2TextWriter(m_pD2ImageDrawer->GetD2Factory(), m_pD2ImageDrawer->GetD2DeviceContext());
	m_pD2TextWriter->SetupOutLinedDrawing(L"C:\\Windows\\Fonts\\yumindb.ttf");

	m_pViewManager = new CViewManager(m_hWnd);

	m_pKamihimeImageTransferor = new CKamihimeImageTransferor(m_pD2ImageDrawer->GetD2DeviceContext());
	SetMenuCheckState(MenuBar::kImage, Menu::kPauseImage, m_pKamihimeImageTransferor->IsPaused());
	SetMenuCheckState(MenuBar::kImage, Menu::kSyncImage, m_pKamihimeImageTransferor->IsImageSynced());

	m_pFontSettingDialogue = new CFontSettingDialogue();

	return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
	if (m_pFontSettingDialogue != nullptr)
	{
		if (m_pFontSettingDialogue->GetHwnd() != nullptr)
		{
			::SendMessage(m_pFontSettingDialogue->GetHwnd(), WM_CLOSE, 0, 0);
			delete m_pFontSettingDialogue;
			m_pFontSettingDialogue = nullptr;
		}
	}
	if (m_pKamihimeImageTransferor != nullptr)
	{
		delete m_pKamihimeImageTransferor;
		m_pKamihimeImageTransferor = nullptr;
	}

	if (m_pViewManager != nullptr)
	{
		delete m_pViewManager;
		m_pViewManager = nullptr;
	}

	if (m_pD2TextWriter != nullptr)
	{
		delete m_pD2TextWriter;
		m_pD2TextWriter = nullptr;
	}

	if (m_pD2ImageDrawer != nullptr)
	{
		delete m_pD2ImageDrawer;
		m_pD2ImageDrawer = nullptr;
	}

	if (m_pAudioPlayer != nullptr)
	{
		delete m_pAudioPlayer;
		m_pAudioPlayer = nullptr;
	}

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_swzClassName, m_hInstance);

	return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	if (m_pD2ImageDrawer == nullptr || m_pD2TextWriter == nullptr || m_pViewManager == nullptr || m_pKamihimeImageTransferor == nullptr)
	{
		::EndPaint(m_hWnd, &ps);
		return 0;
	}

	bool bRet = false;

	m_pD2ImageDrawer->Clear();

	ID2D1Bitmap* pImage = m_pKamihimeImageTransferor->GetCurrentImage();
	if (pImage != nullptr)
	{
		bRet = m_pD2ImageDrawer->Draw(pImage, { m_pViewManager->GetXOffset(), m_pViewManager->GetYOffset() }, m_pViewManager->GetScale());
	}

	if (bRet)
	{
		if (!m_isTextHidden)
		{
			const std::wstring wstr = m_pKamihimeImageTransferor->GetCurrentFormattedText();
			m_pD2TextWriter->OutLinedDraw(wstr.c_str(), static_cast<unsigned long>(wstr.size()));
		}
		m_pD2ImageDrawer->Display();

		UpdateScreen();

		CheckTextClock();
	}

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
	return 0;
}
/*WM_KEYDOWN*/
LRESULT CMainWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_RIGHT:
		if (::GetKeyState(VK_CONTROL) & 0x8000)
		{
			USHORT us = ::GetKeyState(VK_CONTROL);
			if (m_pKamihimeImageTransferor != nullptr)
			{
				m_pKamihimeImageTransferor->UpdateAnimationInterval(true);
			}
		}
		else
		{
			AutoTexting();
		}
		break;
	case VK_LEFT:
		if (HIWORD(::GetKeyState(VK_CONTROL)) & 0x8000)
		{
			if (m_pKamihimeImageTransferor != nullptr)
			{
				m_pKamihimeImageTransferor->UpdateAnimationInterval(false);
			}
		}
		else
		{
			ShiftText(false);
		}
		break;
	default:
		break;
	}

	return 0;
}
/*WM_KEYUP*/
LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;
	case VK_UP:
		KeyUpOnForeFolder();
		break;
	case VK_DOWN:
		KeyUpOnNextFolder();
		break;
	case 'C':
		if (m_pD2TextWriter != nullptr)
		{
			m_pD2TextWriter->SwitchTextColour();
		}
		break;
	case 'T':
		m_isTextHidden ^= true;
		break;
	}
	return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int iControlWnd = LOWORD(lParam);
	if (iControlWnd == 0)
	{
		/*Menus*/
		switch (wmId)
		{
		case Menu::kOpenFolder:
			MenuOnOpenFolder();
			break;
		case Menu::kAudioVolume:
			MenuOnVolume();
			break;
		case Menu::kTextFont:
			MenuOnFont();
			break;
		case Menu::kPauseImage:
			MenuOnPauseImage();
			break;
		case Menu::kSyncImage:
			MenuOnSyncImage();
			break;
		default:
			if (wmId >= Menu::kLabelStartIndex)
			{
				JumpToLabel(static_cast<size_t>(wmId - Menu::kLabelStartIndex));
			}
			break;
		}
	}
	else
	{
		/*Controls*/
	}

	return 0;
}
/*WM_TIMER*/
LRESULT CMainWindow::OnTimer(WPARAM wParam)
{
	return 0;
}
LRESULT CMainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);
	if (usKey == MK_LBUTTON)
	{
		if (m_wasLeftCombinated)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_lastCursorPos.x - pt.x;
		int iY = m_lastCursorPos.y - pt.y;

		if (m_hasLeftBeenDragged)
		{
			if (m_pViewManager != nullptr)
			{
				m_pViewManager->SetOffset(iX, iY);
			}
		}

		m_lastCursorPos = pt;
		m_hasLeftBeenDragged = true;
	}

	return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
	WORD usKey = LOWORD(wParam);

	if (usKey == 0)
	{
		if (m_pKamihimeImageTransferor != nullptr && m_pKamihimeImageTransferor->HasScenarioData())
		{
			if (m_pViewManager != nullptr)
			{
				m_pViewManager->Rescale(iScroll > 0);
			}
		}
	}
	else if (usKey == MK_LBUTTON)
	{
		if (m_pKamihimeImageTransferor != nullptr)
		{
			m_pKamihimeImageTransferor->UpdateAnimationInterval(iScroll > 0);
		}
		m_wasLeftCombinated = true;
	}
	else if (usKey == MK_RBUTTON)
	{
		ShiftText(iScroll > 0);

		m_wasRightCombinated = true;
	}

	return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_lastCursorPos);

	m_wasLeftPressed = true;

	return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_wasLeftCombinated || m_hasLeftBeenDragged)
	{
		m_wasLeftCombinated = false;
		m_hasLeftBeenDragged = false;
		m_wasLeftPressed = false;
		return 0;
	}

	WORD usKey = LOWORD(wParam);

	if (usKey == MK_RBUTTON && m_isBarHidden)
	{
		::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_DOWN;
		::SendInput(1, &input, sizeof(input));

		m_wasRightCombinated = true;
	}

	if (usKey == 0 && m_wasLeftPressed)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_lastCursorPos.x - pt.x;
		int iY = m_lastCursorPos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			if (m_pKamihimeImageTransferor != nullptr)
			{
				if (m_pKamihimeImageTransferor->IsPaused())
				{
					m_pKamihimeImageTransferor->ShiftAnimation();
				}
				else if (!m_pKamihimeImageTransferor->IsImageSynced())
				{
					m_pKamihimeImageTransferor->ShiftImage();
				}
			}
		}
	}

	m_wasLeftPressed = false;

	return 0;
}

LRESULT CMainWindow::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_wasRightCombinated)
	{
		m_wasRightCombinated = false;
		return 0;
	}

	if (m_pKamihimeImageTransferor == nullptr || !m_pKamihimeImageTransferor->HasScenarioData())return 0;

	WORD usKey = LOWORD(wParam);
	if (usKey == 0)
	{
		const auto& labelData = m_pKamihimeImageTransferor->GetLabelData();
		if (labelData.empty())return 0;

		HMENU hPopupMenu = ::CreatePopupMenu();
		if (hPopupMenu != nullptr)
		{
			for (size_t i = 0; i < labelData.size(); ++i)
			{
				::AppendMenuW(hPopupMenu, MF_STRING, Menu::kLabelStartIndex + i, labelData[i].wstrCaption.c_str());
			}

			POINT point{};
			::GetCursorPos(&point);
			::TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, m_hWnd, nullptr);
			::DestroyMenu(hPopupMenu);
		}
	}

	return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);
	if (usKey == 0)
	{
		if (m_pViewManager != nullptr)
		{
			m_pViewManager->ResetZoom();
		}

		if (m_pKamihimeImageTransferor != nullptr)
		{
			m_pKamihimeImageTransferor->ResetAnimationInterval();
		}
	}

	if (usKey == MK_RBUTTON)
	{
		TogglwWindowBorderStyle();

		m_wasRightCombinated = true;
	}

	return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
	HMENU hMenuFolder = nullptr;
	HMENU hMenuSetting = nullptr;
	HMENU hMenuImage = nullptr;
	HMENU hMenuBar = nullptr;
	BOOL iRet = FALSE;

	if (m_hMenuBar != nullptr)return;

	hMenuFolder = ::CreateMenu();
	if (hMenuFolder == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kOpenFolder, "Open");
	if (iRet == 0)goto failed;

	hMenuSetting = ::CreateMenu();
	if (hMenuSetting == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuSetting, MF_STRING, Menu::kAudioVolume, "Audio");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuSetting, MF_STRING, Menu::kTextFont, "Font");
	if (iRet == 0)goto failed;

	hMenuImage = ::CreateMenu();
	iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kPauseImage, "Pause");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kSyncImage, "Sync");
	if (iRet == 0)goto failed;

	hMenuBar = ::CreateMenu();
	if (hMenuBar == nullptr) goto failed;

	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFolder), "Folder");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuSetting), "Setting");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuImage), "Image");
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
	if (hMenuFolder != nullptr)
	{
		::DestroyMenu(hMenuFolder);
	}
	if (hMenuSetting != nullptr)
	{
		::DestroyMenu(hMenuSetting);
	}
	if (hMenuImage != nullptr)
	{
		::DestroyMenu(hMenuImage);
	}
	if (hMenuBar != nullptr)
	{
		::DestroyMenu(hMenuBar);
	}
}
/*フォルダ選択*/
void CMainWindow::MenuOnOpenFolder()
{
	std::wstring wstrPickedupFolder = win_dialogue::SelectWorkFolder(m_hWnd);
	if (!wstrPickedupFolder.empty())
	{
		SetupScenario(wstrPickedupFolder.c_str());
		CreateFolderList(wstrPickedupFolder.c_str());
	}
}
/*音量・再生速度変更*/
void CMainWindow::MenuOnVolume()
{
	CMediaSettingDialogue sMediaSettingDialogue;
	sMediaSettingDialogue.Open(m_hInstance, m_hWnd, m_pAudioPlayer, L"Audio");
}
/*書体設定*/
void CMainWindow::MenuOnFont()
{
	if (m_pFontSettingDialogue != nullptr)
	{
		if (m_pFontSettingDialogue->GetHwnd() == nullptr)
		{
			HWND hWnd = m_pFontSettingDialogue->Open(m_hInstance, m_hWnd, L"Font", m_pD2TextWriter);
			::SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(::LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON_DAGON))));
			::ShowWindow(hWnd, SW_SHOWNORMAL);
		}
		else
		{
			::SetFocus(m_pFontSettingDialogue->GetHwnd());
		}
	}
}
/*一時停止*/
void CMainWindow::MenuOnPauseImage()
{
	if (m_pKamihimeImageTransferor != nullptr)
	{
		bool bRet = SetMenuCheckState(MenuBar::kImage, Menu::kPauseImage, !m_pKamihimeImageTransferor->IsPaused());
		if (bRet)
		{
			m_pKamihimeImageTransferor->TogglePause();
		}
	}
}

void CMainWindow::MenuOnSyncImage()
{
	if (m_pKamihimeImageTransferor != nullptr)
	{
		bool bRet = SetMenuCheckState(MenuBar::kImage, Menu::kSyncImage, !m_pKamihimeImageTransferor->IsImageSynced());
		if (bRet)
		{
			m_pKamihimeImageTransferor->ToggleImageSync();
		}
	}

}
/*次フォルダに移動*/
void CMainWindow::KeyUpOnNextFolder()
{
	if (m_folderList.empty())return;

	++m_nFolderIndex;
	if (m_nFolderIndex >= m_folderList.size())m_nFolderIndex = 0;
	SetupScenario(m_folderList[m_nFolderIndex].c_str());
}
/*前フォルダに移動*/
void CMainWindow::KeyUpOnForeFolder()
{
	if (m_folderList.empty())return;

	--m_nFolderIndex;
	if (m_nFolderIndex >= m_folderList.size())m_nFolderIndex = m_folderList.size() - 1;
	SetupScenario(m_folderList[m_nFolderIndex].c_str());
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

	::SetWindowTextW(m_hWnd, wstr.empty() ? m_swzDefaultWindowName : wstr.c_str());
}
/*表示形式変更*/
void CMainWindow::TogglwWindowBorderStyle()
{
	if (m_pKamihimeImageTransferor == nullptr || !m_pKamihimeImageTransferor->HasScenarioData())return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_isBarHidden ^= true;

	if (m_isBarHidden)
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

	if (m_pViewManager != nullptr)
	{
		m_pViewManager->OnStyleChanged();
	}
}

bool CMainWindow::SetMenuCheckState(unsigned int uiMenuIndex, unsigned int uiItemIndex, bool checked) const
{
	HMENU hMenuBar = ::GetMenu(m_hWnd);
	if (hMenuBar != nullptr)
	{
		HMENU hMenu = ::GetSubMenu(hMenuBar, uiMenuIndex);
		if (hMenu != nullptr)
		{
			DWORD ulRet = ::CheckMenuItem(hMenu, uiItemIndex, checked ? MF_CHECKED : MF_UNCHECKED);
			return ulRet != (DWORD)-1;
		}
	}

	return false;
}
/*フォルダ一覧表作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
	m_folderList.clear();
	m_nFolderIndex = 0;
	win_filesystem::GetFilePathListAndIndex(pwzFolderPath, nullptr, m_folderList, &m_nFolderIndex);

	return m_folderList.size() > 0;
}
/*再生フォルダ設定*/
void CMainWindow::SetupScenario(const wchar_t* pwzFolderPath)
{
	if (pwzFolderPath == nullptr)return;

	if (m_pKamihimeImageTransferor == nullptr)return;

	bool bRet = m_pKamihimeImageTransferor->LoadScenario(pwzFolderPath);
	if (bRet)
	{
		unsigned int uiWidth = 0;
		unsigned int uiHeight = 0;
		m_pKamihimeImageTransferor->GetCurrentImageSize(&uiWidth, &uiHeight);

		if (m_pViewManager != nullptr)
		{
			m_pViewManager->SetBaseSize(uiWidth, uiHeight);
			m_pViewManager->ResetZoom();
		}
	}

	m_textClock.Restart();
	UpdateText();

	ChangeWindowTitle(m_pKamihimeImageTransferor->HasScenarioData() ? pwzFolderPath : nullptr);
}
void CMainWindow::JumpToLabel(size_t nIndex)
{
	if (m_pKamihimeImageTransferor != nullptr && m_pKamihimeImageTransferor->HasScenarioData())
	{
		m_pKamihimeImageTransferor->JumpToLabel(nIndex);
		UpdateText();
	}
}
/*再描画要求*/
void CMainWindow::UpdateScreen() const
{
	::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*文章表示経過時間確認*/
void CMainWindow::CheckTextClock()
{
	if (m_pAudioPlayer != nullptr)
	{
		float fElapsed = m_textClock.GetElapsedTime();
		if (::isgreaterequal(fElapsed, 2000.f))
		{
			m_textClock.Restart();
			if (m_pAudioPlayer->IsEnded())
			{
				AutoTexting();
			}
		}
	}
}
/*文章送り・戻し*/
void CMainWindow::ShiftText(bool bForward)
{
	if (m_pKamihimeImageTransferor != nullptr)
	{
		m_pKamihimeImageTransferor->ShiftScene(bForward);
		UpdateText();
	}
}
/*文章更新*/
void CMainWindow::UpdateText()
{
	if (m_pKamihimeImageTransferor != nullptr)
	{
		if (m_pAudioPlayer != nullptr)
		{
			const wchar_t* pwzVoiceFilePath = m_pKamihimeImageTransferor->GetCurrentVoiceFilePath();
			if (pwzVoiceFilePath != nullptr && *pwzVoiceFilePath != L'\0')
			{
				m_pAudioPlayer->Play(pwzVoiceFilePath);
			}
		}
	}
}
/*自動送り*/
void CMainWindow::AutoTexting()
{
	if (m_pKamihimeImageTransferor != nullptr)
	{
		if (!m_pKamihimeImageTransferor->HasReachedLastScene())
		{
			ShiftText(true);
		}
	}
}
