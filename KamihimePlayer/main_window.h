#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "d2_image_drawer.h"
#include "d2_text_writer.h"
#include "mf_media_player.h"
#include "view_manager.h"
#include "adv.h"
#include "kamihime_image_transferor.h"
#include "win_clock.h"
#include "font_setting_dialogue.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	const wchar_t* m_swzClassName = L"Kamihime player window";
	const wchar_t* m_swzDefaultWindowName = L"Kamihime player";

	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu
	{
		kOpenFolder = 1,
		kAudioVolume, kTextFont,
		kPauseImage
	};
	enum MenuBar
	{
		kFolder, kSetting, kImage
	};

	POINT m_cursorPos{};
	bool m_bLeftDowned = false;
	bool m_bLeftDragged = false;
	bool m_bLeftCombinated = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;
	bool m_bPlayReady = false;
	bool m_bTextHidden = false;

	std::vector<std::wstring> m_folderList;
	size_t m_nFolderIndex = 0;

	void InitialiseMenuBar();

	void MenuOnOpenFolder();

	void MenuOnVolume();
	void MenuOnFont();

	void MenuOnPauseImage();

	void KeyUpOnNextFolder();
	void KeyUpOnForeFolder();

	void ChangeWindowTitle(const wchar_t* pzTitle);
	void SwitchWindowMode();

	bool CreateFolderList(const wchar_t* pwzFolderPath);
	void SetupScenario(const wchar_t* pwzFolderPath);

	void UpdateScreen() const;

	CD2ImageDrawer* m_pD2ImageDrawer = nullptr;
	CD2TextWriter* m_pD2TextWriter = nullptr;
	CMfMediaPlayer* m_pAudioPlayer = nullptr;
	CViewManager* m_pViewManager = nullptr;
	CKamihimeImageTransferor* m_pKamihimeImageTransferor = nullptr;

	CFontSettingDialogue* m_pFontSettingDialogue = nullptr;

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;

	CWinClock m_textClock;

	void ShiftPaintData(bool bForward);

	std::wstring FormatCurrentText();

	void CheckTextClock();
	void ShiftText(bool bForward);
	void UpdateText();
	void AutoTexting();
};

#endif //MAIN_WINDOW_H_