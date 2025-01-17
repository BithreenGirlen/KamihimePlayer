

#include "kamihime_image_transferor.h"

#include "win_filesystem.h"
#include "win_image.h"

CKamihimeImageTransferor::CKamihimeImageTransferor(ID2D1DeviceContext* pD2d1DeviceContext, HWND hWnd)
	: m_pStoredD2d1DeviceContext(pD2d1DeviceContext), m_hRenderWindow(hWnd)
{
	m_winTimer.SetCallback(&CKamihimeImageTransferor::TimerCallback, this);
	m_winTimer.SetDerfaultInterval(Constants::kInterval);
}

CKamihimeImageTransferor::~CKamihimeImageTransferor()
{
	m_winTimer.End();
}

bool CKamihimeImageTransferor::SetImages(std::vector<std::wstring>& imageFilePaths)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	ClearImages();

	for (const auto& imageFilePath : imageFilePaths)
	{
		SImageFrame sWhole{};
		bool bRet = win_image::LoadImageToMemory(imageFilePath.c_str(), &sWhole, 1.f, win_image::ERotation::Deg270);
		if (!bRet)continue;

		std::vector<CComPtr<ID2D1Bitmap>> bitmaps;

		const auto ImportImage = [this, &bitmaps](const SPortion &sPortion, UINT uiStride)
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
			*  8   9   10  11
			*  12  13  14  15
			* 
			* At runtime, this is tranformed by 270deg rotation to:
			*  4  8  12  16
			*  3  7  11  15
			*  2  6  10  14
			*  1  5  9   13
			* 
			* In adddition, there is a loop count specification with second parameter,
			* which performs role only when eye blinking is represented by unequal count of each animation.
			* "film":[
			* "0001-1-2_c1_0.jpg",2,
			* "0001-1-2_c1_1.jpg",2,
			* ],
			* For simplicity, 
			* 
			* 
			* Consideration of height may not affect older animations because they does not have horizontal elements.
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

		if (!bitmaps.empty())
		{
			m_images.push_back(std::move(bitmaps));
		}
	}

	if (!m_images.empty())
	{
		m_winTimer.Start();
	}
	else
	{
		m_winTimer.End();
	}

	return !m_images.empty();
}
/*画像寸法取得*/
void CKamihimeImageTransferor::GetImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
{
	if (m_nImageIndex < m_images.size() || m_nAnimationIndex < m_images[m_nImageIndex].size())
	{
		D2D1_SIZE_U s = m_images[m_nImageIndex][m_nAnimationIndex]->GetPixelSize();
		*uiWidth = s.width;
		*uiHeight = s.height;
	}
}
/*画像移行*/
void CKamihimeImageTransferor::ShiftImage()
{
	if (m_nImageIndex >= m_images.size() || m_nAnimationIndex >= m_images[m_nImageIndex].size())
	{
		return;
	}

	if (m_bPaused)
	{
		ShiftAnimation();
	}
	else
	{
		m_nAnimationIndex = 0;
		if (++m_nImageIndex >= m_images.size())m_nImageIndex = 0;
	}
}
/*現在の画像受け渡し*/
ID2D1Bitmap* CKamihimeImageTransferor::GetCurrentImage()
{
	if (m_nImageIndex >= m_images.size() || m_nAnimationIndex >= m_images[m_nImageIndex].size())
	{
		return nullptr;
	}

	ID2D1Bitmap* p = m_images[m_nImageIndex][m_nAnimationIndex];
	if (!m_bPaused)
	{
		ShiftAnimation();
	}

	return p;
}
/*停止切り替え*/
bool CKamihimeImageTransferor::SwitchPause()
{
	m_bPaused ^= true;
	return m_bPaused;
}
/*コマ送り加速・減速*/
void CKamihimeImageTransferor::RescaleTimer(bool bFaster)
{
	long long llInterval = m_winTimer.GetInterval();

	if (bFaster)
	{
		if (--llInterval <= 1)llInterval = 1;
	}
	else
	{
		++llInterval;
	}

	m_winTimer.SetInterval(llInterval);
}
/*速度初期化*/
void CKamihimeImageTransferor::ResetSpeed()
{
	m_winTimer.ResetInterval();
}
/*消去*/
void CKamihimeImageTransferor::ClearImages()
{
	m_images.clear();
	m_nImageIndex = 0;
	m_nAnimationIndex = 0;

	ResetSpeed();
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
/*再描画要求*/
void CKamihimeImageTransferor::TimerCallback(void* pData)
{
	auto pThis = static_cast<CKamihimeImageTransferor*>(pData);
	if (pThis != nullptr)
	{
		HWND hWnd = pThis->m_hRenderWindow;
		if (::IsWindow(hWnd))
		{
			::InvalidateRect(hWnd, nullptr, FALSE);
		}
	}
}
