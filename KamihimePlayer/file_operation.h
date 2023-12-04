#ifndef FILE_OPERATION_H_
#define FILE_OPERATION_H_

#include <shobjidl.h>
#include <atlbase.h>

struct ComInit
{
    HRESULT m_hrComInit;
    ComInit() : m_hrComInit(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)) {}
    ~ComInit() { if (SUCCEEDED(m_hrComInit)) ::CoUninitialize(); }
};

wchar_t* SelectWorkingFolder();

#endif //FILE_OPERATION_H_