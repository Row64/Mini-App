#ifndef SERVERCONNECT_CPP
#define SERVERCONNECT_CPP

#include "ServerConnect.h"
#include "ByteStream.h"

// Initialize static variables
std::string ServerConnect::BSPrefix = "BS=";
int ServerConnect::DefaultConnID = -1;

connection_metadata::connection_metadata(websocketpp::connection_hdl hdl, std::string uri) :
    m_hdl(hdl), m_status( ServerStatus::connecting ), m_uri(uri), m_server("N/A") {}

void connection_metadata::on_open(client * c, websocketpp::connection_hdl hdl) {
    std::cout << "CONNECTION STATUS - Open" << std::endl;
    m_status = ServerStatus::open;
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
}

void connection_metadata::on_fail(client * c, websocketpp::connection_hdl hdl) {
    std::cout << "CONNECTION STATUS - Failed" << std::endl;
    m_status = ServerStatus::failed;
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
    m_error_reason = con->get_ec().message();
}

void connection_metadata::on_close(client * c, websocketpp::connection_hdl hdl) {
    std::cout << "CONNECTION STATUS - Close" << std::endl;
    m_status = ServerStatus::closed;
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    std::stringstream s;
    s << "close code: " << con->get_remote_close_code() << " (" 
        << websocketpp::close::status::get_string(con->get_remote_close_code()) 
        << "), close reason: " << con->get_remote_close_reason();
    m_error_reason = s.str();
}

void connection_metadata::on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    // TO DO - Process into ByteStream
    std::string & payload = msg->get_raw_payload();
    std::string code = payload.substr( 0, 3 );
    std::cout << "MESSAGE RECEIVED: \"" << code << "\"";
    if ( code == SERVER->BSPrefix ) {
        ByteStream BStream( true );
        BStream.LoadCompressedStream( (unsigned char *) payload.data()+3, payload.size()-3 );
        std::string MSID = "";
        std::string Func = "";
        if ( !BStream.GetString( "Func", Func ) ) { Func = ""; }
        std::cout << ", Func: \"" << Func << "\"";
        if ( BStream.GetString( "MSID", MSID ) ) {
            std::cout << ", MSID: \"" << MSID << "\"\n";
            if ( Func == "ERR" ) {
                std::string Detail = "";
                if ( BStream.GetString( "Detail", Detail ) ) {
                    std::cout << "  ERR Detail: " << Detail << std::endl;
                }
                SERVER->FailureCallback( MSID, Detail );
            }
            else {
                SERVER->SuccessCallback( MSID, BStream );
            }
        }
        else {
            std::cout << "\n";
        }
    }  
}

ServerConnect::ServerConnect() {

    // Initialize Asio Transport
    m_endpoint.init_asio();

    // this will turn off console output for frame header and payload
    m_endpoint.clear_access_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload); 

    // this will turn off everything in console output
    // m_endpoint.clear_access_channels(websocketpp::log::alevel::all); 

    // start the endpoint and kick off run() on a secondary thread
    m_endpoint.start_perpetual();
    m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);

}

ServerConnect::~ServerConnect() {
    m_endpoint.stop_perpetual(); 
    for ( auto conn : m_connection_list ) {
        if (conn->m_status != ServerStatus::open) { continue; } // Only close open connections
        std::cout << "> Closing connection " << conn->m_uri << std::endl;
        websocketpp::lib::error_code ec;
        m_endpoint.close(conn->m_hdl, websocketpp::close::status::going_away, "", ec);
        if (ec) {
            std::cout << "> Error closing connection " << conn->m_uri << ": "  << ec.message() << std::endl;
        }
    }
    m_thread->join();
}

int ServerConnect::Status( int inConnID ) {
    if ( inConnID < 0 ) { inConnID = DefaultConnID; }
    if ( inConnID < 0 || inConnID >= m_connection_list.size() ) { return ServerStatus::none; }
    return m_connection_list[inConnID]->m_status;
}

int ServerConnect::Connect( const std::string & hostname, int port ) {
    // If connection is successful, it returns the connection index.
    // Otherwise, -1 if failed.

    // Check for an existing connection or open connection
    int connID = GetConnection( hostname, port );
    if ( connID != -1 ) {
        if ( m_connection_list[connID]->m_status == ServerStatus::connecting || m_connection_list[connID]->m_status == ServerStatus::open ) {
            return connID;
        }
        m_connection_list.erase( m_connection_list.begin() + connID );
    }

    // Initialize New Connection
    std::string uri = "ws://" + hostname + ":" + std::to_string(port);
    std::cout << "> Connecting to: " << uri << std::endl;

    websocketpp::lib::error_code ec;
    client::connection_ptr con = m_endpoint.get_connection(uri, ec);
    if (ec) {
        std::cout << "> Connect initialization error: " << ec.message() << std::endl;
        return -1;
    }

    // Add the connection
    connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(con->get_handle(), uri);
    connID = (int) m_connection_list.size();
    m_connection_list.push_back( metadata_ptr );    

    // Set callback functions
    con->set_open_handler(websocketpp::lib::bind( &connection_metadata::on_open, metadata_ptr, &m_endpoint, ::_1 ) );
    con->set_fail_handler(websocketpp::lib::bind( &connection_metadata::on_fail, metadata_ptr, &m_endpoint, ::_1 ) );
    con->set_close_handler(websocketpp::lib::bind( &connection_metadata::on_close, metadata_ptr, &m_endpoint, ::_1 ) );
    con->set_message_handler(websocketpp::lib::bind( &connection_metadata::on_message, metadata_ptr, ::_1, ::_2 ) );

    // Connect
    m_endpoint.connect(con);

    // Return the connection index
    return connID;
}

int ServerConnect::GetConnection( const std::string & hostname, int port ) {
    std::string uri = "ws://" + hostname + ":" + std::to_string(port);
    for ( int i = 0; i < m_connection_list.size(); i++ ) {
        if ( m_connection_list[i]->m_uri == uri ) { return i; }
    }
    return -1;
}

bool ServerConnect::SendMsg( const std::string & inMsg, int inConnID ) {
    // Messages sent to the server are 3-Letter codes, followed by any metadata needed.
    // For example: GTS is the message code for "Get Table Summary". So, if you want
    // to request the table summary data for MyTable, you would pass "GTSMyTable".
    websocketpp::lib::error_code ec;
    if ( inConnID < 0 ) { inConnID = DefaultConnID; }
    if ( inConnID < 0 || inConnID >= m_connection_list.size() ) {
        std::cout << "> No connection found with id " << inConnID << std::endl;
        return false;
    }
    m_endpoint.send(m_connection_list[inConnID]->m_hdl, inMsg, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cout << "> Error sending message: " << ec.message() << std::endl;
        return false;
    }
    return true;
}

bool ServerConnect::SendMsg( const char * inMsg, size_t inSize, int inConnID ) {
    // Messages sent to the server are 3-Letter codes, followed by any metadata needed.
    // For example: GTS is the message code for "Get Table Summary". So, if you want
    // to request the table summary data for MyTable, you would pass "GTSMyTable".
    websocketpp::lib::error_code ec;
    if ( inConnID < 0 ) { inConnID = DefaultConnID; }
    if ( inConnID < 0 || inConnID >= m_connection_list.size() ) {
        std::cout << "> No connection found with id " << inConnID << std::endl;
        return false;
    }
    m_endpoint.send(m_connection_list[inConnID]->m_hdl, inMsg, inSize, websocketpp::frame::opcode::binary, ec);
    if (ec) {
        std::cout << "> Error sending message: " << ec.message() << std::endl;
        return false;
    }
    return true;
}

bool ServerConnect::SendMsg( ByteStream & inStream, int inConnID ) {
    websocketpp::lib::error_code ec;
    if ( inConnID < 0 ) { inConnID = DefaultConnID; }
    if ( inConnID < 0 || inConnID >= m_connection_list.size() ) {
        std::cout << "> No connection found with id " << inConnID << std::endl;
        return false;
    }
    // Prepare the COMPRESSED ByteStream message prefixed by "BS="
    //   [ BS= ][ Compressed ByteStream ]
    AppCore::CharBuffer buf( 3 + inStream.GetCompressedSize() );
    char * c = buf.Content;
    memcpy( c, BSPrefix.c_str(), 3 ); c += 3;
    size_t size = inStream.GetCompressedStream( (unsigned char*) c );
    // Send the message
    m_endpoint.send(m_connection_list[inConnID]->m_hdl, buf.Content, size+3, websocketpp::frame::opcode::binary, ec);
    if (ec) {
        std::cout << "> Error sending message: " << ec.message() << std::endl;
        return false;
    }
    return true;
}

void ServerConnect::CloseConnection( const std::string & hostname, int port ) {
    return CloseConnection( GetConnection( hostname, port ) );
}

void ServerConnect::CloseConnection( int inConnID ) {
    if ( inConnID < 0 || inConnID >= m_connection_list.size() ) {
        std::cout << "> No connection found with id " << inConnID << std::endl;
        return;
    }
    websocketpp::lib::error_code ec;
    m_endpoint.close(m_connection_list[inConnID]->m_hdl, websocketpp::close::status::normal, "Closing Connection", ec);
    if (ec) {
        std::cout << "> Error initiating close: " << ec.message() << std::endl;
    }
}

void ServerConnect::AddSuccessCallback( const std::string & inMSID, SuccessFunction&& inFunc, std::shared_ptr<void> inData ) {
    for ( int i = 0; i < m_success_callbacks.size(); i++ ) {
        auto & cb = m_success_callbacks[i];
        if ( cb.MSID == inMSID ) {
            cb.Func = std::forward<SuccessFunction>( inFunc );
            cb.Data = inData;
            return;
        }
    }
    SuccessFunctionData fdata;
    fdata.MSID = inMSID;
    fdata.Func = std::forward<SuccessFunction>( inFunc );
    fdata.Data = inData;
    m_success_callbacks.emplace_back( std::move(fdata) );
}

void ServerConnect::AddFailureCallback( const std::string & inMSID, FailureFunction&& inFunc, std::shared_ptr<void> inData ) {
    for ( int i = 0; i < m_failure_callbacks.size(); i++ ) {
        auto & cb = m_failure_callbacks[i];
        if ( cb.MSID == inMSID ) {
            cb.Func = std::forward<FailureFunction>( inFunc );
            cb.Data = inData;
            return;
        }
    }
    FailureFunctionData fdata;
    fdata.MSID = inMSID;
    fdata.Func = std::forward<FailureFunction>( inFunc );
    fdata.Data = inData;
    m_failure_callbacks.emplace_back( std::move(fdata) );
}

void ServerConnect::SuccessCallback( const std::string & inMSID, ByteStream & inStream ) {
    for ( int i = 0; i < m_success_callbacks.size(); i++ ) {
        auto & cb = m_success_callbacks[i];
        if ( cb.MSID == inMSID ) {
            std::cout << "----- Got SUCCESS callback for: " << cb.MSID << " -----" << std::endl;
            cb.Func( inStream, cb.Data );
            m_success_callbacks.erase( m_success_callbacks.begin() + i );
            return;
        }
    }
}

void ServerConnect::FailureCallback( const std::string & inMSID, const std::string & inDetail ) {
    for ( int i = 0; i < m_failure_callbacks.size(); i++ ) {
        auto & cb = m_failure_callbacks[i];
        if ( cb.MSID == inMSID ) {
            std::cout << "----- Got FAILURE callback for: " << cb.MSID << " -----" << std::endl;
            cb.Func( inDetail, cb.Data );
            m_failure_callbacks.erase( m_failure_callbacks.begin() + i );
            return;
        }
    }
}

ByteStream ServerConnect::InitBStream( const std::string & inFunc, const std::string inMSID ) {   // Creates a ByteStream with Func and MSID
    ByteStream BStream;
    BStream.AddString( "Func", inFunc );
    if ( !inMSID.empty() ) {
        BStream.AddString( "MSID", inMSID );
    }
    return std::move( BStream );
}

std::string ServerConnect::GenerateMSID( const std::string & inPrefix ) {
    return inPrefix + "_" + AppCore::Tools::GetStampYMDHMS();
}

#ifndef SERVER
    ServerConnect* SERVER = NULL;
#endif

void SetSERVER( ServerConnect* inServer ) {     // This is set in main.cpp
    SERVER = inServer;
};

#endif /* SERVERCONNECT_CPP */