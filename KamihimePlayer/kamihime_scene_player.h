#ifndef KAMIHIME_SCENE_PLAYER_H_
#define KAMIHIME_SCENE_PLAYER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>

#include <string>
#include <vector>

struct ImageInfo
{
    UINT uiWidth = 0;
    UINT uiHeight = 0;
    INT iStride = 0;
    std::vector<BYTE> pixels;
};

class CKamihimeScenePlayer
{
public:
    CKamihimeScenePlayer(HWND hWnd);
    ~CKamihimeScenePlayer();

    bool SetFiles(const std::vector<std::wstring>& filePaths);
    bool DrawImage();
    void Display();

    void Next();
    void UpScale();
    void DownScale();
    void ResetScale();
    void SwitchSizeLore(bool bBarHidden);
    void SetOffset(int iX, int iY);
    void SpeedUp();
    void SpeedDown();
    bool SwitchPause();

    ID2D1Factory1* GetD2Factory()const { return m_pD2d1Factory1; }
    ID2D1DeviceContext* GetD2DeviceContext()const { return m_pD2d1DeviceContext; }
private:
    HWND m_hRetWnd = nullptr;

    enum Portion { kInterval = 32, kWidth = 900 };

    HRESULT m_hrComInit = E_FAIL;
    ID2D1Factory1* m_pD2d1Factory1 = nullptr;
    ID2D1DeviceContext* m_pD2d1DeviceContext = nullptr;
    IDXGISwapChain1* m_pDxgiSwapChain1 = nullptr;

    std::vector<ImageInfo> m_image_info;
    UINT m_uiPortion = 0;
    size_t m_nIndex = 0;

    double m_dbScale = 1.0;
    int m_iXOffset = 0;
    int m_iYOffset = 0;
    long long m_interval = Portion::kInterval;

    bool m_bBarHidden = false;
    bool m_bPause = false;

    void Clear();
    bool LoadImageToMemory(const wchar_t* pwzFilePath);
    void Update();
    void ResizeWindow();
    void AdjustOffset(int iXAddOffset, int iYAddOffset);
    void ResizeBuffer();

    void StartThreadpoolTimer();
    void EndThreadpoolTimer();

    PTP_TIMER m_timer = nullptr;
    static void CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer);
};

#endif //KAMIHIME_SCENE_PLAYER_H_
