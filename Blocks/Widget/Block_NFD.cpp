#ifndef BLOCK_NFD_CPP
#define BLOCK_NFD_CPP

#include "c_PipelineManager.h"
using namespace AppCore;

#include <nfd.h>

std::string DrawOpenFileDialog( const std::string & extensions, const std::string & defaultPath ) {

    PMPTR->Pause = true;    // Signal to pause rendering when file dialog is open

    nfdchar_t *impPath = NULL;
    const char * filters = extensions.c_str();
    
    fs::path defPath = defaultPath;
    if ( fs::is_regular_file( defPath ) ) { defPath = defPath.parent_path(); }
    nfdresult_t result = NFD_OpenDialog( filters, CONVERT_PATH(defPath.make_preferred()), &impPath );
    
    string outStr = "";
    if ( result == NFD_OKAY ) {
        outStr = impPath;
        free(impPath);
    }
    else if ( result == NFD_CANCEL ) {
        puts("User pressed cancel.");
    }
    else {
        printf("Error: %s\n", NFD_GetError() );
    }

    PMPTR->Pause = false;   // Signal to resume rendering
    return outStr;
    
}

string DrawSaveFileDialog( const std::string & extension, const std::string & defaultPath ) {

    PMPTR->Pause = true;    // Signal to pause rendering when file dialog is open

    nfdchar_t *expPath = NULL;
    const char * filters = extension.c_str();
    
    const char * defPath  = defaultPath.data();
    nfdresult_t result = NFD_SaveDialog( filters, defPath, &expPath );
    
    string outStr = "";
    if ( result == NFD_OKAY ) {
        outStr = expPath;
        free(expPath);
        if ( extension == "" ) {
            if ( outStr.back() == '.' ) { outStr.pop_back(); }
        }
        else if ( fs::extension( (fs::path) outStr ) != "." + extension ) { 
            outStr += "." + extension;
        }
    }
    else if ( result == NFD_CANCEL ) {
        puts("User pressed cancel.");
    }
    else {
        printf("Error: %s\n", NFD_GetError() );
        #if defined( _WIN32 )
        const char *nfd_error = NFD_GetError();
        const char *error_text = "Error creating ShellItem";
        if ( 0 == std::strncmp( nfd_error, error_text, std::strlen( error_text ) ) ) {
            printf(
                "Error creating Windows resource: %s\n"
                "This is likely caused by a path error (Missing drive specifier or invalid path separators).\n",
                defPath );
        }
        #endif
    }

    PMPTR->Pause = false;   // Signal to resume rendering
    return outStr;
}

#endif /* BLOCK_NFD_CPP */
