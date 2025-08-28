#ifndef KAMIHIME_IMAGE_TRANSFEROR
#define KAMIHIME_IMAGE_TRANSFEROR

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <atlbase.h>

#include <vector>
#include <string>

#include "adv.h"
#include "win_clock.h"

class CKamihimeImageTransferor
{
public:
	CKamihimeImageTransferor(ID2D1DeviceContext* pD2d1DeviceContext);
	~CKamihimeImageTransferor();

	bool LoadScenario(const wchar_t* pwzFolderPath);
	bool HasScenarioData() const;

	void GetCurrentImageSize(unsigned int* uiWidth, unsigned int* uiHeight);
	void GetLargestImageSize(unsigned int* uiWidth, unsigned int* uiHeight);

	void ShiftScene(bool bForward);
	bool HasReachedLastScene();

	ID2D1Bitmap* GetCurrentImage();
	std::wstring GetCurrentFormattedText();
	const wchar_t* GetCurrentVoiceFilePath();

	bool TogglePause();
	bool IsPaused() const;

	void UpdateAnimationInterval(bool bFaster);
	void ResetAnimationInterval();
	void ShiftAnimation();

	bool ToggleImageSync();
	bool IsImageSynced() const;

	void ShiftImage();

	const std::vector<adv::LabelDatum>& GetLabelData() const;
	bool JumpToLabel(size_t nLabelIndex);
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

	std::vector<adv::TextDatum> m_textData;
	std::vector<adv::SceneDatum> m_sceneData;
	size_t m_nSceneIndex = 0;
	std::vector<adv::LabelDatum> m_labelData;

	bool m_bPaused = false;
	bool m_bImageSynced = true;

	void ClearScenarioData();
	bool LoadImages(std::vector<std::vector<std::wstring>>& imageFilePathsList);

	CWinClock m_animationClock;
	int m_iFps = Constants::kDefaultFps;
};
#endif // !KAMIHIME_IMAGE_TRANSFEROR
