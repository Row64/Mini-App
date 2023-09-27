/*
  Native File Dialog

  http://www.frogtoss.com/labs
 */

// Modifications Copyright (c) 2022 Row64

/* only locally define UNICODE in this compilation unit */
#ifndef UNICODE
#define UNICODE
#endif

#ifdef __MINGW32__
// Explicitly setting NTDDI version, this is necessary for the MinGW compiler
#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#define _CRT_SECURE_NO_WARNINGS

#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <ShObjIdl.h>
#include "nfd_common.h"
#include <iostream>         // Added by Row64

// allocs the space in outPath -- call free()
static void CopyWCharToNFDChar( const wchar_t *inStr, nfdchar_t **outStr )
{
    int inStrCharacterCount = static_cast<int>(wcslen(inStr)); 
    int bytesNeeded = WideCharToMultiByte( CP_UTF8, 0,
                                           inStr, inStrCharacterCount,
                                           NULL, 0, NULL, NULL );    
    assert( bytesNeeded );
    bytesNeeded += 1;

    *outStr = (nfdchar_t*)NFDi_Malloc( bytesNeeded );
    if ( !*outStr )
        return;

    int bytesWritten = WideCharToMultiByte( CP_UTF8, 0,
                                            inStr, -1,
                                            *outStr, bytesNeeded,
                                            NULL, NULL );
    assert( bytesWritten ); _NFD_UNUSED( bytesWritten );
}

/* includes NULL terminator byte in return */
static size_t GetUTF8ByteCountForWChar( const wchar_t *str )
{
    int bytesNeeded = WideCharToMultiByte( CP_UTF8, 0,
                                           str, -1,
                                           NULL, 0, NULL, NULL );
    assert( bytesNeeded );
    return bytesNeeded+1;
}

// write to outPtr -- no free() necessary.  No memory stomp tests are done -- they must be done
// before entering this function.
static int CopyWCharToExistingNFDCharBuffer( const wchar_t *inStr, nfdchar_t *outPtr )
{
    int inStrCharacterCount = static_cast<int>(wcslen(inStr));
    int bytesNeeded = static_cast<int>(GetUTF8ByteCountForWChar( inStr ));

    /* invocation copies null term */
    int bytesWritten = WideCharToMultiByte( CP_UTF8, 0,
                                            inStr, -1,
                                            outPtr, bytesNeeded,
                                            NULL, 0 );
    assert( bytesWritten );

    return bytesWritten;

}


// allocs the space in outStr -- call free()
static void CopyNFDCharToWChar( const nfdchar_t *inStr, wchar_t **outStr )
{
    int inStrByteCount = static_cast<int>(strlen(inStr));
    int charsNeeded = MultiByteToWideChar(CP_UTF8, 0,
                                          inStr, inStrByteCount,
                                          NULL, 0 );    
    assert( charsNeeded );
    assert( !*outStr );
    charsNeeded += 1; // terminator
    
    *outStr = (wchar_t*)NFDi_Malloc( charsNeeded * sizeof(wchar_t) );    
    if ( !*outStr )
        return;        

    int ret = MultiByteToWideChar(CP_UTF8, 0,
                                  inStr, inStrByteCount,
                                  *outStr, charsNeeded);
    (*outStr)[charsNeeded-1] = '\0';

#ifdef _DEBUG
    int inStrCharacterCount = static_cast<int>(NFDi_UTF8_Strlen(inStr));
    assert( ret == inStrCharacterCount );
#else
    _NFD_UNUSED(ret);
#endif
}


/* ext is in format "jpg", no wildcards or separators */
static int AppendExtensionToSpecBuf( const char *ext, char *specBuf, size_t specBufLen )
{
    const char SEP[] = ";";
    assert( specBufLen > strlen(ext)+3 );
    
    if ( strlen(specBuf) > 0 )
    {
        strncat( specBuf, SEP, specBufLen - strlen(specBuf) - 1 );
        specBufLen += strlen(SEP);
    }

    char extWildcard[NFD_MAX_STRLEN];
    int bytesWritten = sprintf_s( extWildcard, NFD_MAX_STRLEN, "*.%s", ext );
    assert( bytesWritten == strlen(ext)+2 );
    
    strncat( specBuf, extWildcard, specBufLen - strlen(specBuf) - 1 );

    return NFD_OKAY;
}

static nfdresult_t AddFiltersToDialog( ::IFileDialog *fileOpenDialog, const char *filterList )
{
    const wchar_t EMPTY_WSTR[] = L"";
    const wchar_t WILDCARD[] = L"*.*";

    if ( !filterList || strlen(filterList) == 0 )
        return NFD_OKAY;

    // Count rows to alloc
    UINT filterCount = 1; /* guaranteed to have one filter on a correct, non-empty parse */
    const char *p_filterList;
    for ( p_filterList = filterList; *p_filterList; ++p_filterList )
    {
        if ( *p_filterList == ';' )
            ++filterCount;
    }    

    assert(filterCount);
    if ( !filterCount )
    {
        NFDi_SetError("Error parsing filters.");
        return NFD_ERROR;
    }

    /* filterCount plus 1 because we hardcode the *.* wildcard after the while loop */
    COMDLG_FILTERSPEC *specList = (COMDLG_FILTERSPEC*)NFDi_Malloc( sizeof(COMDLG_FILTERSPEC) * (filterCount + 1) );
    if ( !specList )
    {
        return NFD_ERROR;
    }
    for (size_t i = 0; i < filterCount+1; ++i )
    {
        specList[i].pszName = NULL;
        specList[i].pszSpec = NULL;
    }

    size_t specIdx = 0;
    p_filterList = filterList;
    char typebuf[NFD_MAX_STRLEN] = {0};  /* one per comma or semicolon */
    char *p_typebuf = typebuf;
    char filterName[NFD_MAX_STRLEN] = {0};

    char specbuf[NFD_MAX_STRLEN] = {0}; /* one per semicolon */

    while ( 1 ) 
    {
        if ( NFDi_IsFilterSegmentChar(*p_filterList) )
        {
            /* append a type to the specbuf (pending filter) */
            AppendExtensionToSpecBuf( typebuf, specbuf, NFD_MAX_STRLEN );            

            p_typebuf = typebuf;
            memset( typebuf, 0, sizeof(char)*NFD_MAX_STRLEN );
        }

        if ( *p_filterList == ';' || *p_filterList == '\0' )
        {
            /* end of filter -- add it to specList */

            // Empty filter name -- Windows describes them by extension.            
            specList[specIdx].pszName = EMPTY_WSTR;
            CopyNFDCharToWChar( specbuf, (wchar_t**)&specList[specIdx].pszSpec );
                        
            memset( specbuf, 0, sizeof(char)*NFD_MAX_STRLEN );
            ++specIdx;
            if ( specIdx == filterCount )
                break;
        }

        if ( !NFDi_IsFilterSegmentChar( *p_filterList ))
        {
            *p_typebuf = *p_filterList;
            ++p_typebuf;
        }

        ++p_filterList;
    }

    /* Add wildcard */
    specList[specIdx].pszSpec = WILDCARD;
    specList[specIdx].pszName = EMPTY_WSTR;
    
    fileOpenDialog->SetFileTypes( filterCount+1, specList );

    /* free speclist */
    for ( size_t i = 0; i < filterCount; ++i )
    {
        NFDi_Free( (void*)specList[i].pszSpec );
    }
    NFDi_Free( specList );    

    return NFD_OKAY;
}

static nfdresult_t AllocPathSet( IShellItemArray *shellItems, nfdpathset_t *pathSet )
{
    const char ERRORMSG[] = "Error allocating pathset.";

    assert(shellItems);
    assert(pathSet);
    
    // How many items in shellItems?
    DWORD numShellItems;
    HRESULT result = shellItems->GetCount(&numShellItems);
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError(ERRORMSG);
        return NFD_ERROR;
    }

    pathSet->count = static_cast<size_t>(numShellItems);
    assert( pathSet->count > 0 );

    pathSet->indices = (size_t*)NFDi_Malloc( sizeof(size_t)*pathSet->count );
    if ( !pathSet->indices )
    {
        return NFD_ERROR;
    }

    /* count the total bytes needed for buf */
    size_t bufSize = 0;
    for ( DWORD i = 0; i < numShellItems; ++i )
    {
        ::IShellItem *shellItem;
        result = shellItems->GetItemAt(i, &shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError(ERRORMSG);
            return NFD_ERROR;
        }

        // Confirm SFGAO_FILESYSTEM is true for this shellitem, or ignore it.
        SFGAOF attribs;
        result = shellItem->GetAttributes( SFGAO_FILESYSTEM, &attribs );
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError(ERRORMSG);
            return NFD_ERROR;
        }
        if ( !(attribs & SFGAO_FILESYSTEM) )
            continue;

        LPWSTR name;
        shellItem->GetDisplayName(SIGDN_FILESYSPATH, &name);

        // Calculate length of name with UTF-8 encoding
        bufSize += GetUTF8ByteCountForWChar( name );
    }

    assert(bufSize);

    pathSet->buf = (nfdchar_t*)NFDi_Malloc( sizeof(nfdchar_t) * bufSize );
    memset( pathSet->buf, 0, sizeof(nfdchar_t) * bufSize );

    /* fill buf */
    nfdchar_t *p_buf = pathSet->buf;
    for (DWORD i = 0; i < numShellItems; ++i )
    {
        ::IShellItem *shellItem;
        result = shellItems->GetItemAt(i, &shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError(ERRORMSG);
            return NFD_ERROR;
        }

        // Confirm SFGAO_FILESYSTEM is true for this shellitem, or ignore it.
        SFGAOF attribs;
        result = shellItem->GetAttributes( SFGAO_FILESYSTEM, &attribs );
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError(ERRORMSG);
            return NFD_ERROR;
        }
        if ( !(attribs & SFGAO_FILESYSTEM) )
            continue;

        LPWSTR name;
        shellItem->GetDisplayName(SIGDN_FILESYSPATH, &name);

        int bytesWritten = CopyWCharToExistingNFDCharBuffer(name, p_buf);

        ptrdiff_t index = p_buf - pathSet->buf;
        assert( index >= 0 );
        pathSet->indices[i] = static_cast<size_t>(index);
        
        p_buf += bytesWritten; 
    }
     
    return NFD_OKAY;
}


static nfdresult_t SetDefaultPath( IFileDialog *dialog, const char *defaultPath )
{
    if ( !defaultPath || strlen(defaultPath) == 0 )
        return NFD_OKAY;

    wchar_t *defaultPathW = {0};
    CopyNFDCharToWChar( defaultPath, &defaultPathW );

    IShellItem *folder;
    HRESULT result = SHCreateItemFromParsingName( defaultPathW, NULL, IID_PPV_ARGS(&folder) );

    // Valid non results.
    if ( result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) || result == HRESULT_FROM_WIN32(ERROR_INVALID_DRIVE) )
    {
        NFDi_Free( defaultPathW );
        return NFD_OKAY;
    }

    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Error creating ShellItem");
        NFDi_Free( defaultPathW );
        return NFD_ERROR;
    }
    
    // Could also call SetDefaultFolder(), but this guarantees defaultPath -- more consistency across API.
    dialog->SetFolder( folder );

    NFDi_Free( defaultPathW );
    folder->Release();
    
    return NFD_OKAY;
}

// ------ Added by Row64 ------

// WndProc and MsgProc functions to handle the positioning of these dialogs, taking into account multiple monitors.
// - Most of the inspiration for this came from:
//   https://stackoverflow.com/questions/13476169/possible-to-specify-the-start-position-of-an-ifiledialog, augmented to make it
//   monitor-aware.
static HHOOK hhk = NULL;
static WNDPROC oldwp = NULL;
static WCHAR *titleFileOpen = L"Open";
static WCHAR *titleFileSaveAs = L"Save As";
static WCHAR *titleProjectOpen = L"Select a Project Folder To Open";
static WCHAR *titleProjectSaveAs = L"Set Project Folder Name To Save";


LRESULT DialogPositionWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_WINDOWPOSCHANGING)
    {
        // Determine which monitor Row64 is running on.
        // - Normally, we would like to use MonitorFromWindow() using the HWND of the top-level Row64 window, but that is not
        //   accessible here.  (GetParentWindow and GetAncestor do not seem to work).
        // - Instead, use the position of the mouse at the time the dialog creation is triggered.
        // - If the application is straddling monitors, this has the (questionable) advantage of having the dialog appear on the
        //   monitor that the mouse is in.
        HMONITOR hMonitor;
        MONITORINFO mi;
        POINT p;
        GetCursorPos(&p);
        hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST);
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(hMonitor, &mi);
        // Calculate the position of the dialog to place it in the middle of the chosen monitor
        RECT rc, rcDlg;
        GetWindowRect(hwnd, &rcDlg);                  // Set dialog rectangle
        CopyRect(&rc, &mi.rcWork);                    // Set monitor rectangle
        OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);  // Put the dialog size in right, bottom
        OffsetRect(&rc, -rc.left, -rc.top);           // Put the monitor size in right, bottom
        OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); // Discard space on the monitor taken up by the dialog
        WINDOWPOS* wp = (WINDOWPOS*)lParam;
        wp->x = mi.rcMonitor.left + rc.right / 2;
        wp->y = mi.rcMonitor.top + rc.bottom / 2;
        (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldwp);  // Restore old winproc
    }
    return CallWindowProc(oldwp, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK DialogPositionMsgProc(
    _In_ int    code,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    )
{
    if (code == HC_ACTION)
    {
        MSG* msg = (MSG*)lParam;
        WCHAR title[120];
        GetWindowText(msg->hwnd, title, sizeof(title));
        if (lstrcmpW(title, titleFileOpen) == 0 ||
            lstrcmpW(title, titleFileSaveAs) == 0 ||
            lstrcmpW(title, titleProjectOpen) == 0 ||
            lstrcmpW(title, titleProjectSaveAs) == 0)
        {
            // Set a new winproc to point to our winproc, saving the old one to restore later
            #if (_WIN64)
                oldwp = (WNDPROC)SetWindowLongPtr(msg->hwnd, GWLP_WNDPROC, (LONG_PTR)DialogPositionWndProc);
            #else
                oldwp = (WNDPROC)SetWindowLong(msg->hwnd, GWLP_WNDPROC, (LONG_PTR)DialogPositionWndProc);
            #endif
            LRESULT r = CallNextHookEx(hhk, code, wParam, lParam);
            UnhookWindowsHookEx(hhk);
            hhk = NULL;
            return r;
        }
    }
    return CallNextHookEx(hhk, code, wParam, lParam);
}
// ----- End Added by Row64 -----

/* public */


nfdresult_t NFD_OpenDialog( const char *filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath )
{
    nfdresult_t nfdResult = NFD_ERROR;
    
    // Init COM library.
    HRESULT result = ::CoInitializeEx(NULL,
                                      ::COINIT_APARTMENTTHREADED |
                                      ::COINIT_DISABLE_OLE1DDE );

    ::IFileOpenDialog *fileOpenDialog(NULL);

    if ( !SUCCEEDED(result))
    {
        NFDi_SetError("Could not initialize COM.");
        goto end;
    }

    // Create dialog
    result = ::CoCreateInstance(::CLSID_FileOpenDialog, NULL,
                                CLSCTX_ALL, ::IID_IFileOpenDialog,
                                reinterpret_cast<void**>(&fileOpenDialog) );
                                
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not create dialog.");
        goto end;
    }

    fileOpenDialog->SetTitle(titleFileOpen);  // Added by Row64

    // Build the filter list
    if ( !AddFiltersToDialog( fileOpenDialog, filterList ) )
    {
        goto end;
    }

    // Set the default path
    if ( !SetDefaultPath( fileOpenDialog, defaultPath ) )
    {
        goto end;
    }    

    // Show the dialog.
    hhk = SetWindowsHookEx(WH_GETMESSAGE, DialogPositionMsgProc, NULL, GetCurrentThreadId());  // Added by Row64
    result = fileOpenDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        // Get the file name
        ::IShellItem *shellItem(NULL);
        result = fileOpenDialog->GetResult(&shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get shell item from dialog.");
            goto end;
        }
        wchar_t *filePath(NULL);
        result = shellItem->GetDisplayName(::SIGDN_FILESYSPATH, &filePath);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get file path for selected.");
            goto end;
        }

        CopyWCharToNFDChar( filePath, outPath );
        CoTaskMemFree(filePath);
        if ( !*outPath )
        {
            /* error is malloc-based, error message would be redundant */
            goto end;
        }

        nfdResult = NFD_OKAY;
        shellItem->Release();
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("File dialog box show failed.");
        nfdResult = NFD_ERROR;
    }

 end:
    ::CoUninitialize();
    
    return nfdResult;
}

nfdresult_t NFD_OpenDialogMultiple( const nfdchar_t *filterList,
                                    const nfdchar_t *defaultPath,
                                    nfdpathset_t *outPaths )
{
    nfdresult_t nfdResult = NFD_ERROR;
    
    // Init COM library.
    HRESULT result = ::CoInitializeEx(NULL,
                                      ::COINIT_APARTMENTTHREADED |
                                      ::COINIT_DISABLE_OLE1DDE );
    if ( !SUCCEEDED(result))
    {
        NFDi_SetError("Could not initialize COM.");
        return NFD_ERROR;
    }

    ::IFileOpenDialog *fileOpenDialog(NULL);

    // Create dialog
    result = ::CoCreateInstance(::CLSID_FileOpenDialog, NULL,
                                CLSCTX_ALL, ::IID_IFileOpenDialog,
                                reinterpret_cast<void**>(&fileOpenDialog) );
                                
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not create dialog.");
        goto end;
    }

    // Build the filter list
    if ( !AddFiltersToDialog( fileOpenDialog, filterList ) )
    {
        goto end;
    }

    fileOpenDialog->SetTitle(titleFileOpen);  // Added by Row64

    // Set the default path
    if ( !SetDefaultPath( fileOpenDialog, defaultPath ) )
    {
        goto end;
    }

    // Set a flag for multiple options
    DWORD dwFlags;
    result = fileOpenDialog->GetOptions(&dwFlags);
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not get options.");
        goto end;
    }
    result = fileOpenDialog->SetOptions(dwFlags | FOS_ALLOWMULTISELECT);
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not set options.");
        goto end;
    }
 
    // Show the dialog.
    hhk = SetWindowsHookEx(WH_GETMESSAGE, DialogPositionMsgProc, NULL, GetCurrentThreadId());  // Added by Row64
    result = fileOpenDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        IShellItemArray *shellItems;
        result = fileOpenDialog->GetResults( &shellItems );
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get shell items.");
            goto end;
        }
        
        if ( AllocPathSet( shellItems, outPaths ) == NFD_ERROR )
        {
            goto end;
        }

        shellItems->Release();
        nfdResult = NFD_OKAY;
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("File dialog box show failed.");
        nfdResult = NFD_ERROR;
    }

 end:
    ::CoUninitialize();
    
    return nfdResult;
}
// ------ Added by Row64 ------
nfdresult_t NFD_SaveDialogWithName( const nfdchar_t *filterList,
                            const nfdchar_t *defaultPath,const nfdchar_t *inFileName,
                            nfdchar_t **outPath )
{
    nfdresult_t nfdResult = NFD_ERROR;
    
    // Init COM library.
    HRESULT result = ::CoInitializeEx(NULL,
                                      ::COINIT_APARTMENTTHREADED |
                                      ::COINIT_DISABLE_OLE1DDE );
    if ( !SUCCEEDED(result))
    {
        NFDi_SetError("Could not initialize COM.");
        return NFD_ERROR;
    }

    ::IFileSaveDialog *fileSaveDialog(NULL);

    // Create dialog
    result = ::CoCreateInstance(::CLSID_FileSaveDialog, NULL,
                                CLSCTX_ALL, ::IID_IFileSaveDialog,
                                reinterpret_cast<void**>(&fileSaveDialog) );

    fileSaveDialog->SetTitle(titleFileSaveAs);

    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not create dialog.");
        goto end;
    }

    // Set Filename
    size_t origsize = strlen(inFileName) + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t wcstring[newsize];
    mbstowcs_s(&convertedChars, wcstring, origsize, inFileName, _TRUNCATE);
    fileSaveDialog->SetFileName(wcstring);


    // Build the filter list
    if ( !AddFiltersToDialog( fileSaveDialog, filterList ) )
    {
        goto end;
    }

    // Set the default path
    if ( !SetDefaultPath( fileSaveDialog, defaultPath ) )
    {
        goto end;
    }

    // Show the dialog.
    hhk = SetWindowsHookEx(WH_GETMESSAGE, DialogPositionMsgProc, NULL, GetCurrentThreadId());
    result = fileSaveDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        // Get the file name
        ::IShellItem *shellItem;
        result = fileSaveDialog->GetResult(&shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get shell item from dialog.");
            goto end;
        }
        wchar_t *filePath(NULL);
        result = shellItem->GetDisplayName(::SIGDN_FILESYSPATH, &filePath);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get file path for selected.");
            goto end;
        }

        CopyWCharToNFDChar( filePath, outPath );
        CoTaskMemFree(filePath);
        if ( !*outPath )
        {
            /* error is malloc-based, error message would be redundant */
            goto end;
        }

        nfdResult = NFD_OKAY;
        shellItem->Release();
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("File dialog box show failed.");
        nfdResult = NFD_ERROR;
    }
    
 end:
    ::CoUninitialize();
        
    return nfdResult;
}
// ----- End Added by Row64 -----

nfdresult_t NFD_SaveDialog( const nfdchar_t *filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath )
{
    nfdresult_t nfdResult = NFD_ERROR;
    
    // Init COM library.
    HRESULT result = ::CoInitializeEx(NULL,
                                      ::COINIT_APARTMENTTHREADED |
                                      ::COINIT_DISABLE_OLE1DDE );
    if ( !SUCCEEDED(result))
    {
        NFDi_SetError("Could not initialize COM.");
        return NFD_ERROR;
    }

    ::IFileSaveDialog *fileSaveDialog(NULL);

    // Create dialog
    result = ::CoCreateInstance(::CLSID_FileSaveDialog, NULL,
                                CLSCTX_ALL, ::IID_IFileSaveDialog,
                                reinterpret_cast<void**>(&fileSaveDialog) );

    fileSaveDialog->SetTitle(titleFileSaveAs);  // Added by Row64

    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not create dialog.");
        goto end;
    }

    // Build the filter list
    if ( !AddFiltersToDialog( fileSaveDialog, filterList ) )
    {
        goto end;
    }

    // Set the default path
    if ( !SetDefaultPath( fileSaveDialog, defaultPath ) )
    {
        goto end;
    }

    // Show the dialog.
    hhk = SetWindowsHookEx(WH_GETMESSAGE, DialogPositionMsgProc, NULL, GetCurrentThreadId());  // Added by Row64
    result = fileSaveDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        // Get the file name
        ::IShellItem *shellItem;
        result = fileSaveDialog->GetResult(&shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get shell item from dialog.");
            goto end;
        }
        wchar_t *filePath(NULL);
        result = shellItem->GetDisplayName(::SIGDN_FILESYSPATH, &filePath);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get file path for selected.");
            goto end;
        }

        CopyWCharToNFDChar( filePath, outPath );
        CoTaskMemFree(filePath);
        if ( !*outPath )
        {
            /* error is malloc-based, error message would be redundant */
            goto end;
        }

        nfdResult = NFD_OKAY;
        shellItem->Release();
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("File dialog box show failed.");
        nfdResult = NFD_ERROR;
    }
    
 end:
    ::CoUninitialize();
        
    return nfdResult;
}
// ----- Added by Row64 -----
nfdresult_t NFD_SaveProjectDialog( const nfdchar_t *filterList,
                            const nfdchar_t *defaultPath, const nfdchar_t *inFileName,
                            nfdchar_t **outPath )
{
    nfdresult_t nfdResult = NFD_ERROR;
    
    // Init COM library.
    HRESULT result = ::CoInitializeEx(NULL,
                                      ::COINIT_APARTMENTTHREADED |
                                      ::COINIT_DISABLE_OLE1DDE );
    if ( !SUCCEEDED(result))
    {
        NFDi_SetError("Could not initialize COM.");
        return NFD_ERROR;
    }

    ::IFileSaveDialog *fileSaveDialog(NULL);

    // Create dialog
    result = ::CoCreateInstance(::CLSID_FileSaveDialog, NULL,
                                CLSCTX_ALL, ::IID_IFileSaveDialog,
                                reinterpret_cast<void**>(&fileSaveDialog) );

    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not create dialog.");
        goto end;
    }

    fileSaveDialog->SetTitle(titleProjectSaveAs);
    fileSaveDialog->SetFileNameLabel(L"Folder Name: ");
    fileSaveDialog->SetOkButtonLabel(L"Save To Folder Name");

    // Set Filename
    size_t origsize = strlen(inFileName) + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t wcstring[newsize];
    mbstowcs_s(&convertedChars, wcstring, origsize, inFileName, _TRUNCATE);
    fileSaveDialog->SetFileName(wcstring);

    
    // Set the default path
    if ( !SetDefaultPath( fileSaveDialog, defaultPath ) )
    {
        goto end;
    }

    // Show the dialog.
    hhk = SetWindowsHookEx(WH_GETMESSAGE, DialogPositionMsgProc, NULL, GetCurrentThreadId());
    result = fileSaveDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        // Get the file name
        ::IShellItem *shellItem;
        result = fileSaveDialog->GetResult(&shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get shell item from dialog.");
            goto end;
        }
        wchar_t *filePath(NULL);
        result = shellItem->GetDisplayName(::SIGDN_FILESYSPATH, &filePath);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get file path for selected.");
            goto end;
        }

        CopyWCharToNFDChar( filePath, outPath );
        CoTaskMemFree(filePath);
        if ( !*outPath )
        {
            /* error is malloc-based, error message would be redundant */
            goto end;
        }

        nfdResult = NFD_OKAY;
        shellItem->Release();
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("File dialog box show failed.");
        nfdResult = NFD_ERROR;
    }
    
 end:
    ::CoUninitialize();
        
    return nfdResult;
}
// ----- End Added by Row64 -----

class AutoCoInit
{
public:
    AutoCoInit()
    {
        mResult = ::CoInitializeEx(NULL,
            ::COINIT_APARTMENTTHREADED |
            ::COINIT_DISABLE_OLE1DDE);
    }

    ~AutoCoInit()
    {
        if (SUCCEEDED(mResult))
        {
            ::CoUninitialize();
        }
    }

    HRESULT Result() const { return mResult; }
private:
    HRESULT mResult;
};

// VS2010 hasn't got a copy of CComPtr - this was first added in the 2003 SDK, so we make our own small CComPtr instead
template<class T>
class ComPtr
{
public:
    ComPtr() : mPtr(NULL) { }
    ~ComPtr()
    {
        if (mPtr)
        {
            mPtr->Release();
        }
    }

    T* Ptr() const { return mPtr; }
    T** operator&() { return &mPtr; }
    T* operator->() const { return mPtr; }
private:
    // Don't allow copy or assignment
    ComPtr(const ComPtr&);
    ComPtr& operator = (const ComPtr&) const;
    T* mPtr;
};

nfdresult_t NFD_PickFolder(const nfdchar_t *defaultPath,
    nfdchar_t **outPath)
{
    // Init COM
    AutoCoInit autoCoInit;
    if (!SUCCEEDED(autoCoInit.Result()))
    {
        NFDi_SetError("CoInitializeEx failed.");
        return NFD_ERROR;
    }

    HRESULT hr = S_OK;
    ComPtr<IFileDialog> pFileDialog;
    hr = CoCreateInstance( CLSID_FileOpenDialog,NULL,CLSCTX_ALL,IID_PPV_ARGS(&pFileDialog) );
    if (FAILED(hr)) { return NFD_ERROR;}

    hr = pFileDialog->SetTitle(titleProjectOpen);
    hr = pFileDialog->SetFileNameLabel(L"Project Folder Name: ");
    pFileDialog->SetOkButtonLabel(L"Open Project Folder");

    // Set the default path
    if (SetDefaultPath(pFileDialog.Ptr(), defaultPath) != NFD_OKAY)
    {
        NFDi_SetError("SetDefaultPath failed.");
        return NFD_ERROR;
    }
    
    // Get the dialogs options
    DWORD dwOptions = 0;
    if (!SUCCEEDED(pFileDialog->GetOptions(&dwOptions)))
    {
        NFDi_SetError("GetOptions for IFileDialog failed.");
        return NFD_ERROR;
    }
    
    //pFileDialog->ShowNewFolderButton = true;

    // Add in FOS_PICKFOLDERS which hides files and only allows selection of folders
    if (!SUCCEEDED(pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS)))
    {
        NFDi_SetError("SetOptions for IFileDialog failed.");
        return NFD_ERROR;
    }

    // Show the dialog to the user
    hhk = SetWindowsHookEx(WH_GETMESSAGE, DialogPositionMsgProc, NULL, GetCurrentThreadId());
    const HRESULT result = pFileDialog->Show(NULL);
    if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        return NFD_CANCEL;
    }
    else if (!SUCCEEDED(result))
    {
        NFDi_SetError("Show for IFileDialog failed.");
        return NFD_ERROR;
    }

    // Get the shell item result
    ComPtr<IShellItem> pShellItem;
    if (!SUCCEEDED(pFileDialog->GetResult(&pShellItem)))
    {
        return NFD_OKAY;
    }

    // Finally get the path
    wchar_t *path = NULL;
    if (!SUCCEEDED(pShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path)))
    {
        NFDi_SetError("GetDisplayName for IShellItem failed.");
        return NFD_ERROR;
    }

    // Convert string
    CopyWCharToNFDChar(path, outPath);
    CoTaskMemFree(path);
    if (!*outPath)
    {
        // error is malloc-based, error message would be redundant
        return NFD_ERROR;
    }

    return NFD_OKAY;
}
