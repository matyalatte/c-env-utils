// For getOSInfoFromWMI().
#include <stdio.h>
#include <comdef.h>
#include <wbemidl.h>

#include "env_utils.h"
#include "env_utils_windows.h"
#include "env_utils_priv.h"

#if _MSC_VER && !__INTEL_COMPILER
#pragma comment(lib, "wbemuuid.lib")
#endif

// Most code is from the official document
// https://learn.microsoft.com/en-us/windows/win32/wmisdk/example-creating-a-wmi-application
wchar_t *getOSInfoFromWMI(const wchar_t *key) {
    HRESULT hres;

    // Initialize COM
    hres =  CoInitializeEx(0, COINIT_MULTITHREADED);
    // Note: We can still use COM when hres == RPC_E_CHANGED_MODE.
    if (FAILED(hres) && hres != RPC_E_CHANGED_MODE) {
        // Failed to initialize COM library.
        return NULL;
    }

    // Set general COM security levels
    hres =  CoInitializeSecurity(
        NULL,
        -1,                           // COM authentication
        NULL,                         // Authentication services
        NULL,                         // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,    // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE,  // Default Impersonation
        NULL,                         // Authentication info
        EOAC_NONE,                    // Additional capabilities
        NULL);

    // Note: We can still use COM when hres == RPC_E_TOO_LATE.
    if (FAILED(hres) && hres != RPC_E_TOO_LATE) {
        // Failed to initialize security.
        CoUninitialize();
        return NULL;
    }

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *) &pLoc);

    if (FAILED(hres)) {
        // Failed to create IWbemLocator object.
        CoUninitialize();
        return NULL;
    }

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    IWbemServices *pSvc = NULL;

    // Note: We should not use bstr_t() or mingw can't compile.
    // https://stackoverflow.com/questions/51363689/qt5-mingw-undefined-reference-to-convertstringtobstr
    BSTR ns = SysAllocString(L"ROOT\\CIMV2");
    hres = pLoc->ConnectServer(
        ns,    // Object path of WMI namespace
        NULL,  // User name. NULL = current user
        NULL,  // User password. NULL = current
        0,     // Locale. NULL indicates current
        NULL,  // Security flags.
        0,     // Authority (for example, Kerberos)
        0,     // Context object
        &pSvc);
    SysFreeString(ns);

    if (FAILED(hres)) {
        // Failed to connect.
        pLoc->Release();
        CoUninitialize();
        return NULL;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                         // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,            // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,             // RPC_C_AUTHZ_xxx
        NULL,                         // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,       // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE,  // RPC_C_IMP_LEVEL_xxx
        NULL,                         // client identity
        EOAC_NONE);

    if (FAILED(hres)) {
        // Failed to set proxy blanket.
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return NULL;
    }

    // Use the IWbemServices pointer to make requests of WMI
    BSTR lang = SysAllocString(L"WQL");
    BSTR query = SysAllocString(L"SELECT * FROM Win32_OperatingSystem");
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        lang,
        query,
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);
    SysFreeString(lang);
    SysFreeString(query);

    if (FAILED(hres)) {
        // Query for operating system failed.
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return NULL;
    }

    // Get the data from the query.
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
    wchar_t *value = NULL;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn)
            break;

        VARIANT vtProp;

        VariantInit(&vtProp);
        // Get the value of a property (key).
        hr = pclsObj->Get(key, 0, &vtProp, 0, 0);
        value = AllocWstrWithConst(vtProp.bstrVal);
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return value;
}
