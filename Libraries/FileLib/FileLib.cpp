#ifndef FILELIB_CPP
#define FILELIB_CPP


#if defined( _WIN32 )
    #define _CRT_SECURE_NO_WARNINGS 1
    #include "shlwapi.h"
#endif

#if defined( _WIN32 )
    #ifndef RUN_SHELL_COMMAND
        // See: 
        // https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecuteexa
        // https://docs.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfoa
        #define RUN_SHELL_COMMAND(c, p)                                       \
        {                                                                   \
            SHELLEXECUTEINFO sInfo {                                        \
                sizeof(SHELLEXECUTEINFO),   /* DWORD     cbSize */          \
                SEE_MASK_DEFAULT,           /* ULONG     fMask */           \
                NULL,                       /* HWND      hwnd */            \
                "open",                     /* LPCSTR    lpVerb */          \
                c.c_str(),                  /* LPCSTR    lpFile */          \
                p.c_str(),                  /* LPCSTR    lpParameters */    \
                NULL,                       /* LPCSTR    lpDirectory */     \
                SW_SHOW                     /* int       nShow */           \
            };                                                              \
            ShellExecuteEx(&sInfo);                                         \
        }
    #endif 
#endif 

#include <vector>
#include "FileLib.h"
#include "StringLib.h"

using namespace std;

namespace FLib {

void PreparePath(std::string &s) {
#if defined( _WIN32 )
    if ( s.empty() ) { return; }
    size_t b = 0;
    const std::string sslash = "\\";
    const std::string dslash = "\\\\";
    while( b < std::string::npos ) {
        b = s.find( sslash, b);
        if (b == std::string::npos) break;
        s.replace(b, sslash.size(), dslash);
        b += dslash.size();
    }
    if ( s.front() != '"' && s.back() != '"' ) { s = "\"" + s + "\""; }
#endif
}
void FLOpenPythonTerminal( const std::string & inPyPath, const std::string & inScriptsPath ){
    
    string key = "PATH";  
    char * pathVal;                                                                        
    pathVal = getenv( key.c_str() );                                                       
    std::string pathStr = "";                                                           
    if (pathVal == NULL) { return;}                                                        
    pathStr = pathVal;
    vector<string> pList = SLib::STR_SplitByDelimiter( pathStr,';');

    string finalStr = "PATH="+inPyPath+";"+inScriptsPath;
    string searchStr = "python";
    string pCopy = "";
    for(int i=0;i<pList.size();i++){
        pCopy=pList[i];
        SLib::STR_ToLower(pCopy);
        if (pCopy.find(searchStr) != std::string::npos) {}
        else{finalStr += ";" + pList[i];}
    }
    std::string command = "";
    #if defined( _WIN32 )
        command = "start ";
        command += "cmd.exe /k set \"" + finalStr + "\"";
        system( command.c_str() );
    #endif

}
void FLOpenPythonLink( const std::string & inLink ) {
        std::string command = "";
        std::string param = "";
        #if defined( _WIN32 )
            // get the default text editor in windows
            DWORD length = 0;
            LPCTSTR extension = ".txt";
            char pathStr[2000];
            LPSTR exePath = (LPSTR) &pathStr;
            command = "start  ";
            HRESULT res = AssocQueryStringA(ASSOCF_NONE, ASSOCSTR_EXECUTABLE, extension, NULL, NULL, &length );
            if ( length > 0 ) {
                res = AssocQueryStringA(ASSOCF_NONE, ASSOCSTR_EXECUTABLE, extension, NULL, exePath, &length );
                if ( res == S_OK ) {
                    if ( length > 0 ) {
                        command = exePath;
                        PreparePath( command );
                    }
                }
                else if ( res == E_POINTER ) {
                    // std::cout << "ERROR: E_POINTER" << std::endl;
                }
                else if ( res == S_FALSE ) {
                    // std::cout << "ERROR: S_FALSE" << std::endl;
                }
                else {
                    // std::cout << "ERROR: " << res << std::endl;
                }
            }           

            param = inLink;
            PreparePath( param );
            RUN_SHELL_COMMAND( command, param )

        #elif defined( __linux__ )
            command = "xdg-open  " + inLink; 
            system( command.c_str() );
        #else
            command = "open  " + inLink;
            system( command.c_str() );
        #endif
    };
    
    void FLOpenTextLink( const std::string & inLink ) {
        std::string command = "";
        #if defined( _WIN32 )
            command = "start  " + inLink;
        #elif defined( __linux__ )
            command = "xdg-open  " + inLink;
        #else
            command = "open  " + inLink;
        #endif
        system( command.c_str() );
    };

     void FLOpenWebLink( const std::string & inLink ) {
        std::string command = "";
        #if defined( _WIN32 )
            command = "start  " + inLink;
        #elif defined( __linux__ )
            command = "xdg-open  " + inLink;
        #else
            command = "open  " + inLink;
        #endif
        system( command.c_str() );
    };
    
}


#endif /* FILELIB_CPP */
