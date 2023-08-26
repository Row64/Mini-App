#ifndef BLOCK_CONTEXTMENU_CPP
#define BLOCK_CONTEXTMENU_CPP

// Example includes ---------------------------------------------
// Note: These have to be included BEFORE c_PipelineManager.h.

// Includes for HTTP
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
    #define CPPHTTPLIB_OPENSSL_SUPPORT 1
#endif

#include "httplib.h"
#ifdef __GNUC__
    #if (OPENSSL_VERSION_MAJOR >= 3)
    #define SSL_get_peer_certificate SSL_get1_peer_certificate
    #endif
#endif

// Includes for Server connection
#include "ServerConnect.h"
#include "ByteStream.h"
// --------------------------------------------------------------

#include "c_PipelineManager.h"
using namespace AppCore;


namespace Blocks {

    void ProcessContextClick( const string & inMenuType, const string & inItem){
        
        if(inMenuType == "Example_RightClick"){
            ProcessExamples(inItem);
            return;
        }
        
    }

    void ProcessExamples(const string & inItem) {
        std::cout << "Clicked on: " << inItem << std::endl;

        if ( inItem == "Add Floating PPG" ) {
            static int index = 0;
            OpenTestPPG( "Floating PPG #"+to_string(++index), PropertyPage::mode_float );
        }
        else if ( inItem == "Open Underlay PPG" ) {
            OpenTestPPG( "Underlay PPG", PropertyPage::mode_fill_pane );
        }
        else if ( inItem == "Close Underlay PPG" ) {
            int ppgIndex = GetPpgIndex( "Underlay PPG" );
            if ( ppgIndex != -1 ) { ADIO->MM.RemovePPG(ppgIndex); }
        }
        else if ( inItem == "Open Overlay PPG" ) {
            OpenTestPPG( "Overlay PPG", PropertyPage::mode_left_auto_hide );
        }
        else if ( inItem == "Close Overlay PPG" ) {
            int ppgIndex = GetPpgIndex( "Overlay PPG" );
            if ( ppgIndex != -1 ) { ADIO->MM.RemovePPG(ppgIndex); }
        }
        else if ( inItem == "Connect Server" ) {
            string Hostname = "localhost";
            int Port = 9002;
            SERVER->DefaultConnID = SERVER->Connect( Hostname, Port );
        }
        else if ( inItem == "Test Server" ) {
            string MSID = SERVER->GenerateMSID( "PING" );
            std::cout << "Pinging Server with MSID: " << MSID << std::endl;
            SERVER->AddSuccessCallback( MSID, ExampleSuccessFunc, std::make_shared<std::string>( "Yay!" ) );
            SERVER->AddFailureCallback( MSID, ExampleFailureFunc, std::make_shared<std::string>( "Nay!" ) );
            ByteStream BStream = SERVER->InitBStream( "PING", MSID );
            SERVER->SendMsg( BStream );
        }
        else if ( inItem == "Test HTTP" ) {
            httplib::Client cli("httpbin.org", 80);
            cli.set_connection_timeout( 2 );
            string body = "This is an example...";
            auto res = cli.Post("/post", body, "application/octet-stream");
            if ( res.error() == httplib::Error::Success ) {
                printf( "POST Result: %s\n", to_string(res.error()).c_str() );
                printf( "    status: %d\n", res->status );
                printf( "    body: %s\n", res->body.c_str() );
            }
            else {
                printf( "POST Error: %s\n", to_string(res.error()).c_str() );
            }
        }
    }

} // end namespace Blocks

#endif // BLOCK_CONTEXTMENU_CPP