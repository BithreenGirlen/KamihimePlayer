
#include <Windows.h>
#include <CommCtrl.h>

#include "main_window.h"

#include "Resource.h"
#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"
#include "kmhm.h"
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

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		m_hWnd = ::CreateWindowW(m_class_name.c_str(), m_window_name.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
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
	case WM_KEYUP:
		return OnKeyUp(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_TIMER:
		return OnTimer(wParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return OnMButtonUp(wParam, lParam);
	case EventMessage::kAudioPlayer:
		OnAudioPlayerEvent(static_cast<unsigned long>(lParam));
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
	m_pAudioPlayer->SetPlaybackWindow(m_hWnd, EventMessage::kAudioPlayer);

	m_pD2TextWriter = new CD2TextWriter(m_pD2ImageDrawer->GetD2Factory(), m_pD2ImageDrawer->GetD2DeviceContext());
	m_pD2TextWriter->SetupOutLinedDrawing(L"C:\\Windows\\Fonts\\yumindb.ttf");

	m_pViewManager = new CViewManager(m_hWnd);

	m_pKamihimeImageTransferor = new CKamihimeImageTransferor(m_pD2ImageDrawer->GetD2DeviceContext(), m_hWnd);

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
	::KillTimer(m_hWnd, Timer::kText);

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
	::UnregisterClassW(m_class_name.c_str(), m_hInstance);

	return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	if (m_pD2ImageDrawer == nullptr || m_pD2TextWriter == nullptr
		|| m_pViewManager == nullptr || m_pKamihimeImageTransferor == nullptr)
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
		if (!m_bTextHidden)
		{
			const std::wstring wstr = FormatCurrentText();
			m_pD2TextWriter->OutLinedDraw(wstr.c_str(), static_cast<unsigned long>(wstr.size()));
		}
		m_pD2ImageDrawer->Display();
	}

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
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
			UpdateScreen();
		}
		break;
	case 'T':
		m_bTextHidden ^= true;
		UpdateScreen();
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
		case Menu::kAudioLoop:
			MenuOnLoop();
			break;
		case Menu::kAudioVolume:
			MenuOnVolume();
			break;
		case Menu::kPauseImage:
			MenuOnPauseImage();
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
	switch (wParam)
	{
	case Timer::kText:
		if (m_pAudioPlayer != nullptr)
		{
			if (m_pAudioPlayer->IsEnded())
			{
				AutoTexting();
			}
		}
		break;
	default:
		break;
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
		if (m_pViewManager != nullptr)
		{
			m_pViewManager->Rescale(iScroll > 0);
		}
	}

	if (usKey == MK_LBUTTON)
	{
		if (m_pKamihimeImageTransferor != nullptr)
		{
			m_pKamihimeImageTransferor->RescaleTimer(iScroll > 0);
		}
		m_bLeftCombinated = true;
	}

	if (usKey == MK_RBUTTON)
	{
		ShiftText(iScroll > 0);
	}

	return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_CursorPos);

	m_bLeftDowned = true;

	return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_bLeftCombinated)
	{
		m_bLeftCombinated = false;
		return 0;
	}

	WORD usKey = LOWORD(wParam);

	if (usKey == MK_RBUTTON && m_bBarHidden)
	{
		::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_DOWN;
		::SendInput(1, &input, sizeof(input));
	}

	if (usKey == 0 && m_bLeftDowned)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_CursorPos.x - pt.x;
		int iY = m_CursorPos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			ShiftPaintData(true);
		}
		else
		{
			if (m_pViewManager != nullptr)
			{
				m_pViewManager->SetOffset(iX, iY);
			}
		}
	}

	m_bLeftDowned = false;

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
			m_pKamihimeImageTransferor->ResetSpeed();
		}
	}

	if (usKey == MK_RBUTTON)
	{
		SwitchWindowMode();
	}

	return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
	HMENU hMenuFolder = nullptr;
	HMENU hMenuAudio = nullptr;
	HMENU hMenuImage = nullptr;
	HMENU hMenuBar = nullptr;
	BOOL iRet = FALSE;

	if (m_hMenuBar != nullptr)return;

	hMenuFolder = ::CreateMenu();
	if (hMenuFolder == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kOpenFolder, "Open");
	if (iRet == 0)goto failed;

	hMenuAudio = ::CreateMenu();
	if (hMenuAudio == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioLoop, "Loop");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioVolume, "Setting");
	if (iRet == 0)goto failed;

	hMenuImage = ::CreateMenu();
	iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kPauseImage, "Pause");
	if (iRet == 0)goto failed;

	hMenuBar = ::CreateMenu();
	if (hMenuBar == nullptr) goto failed;

	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFolder), "Folder");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuAudio), "Audio");
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
	if (hMenuAudio != nullptr)
	{
		::DestroyMenu(hMenuAudio);
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
/*音声ループ設定変更*/
void CMainWindow::MenuOnLoop()
{
	if (m_pAudioPlayer != nullptr)
	{
		HMENU hMenuBar = ::GetMenu(m_hWnd);
		if (hMenuBar != nullptr)
		{
			HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kAudio);
			if (hMenu != nullptr)
			{
				BOOL iRet = m_pAudioPlayer->SwitchLoop();
				::CheckMenuItem(hMenu, Menu::kAudioLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
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
		pAudioVolumeDialogue->Open(m_hInstance, m_hWnd, m_pAudioPlayer);

		delete pAudioVolumeDialogue;
	}
}
/*一時停止*/
void CMainWindow::MenuOnPauseImage()
{
	if (m_pKamihimeImageTransferor != nullptr)
	{
		HMENU hMenuBar = ::GetMenu(m_hWnd);
		if (hMenuBar != nullptr)
		{
			HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kImage);
			if (hMenu != nullptr)
			{
				bool bRet = m_pKamihimeImageTransferor->SwitchPause();
				::CheckMenuItem(hMenu, Menu::kPauseImage, bRet ? MF_CHECKED : MF_UNCHECKED);
			}
		}
	}
}
/*次フォルダに移動*/
void CMainWindow::KeyUpOnNextFolder()
{
	if (m_folderList.empty())return;

	++m_nFolderIndex;
	if (m_nFolderIndex >= m_folderList.size())m_nFolderIndex = 0;
	SetupScenario(m_folderList.at(m_nFolderIndex).c_str());
}
/*前フォルダに移動*/
void CMainWindow::KeyUpOnForeFolder()
{
	if (m_folderList.empty())return;

	--m_nFolderIndex;
	if (m_nFolderIndex >= m_folderList.size())m_nFolderIndex = m_folderList.size() - 1;
	SetupScenario(m_folderList.at(m_nFolderIndex).c_str());
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

	m_bBarHidden ^= true;

	if (m_bBarHidden)
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
	bool bRet = false;
	m_textData.clear();
	m_nTextIndex = 0;

	std::vector<std::vector<std::wstring>> imageFileNamesList;

	std::vector<std::wstring> textFile;
	win_filesystem::CreateFilePathList(pwzFolderPath, L".json", textFile);
	if (!textFile.empty())
	{
		std::string scenarioText = win_filesystem::LoadFileAsString(textFile.at(0).c_str());

		std::vector<adv::TextDatum> textData;
		kmhm::LoadScenarioFile(scenarioText, textData, imageFileNamesList);

		const std::wstring wstrDirectory = std::wstring(pwzFolderPath).append(L"\\");
		for (auto& textDatum : textData)
		{
			if (!textDatum.wstrVoicePath.empty())
			{
				textDatum.wstrVoicePath = wstrDirectory + textDatum.wstrVoicePath;
			}
		}
		m_textData = std::move(textData);

		for (auto& imageFiles : imageFileNamesList)
		{
			for (auto& imageFileName : imageFiles)
			{
				imageFileName = wstrDirectory + imageFileName;
			}
		}
	}
	if (m_textData.empty())
	{
		/* 台本ファイルなし */
		std::vector<std::wstring> audioFilePaths;
		win_filesystem::CreateFilePathList(pwzFolderPath, L".mp3", audioFilePaths);

		for (const auto& voicePath : audioFilePaths)
		{
			m_textData.emplace_back(adv::TextDatum{L"", voicePath});
		}

		std::vector<std::wstring> imageFilePaths;
		bool bRet = win_filesystem::CreateFilePathList(pwzFolderPath, L".jpg", imageFilePaths);
		if (m_pKamihimeImageTransferor != nullptr)
		{
			for (const auto& imageFilePath : imageFilePaths)
			{
				std::vector<std::wstring> vBuffer;
				vBuffer.push_back(imageFilePath);
				imageFileNamesList.push_back(vBuffer);
			}
		}
	}

	if (m_pKamihimeImageTransferor != nullptr)
	{
		bRet = m_pKamihimeImageTransferor->SetImages(imageFileNamesList);
		if (bRet)
		{
			unsigned int uiWidth = 0;
			unsigned int uiHeight = 0;
			m_pKamihimeImageTransferor->GetImageSize(&uiWidth, &uiHeight);

			if (m_pViewManager != nullptr)
			{
				m_pViewManager->SetBaseSize(uiWidth, uiHeight);
				m_pViewManager->ResetZoom();
			}
		}
	}

	UpdateText();
	UpdatePaintData();

	m_bPlayReady = bRet;

	ChangeWindowTitle(m_bPlayReady ? pwzFolderPath : nullptr);
}
/*再描画要求*/
void CMainWindow::UpdateScreen()
{
	::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*表示図画送り・戻し*/
void CMainWindow::ShiftPaintData(bool bForward)
{
	if (m_pKamihimeImageTransferor != nullptr)
	{
		m_pKamihimeImageTransferor->ShiftImage();
	}

	UpdatePaintData();
}
/*図画データ更新*/
void CMainWindow::UpdatePaintData()
{
	UpdateScreen();
}
/*表示文作成*/
std::wstring CMainWindow::FormatCurrentText()
{
	const adv::TextDatum& t = m_textData.at(m_nTextIndex);
	std::wstring wstr = t.wstrText;
	constexpr unsigned int kLineThreashold = 24;
	for (size_t i = kLineThreashold; i < wstr.size(); i += kLineThreashold)
	{
		wstr.insert(i, L"\n");
	}
	if (!t.wstrText.empty() && t.wstrText.back() != L'\n') wstr += L"\n ";
	wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
	return wstr;
}
/*文章送り・戻し*/
void CMainWindow::ShiftText(bool bForward)
{
	if (bForward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	UpdateText();
}
/*文章更新*/
void CMainWindow::UpdateText()
{
	if (!m_textData.empty())
	{
		const adv::TextDatum& t = m_textData.at(m_nTextIndex);
		if (!t.wstrVoicePath.empty())
		{
			if (m_pAudioPlayer != nullptr)
			{
				m_pAudioPlayer->Play(t.wstrVoicePath.c_str());
			}
		}
		constexpr unsigned int kTimerInterval = 2000;
		::SetTimer(m_hWnd, Timer::kText, kTimerInterval, nullptr);
	}

	::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*IMFMediaEngineNotify::EventNotify*/
void CMainWindow::OnAudioPlayerEvent(unsigned long ulEvent)
{
	switch (ulEvent)
	{
	case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:

		break;
	case MF_MEDIA_ENGINE_EVENT_ENDED:
		AutoTexting();
		break;
	default:
		break;
	}
}
/*自動送り*/
void CMainWindow::AutoTexting()
{
	if (m_nTextIndex < m_textData.size() - 1)ShiftText(true);
}
