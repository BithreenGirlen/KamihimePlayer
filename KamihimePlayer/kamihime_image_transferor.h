#ifndef KAMIHIME_IMAGE_TRANSFEROR
#define KAMIHIME_IMAGE_TRANSFEROR

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <atlbase.h>

#include <vector>
#include <string>

#include "win_clock.h"

class CKamihimeImageTransferor
{
public:
	CKamihimeImageTransferor(ID2D1DeviceContext* pD2d1DeviceContext);
	~CKamihimeImageTransferor();

	bool SetImages(std::vector<std::vector<std::wstring>>& imageFilePathsList);
	void GetImageSize(unsigned int* uiWidth, unsigned int* uiHeight);

	void ShiftImage();
	ID2D1Bitmap* GetCurrentImage();

	bool TogglePause();
	void UpdateAnimationInterval(bool bFaster);
	void ResetAnimationInterval();
private:
	enum Constants
	{
		kDefaultFps = 24, kWidth = 900, kHeight = 640
	};

	struct SPortion
	{
		UINT uiWidth = Constants::kWidth;
		UINT uiHeight = Constants::kHeight;
		void* pData = nullptr;
	};

	ID2D1DeviceContext* m_pStoredD2d1DeviceContext = nullptr;

	std::vector<std::vector<CComPtr<ID2D1Bitmap>>> m_images;
	size_t m_nImageIndex = 0;
	size_t m_nAnimationIndex = 0;

	bool m_bPaused = false;

	void ClearImages();

	void ShiftAnimation();

	CWinClock m_animationClock;
	int m_iFps = Constants::kDefaultFps;
};
#endif // !KAMIHIME_IMAGE_TRANSFEROR
