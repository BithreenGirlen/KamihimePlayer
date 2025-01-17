#ifndef KAMIHIME_IMAGE_TRANSFEROR
#define KAMIHIME_IMAGE_TRANSFEROR

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <atlbase.h>

#include <vector>
#include <string>

#include "win_timer.h"

class CKamihimeImageTransferor
{
public:
	CKamihimeImageTransferor(ID2D1DeviceContext* pD2d1DeviceContext, HWND hWnd);
	~CKamihimeImageTransferor();

	bool SetImages(std::vector<std::vector<std::wstring>>& imageFilePathsList);
	void GetImageSize(unsigned int* uiWidth, unsigned int* uiHeight);

	void ShiftImage();
	ID2D1Bitmap* GetCurrentImage();

	bool SwitchPause();
	void RescaleTimer(bool bFaster);
	void ResetSpeed();
private:
	enum Constants
	{
		kInterval = 32, kWidth = 900, kHeight = 640
	};

	struct SPortion
	{
		UINT uiWidth = Constants::kWidth;
		UINT uiHeight = Constants::kHeight;
		void* pData = nullptr;
	};

	ID2D1DeviceContext* m_pStoredD2d1DeviceContext = nullptr;
	HWND m_hRenderWindow = nullptr;

	std::vector<std::vector<CComPtr<ID2D1Bitmap>>> m_images;
	size_t m_nImageIndex = 0;
	size_t m_nAnimationIndex = 0;

	bool m_bPaused = false;

	void ClearImages();

	void ShiftAnimation();

	CWinTimer m_winTimer;

	static void TimerCallback(void* pData);
};
#endif // !KAMIHIME_IMAGE_TRANSFEROR
