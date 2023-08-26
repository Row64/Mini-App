#ifndef USER_APPDATAIO_CPP
#define USER_APPDATAIO_CPP

#include "user_AppDataIO.h"

namespace AppCore {

    AppDataIO::AppDataIO( char* inPath ) :
        AppPath( fs::absolute((fs::path) inPath )) {
            RootPath = AppPath.parent_path();
            SetADIO( &*this );
            DefaultScroller.BarBkgd = 0x65ABD6;
            DefaultScroller.OverBarBkgd = 0x499DCF;
            DefaultScroller.DownBarBkgd = 0x258BC5;
    }
    
    void AppDataIO::PreRenderSetup() {
        MM.PreRenderSetup();
        FM.PreRenderSetup();
    }

    void AppDataIO::PostRenderSetup() {
        MM.PostRenderSetup();
        FM.PostRenderSetup();
    }


    #ifndef PMPTR
        PipelineManager*   PMPTR = NULL;
    #endif

    #ifndef ADIO
        AppDataIO* ADIO = NULL;
    #endif

    void SetPMPTR( PipelineManager* inPM ) {          // This is called in the PipelineManager Constructor
        PMPTR = inPM;
    };

    void SetADIO( AppDataIO* inAD ) {                 // This is called in the AppData Constructor
        ADIO = inAD;
    };


} // end namespace AppCore


#endif /* USER_APPDATAIO_CPP */
