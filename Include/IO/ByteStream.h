#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#include "ch_Tools.h"

#include <vector>
#include <string>
#include <cstring>

class ByteStream;

enum ByteStreamDataType : uint8_t {

    dt_none = 0,

    dt_int8, dt_uint8,
    dt_int16, dt_uint16,
    dt_int32, dt_uint32,
    dt_int64, dt_uint64,
    dt_float, dt_double,
    dt_string,
    dt_vec2, dt_vec3, dt_vec4,  // FUTURE
    dt_mat2, dt_mat3, dt_mat4,  // FUTURE

    dt_int8_vector, dt_uint8_vector,
    dt_int16_vector, dt_uint16_vector,
    dt_int32_vector, dt_uint32_vector,
    dt_int64_vector, dt_uint64_vector,
    dt_float_vector, dt_double_vector,
    dt_string_vector,
    dt_vec2_vector, dt_vec3_vector, dt_vec4_vector,  // FUTURE
    dt_mat2_vector, dt_mat3_vector, dt_mat4_vector,  // FUTURE

    dt_int8_vector2D, dt_uint8_vector2D,
    dt_int16_vector2D, dt_uint16_vector2D,
    dt_int32_vector2D, dt_uint32_vector2D,
    dt_int64_vector2D, dt_uint64_vector2D,
    dt_float_vector2D, dt_double_vector2D,
    dt_string_vector2D,
    dt_vec2_vector2D, dt_vec3_vector2D, dt_vec4_vector2D,  // FUTURE
    dt_mat2_vector2D, dt_mat3_vector2D, dt_mat4_vector2D,  // FUTURE

    dt_buffer,  // arbitrary buffer
    dt_stream, dt_stream_vector,  // ByteStream
    dt_table,   // Frad
    dt_json,

    dt_last     // last enum value

};

inline static const char * ByteStreamDataStrings[] = {
    "dt_none",

    "dt_int8", "dt_uint8",
    "dt_int16", "dt_uint16",
    "dt_int32", "dt_uint32",
    "dt_int64", "dt_uint64",
    "dt_float", "dt_double",
    "dt_string",
    "dt_vec2", "dt_vec3", "dt_vec4",
    "dt_mat2", "dt_mat3", "dt_mat4",

    "dt_int8_vector", "dt_uint8_vector",
    "dt_int16_vector", "dt_uint16_vector",
    "dt_int32_vector", "dt_uint32_vector",
    "dt_int64_vector", "dt_uint64_vector",
    "dt_float_vector", "dt_double_vector",
    "dt_string_vector",
    "dt_vec2_vector", "dt_vec3_vector", "dt_vec4_vector",
    "dt_mat2_vector", "dt_mat3_vector", "dt_mat4_vector",

    "dt_int8_vector2D", "dt_uint8_vector2D",
    "dt_int16_vector2D", "dt_uint16_vector2D",
    "dt_int32_vector2D", "dt_uint32_vector2D",
    "dt_int64_vector2D", "dt_uint64_vector2D",
    "dt_float_vector2D", "dt_double_vector2D",
    "dt_string_vector2D",
    "dt_vec2_vector2D", "dt_vec3_vector2D", "dt_vec4_vector2D",
    "dt_mat2_vector2D", "dt_mat3_vector2D", "dt_mat4_vector2D",

    "dt_buffer", // arbitrary buffer
    "dt_stream", "dt_stream_vector", // ByteStream
    "dt_table",  // Frad
    "dt_json",

    "dt_last"
};

// ByteStreamBuffer is the structure for each dictionary record.
// Each record of the dictionary stores the Field Type, Key, and Value:
// [ Type (uint8_t)][ Size of Key (uint64_t) ][ Key (string) ][ Value (T) ]

struct ByteStreamBuffer {

    ByteStreamBuffer( bool viewOnly = false );
    ByteStreamBuffer( const std::string & inKey, ByteStreamDataType inType, size_t inValSize );
    ~ByteStreamBuffer();

    // no copy constructor and assignment
    ByteStreamBuffer(const ByteStreamBuffer&) = delete;
    ByteStreamBuffer &operator=(const ByteStreamBuffer&) = delete;

    // move constructor and assignment
    ByteStreamBuffer( ByteStreamBuffer && other );
    ByteStreamBuffer & operator=( ByteStreamBuffer && other );

    char*           Content;
    size_t          Size;       // Bytes
    bool            ViewOnly;

    void                SetView( char * inBuf, size_t inSize );
    void                Clear();
    bool                Resize( size_t inSize );

    bool                IsValid();          // Returns true if the Content contains valid dictionary metadata ( key & type )
    ByteStreamDataType  GetType();          // Returns the enum for the [Type]
    std::string         GetTypeAsString();  // Returns the name of the [Type] enum as a string
    size_t              GetKeySize();       // Returns the size of the they [Key] section
    std::string         GetKey();           // Returns the [Key] as a string
    char                GetKeyFirstLetter();
    size_t              GetValueSize();     // Returns the size of the [Value] section
    char *              ValuePtr();         // Returns a pointer to the start of the [Value] section

    bool                SetValue( const void * inVal ); // This sets the [Value] section.

    template<typename T>
    bool SetVectorValue( std::vector<T> inVals ) {  // This sets the [Value] section from a vector
        // NOTE: The first 8 bytes of the [Value] section for a vector is a uint64_t of the count.
        //       [ Value ] --> [ Count ][ Item 0 ][ Item 1 ] ... [ Item N ]
        // NOTE: If T is a std::string type, then all the offsets (uint64_t) are stored first, 
        //       then the strings are stored back to back with no delimiter.
        //       [ Value ] --> [ Count ][ Offset 0 ][ Offset 1 ] ... [ Offset N ][ End Pos ][ Str 0 ][ Str 1 ] ... [ Str N ]

        // Validate the key & type
        if ( !IsValid() ) { return false; }
        // Calculate how much space is needed for the vector
        uint64_t count = inVals.size();
        uint64_t valSize = 8;
        if constexpr(std::is_same<T, std::string>::value) {
            valSize += ( count + 1 ) * 8;
            for ( auto & val : inVals ) { valSize += val.size(); }
        }
        else {
            valSize += sizeof(T) * inVals.size();
        }
        // Resize if necessary
        if ( !Resize( 9 + GetKeySize() + valSize ) ) { return false; }
        // Write the count and items to the [ Value ] section
        char * buf = ValuePtr();
        memcpy( buf, &count, 8 ); buf += 8;
        if constexpr(std::is_same<T, std::string>::value) {
            uint64_t offset = 0;
            char * sbuf = buf + ( count + 1 ) * 8;
            for ( auto & val : inVals ) {
                memcpy( buf, &offset, 8 ); buf += 8;    // write the offset
                valSize = val.size();
                offset += valSize;
                if ( valSize > 0 ) {                    // write the string
                    memcpy( sbuf, val.c_str(), valSize ); sbuf += valSize;
                }
            }
            memcpy( buf, &offset, 8 );                  // write the end pos
        }
        else {
            memcpy( buf, inVals.data(), valSize-8 );
        }
        return true;
    };

    template<typename T>
    bool SetVector2DValue( std::vector<std::vector<T>> inVals ) {  // This sets the [Value] section from a 2D vector
        // NOTE: The first 8 bytes of the [Value] section for a vector is a uint64_t of the count.
        //       [ Value ] --> [ Count ]
        //                     [ SubVector 0 Count][ SubVector 1 Count ] ... [ SubVector N Count ]
        //                     [ SubVector 0 Data ][ SubVector 1 Data  ] ... [ SubVector N Data  ]
        // NOTE: If T is a std::string type, then all the offset positions (uint64_t) are stored first, 
        //       then the strings are stored back to back with no delimiter.
        //       [ Value ] --> [ Count ]
        //                     [ SubVector 0 Count ][ SubVector 1 Count ] ... [ SubVector N Count ]
        //                          [ SubVector 0 Pos 0 ][ SubVector 0 Pos 1 ] ... [ SubVector 0 End Pos ]
        //                          [ SubVector 1 Pos 0 ][ SubVector 1 Pos 1 ] ... [ SubVector 1 End Pos ]
        //                              ...
        //                          [ SubVector N Pos 0 ][ SubVector N Pos 1 ] ... [ SubVector N End Pos ]
        //                     [ SubVector 0 Data ][ SubVector 1 Data  ] ... [ SubVector N Data  ]

        // Validate the key & type
        if ( !IsValid() ) { return false; }
        // Calculate how much space is needed for the 2D vector
        uint64_t count = inVals.size();
        std::vector<uint64_t> subcounts; subcounts.reserve( count );
        uint64_t valSize = 8 * ( count + 1 );   // add space for the count and subcounts
        uint64_t totalcount = 0;  // total count of values (used by string types only)
        if constexpr(std::is_same<T, std::string>::value) {
            for ( auto & subvec : inVals ) {
                subcounts.push_back( subvec.size() );
                totalcount += subvec.size();
                valSize += subvec.size() * 8;   // add space for the string offset pos
                for ( auto & val : subvec ) {
                    valSize += val.size();      // add space for the string value
                }
            }
            valSize += 8;                       // add space for the string end pos
        }
        else {
            for ( auto & subvec : inVals ) {
                subcounts.push_back( subvec.size() );
                valSize += sizeof(T) * subvec.size();
            }
        }
        // Resize if necessary
        if ( !Resize( 9 + GetKeySize() + valSize ) ) { return false; }
        // Write the counts and subcounts
        char * buf = ValuePtr();
        memcpy( buf, &count, 8 ); buf += 8;
        memcpy( buf, subcounts.data(), count*8 ); buf += count*8;
        // Write the data
        if constexpr(std::is_same<T, std::string>::value) {
            uint64_t offset = 0;
            char * sbuf = buf + ( totalcount + 1 ) * 8;     // string buffer
            for ( auto & subvec : inVals ) {
                for ( auto & val : subvec ) {
                    memcpy( buf, &offset, 8 ); buf += 8;    // write the offset
                    valSize = val.size();
                    offset += valSize;
                    if ( valSize > 0 ) {                    // write the string
                        memcpy( sbuf, val.c_str(), valSize ); sbuf += valSize;
                    }
                }
            }
            memcpy( buf, &offset, 8 );                  // write the end pos
        }
        else {
            uint64_t subsize = 0;
            for ( auto & subvec : inVals ) {
                subsize = sizeof(T) * subvec.size();
                memcpy( buf, subvec.data(), subsize ); buf += subsize;
            }
        }
        return true;
    };
    
    template<typename T>
    bool GetValue( T & outVal ) {
        if ( !IsValid() ) { return false; }
        uint64_t valSize = GetValueSize();
        char * buf = ValuePtr();
        if constexpr(std::is_same<T, std::string>::value) {                
            outVal.assign( buf, valSize );
        }
        else {
            memcpy( &outVal, buf, valSize );
        }
        return true;
    };

    template<typename T>
    bool GetVectorValue( std::vector<T> & outVal ) {
        if ( !IsValid() ) { outVal.clear(); return false; }
        char * buf = ValuePtr();
        // Get the count and resize the output vector
        uint64_t count;
        memcpy( &count, buf, 8 ); buf += 8;
        outVal.resize( count );
        // Fill the output vector
        if constexpr(std::is_same<T, std::string>::value) {
            char * sbuf = buf + ( count + 1 ) * 8;
            uint64_t * offsets = ( uint64_t * ) buf;
            uint64_t strSize = 0;
            for ( uint64_t i = 0; i < count; i++ ) {
                strSize = offsets[i+1] - offsets[i];
                if ( strSize == 0 ) { outVal[i] = ""; }
                else { 
                    outVal[i].assign( &sbuf[ offsets[i] ], strSize);
                }
            }
        }
        else {
            memcpy( outVal.data(), buf, GetValueSize()-8 );
        }
        return true;
    };

    template<typename T>
    bool GetVector2DValue( std::vector<std::vector<T>> & outVal ) {
        if ( !IsValid() ) { outVal.clear(); return false; }
        char * buf = ValuePtr();
        // Get the count and resize the output vector
        uint64_t count, totalcount = 0;
        memcpy( &count, buf, 8 ); buf += 8;
        outVal.resize( count );
        // Get the subcounts and resize the output subvectors
        uint64_t * subcounts = ( uint64_t * ) buf; buf += count * 8;
        for ( uint64_t i = 0; i < count; i++ ) {
            totalcount += subcounts[i];
            outVal[i].resize( subcounts[i] );
        }
        // Fill the output vector
        if constexpr(std::is_same<T, std::string>::value) {
            char * sbuf = buf + ( totalcount + 1 ) * 8;
            uint64_t * offsets = ( uint64_t * ) buf;
            uint64_t strSize = 0;
            uint64_t index = 0;
            for ( uint64_t i = 0; i < count; i++ ) {
                for ( uint64_t j = 0; j < subcounts[i]; j++ ) {
                    strSize = offsets[index+1] - offsets[index];
                    if ( strSize == 0 ) { outVal[i][j] = ""; }
                    else { 
                        outVal[i][j].assign( &sbuf[ offsets[index] ], strSize);
                    }
                    index++;
                }
            }
        }
        else {
            uint64_t subsize = 0;
            for ( uint64_t i = 0; i < count; i++ ) {
                subsize = sizeof(T) * subcounts[i];
                memcpy( outVal[i].data(), buf, subsize ); buf += subsize;
            }
        }
        return true;
    };

    template<typename T>
    std::string ValueToString() {
        if ( !IsValid() ) { return ""; }
        if constexpr(std::is_same<T, std::string>::value) {
            std::string str;
            GetValue<std::string>( str );
            str = '"' + str + '"';
            return std::move(str);
        }
        else {
            T * buf = ( T* ) ValuePtr();
            return std::to_string( *buf );
        }
    };

    template<typename T>
    std::string VectorValueToString() {
        if ( !IsValid() ) { return "{}"; }
        std::string str = "{ ";
        std::string dlm = "";
        char * buf = ValuePtr();
        uint64_t count;
        memcpy( &count, buf, 8 ); buf += 8;
        if constexpr(std::is_same<T, std::string>::value) {
            char * sbuf = buf + ( count + 1 ) * 8;
            uint64_t * offsets = ( uint64_t * ) buf;
            uint64_t strSize = 0;
            for ( uint64_t i = 0; i < count; i++ ) {
                str += dlm; dlm = ", ";
                strSize = offsets[i+1] - offsets[i];
                str += '"';
                for ( uint64_t j = 0; j < strSize; j++ ) {
                    str += *sbuf; sbuf++;
                }
                str += '"';      
            }
        }
        else {
            T * tBuf = ( T* ) buf;
            for ( uint64_t i = 0; i < count; i++ ) {
                str += dlm + std::to_string( tBuf[i] ); dlm = ", ";
            }
        }
        str += " }";
        return std::move(str);
    };

};

// ByteStream is a dictionary format with type information.
// The ByteStream consists of multiple ByteStreamBuffers stored back to back with some
// metadata at the front of the stream.
//    First store the buffer count:            [ Nb of Buf (8 bytes) ]
//    Then store the start positions:          [ Buf 0 Start Pos (8 bytes) ][ Buf 1 Start Pos (8 bytes) ] ... [ Last Buf StartPos (8 bytes) ]
//    Store the file end position:             [ File End Pos (8 bytes) ]
//    Then store the buffer data back to back: [ Buf 0 ][Buf 1] ... [Last Buf]
class ByteStream {

public:

    ByteStream( bool readOnly = false );

    void ClearAll();
    void Reset();

    size_t GetStreamSize();  // Get the total size required for writing the ByteStream to a single buffer
    void GetStream( char * outBuf );   // Convert the ByteStream Dict to a single buffer. The buffer MUST be pre-allocated.
    void LoadStream( char * inBuf, size_t inSize );  // Fill the ByteStream Dict from a single buffer.
    
    size_t GetCompressedSize();
    size_t GetCompressedStream( unsigned char * outBuf );   // Convert the ByteStream Dict to a single buffer and compresses it. The buffer MUST be pre-allocated.
    size_t GetUncompressedSize( unsigned char * inBuf );
    void LoadCompressedStream( unsigned char * inBuf, size_t inSize );  // Fill the ByteStream Dict from a single compressed buffer.

#ifndef R64_CLIENT
    void Save( const std::string & path );
    void Read( const std::string & path );
    void SaveCompressed( const std::string & path );
    void ReadCompressed( const std::string & path );
#endif

    int64_t  GetRecordIndex( const std::string & inKey );
    size_t   GetRecordValueSize( const std::string & inKey );
    bool     HasRecord( const std::string & inKey );

    void AddBool( const std::string & inKey, const bool & inVal );
    void AddChar( const std::string & inKey, const char & inVal );
    void AddInt8( const std::string & inKey, const int8_t & inVal );
    void AddUInt8( const std::string & inKey, const uint8_t & inVal );
    void AddInt16( const std::string & inKey, const int16_t & inVal );
    void AddUInt16( const std::string & inKey, const uint16_t & inVal );
    void AddInt32( const std::string & inKey, const int32_t & inVal );
    void AddUInt32( const std::string & inKey, const uint32_t & inVal );
    void AddInt64( const std::string & inKey, const int64_t & inVal );
    void AddUInt64( const std::string & inKey, const uint64_t & inVal );
    void AddFloat( const std::string & inKey, const float & inVal );
    void AddDouble( const std::string & inKey, const double & inVal );
    void AddString( const std::string & inKey, const std::string & inVal );
    void AddBuffer( const std::string & inKey, const AppCore::CharBuffer & inVal );
    void AddBuffer( const std::string & inKey, const void * inVal, size_t inSize );
    char * AddBuffer( const std::string & inKey, size_t inSize );
    void AddStream( const std::string & inKey, ByteStream & inVal );

    void AddInt8Vector( const std::string & inKey, const std::vector<int8_t> & inVal );
    void AddUInt8Vector( const std::string & inKey, const std::vector<uint8_t> & inVal );
    void AddInt16Vector( const std::string & inKey, const std::vector<int16_t> & inVal );
    void AddUInt16Vector( const std::string & inKey, const std::vector<uint16_t> & inVal );
    void AddInt32Vector( const std::string & inKey, const std::vector<int32_t> & inVal );
    void AddUInt32Vector( const std::string & inKey, const std::vector<uint32_t> & inVal );
    void AddInt64Vector( const std::string & inKey, const std::vector<int64_t> & inVal );
    void AddUInt64Vector( const std::string & inKey, const std::vector<uint64_t> & inVal );
    void AddFloatVector( const std::string & inKey, const std::vector<float> & inVal );
    void AddDoubleVector( const std::string & inKey, const std::vector<double> & inVal );
    void AddStringVector( const std::string & inKey, const std::vector<std::string> & inVal );
    void AddStreamVector( const std::string & inKey, std::vector<ByteStream> & inVal );

    void AddInt8Vector2D( const std::string & inKey, const std::vector<std::vector<int8_t>> & inVal );
    void AddUInt8Vector2D( const std::string & inKey, const std::vector<std::vector<uint8_t>> & inVal );
    void AddInt16Vector2D( const std::string & inKey, const std::vector<std::vector<int16_t>> & inVal );
    void AddUInt16Vector2D( const std::string & inKey, const std::vector<std::vector<uint16_t>> & inVal );
    void AddInt32Vector2D( const std::string & inKey, const std::vector<std::vector<int32_t>> & inVal );
    void AddUInt32Vector2D( const std::string & inKey, const std::vector<std::vector<uint32_t>> & inVal );
    void AddInt64Vector2D( const std::string & inKey, const std::vector<std::vector<int64_t>> & inVal );
    void AddUInt64Vector2D( const std::string & inKey, const std::vector<std::vector<uint64_t>> & inVal );
    void AddFloatVector2D( const std::string & inKey, const std::vector<std::vector<float>> & inVal );
    void AddDoubleVector2D( const std::string & inKey, const std::vector<std::vector<double>> & inVal );
    void AddStringVector2D( const std::string & inKey, const std::vector<std::vector<std::string>> & inVal );    

    bool GetBool( const std::string & inKey, bool & outVal );
    bool GetChar( const std::string & inKey, char & outVal );
    bool GetInt8( const std::string & inKey, int8_t & outVal );
    bool GetUInt8( const std::string & inKey, uint8_t & outVal );
    bool GetInt16( const std::string & inKey, int16_t & outVal );
    bool GetUInt16( const std::string & inKey, uint16_t & outVal );
    bool GetInt32( const std::string & inKey, int32_t & outVal );
    bool GetUInt32( const std::string & inKey, uint32_t & outVal );
    bool GetInt64( const std::string & inKey, int64_t & outVal );
    bool GetUInt64( const std::string & inKey, uint64_t & outVal );
    bool GetFloat( const std::string & inKey, float & outVal );
    bool GetDouble( const std::string & inKey, double & outVal );
    bool GetString( const std::string & inKey, std::string  & outVal );
    bool GetBuffer( const std::string & inKey, AppCore::CharBuffer & outVal );
    bool GetBuffer( const std::string & inKey, void * outVal );
    char * GetBufferPtr( const std::string & inKey, uint64_t & outSize );
    bool GetStream( const std::string & inKey, ByteStream & outVal );

    bool GetInt8Vector( const std::string & inKey, std::vector<int8_t> & outVal );
    bool GetUInt8Vector( const std::string & inKey, std::vector<uint8_t> & outVal );
    bool GetInt16Vector( const std::string & inKey, std::vector<int16_t> & outVal );
    bool GetUInt16Vector( const std::string & inKey, std::vector<uint16_t> & outVal );
    bool GetInt32Vector( const std::string & inKey, std::vector<int32_t> & outVal );
    bool GetUInt32Vector( const std::string & inKey, std::vector<uint32_t> & outVal );
    bool GetInt64Vector( const std::string & inKey, std::vector<int64_t> & outVal );
    bool GetUInt64Vector( const std::string & inKey, std::vector<uint64_t> & outVal );
    bool GetFloatVector( const std::string & inKey, std::vector<float> & outVal );
    bool GetDoubleVector( const std::string & inKey, std::vector<double> & outVal );
    bool GetStringVector( const std::string & inKey, std::vector<std::string> & outVal );
    bool GetStreamVector( const std::string & inKey, std::vector<ByteStream> & outVal );

    bool GetInt8Vector2D( const std::string & inKey, std::vector<std::vector<int8_t>> & outVal );
    bool GetUInt8Vector2D( const std::string & inKey, std::vector<std::vector<uint8_t>> & outVal );
    bool GetInt16Vector2D( const std::string & inKey, std::vector<std::vector<int16_t>> & outVal );
    bool GetUInt16Vector2D( const std::string & inKey, std::vector<std::vector<uint16_t>> & outVal );
    bool GetInt32Vector2D( const std::string & inKey, std::vector<std::vector<int32_t>> & outVal );
    bool GetUInt32Vector2D( const std::string & inKey, std::vector<std::vector<uint32_t>> & outVal );
    bool GetInt64Vector2D( const std::string & inKey, std::vector<std::vector<int64_t>> & outVal );
    bool GetUInt64Vector2D( const std::string & inKey, std::vector<std::vector<uint64_t>> & outVal );
    bool GetFloatVector2D( const std::string & inKey, std::vector<std::vector<float>> & outVal );
    bool GetDoubleVector2D( const std::string & inKey, std::vector<std::vector<double>> & outVal );
    bool GetStringVector2D( const std::string & inKey, std::vector<std::vector<std::string>> & outVal );

    // Logging Functions (DEBUG only)
    void LogInfo();
    void LogRecords( int indent = 0, bool compact = false);
    void LogRecord( ByteStreamBuffer & rec, int indent, bool compact );

private:

    bool                                ReadOnly;
    AppCore::CharBuffer                 Stream;         // Helper CharBuffer to store read-only stream data

    std::vector<ByteStreamBuffer>       Dict = {};
    std::vector<std::vector<int64_t>>   Lookup;         // Dictionary lookup -- a-z and index 26 is special characters
    uint64_t                            StreamSize = 0; // Total ByteStream size

    ByteStreamBuffer & AddRecord( ByteStreamBuffer & inRec );

};
    




#endif /* BYTESTREAM_H */
