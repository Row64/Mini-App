#include "ServerConnect.h"
#include "c_PipelineManager.h"

#include <boost/filesystem.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
namespace fs = boost::filesystem;

#include <iostream>
#include <fstream>

int main( int argc, char **argv ) {

    ServerConnect server;
    SetSERVER( &server );

    ch_ThreadPool thread_pool( MAX_THREADS );
    SetTPOOL( &thread_pool );

    AppCore::AppDataIO app_data( argv[0] );
    
    // Main Application Loop
    try {
        
        AppCore::Window window;
        AppCore::PipelineManager sample("Row64 Mini-App", app_data);
        window.Create( "Row64", 0, 0 );                         // Window creation size of splash
        sample.InitPipelines( window.GetParameters() );             // DX11 preparations and initialization
        window.RenderingLoop( sample );                             // Rendering loop - displays window

    } catch (const std::runtime_error & e) {
        std::cout << "Runtime Error: " << e.what() << ", line " << __LINE__ << std::endl;
    } catch( const std::exception &e ) {
        std::cout << "Exception: " << e.what() << ", line " << __LINE__ << std::endl;
        return -1;
    }
    
    // Shut down threads 
    thread_pool.JoinThreads();

    // Catch exceptions from threadpool
    if ( globalExceptionPtr != nullptr ) {
        try {
            std::rethrow_exception(globalExceptionPtr);
        } catch (const std::runtime_error & e) {
            std::cout << "Thread exited with Runtime Error: " << e.what() << ", line " << __LINE__ << std::endl;
        } catch( const std::exception &e ) {
            std::cout << "Thread exited with Exception: " << e.what() << ", line " << __LINE__ << std::endl;
            return -1;
        }
    }
    
    return 0;
}

