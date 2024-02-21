

#include "kamihime_scene_player.h"

#include <math.h>
#include <shobjidl.h>
#include <atlbase.h>
#include <wincodec.h>

#pragma comment (lib,"Windowscodecs.lib")
#pragma comment (lib,"D2d1.lib")
#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"dxguid.lib")

CKamihimeScenePlayer::CKamihimeScenePlayer(HWND hWnd)
	:m_hRetWnd(hWnd)
{
	m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(m_hrComInit))return;

	CComPtr<ID3D11Device>pD3d11Device;
	HRESULT hr = ::D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED, nullptr, 0, D3D11_SDK_VERSION,
		&pD3d11Device, nullptr, nullptr);
	if (FAILED(hr))return;

	CComPtr<IDXGIDevice1> pDxgDevice1;
	hr = pD3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDxgDevice1);
	if (FAILED(hr))return;

	hr = pDxgDevice1->SetMaximumFrameLatency(1);
	if (FAILED(hr))return;

	CComPtr<IDXGIAdapter> pDxgiAdapter;
	hr = pDxgDevice1->GetAdapter(&pDxgiAdapter);
	if (FAILED(hr))return;

	CComPtr<IDXGIFactory2> pDxgiFactory2;
	hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory2));
	if (FAILED(hr))return;

	hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2d1Factory1);
	if (FAILED(hr))return;

	CComPtr<ID2D1Device> pD2d1Device;
	hr = m_pD2d1Factory1->CreateDevice(pDxgDevice1, &pD2d1Device);
	if (FAILED(hr))return;

	hr = pD2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2d1DeviceContext);
	if (FAILED(hr))return;

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 1;
	desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	hr = pDxgiFactory2->CreateSwapChainForHwnd(pDxgDevice1, hWnd, &desc, nullptr, nullptr, &m_pDxgiSwapChain1);
	if (FAILED(hr))return;

	m_pD2d1DeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_pD2d1DeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
	m_pD2d1DeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
	D2D1_RENDERING_CONTROLS sRenderings{};
	m_pD2d1DeviceContext->GetRenderingControls(&sRenderings);
	sRenderings.bufferPrecision = D2D1_BUFFER_PRECISION_8BPC_UNORM_SRGB;
	m_pD2d1DeviceContext->SetRenderingControls(sRenderings);

}

CKamihimeScenePlayer::~CKamihimeScenePlayer()
{
	EndThreadpoolTimer();

	if (m_pDxgiSwapChain1 != nullptr)
	{
		m_pDxgiSwapChain1->Release();
		m_pDxgiSwapChain1 = nullptr;
	}

	if (m_pD2d1DeviceContext != nullptr)
	{
		m_pD2d1DeviceContext->Release();
		m_pD2d1DeviceContext = nullptr;
	}

	if (m_pD2d1Factory1 != nullptr)
	{
		m_pD2d1Factory1->Release();
		m_pD2d1Factory1 = nullptr;
	}

	if (SUCCEEDED(m_hrComInit))
	{
		::CoUninitialize();
	}

}
/*ファイル設定*/
bool CKamihimeScenePlayer::SetFiles(const std::vector<std::wstring>& filePaths)
{
	Clear();
	for (const std::wstring& filePath : filePaths)
	{
		LoadImageToMemory(filePath.c_str());
	}
	ResetScale();

	return m_image_info.size() > 0;
}
/*描画*/
bool CKamihimeScenePlayer::DisplayImage()
{
	if (m_image_info.empty() || m_nIndex >= m_image_info.size() || m_pD2d1DeviceContext == nullptr || m_pDxgiSwapChain1 == nullptr)
	{
		return false;
	}

	ImageInfo s = m_image_info.at(m_nIndex);

	CComPtr<ID2D1Bitmap> pD2d1Bitmap;
	HRESULT hr = E_FAIL;
	UINT uiWidth = s.uiWidth;
	UINT uiHeight = s.uiHeight;
	INT iStride = s.iStride;

	UINT uiDiv = s.uiWidth / Portion::kWidth;
	if (uiDiv > 1)
	{
		StartThreadpoolTimer();

		uiWidth = s.uiWidth / uiDiv;
		iStride = s.iStride / uiDiv;

		hr = m_pD2d1DeviceContext->CreateBitmap(D2D1::SizeU(uiWidth, uiHeight),
			D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&pD2d1Bitmap);
		if (SUCCEEDED(hr))
		{
			D2D1_RECT_U rc = {0, 0, uiWidth, uiHeight };
			hr = pD2d1Bitmap->CopyFromMemory(&rc, s.pixels.data() + iStride * m_uiPortion, s.iStride);
		}

		if (!m_bPause)
		{
			++m_uiPortion;
			if (m_uiPortion >= uiDiv)m_uiPortion = 0;
		}
	}
	else
	{
		hr = m_pD2d1DeviceContext->CreateBitmap(D2D1::SizeU(uiWidth, uiHeight),
			D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&pD2d1Bitmap);
		if (SUCCEEDED(hr))
		{
			hr = pD2d1Bitmap->CopyFromMemory(nullptr, s.pixels.data(), s.iStride);
		}
	}

	if (SUCCEEDED(hr))
	{
		FLOAT fScale = static_cast<FLOAT>(::round(m_dbScale * 1000) / 1000);

		CComPtr<ID2D1Effect> pD2d1Effect;
		hr = m_pD2d1DeviceContext->CreateEffect(CLSID_D2D1Scale, &pD2d1Effect);
		pD2d1Effect->SetInput(0, pD2d1Bitmap);
		hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_CENTER_POINT, D2D1::Vector2F(static_cast<FLOAT>(m_iXOffset), static_cast<FLOAT>(m_iYOffset)));
		hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(fScale, fScale));
		m_pD2d1DeviceContext->BeginDraw();
		m_pD2d1DeviceContext->DrawImage(pD2d1Effect, D2D1::Point2F(0.f, 0.f), D2D1::RectF(static_cast<FLOAT>(m_iXOffset), static_cast<FLOAT>(m_iYOffset), uiWidth * fScale, uiHeight * fScale), D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, D2D1_COMPOSITE_MODE_SOURCE_COPY);
		m_pD2d1DeviceContext->EndDraw();

		DXGI_PRESENT_PARAMETERS params{};
		m_pDxgiSwapChain1->Present1(1, 0, &params);
	}

	return true;
}
/*次画像*/
void CKamihimeScenePlayer::Next()
{
	if (m_bPause && m_nIndex < m_image_info.size())
	{
		UINT uiDiv = m_image_info.at(m_nIndex).uiWidth / Portion::kWidth;
		if (uiDiv > 1)
		{
			++m_uiPortion;
			if (m_uiPortion >= uiDiv)m_uiPortion = 0;
		}
	}
	else
	{
		EndThreadpoolTimer();
		++m_nIndex;
		if (m_nIndex >= m_image_info.size())m_nIndex = 0;
	}

	Update();
}
/*拡大*/
void CKamihimeScenePlayer::UpScale()
{
	if (m_dbScale < 3.99)
	{
		m_dbScale += 0.05;
		ResizeWindow();
	}
}
/*縮小*/
void CKamihimeScenePlayer::DownScale()
{
	if (m_dbScale > 0.51)
	{
		m_dbScale -= 0.05;
		ResizeWindow();
	}
}
/*原寸表示*/
void CKamihimeScenePlayer::ResetScale()
{
	m_dbScale = 1.0;
	m_iXOffset = 0;
	m_iYOffset = 0;
	m_interval = Portion::kInterval;
	ResizeWindow();
}
/*窓枠寸法計算法切り替え*/
void CKamihimeScenePlayer::SwitchSizeLore(bool bBarHidden)
{
	m_bBarHidden = bBarHidden;
	ResizeWindow();
}
/*原点位置移動*/
void CKamihimeScenePlayer::SetOffset(int iX, int iY)
{
	AdjustOffset(iX, iY);
	Update();
}
/*コマ送り加速*/
void CKamihimeScenePlayer::SpeedUp()
{
	if (m_interval > 1)
	{
		--m_interval;
	}
}
/*コマ送り減速*/
void CKamihimeScenePlayer::SpeedDown()
{
	if (m_interval < 1000)
	{
		++m_interval;
	}
}
/*一時停止切り替え*/
bool CKamihimeScenePlayer::SwitchPause()
{
	m_bPause ^= true;
	return m_bPause;
}
/*消去*/
void CKamihimeScenePlayer::Clear()
{
	m_image_info.clear();
	m_nIndex = 0;
}
/*画像ファイル取り込み*/
bool CKamihimeScenePlayer::LoadImageToMemory(const wchar_t* pwzFilePath)
{
	ImageInfo s;

	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
	hr = pWicImageFactory->CreateDecoderFromFilename(pwzFilePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pWicBitmapDecoder);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapFrameDecode> pWicFrameDecode;
	hr = pWicBitmapDecoder->GetFrame(0, &pWicFrameDecode);
	if (FAILED(hr))return false;

	CComPtr<IWICFormatConverter> pWicFormatConverter;
	hr = pWicImageFactory->CreateFormatConverter(&pWicFormatConverter);
	if (FAILED(hr))return false;

	pWicFormatConverter->Initialize(pWicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapFlipRotator> pWicFlipRotator;
	hr = pWicImageFactory->CreateBitmapFlipRotator(&pWicFlipRotator);
	if (FAILED(hr))return false;

	hr = pWicFlipRotator->Initialize(pWicFormatConverter, WICBitmapTransformRotate270);
	if (FAILED(hr))return false;

	hr = pWicFlipRotator->GetSize(&s.uiWidth, &s.uiHeight);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmap> pWicBitmap;
	hr = pWicImageFactory->CreateBitmapFromSource(pWicFlipRotator, WICBitmapCacheOnDemand, &pWicBitmap);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapLock> pWicBitmapLock;
	WICRect wicRect{ 0, 0, static_cast<INT>(s.uiWidth), static_cast<INT>(s.uiHeight) };
	hr = pWicBitmap->Lock(&wicRect, WICBitmapLockRead, &pWicBitmapLock);
	if (FAILED(hr))return false;

	UINT uiStride;
	hr = pWicBitmapLock->GetStride(&uiStride);
	if (FAILED(hr))return false;

	s.iStride = static_cast<INT>(uiStride);
	s.pixels.resize(static_cast<size_t>(s.iStride * s.uiHeight));
	hr = pWicBitmap->CopyPixels(nullptr, uiStride, static_cast<UINT>(s.pixels.size()), s.pixels.data());
	if (FAILED(hr))return false;

	m_image_info.push_back(s);

	return true;
}
/*画面更新*/
void CKamihimeScenePlayer::Update()
{
	if (m_hRetWnd != nullptr)
	{
		::InvalidateRect(m_hRetWnd, NULL, TRUE);
	}
}
/*窓枠寸法調整*/
void CKamihimeScenePlayer::ResizeWindow()
{
	if (!m_image_info.empty() && m_hRetWnd != nullptr)
	{
		RECT rect;
		if (!m_bBarHidden)
		{
			::GetWindowRect(m_hRetWnd, &rect);
		}
		else
		{
			::GetClientRect(m_hRetWnd, &rect);
		}

		int iX = static_cast<int>(::round(Portion::kWidth * (m_dbScale * 1000) / 1000));
		int iY = static_cast<int>(::round(m_image_info.at(0).uiHeight * (m_dbScale * 1000) / 1000));
		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		if (!m_bBarHidden)
		{
			LONG lStyle = ::GetWindowLong(m_hRetWnd, GWL_STYLE);
			::AdjustWindowRect(&rect, lStyle, TRUE);
			::SetWindowPos(m_hRetWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			RECT rc;
			::GetWindowRect(m_hRetWnd, &rc);
			::MoveWindow(m_hRetWnd, rc.left, rc.top, rect.right, rect.bottom, TRUE);
		}

		ResizeBuffer();
		AdjustOffset(0, 0);
		Update();
	}

}
/*原点位置調整*/
void CKamihimeScenePlayer::AdjustOffset(int iXAddOffset, int iYAddOffset)
{
	if (!m_image_info.empty() && m_hRetWnd != nullptr)
	{
		int iX = static_cast<int>(::round(Portion::kWidth * (m_dbScale * 1000) / 1000));
		int iY = static_cast<int>(::round(m_image_info.at(0).uiHeight * (m_dbScale * 1000) / 1000));

		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iXOffsetMax = iX > iClientWidth ? static_cast<int>(::floor((iX - iClientWidth) / ((m_dbScale * 1000) / 1000))) : 0;
		int iYOffsetMax = iY > iClientHeight ? static_cast<int>(::floor((iY - iClientHeight) / ((m_dbScale * 1000) / 1000))) : 0;

		m_iXOffset += iXAddOffset;
		if (m_iXOffset < 0) m_iXOffset = 0;
		if (m_iXOffset > iXOffsetMax)m_iXOffset = iXOffsetMax;
		m_iYOffset += iYAddOffset;
		if (m_iYOffset < 0) m_iYOffset = 0;
		if (m_iYOffset > iYOffsetMax)m_iYOffset = iYOffsetMax;
	}

}
/*原版寸法変更*/
void CKamihimeScenePlayer::ResizeBuffer()
{
	if (m_pDxgiSwapChain1 != nullptr && m_pD2d1DeviceContext != nullptr && m_hRetWnd != nullptr)
	{
		m_pD2d1DeviceContext->SetTarget(nullptr);

		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);
		HRESULT hr = m_pDxgiSwapChain1->ResizeBuffers(0, rc.right - rc.left, rc.bottom - rc.top, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

		CComPtr<IDXGISurface> pDxgiSurface;
		hr = m_pDxgiSwapChain1->GetBuffer(0, IID_PPV_ARGS(&pDxgiSurface));

		CComPtr<ID2D1Bitmap1> pD2d1Bitmap1;
		hr = m_pD2d1DeviceContext->CreateBitmapFromDxgiSurface(pDxgiSurface, nullptr, &pD2d1Bitmap1);

		m_pD2d1DeviceContext->SetTarget(pD2d1Bitmap1);
	}
}
/*コマ送り開始*/
void CKamihimeScenePlayer::StartThreadpoolTimer()
{
	if (m_timer != nullptr)return;

	m_timer = ::CreateThreadpoolTimer(TimerCallback, this, nullptr);
	if (m_timer != nullptr)
	{
		m_uiPortion = 0;

		FILETIME FileDueTime{};
		ULARGE_INTEGER ulDueTime{};
		ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * m_interval));
		FileDueTime.dwHighDateTime = ulDueTime.HighPart;
		FileDueTime.dwLowDateTime = ulDueTime.LowPart;
		::SetThreadpoolTimer(m_timer, &FileDueTime, 0, 0);
	}

}
/*コマ送り終了*/
void CKamihimeScenePlayer::EndThreadpoolTimer()
{
	if (m_timer != nullptr)
	{
		::SetThreadpoolTimer(m_timer, nullptr, 0, 0);
		::WaitForThreadpoolTimerCallbacks(m_timer, TRUE);
		::CloseThreadpoolTimer(m_timer);
		m_timer = nullptr;
	}
}
/*コマ送り処理スレッドプール*/
void CKamihimeScenePlayer::TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
	CKamihimeScenePlayer* pThis = static_cast<CKamihimeScenePlayer*>(Context);
	if (pThis != nullptr)
	{
		pThis->Update();

		FILETIME FileDueTime{};
		ULARGE_INTEGER ulDueTime{};
		ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * pThis->m_interval));
		FileDueTime.dwHighDateTime = ulDueTime.HighPart;
		FileDueTime.dwLowDateTime = ulDueTime.LowPart;

		::SetThreadpoolTimer(Timer, &FileDueTime, 0, 0);
	}

}