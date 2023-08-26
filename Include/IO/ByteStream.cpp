#ifndef BYTESTREAM_CPP
#define BYTESTREAM_CPP


#if defined( _WIN32 )
    #define NOMINMAX
#endif

#include "ByteStream.h"

#ifdef R64_CLIENT
    #define MINIZ_NO_TIME
    #define MINIZ_NO_ARCHIVE_WRITING_APIS
#endif
#include "miniz.h"

#include <iostream>

using namespace AppCore;
using namespace std;

// ==========================================================================================================
//                                            BYTE STREAM BUFFER
// ==========================================================================================================

ByteStreamBuffer::ByteStreamBuffer( bool viewOnly ) :
    Content( nullptr ), Size( 0 ), ViewOnly( viewOnly ) {};

ByteStreamBuffer::ByteStreamBuffer( const std::string & inKey, ByteStreamDataType inType, size_t inValSize ) :
    Size( 9 + inKey.size() + inValSize ), ViewOnly( false ) {
    Content = static_cast<char*>( malloc(Size) );
    uint64_t keySize = inKey.size();
    memcpy( Content, &inType, 1 );             // write the type ( 1 byte )
    memcpy( &Content[1], &keySize, 8 );        // write the key size ( 8 bytes )
    memcpy( &Content[9], &inKey[0], keySize ); // write the key
}

ByteStreamBuffer::~ByteStreamBuffer() {
    Clear();
}

ByteStreamBuffer::ByteStreamBuffer( ByteStreamBuffer && other ) :
    Size( other.Size ),
    Content( other.Content ),
    ViewOnly( other.ViewOnly ) {
    other.Size = 0;
    other.Content = nullptr;
    other.ViewOnly = true;
}

ByteStreamBuffer & ByteStreamBuffer::operator=( ByteStreamBuffer && other ) {
    if ( Size > 0 ) { Clear(); }
    Size = other.Size;
    Content = other.Content;
    ViewOnly = other.ViewOnly;
    other.Size = 0;
    other.Content = nullptr;
    other.ViewOnly = true;
    return *this;
}

void ByteStreamBuffer::SetView( char * inBuf, size_t inSize ) {
    Clear();
    Content = inBuf;
    Size = inSize;
    ViewOnly = true;
}

void ByteStreamBuffer::Clear() {
    if ( !ViewOnly ) {
        if (Content != nullptr) {  
            free(Content); 
        } else if (Content != NULL && Content[0] != '\0') {  
            free(Content); 
        }
    }
    Content = nullptr; Size = 0;
}

bool ByteStreamBuffer::Resize( size_t inSize ) {
    if ( ViewOnly ) { return false; }
    if ( inSize == Size ) { return true; }
    if ( inSize == 0 ) {
        Clear();
        return true;
    }
    if( void* mem = realloc( Content, inSize ) ) {
        Content = static_cast<char*>(mem);
        Size = inSize;
        return true;
    }
    return false;
}

bool ByteStreamBuffer::IsValid() {
    if ( Size < 9 ) { return false; }                // Not enough memory in Content buffer
    if ( 9 + GetKeySize() > Size ) { return false; } // Not enough memory in Content buffer
    if ( GetType() < 0 || GetType() >= dt_last ) { return false; }  // Invalid type
    return true;
}

ByteStreamDataType ByteStreamBuffer::GetType() {
    return *( (ByteStreamDataType*)Content );
}

std::string ByteStreamBuffer::GetTypeAsString() {
    return ByteStreamDataStrings[ GetType() ];
}

size_t ByteStreamBuffer::GetKeySize() {
    return (size_t) *( (uint64_t*)&Content[1] );
}

std::string ByteStreamBuffer::GetKey() {
    size_t keySize = GetKeySize();
    return { &Content[9], keySize };
}

char ByteStreamBuffer::GetKeyFirstLetter() {
    return std::tolower( Content[9] );
}

size_t ByteStreamBuffer::GetValueSize() {
    return Size - GetKeySize() - 9;
}

char * ByteStreamBuffer::ValuePtr() {
    return &Content[9+GetKeySize()];
}

bool ByteStreamBuffer::SetValue( const void * inVal ) {
    // This sets the [Value] section. The size of the [Value] section should already be set
    // prior to calling this function. The implementation is reponsible for making sure the
    // size is correct.
    if ( !IsValid() ) { return false; }
    uint64_t valSize = GetValueSize();
    if ( valSize == 0 ) { return false; }   // Nothing set
    memcpy( ValuePtr(), inVal, valSize );
    return true;
};


// ==========================================================================================================
//                                                 BYTE STREAM
// ==========================================================================================================

ByteStream::ByteStream( bool readOnly ) : ReadOnly( readOnly ) {
    Reset();
    // std::cout << "New ByteStream: " << ( ReadOnly ? "Read Only" : "Write OK" ) << std::endl;
}

void ByteStream::ClearAll() {
    Reset();
    Stream.Clear();
}

void ByteStream::Reset() {
    Dict.clear();
    Lookup.clear();
    Lookup.resize( 27, {} );// a-z and index 26 is special characters
    StreamSize = 16;        // 8 bytes for [ Nb of Buf ] and 8 bytes for [ File End Pos ]
}

size_t ByteStream::GetStreamSize() {
    return StreamSize;
}

void ByteStream::GetStream( char * buf ) {
    uint64_t NbBuffers = Dict.size();
    uint64_t pos = ( NbBuffers + 2 ) * 8;
    std::vector<uint64_t> PosList(NbBuffers+1);   // vector of starting positions + the file end pos
    for ( int i = 0; i < NbBuffers; i++ ) {
        PosList[i] = pos;
        pos += Dict[i].Size;
    }
    PosList[NbBuffers] = pos;
    // Copy the buffer count
    memcpy( buf, &NbBuffers, 8 ); buf += 8;
    // Copy the positions list
    memcpy( buf, &PosList[0], PosList.size()*8 ); buf += PosList.size()*8;
    // Copy the buffer data
    for (int i = 0; i < NbBuffers; i++ ) {
        memcpy( buf, Dict[i].Content, Dict[i].Size ); buf += Dict[i].Size;
    }
}

void ByteStream::LoadStream( char * buf, size_t bufSize ) {
    if ( bufSize < 8 ) { return; }
    uint64_t NbBuffers;
    std::vector<uint64_t> PosList;   // vector of starting positions + the file end pos
    Reset();
    // Copy the buffer count
    memcpy( &NbBuffers, buf, 8 ); buf += 8;
    // Copy the positions list
    if ( bufSize < (NbBuffers+2)*8 ) { return; }
    PosList.resize(NbBuffers+1,0);
    memcpy( &PosList[0], buf, PosList.size()*8 ); buf += PosList.size()*8;
    // Copy the buffer data
    for (int i = 0; i < NbBuffers; i++ ) {
        if ( bufSize < PosList[i+1] ) { return; }
        if ( PosList[i+1] <= PosList[i] ) { continue; }
        ByteStreamBuffer bsb( ReadOnly );
        if ( ReadOnly ) {
            bsb.Content = buf;
            bsb.Size = PosList[i+1] - PosList[i];
        }
        else {
            bsb.Resize( PosList[i+1] - PosList[i] );
            memcpy( bsb.Content, buf, bsb.Size );
        }
        buf += bsb.Size;
        int fli = bsb.GetKeyFirstLetter() - 'a';
        if ( fli < 0 || fli > 25 ) { fli = 26; }
        StreamSize += 8 + bsb.Size;
        Lookup[fli].push_back( Dict.size() );
        Dict.push_back( std::move( bsb ) );
    }
}

size_t ByteStream::GetCompressedSize() {
    // This function returns an upper bound on the amount of space need for the compressed buffer.
    // First 8 bytes stores the uncompressed size. Remaining bytes stores the compressed data.
    return (size_t) mz_compressBound( static_cast<mz_ulong>(StreamSize) ) + 8;
}

size_t ByteStream::GetCompressedStream( unsigned char * outBuf ) {
    // Get the stream buffer
    CharBuffer CBuf( StreamSize );
    GetStream( CBuf.Content );
    // Write the stream size to the first 8 bytes
    memcpy( outBuf, &StreamSize, 8 );
    // Write the compressed data to output buffer
    unsigned char * buf = outBuf + 8;
    mz_ulong buflen = mz_compressBound( static_cast<mz_ulong>(CBuf.Size) );

    auto res = mz_compress2(buf, &buflen, (unsigned char *) CBuf.Content, static_cast<mz_ulong>(CBuf.Size), MZ_DEFAULT_LEVEL );
    if ( res != MZ_OK ) {
        printf( "Failed to compress ByteStream with error: %s\n", mz_error( res ) );
        return 0;
    }
    // Return the total size written to
    return buflen + 8;
}

size_t ByteStream::GetUncompressedSize( unsigned char * inBuf ) {
    return *( (uint64_t*) inBuf );
}

void ByteStream::LoadCompressedStream( unsigned char * inBuf, size_t inSize ) {
    if ( inSize < 8 ) { return; }
    Stream.Grow( GetUncompressedSize( inBuf ) );
    unsigned char * buf = inBuf + 8; // first 8 bytes of the compressed stream is a uint64_t of the uncompressed size
    mz_ulong bufsize = static_cast<mz_ulong>(inSize - 8);
    mz_ulong buflen = static_cast<mz_ulong>(Stream.Size);
    auto res = mz_uncompress2( (unsigned char *) Stream.Content, &buflen, buf, &bufsize );
    if ( res != MZ_OK ) {
        printf( "Failed to uncompress ByteStream with error: %s\n", mz_error( res ) );
        return;
    }
    Stream.Resize( buflen );
    LoadStream( Stream.Content, buflen );
}

#ifndef R64_CLIENT

void ByteStream::Save( const std::string & path ) {
    FileEngine ListFile( true );
    if ( !ListFile.NewFile( path,  StreamSize ) ) { printf("Failed to create new file: %s\n", path.c_str() ); return; }
    GetStream( (char * ) ListFile.Content );
    ListFile.ClearAll();
}

void ByteStream::Read( const std::string & path ) {
    FileEngine ListFile( true );
    if ( !ListFile.OpenFile( path ) ) { printf("Failed to open file: %s\n", path.c_str() ); return; }
    if ( ReadOnly ) {
        Stream.Resize( ListFile.FileSize() );
        memcpy( Stream.Content, ListFile.Content, ListFile.FileSize() );
        LoadStream( Stream.Content, Stream.Size );  // TODO: This currently makes a copy of the file data to the Stream buffer.
    }
    else {
        LoadStream( (char * ) ListFile.Content, ListFile.FileSize() );
    }
}

void ByteStream::SaveCompressed( const std::string & path ) {
    FileEngine ListFile( true );
    if ( !ListFile.NewFile( path, GetCompressedSize() ) ) { printf("Failed to create new file: %s\n", path.c_str() ); return; }
    auto finalSize = GetCompressedStream( ListFile.Content );
    ListFile.ResizeFile( finalSize );
    ListFile.ClearAll();
}

void ByteStream::ReadCompressed( const std::string & path ) {
    FileEngine ListFile( true );
    if ( !ListFile.OpenFile( path ) ) { printf("Failed to open file: %s\n", path.c_str() ); return; }
    LoadCompressedStream( ListFile.Content, ListFile.FileSize() );
}

#endif // #ifndef R64_CLIENT

int64_t ByteStream::GetRecordIndex( const std::string & inKey ) {
    if ( inKey.empty() ) { return -1; }
    char firstLetter = std::tolower( inKey[0] );
    int fli = firstLetter - 'a';
    if ( fli < 0 || fli > 25 ) { fli = 26; }
    for ( int i = 0; i < Lookup[fli].size(); i++ ) {
        ByteStreamBuffer & rec = Dict[ Lookup[fli][i] ];
        if ( !rec.IsValid() ) { continue; }
        if ( rec.GetKey() == inKey ) { return Lookup[fli][i]; }
    }
    return -1;
}

size_t ByteStream::GetRecordValueSize( const std::string & inKey ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    return Dict[ ind ].GetValueSize();
}

bool ByteStream::HasRecord( const std::string & inKey ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    return true;
}

// ==================================================================================
//                              PUBLIC RECORD FUNCTIONS
// ==================================================================================

void ByteStream::AddBool( const std::string & inKey, const bool & inVal ) {
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint8, sizeof(uint8_t) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
};

void ByteStream::AddChar( const std::string & inKey, const char & inVal ) {
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int8, sizeof(int8_t) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt8( const std::string & inKey, const int8_t & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int8, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt8( const std::string & inKey, const uint8_t & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint8, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt16( const std::string & inKey, const int16_t & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int16, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt16( const std::string & inKey, const uint16_t & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint16, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt32( const std::string & inKey, const int32_t & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int32, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt32( const std::string & inKey, const uint32_t & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint32, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt64( const std::string & inKey, const int64_t & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int64, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt64( const std::string & inKey, const uint64_t & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint64, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddFloat( const std::string & inKey, const float & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_float, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddDouble( const std::string & inKey, const double & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_double, sizeof(inVal) );
    SBuf.SetValue( &inVal );
    AddRecord( SBuf );
}

void ByteStream::AddString( const std::string & inKey, const std::string & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_string, inVal.size() );
    SBuf.SetValue( inVal.c_str() );
    AddRecord( SBuf );
}

void ByteStream::AddBuffer( const std::string & inKey, const AppCore::CharBuffer & inVal ) {
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_buffer, inVal.Size );
    SBuf.SetValue( inVal.Content );
    AddRecord( SBuf );
}

void ByteStream::AddBuffer( const std::string & inKey, const void * inVal, size_t inSize ) {
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_buffer, inSize );
    SBuf.SetValue( inVal );
    AddRecord( SBuf );
}

char * ByteStream::AddBuffer( const std::string & inKey, size_t inSize ) {
    if ( ReadOnly ) { return nullptr; }
    ByteStreamBuffer SBuf( inKey, dt_buffer, inSize );
    return AddRecord( SBuf ).ValuePtr();
}

void ByteStream::AddStream( const std::string & inKey, ByteStream & inVal ) {
    if ( ReadOnly ) { return; }
    auto streamSize = inVal.GetStreamSize();
    ByteStreamBuffer SBuf( inKey, dt_stream, streamSize );
    char * streamBuf = SBuf.ValuePtr();
    inVal.GetStream( streamBuf );
    AddRecord( SBuf );
}

void ByteStream::AddInt8Vector( const std::string & inKey, const std::vector<int8_t> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int8_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt8Vector( const std::string & inKey, const std::vector<uint8_t> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint8_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt16Vector( const std::string & inKey, const std::vector<int16_t> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int16_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt16Vector( const std::string & inKey, const std::vector<uint16_t> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint16_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt32Vector( const std::string & inKey, const std::vector<int32_t> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int32_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt32Vector( const std::string & inKey, const std::vector<uint32_t> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint32_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt64Vector( const std::string & inKey, const std::vector<int64_t> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int64_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt64Vector( const std::string & inKey, const std::vector<uint64_t> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint64_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddFloatVector( const std::string & inKey, const std::vector<float> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_float_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddDoubleVector( const std::string & inKey, const std::vector<double> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_double_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddStringVector( const std::string & inKey, const std::vector<std::string> & inVal ) {
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_string_vector, 0 );
    SBuf.SetVectorValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddStreamVector( const std::string & inKey, std::vector<ByteStream> & inVal ) {
    if ( ReadOnly ) { return; }
    // NOTE: All the offsets (uint64_t) are stored first, then the streams are stored back to back with no delimiter.
    //       [ Value ] --> [ Count ][ Offset 0 ][ Offset 1 ] ... [ Offset N ][ End Pos ][ Str 0 ][ Str 1 ] ... [ Str N ]

    // Calculate how much space is needed for the vector
    uint64_t count = inVal.size();
    uint64_t valSize = 8;
    valSize += ( count + 1 ) * 8;
    for ( auto & val : inVal ) { valSize += val.GetStreamSize(); }
    // Create the buffer
    ByteStreamBuffer SBuf( inKey, dt_stream_vector, valSize );
    // Write the count
    char * buf = SBuf.ValuePtr();
    memcpy( buf, &count, 8 ); buf += 8;
    // Write the offsets and streams
    uint64_t offset = 0;
    char * sbuf = buf + ( count + 1 ) * 8;
    for ( auto & val : inVal ) {
        memcpy( buf, &offset, 8 ); buf += 8;    // write the offset
        valSize = val.GetStreamSize();
        offset += valSize;
        if ( valSize > 0 ) {                    // write the stream
            val.GetStream( sbuf ); sbuf += valSize;
        }
    }
    memcpy( buf, &offset, 8 );                  // write the end pos
    // Add the record
    AddRecord( SBuf );
}

void ByteStream::AddInt8Vector2D( const std::string & inKey, const std::vector<std::vector<int8_t>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int8_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt8Vector2D( const std::string & inKey, const std::vector<std::vector<uint8_t>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint8_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt16Vector2D( const std::string & inKey, const std::vector<std::vector<int16_t>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int16_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt16Vector2D( const std::string & inKey, const std::vector<std::vector<uint16_t>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint16_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt32Vector2D( const std::string & inKey, const std::vector<std::vector<int32_t>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int32_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt32Vector2D( const std::string & inKey, const std::vector<std::vector<uint32_t>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint32_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddInt64Vector2D( const std::string & inKey, const std::vector<std::vector<int64_t>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_int64_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddUInt64Vector2D( const std::string & inKey, const std::vector<std::vector<uint64_t>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_uint64_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddFloatVector2D( const std::string & inKey, const std::vector<std::vector<float>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_float_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddDoubleVector2D( const std::string & inKey, const std::vector<std::vector<double>> & inVal ) { 
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_double_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}

void ByteStream::AddStringVector2D( const std::string & inKey, const std::vector<std::vector<std::string>> & inVal ) {
    if ( ReadOnly ) { return; }
    ByteStreamBuffer SBuf( inKey, dt_string_vector2D, 0 );
    SBuf.SetVector2DValue( inVal );
    AddRecord( SBuf );
}


bool ByteStream::GetBool( const std::string & inKey, bool & outVal ) {
    uint8_t val;
    if ( GetUInt8( inKey, val ) ) { outVal = val; return true; }
    return false;
}

bool ByteStream::GetChar( const std::string & inKey, char & outVal ) {
    int8_t val;
    if ( GetInt8( inKey, val ) ) { outVal = val; return true; }
    return false;
}

bool ByteStream::GetInt8( const std::string & inKey, int8_t & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int8 ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetUInt8( const std::string & inKey, uint8_t & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint8 ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetInt16( const std::string & inKey, int16_t & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int16 ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetUInt16( const std::string & inKey, uint16_t & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint16 ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetInt32( const std::string & inKey, int32_t & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int32 ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetUInt32( const std::string & inKey, uint32_t & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint32 ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetInt64( const std::string & inKey, int64_t & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int64 ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetUInt64( const std::string & inKey, uint64_t & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint64 ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetFloat( const std::string & inKey, float & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_float ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetDouble( const std::string & inKey, double & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_double ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetString( const std::string & inKey, std::string & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_string ) { return false; }
    return rec.GetValue( outVal );
}

bool ByteStream::GetBuffer( const std::string & inKey, AppCore::CharBuffer & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_buffer ) { return false; }
    uint64_t valSize = rec.GetValueSize();
    if ( valSize == 0 ) { return false; }
    outVal.Resize( valSize );
    memcpy( outVal.Content, rec.ValuePtr(), valSize );
    return true;
}

bool ByteStream::GetBuffer( const std::string & inKey, void * outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_buffer ) { return false; }
    uint64_t valSize = rec.GetValueSize();
    if ( valSize == 0 ) { return false; }
    memcpy( outVal, rec.ValuePtr(), valSize );
    return true;
}

char * ByteStream::GetBufferPtr( const std::string & inKey, uint64_t & outSize ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { outSize = 0; return nullptr; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_buffer ) { outSize = 0; return nullptr; }
    outSize = rec.GetValueSize();
    if ( outSize == 0 ) { return nullptr; }
    return rec.ValuePtr();
}

bool ByteStream::GetStream( const std::string & inKey, ByteStream & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_stream ) { return false; }
    uint64_t valSize = rec.GetValueSize();
    if ( valSize == 0 ) { return false; }
    outVal.LoadStream( rec.ValuePtr(), valSize );
    return true;
}

bool ByteStream::GetInt8Vector( const std::string & inKey, std::vector<int8_t> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int8_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetUInt8Vector( const std::string & inKey, std::vector<uint8_t> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint8_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetInt16Vector( const std::string & inKey, std::vector<int16_t> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int16_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetUInt16Vector( const std::string & inKey, std::vector<uint16_t> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint16_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetInt32Vector( const std::string & inKey, std::vector<int32_t> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int32_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetUInt32Vector( const std::string & inKey, std::vector<uint32_t> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint32_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetInt64Vector( const std::string & inKey, std::vector<int64_t> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int64_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetUInt64Vector( const std::string & inKey, std::vector<uint64_t> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint64_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetFloatVector( const std::string & inKey, std::vector<float> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_float_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetDoubleVector( const std::string & inKey, std::vector<double> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_double_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetStringVector( const std::string & inKey, std::vector<std::string> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_string_vector ) { return false; }
    return rec.GetVectorValue( outVal );
}

bool ByteStream::GetStreamVector( const std::string & inKey, std::vector<ByteStream> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_stream_vector ) { return false; }
    if ( !rec.IsValid() ) { return false; }
    char * buf = rec.ValuePtr();
    // Get the count and resize the output vector
    uint64_t count;
    memcpy( &count, buf, 8 ); buf += 8;
    outVal.clear();
    // Fill the output vector
    char * sbuf = buf + ( count + 1 ) * 8;
    uint64_t * offsets = ( uint64_t * ) buf;
    uint64_t strSize = 0;
    for ( uint64_t i = 0; i < count; i++ ) {
        ByteStream bstream( ReadOnly );
        strSize = offsets[i+1] - offsets[i];
        if ( strSize > 0 ) {
            bstream.LoadStream( &sbuf[ offsets[i] ], strSize );
        }
        outVal.push_back( std::move( bstream ) );
    }
    return true;
}

bool ByteStream::GetInt8Vector2D( const std::string & inKey, std::vector<std::vector<int8_t>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int8_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetUInt8Vector2D( const std::string & inKey, std::vector<std::vector<uint8_t>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint8_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetInt16Vector2D( const std::string & inKey, std::vector<std::vector<int16_t>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int16_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetUInt16Vector2D( const std::string & inKey, std::vector<std::vector<uint16_t>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint16_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetInt32Vector2D( const std::string & inKey, std::vector<std::vector<int32_t>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int32_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetUInt32Vector2D( const std::string & inKey, std::vector<std::vector<uint32_t>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint32_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetInt64Vector2D( const std::string & inKey, std::vector<std::vector<int64_t>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_int64_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetUInt64Vector2D( const std::string & inKey, std::vector<std::vector<uint64_t>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_uint64_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetFloatVector2D( const std::string & inKey, std::vector<std::vector<float>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_float_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetDoubleVector2D( const std::string & inKey, std::vector<std::vector<double>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_double_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

bool ByteStream::GetStringVector2D( const std::string & inKey, std::vector<std::vector<std::string>> & outVal ) {
    int64_t ind = GetRecordIndex( inKey );
    if ( ind < 0 || ind >= static_cast<int64_t>(Dict.size()) ) { return false; }
    ByteStreamBuffer & rec = Dict[ ind ];
    if ( rec.GetType() != dt_string_vector2D ) { return false; }
    return rec.GetVector2DValue( outVal );
}

// ==================================================================================
//                              PRIVATE RECORD FUNCTIONS
// ==================================================================================

ByteStreamBuffer & ByteStream::AddRecord( ByteStreamBuffer & inRec ) {
    int64_t recI = GetRecordIndex( inRec.GetKey() );
    if ( recI >= 0 ) {              // if the key exists, replace it
        StreamSize += inRec.Size - Dict[recI].Size;
        Dict[recI] = std::move(inRec);
    }
    else if ( inRec.IsValid() ) {   // otherwise, add to the end if record is valid
        recI = Dict.size();
        int fli = inRec.GetKeyFirstLetter() - 'a';
        if ( fli < 0 || fli > 25 ) { fli = 26; }
        StreamSize += 8 + inRec.Size;
        Dict.push_back( std::move(inRec) );
        Lookup[fli].push_back( recI );
    }
    return Dict[recI];
}

// ==================================================================================
//                                  LOGGING FUNCTIONS
// ==================================================================================

void ByteStream::LogInfo() {
    for ( auto & rec : Dict ) {
        printf( "Type: %s, Key: %s, Values Size: %lu, Entry Size %lu\n", rec.GetTypeAsString().c_str(), rec.GetKey().c_str(), (unsigned long) rec.GetValueSize(), (unsigned long) rec.Size );
    }
}

void ByteStream::LogRecords( int indent, bool compact ) {
    for ( auto & rec : Dict ) {
        LogRecord(rec, indent, compact );
    }
}

void ByteStream::LogRecord( ByteStreamBuffer & rec, int indent, bool compact ) {
    if ( !rec.IsValid() ) { return; }

    if ( indent > 0 ) { printf( "%s", string(indent, ' ' ).c_str() ); }
    if ( compact ) {
        printf( "%s (%s) --> ", rec.GetKey().c_str(), rec.GetTypeAsString().c_str() );
    }
    else {
        printf( "Type: %s, Key: %s, Values Size: %lu, Entry Size %lu\n", rec.GetTypeAsString().c_str(), rec.GetKey().c_str(), (unsigned long) rec.GetValueSize(), (unsigned long) rec.Size );
    }

    ByteStreamDataType type = rec.GetType();
    if ( type != dt_stream && type != dt_stream_vector ) {
        if ( !compact ) {
            if ( indent > 0 ) { printf( "%s", string( indent, ' ' ).c_str() ); }
            printf( "%s", string(4, ' ').c_str() );
        }
    }

    if      ( type == dt_int8 ) { printf( "%s\n", rec.ValueToString<int8_t>().c_str() ); }
    else if ( type == dt_uint8 ) { printf( "%s\n", rec.ValueToString<uint8_t>().c_str() ); }
    else if ( type == dt_int16 ) { printf( "%s\n", rec.ValueToString<int16_t>().c_str() ); }
    else if ( type == dt_uint16 ) { printf( "%s\n", rec.ValueToString<uint16_t>().c_str() ); }
    else if ( type == dt_int32 ) { printf( "%s\n", rec.ValueToString<int32_t>().c_str() ); }
    else if ( type == dt_uint32 ) { printf( "%s\n", rec.ValueToString<uint32_t>().c_str() ); }
    else if ( type == dt_int64 ) { printf( "%s\n", rec.ValueToString<int64_t>().c_str() ); }
    else if ( type == dt_uint64 ) { printf( "%s\n", rec.ValueToString<uint64_t>().c_str() ); }
    else if ( type == dt_float ) { printf( "%s\n", rec.ValueToString<float>().c_str() ); }
    else if ( type == dt_double ) { printf( "%s\n", rec.ValueToString<double>().c_str() ); }
    else if ( type == dt_string ) { printf( "%s\n", rec.ValueToString<string>().c_str() ); }
    // else if ( type == dt_vec2 ) { printf( "%s\n", rec.ValueToString<vec2>().c_str() ); }
    // else if ( type == dt_vec3 ) { printf( "%s\n", rec.ValueToString<vec3>().c_str() ); }
    // else if ( type == dt_vec4 ) { printf( "%s\n", rec.ValueToString<vec4>().c_str() ); }
    // else if ( type == dt_mat2 ) { printf( "%s\n", rec.ValueToString<mat2>().c_str() ); }
    // else if ( type == dt_mat3 ) { printf( "%s\n", rec.ValueToString<mat3>().c_str() ); }
    // else if ( type == dt_mat4 ) { printf( "%s\n", rec.ValueToString<mat4>().c_str() ); }

    else if ( type == dt_int8_vector ) { printf( "%s\n", rec.VectorValueToString<int8_t>().c_str() ); }
    else if ( type == dt_uint8_vector ) { printf( "%s\n", rec.VectorValueToString<uint8_t>().c_str() ); }
    else if ( type == dt_int16_vector ) { printf( "%s\n", rec.VectorValueToString<int16_t>().c_str() ); }
    else if ( type == dt_uint16_vector ) { printf( "%s\n", rec.VectorValueToString<uint16_t>().c_str() ); }
    else if ( type == dt_int32_vector ) { printf( "%s\n", rec.VectorValueToString<int32_t>().c_str() ); }
    else if ( type == dt_uint32_vector ) { printf( "%s\n", rec.VectorValueToString<uint32_t>().c_str() ); }
    else if ( type == dt_int64_vector ) { printf( "%s\n", rec.VectorValueToString<int64_t>().c_str() ); }
    else if ( type == dt_uint64_vector ) { printf( "%s\n", rec.VectorValueToString<uint64_t>().c_str() ); }
    else if ( type == dt_float_vector ) { printf( "%s\n", rec.VectorValueToString<float>().c_str() ); }
    else if ( type == dt_double_vector ) { printf( "%s\n", rec.VectorValueToString<double>().c_str() ); }
    else if ( type == dt_string_vector ) { printf( "%s\n", rec.VectorValueToString<string>().c_str() ); }
    // else if ( type == dt_vec2_vector ) { printf( "%s\n", rec.VectorValueToString<vec2>().c_str() ); }
    // else if ( type == dt_vec3_vector ) { printf( "%s\n", rec.VectorValueToString<vec3>().c_str() ); }
    // else if ( type == dt_vec4_vector ) { printf( "%s\n", rec.VectorValueToString<vec4>().c_str() ); }
    // else if ( type == dt_mat2_vector ) { printf( "%s\n", rec.VectorValueToString<mat2>().c_str() ); }
    // else if ( type == dt_mat3_vector ) { printf( "%s\n", rec.VectorValueToString<mat3>().c_str() ); }
    // else if ( type == dt_mat4_vector ) { printf( "%s\n", rec.VectorValueToString<mat4>().c_str() ); }

    // else if ( type == dt_int8_vector2D ) { printf( "%s\n", rec.ValueToString<int8_t>().c_str() ); }
    // else if ( type == dt_uint8_vector2D ) { printf( "%s\n", rec.ValueToString<uint8_t>().c_str() ); }
    // else if ( type == dt_int16_vector2D ) { printf( "%s\n", rec.ValueToString<int16_t>().c_str() ); }
    // else if ( type == dt_uint16_vector2D ) { printf( "%s\n", rec.ValueToString<uint16_t>().c_str() ); }
    // else if ( type == dt_int32_vector2D ) { printf( "%s\n", rec.ValueToString<int32_t>().c_str() ); }
    // else if ( type == dt_uint32_vector2D ) { printf( "%s\n", rec.ValueToString<uint32_t>().c_str() ); }
    // else if ( type == dt_int64_vector2D ) { printf( "%s\n", rec.ValueToString<int64_t>().c_str() ); }
    // else if ( type == dt_uint64_vector2D ) { printf( "%s\n", rec.ValueToString<uint64_t>().c_str() ); }
    // else if ( type == dt_float_vector2D ) { printf( "%s\n", rec.ValueToString<float>().c_str() ); }
    // else if ( type == dt_double_vector2D ) { printf( "%s\n", rec.ValueToString<double>().c_str() ); }
    // else if ( type == dt_string_vector2D ) { printf( "%s\n", rec.ValueToString<string>().c_str() ); }
    // else if ( type == dt_vec2_vector2D ) { printf( "%s\n", rec.ValueToString<vec2>().c_str() ); }
    // else if ( type == dt_vec3_vector2D ) { printf( "%s\n", rec.ValueToString<vec3>().c_str() ); }
    // else if ( type == dt_vec4_vector2D ) { printf( "%s\n", rec.ValueToString<vec4>().c_str() ); }
    // else if ( type == dt_mat2_vector2D ) { printf( "%s\n", rec.ValueToString<mat2>().c_str() ); }
    // else if ( type == dt_mat3_vector2D ) { printf( "%s\n", rec.ValueToString<mat3>().c_str() ); }
    // else if ( type == dt_mat4_vector2D ) { printf( "%s\n", rec.ValueToString<mat4>().c_str() ); }

    else if ( type == dt_buffer ) { printf( " [Buffer of Size %lu]\n", (unsigned long) rec.GetValueSize() ); }
    else if ( type == dt_stream ) { 
        if ( compact ) { printf("\n"); } 
        ByteStream bstr; GetStream( rec.GetKey(), bstr ); bstr.LogRecords( indent + 4, compact );  
    }
    else if ( type == dt_stream_vector ) { 
        if ( compact ) { printf("\n"); } 
        vector<ByteStream> bstrVector;
        GetStreamVector( rec.GetKey(), bstrVector );
        for ( int i = 0; i < bstrVector.size(); i++ ) {
            if ( indent > 0 ) { printf( "%s", string(indent, ' ' ).c_str() ); }
            printf("    ------------ [%i] ------------\n", i);
            bstrVector[i].LogRecords( indent + 4, compact );
        }
    }
    // else if ( type == dt_table ) { printf( "%s\n", rec.ValueToString<table>().c_str() ); }
    // else if ( type == dt_json ) { printf( "%s\n", rec.ValueToString<json>().c_str() ); }

}


#endif /* BYTESTREAM_CPP */
