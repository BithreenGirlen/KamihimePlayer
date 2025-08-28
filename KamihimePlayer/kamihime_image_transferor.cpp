

#include "kamihime_image_transferor.h"

#include "kmhm.h"
#include "win_filesystem.h"
#include "win_image.h"

CKamihimeImageTransferor::CKamihimeImageTransferor(ID2D1DeviceContext* pD2d1DeviceContext)
	: m_pStoredD2d1DeviceContext(pD2d1DeviceContext)
{

}

CKamihimeImageTransferor::~CKamihimeImageTransferor()
{

}

bool CKamihimeImageTransferor::LoadScenario(const wchar_t* pwzFolderPath)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	ClearScenarioData();

	std::vector<std::vector<std::wstring>> imageFileNamesList;
	bool bRet = kmhm::ReadScenario(pwzFolderPath, m_textData, imageFileNamesList, m_sceneData, m_labelData);
	if (!bRet)return false;

	bRet = LoadImages(imageFileNamesList);
	m_animationClock.Restart();

	return bRet;
}

bool CKamihimeImageTransferor::HasScenarioData() const
{
	return !m_sceneData.empty();
}
/* 現在の画像寸法取得 */
void CKamihimeImageTransferor::GetCurrentImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
{
	if (m_nImageIndex < m_images.size() || m_nAnimationIndex < m_images[m_nImageIndex].size())
	{
		D2D1_SIZE_U s = m_images[m_nImageIndex][m_nAnimationIndex]->GetPixelSize();
		if (uiWidth != nullptr)*uiWidth = s.width;
		if (uiHeight != nullptr)*uiHeight = s.height;
	}
}
/* 最大の画像寸法取得 */
void CKamihimeImageTransferor::GetLargestImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
{
	unsigned int uiMaxWidth = 0;
	unsigned int uiMaxHeight = 0;

	for (const auto& imageList : m_images)
	{
		for (const auto& pD2Bitmap : imageList)
		{
			D2D1_SIZE_U s = pD2Bitmap->GetPixelSize();

			uiMaxWidth = (std::max)(uiMaxWidth, s.width);
			uiMaxHeight = (std::max)(uiMaxHeight, s.height);;
		}
	}

	if (uiWidth != nullptr)*uiWidth = uiMaxWidth;
	if (uiHeight != nullptr)*uiHeight = uiMaxHeight;
}
/*場面移行*/
void CKamihimeImageTransferor::ShiftScene(bool bForward)
{
	if (m_sceneData.empty())return;

	if (bForward)
	{
		if (++m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = 0;
		}
	}
	else
	{
		if (--m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = m_sceneData.size() - 1;
		}
	}
}
/*最終場面是否*/
bool CKamihimeImageTransferor::HasReachedLastScene()
{
	return m_nSceneIndex == m_sceneData.size() - 1;
}
/*現在の画像受け渡し*/
ID2D1Bitmap* CKamihimeImageTransferor::GetCurrentImage()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		if (m_bImageSynced)
		{
			m_nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
		}

		if (m_nImageIndex < m_images.size())
		{
			if (m_nAnimationIndex >= m_images[m_nImageIndex].size())
			{
				m_nAnimationIndex = 0;
			}

			ID2D1Bitmap* p = m_images[m_nImageIndex][m_nAnimationIndex];
			if (!m_bPaused)
			{
				float fElapsed = m_animationClock.GetElapsedTime();
				if (::isgreaterequal(fElapsed, 1000 / static_cast<float>(m_iFps)))
				{
					ShiftAnimation();
					m_animationClock.Restart();
				}
			}

			return p;
		}
	}

	return nullptr;
}

std::wstring CKamihimeImageTransferor::GetCurrentFormattedText()
{
	std::wstring wstr;
	if (m_nSceneIndex < m_sceneData.size())
	{
		wstr.reserve(128);
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;

		if (nTextIndex < m_textData.size())
		{
			wstr = m_textData[nTextIndex].wstrText;
			if (!wstr.empty() && wstr.back() != L'\n') wstr += L"\n ";
			wstr += std::to_wstring(nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
		}
	}

	return wstr;
}

const wchar_t* CKamihimeImageTransferor::GetCurrentVoiceFilePath()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
		if (nTextIndex < m_textData.size())
		{
			return m_textData[nTextIndex].wstrVoicePath.c_str();
		}
	}

	return nullptr;
}
/*停止切り替え*/
bool CKamihimeImageTransferor::TogglePause()
{
	m_animationClock.Restart();

	m_bPaused ^= true;
	return m_bPaused;
}

bool CKamihimeImageTransferor::IsPaused() const
{
	return m_bPaused;
}
/*コマ送り加速・減速*/
void CKamihimeImageTransferor::UpdateAnimationInterval(bool bFaster)
{
	if (bFaster)
	{
		++m_iFps;
	}
	else
	{
		if (--m_iFps <= 1)m_iFps = 1;
	}
}
/*速度初期化*/
void CKamihimeImageTransferor::ResetAnimationInterval()
{
	m_iFps = Constants::kDefaultFps;
}
/*コマ送り*/
void CKamihimeImageTransferor::ShiftAnimation()
{
	if (m_nImageIndex >= m_images.size() || m_nAnimationIndex >= m_images[m_nImageIndex].size())
	{
		return;
	}

	if (++m_nAnimationIndex >= m_images[m_nImageIndex].size())
	{
		m_nAnimationIndex = 0;
	}
}

bool CKamihimeImageTransferor::ToggleImageSync()
{
	m_bImageSynced ^= true;

	return m_bImageSynced;
}

bool CKamihimeImageTransferor::IsImageSynced() const
{
	return m_bImageSynced;
}

void CKamihimeImageTransferor::ShiftImage()
{
	if (!m_bImageSynced)
	{
		++m_nImageIndex;
		if (m_nImageIndex >= m_images.size())m_nImageIndex = 0;
	}
}

const std::vector<adv::LabelDatum>& CKamihimeImageTransferor::GetLabelData() const
{
	return m_labelData;
}

bool CKamihimeImageTransferor::JumpToLabel(size_t nLabelIndex)
{
	if (nLabelIndex < m_labelData.size())
	{
		const auto& labelDatum = m_labelData[nLabelIndex];

		if (labelDatum.nSceneIndex < m_sceneData.size())
		{
			m_nSceneIndex = labelDatum.nSceneIndex;

			return true;
		}
	}
	return false;
}

void CKamihimeImageTransferor::ClearScenarioData()
{
	m_textData.clear();

	m_sceneData.clear();
	m_nSceneIndex = 0;

	m_images.clear();
	m_nImageIndex = 0;
	m_nAnimationIndex = 0;

	m_labelData.clear();

	ResetAnimationInterval();
}
bool CKamihimeImageTransferor::LoadImages(std::vector<std::vector<std::wstring>>& imageFilePathsList)
{
	for (const auto& imageFilePaths : imageFilePathsList)
	{
		std::vector<CComPtr<ID2D1Bitmap>> bitmaps;

		for (const auto& imageFilePath : imageFilePaths)
		{
			SImageFrame sWhole{};
			bool bRet = win_image::LoadImageToMemory(imageFilePath.c_str(), &sWhole, 1.f, win_image::ERotation::Deg270);
			if (!bRet)continue;

			const auto ImportImage = [this, &bitmaps](const SPortion& sPortion, UINT uiStride)
				-> void
				{
					CComPtr<ID2D1Bitmap> pD2d1Bitmap;

					HRESULT hr = m_pStoredD2d1DeviceContext->CreateBitmap
					(
						D2D1::SizeU(sPortion.uiWidth, sPortion.uiHeight),
						D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
						&pD2d1Bitmap
					);

					D2D1_RECT_U rc{ 0, 0, sPortion.uiWidth, sPortion.uiHeight };

					hr = pD2d1Bitmap->CopyFromMemory(&rc, sPortion.pData, uiStride);
					if (SUCCEEDED(hr))
					{
						bitmaps.push_back(std::move(pD2d1Bitmap));
					}
				};

			UINT uiDivX = sWhole.uiWidth / Constants::kWidth;
			UINT uiDivY = sWhole.uiHeight / Constants::kHeight;

			if (uiDivX == 0 || uiDivY == 0)continue;

			if (uiDivX == 1)
			{
				SPortion sPortion{};
				sPortion.pData = sWhole.pixels.data();

				ImportImage(sPortion, sWhole.iStride);
			}
			else
			{
				SPortion sPortion{};
				sPortion.uiHeight = sWhole.uiHeight / uiDivY;
				sPortion.uiWidth = sWhole.uiWidth / uiDivX;

				INT iStride = sWhole.iStride / uiDivX;

				/*
				* Split sequence of burst scene animations is:
				*  1   2   3   4
				*  5   6   7   8
				*  9   10  11  12
				*  13  14  15  16
				*
				* At runtime, this is tranformed by 270deg rotation to:
				*  4  8  12  16
				*  3  7  11  15
				*  2  6  10  14
				*  1  5  9   13
				*
				* This does not affect older animations because they do not have horizontal elements.
				*/

				for (size_t nPortionX = 0; nPortionX < uiDivX; ++nPortionX)
				{
					for (long long nPortionY = uiDivY - 1LL; nPortionY >= 0; --nPortionY)
					{
						sPortion.pData = sWhole.pixels.data() + (nPortionX * iStride) + (nPortionY * sWhole.iStride * sPortion.uiHeight);

						ImportImage(sPortion, sWhole.iStride);
					}
				}
			}
		}

		if (!bitmaps.empty())
		{
			m_images.push_back(std::move(bitmaps));
		}
	}

	m_animationClock.Restart();

	return !m_images.empty();
}
