#ifndef SERVERCONNECT_H
#define SERVERCONNECT_H


// Adapted from: https://github.com/zaphoyd/websocketpp/blob/master/examples/utility_client/utility_client.cpp

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <functional>

typedef websocketpp::client<websocketpp::config::asio_client> client;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

class ByteStream;

using SuccessFunction = std::function<void(ByteStream&,std::shared_ptr<void>)>;
using FailureFunction = std::function<void(std::string, std::shared_ptr<void>)>;

namespace ServerStatus {
    enum Status {
        none = 0,
        connecting,
        open,
        failed,
        closed
    };
}

struct connection_metadata {
    typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

    websocketpp::connection_hdl m_hdl;
    int m_status;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;

    connection_metadata(websocketpp::connection_hdl hdl, std::string uri);
    
    void on_open(client * c, websocketpp::connection_hdl hdl);
    void on_fail(client * c, websocketpp::connection_hdl hdl);
    void on_close(client * c, websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl, client::message_ptr msg);
    
};

class ServerConnect {

public:

    static std::string      BSPrefix;
    static int              DefaultConnID;

    ServerConnect();
    ~ServerConnect();

    int Status( int inConnID = -1 );
    
    int Connect( const std::string & hostname, int port );
    int GetConnection( const std::string & hostname, int port );
    
    bool SendMsg( const std::string & inMsg, int inConnID = -1 );
    bool SendMsg( const char * inMsg, size_t inSize, int inConnID = -1 );
    bool SendMsg( ByteStream & inStream, int inConnID = -1 );

    void CloseConnection( const std::string & hostname, int port );
    void CloseConnection( int inConnID );

    ByteStream InitBStream( const std::string & inFunc, const std::string inMSID = "" );
    std::string GenerateMSID( const std::string & inPrefix = "" );
        
    void AddSuccessCallback( const std::string & inMSID, SuccessFunction&& inFunc, std::shared_ptr<void> inData );
    void AddFailureCallback( const std::string & inMSID, FailureFunction&& inFunc, std::shared_ptr<void> inData );

    void SuccessCallback( const std::string & inMSID, ByteStream & inStream );
    void FailureCallback( const std::string & inMSID, const std::string & inDetail );    


private:

    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    std::vector<connection_metadata::ptr> m_connection_list;

    struct SuccessFunctionData {
        std::string             MSID;
        SuccessFunction         Func;
        std::shared_ptr<void>   Data;
    };

    struct FailureFunctionData {
        std::string             MSID;
        FailureFunction         Func;
        std::shared_ptr<void>   Data;
    };

    std::vector<SuccessFunctionData> m_success_callbacks;
    std::vector<FailureFunctionData> m_failure_callbacks;

};

#ifndef SERVER
    extern ServerConnect* SERVER;
#endif

void SetSERVER( ServerConnect* inServer );

// ================================================================================
//                                Callback Examples                                
// ================================================================================

static void ExampleSuccessFunc( ByteStream & inStream, std::shared_ptr<void> inData ) {
    if ( inData ) { 
        auto data = std::static_pointer_cast<std::string>(inData); 
        std::cout << "Example Success Function Data: " << *data << std::endl;
    }
    else {
        std::cout << "Example Success Function -- Empty Data Ptr " << std::endl;
    }
};

static void ExampleFailureFunc( std::string inErrMsg, std::shared_ptr<void> inData ) {
    if ( inData ) { 
        auto data = std::static_pointer_cast<std::string>(inData); 
        std::cout << "Example Failure Function Data: " << *data << std::endl;
    }
    else {
        std::cout << "Example Failure Function -- Empty Data Ptr " << std::endl;
    }
};

struct ExampleData {
    std::string strVal;
    int intVal;
    
    ExampleData( const std::string & inStr, int inInt ) :
        strVal( inStr ), intVal( inInt ) {
        printf( "Got to ExampleData Constructor\n" );
    };

    ~ExampleData() {
        printf( "Got to ExampleData Destructor\n" );
    }

};

static void ExampleSuccessFuncWithStruct( ByteStream & inStream, std::shared_ptr<void> inData ) {
    if ( inData ) { 
        auto data = std::static_pointer_cast<ExampleData>(inData); 
        std::cout << "Example Success Function With Struct Data: " << data->strVal << ", " << data->intVal << std::endl;
    }
    else {
        std::cout << "Example Success Function With Struct  -- Empty Data Ptr " << std::endl;
    }
};

static void ExampleFailureFuncWithStruct( std::string inErrMsg, std::shared_ptr<void> inData ) {
    if ( inData ) { 
        auto data = std::static_pointer_cast<ExampleData>(inData); 
        std::cout << "Example Failure Function With Struct  Data: " << data->strVal << ", " << data->intVal << std::endl;
    }
    else {
        std::cout << "Example Failure Function With Struct  -- Empty Data Ptr " << std::endl;
    }
};



#endif /* SERVERCONNECT_H */
