#pragma once
#include <windows.h>
#include <Ole2.h>

struct DropTarget : IDropTarget {

    explicit DropTarget(HWND hwnd);

    LONG usersAmount;
    HWND hwnd;

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;

    ULONG __stdcall AddRef() override;

    ULONG __stdcall Release() override;

    HRESULT __stdcall DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

    HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

    HRESULT __stdcall DragLeave() override;

    HRESULT __stdcall Drop(IDataObject* pDataObj, DWORD grfKeyState,POINTL pt, DWORD* pdwEffect) override;

    bool getFilePaths(IDataObject* pDataObj, TCHAR* path);

};