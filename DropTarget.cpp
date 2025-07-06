#include "DropTarget.h"
#include <fmt/base.h>
#include <string>

DropTarget::DropTarget(HWND hwnd) : usersAmount(1), hwnd(hwnd) {}

HRESULT __stdcall DropTarget::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) return E_POINTER;

    if (riid == IID_IUnknown || riid == IID_IDropTarget) {
        *ppvObject = static_cast<IDropTarget*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG __stdcall DropTarget::AddRef() {
    return InterlockedIncrement(&usersAmount);
}

ULONG __stdcall DropTarget::Release() {
    ULONG count = InterlockedDecrement(&usersAmount);
    if (count == 0)
        delete this;
    return count;
}

HRESULT __stdcall DropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
    fmt::println("test");
    TCHAR path[MAX_PATH];
    getFilePaths(pDataObj, path);
    //*pdwEffect = DROPEFFECT_COPY; // Default accept copy
    return S_OK;
}

HRESULT __stdcall DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
    // TODO: Add your drag over logic here
    //*pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}

HRESULT __stdcall DropTarget::DragLeave() {
    return S_OK;
}

HRESULT __stdcall DropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
    //*pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}

bool DropTarget::getFilePaths(IDataObject* pDataObj, TCHAR* path) {

    FORMATETC fmt = {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stg = {};

    if (FAILED(pDataObj->GetData(&fmt, &stg))) {
        fmt::println("drag drop failed");
        return false;
    }

    auto hDrop = (HDROP)GlobalLock(stg.hGlobal);
    if (!hDrop) {
        fmt::println("drag drop failed");
        ReleaseStgMedium(&stg);
        return false;
    }

    auto success = false;
    if (DragQueryFile(hDrop, 0, path, MAX_PATH)) {
        success = true;
    }

    auto fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
    for (auto i = 0; i < fileCount; ++i) {
        TCHAR filePath[MAX_PATH];
        if (DragQueryFile(hDrop, i, filePath, MAX_PATH)) {
            wprintf(L"%s", filePath);
        }
    }

    GlobalUnlock(stg.hGlobal);
    ReleaseStgMedium(&stg);

    return success;

}