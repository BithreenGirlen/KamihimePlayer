#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>

#include "kamihime_scene_player.h"
#include "mf_media_player.h"
#include "d2_text_writer.h"
#include "adv.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	std::wstring m_class_name = L"Kamihime player window";
	std::wstring m_window_name = L"Kamihime player";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu
	{
		kOpenFolder = 1,
		kAudioLoop, kAudioVolume,
		kPauseImage
	};
	enum MenuBar
	{
		kFolder, kAudio, kImage
	};
	enum EventMessage
	{
		kAudioPlayer = WM_USER + 1
	};
	enum Timer
	{
		kText = 1,
	};
	POINT m_CursorPos{};
	bool m_bSpeedHavingChanged = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;
	bool m_bPlayReady = false;
	bool m_bTextHidden = false;

	std::vector<std::wstring> m_folderList;
	size_t m_nFolderIndex = 0;

	void InitialiseMenuBar();

	void MenuOnOpenFolder();

	void MenuOnLoop();
	void MenuOnVolume();

	void MenuOnPauseImage();

	void KeyUpOnNextFolder();
	void KeyUpOnForeFolder();

	void ChangeWindowTitle(const wchar_t* pzTitle);
	void SwitchWindowMode();

	bool CreateFolderList(const wchar_t* pwzFolderPath);
	void SetupScenario(const wchar_t* pwzFolderPath);

	void UpdateScreen();

	CKamihimeScenePlayer* m_pKamihimeScenePlayer = nullptr;
	CMfMediaPlayer* m_pAudioPlayer = nullptr;
	CD2TextWriter* m_pD2TextWriter = nullptr;

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;

	std::wstring FormatCurrentText();

	void ShiftText(bool bForward);
	void UpdateText();
	void OnAudioPlayerEvent(unsigned long ulEvent);
	void AutoTexting();
};

#endif //MAIN_WINDOW_H_