#ifndef CH_TOOLS_CPP
#define CH_TOOLS_CPP

#if defined( _WIN32 )
    #define _CRT_SECURE_NO_WARNINGS 1
    #pragma warning(disable:4996)  // stop Microsoft warning on standard C fope
    #include "shlwapi.h"
#elif defined(  __linux__  )
    #include <sys/types.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
#endif


#include "ch_Tools.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <time.h>
#include <cstring>

namespace AppCore {

    // ============================================================================================ //
    //                                          CHAR BUFFER                                         //
    // ============================================================================================ //

    CharBuffer::CharBuffer() :
        Size( 0 ),
        Content( nullptr ) {
    }  

    CharBuffer::CharBuffer( char* inBuffer, size_t inSize ) :
        Size( inSize ), 
        Content( inBuffer ) {
    }

    CharBuffer::CharBuffer( unsigned char* inBuffer, size_t inSize ) :
        Size( inSize ), 
        Content( reinterpret_cast<char*>(inBuffer) ) {
    }

    CharBuffer::CharBuffer( size_t inSize ) :
        Size(inSize),
        Content( static_cast<char*>( malloc(inSize) ) ) {
    }

    CharBuffer::CharBuffer( size_t inCount, size_t inSize ) :
        Size( inCount * inSize ),
        Content( static_cast<char*>( calloc( inCount, inSize ) ) ) {
    }
    
    CharBuffer::~CharBuffer() {
        Clear();
    }

    CharBuffer::CharBuffer( CharBuffer && other ) :
        Size( other.Size ),
        Content( other.Content ) {
        other.Size = 0;
        other.Content = nullptr;
    }

    CharBuffer & CharBuffer::operator=( CharBuffer && other ) {
        if ( Size > 0 ) { Clear(); }
        Size = other.Size;
        Content = other.Content;
        other.Size = 0;
        other.Content = nullptr;
        return *this;
    }

    bool CharBuffer::operator== (const CharBuffer& o) const {
        return ( Size == o.Size && memcmp(Content, o.Content, Size) == 0 );
    }

    bool CharBuffer::operator!= (const CharBuffer& o) const { return !(*this == o); }

    void CharBuffer::Clear() {
        if (Content != nullptr) {  
            free(Content); 
        } else if (Content != NULL && Content[0] != '\0') {  
            free(Content); 
        }
        Content = nullptr; Size = 0;
    }

    void CharBuffer::Resize( size_t inSize ) {
        if ( inSize == Size ) { return; }
        if ( inSize == 0 ) {
            Clear();
        } else {
            if( void* mem = realloc( Content, inSize ) ) {
                Content = static_cast<char*>(mem);
                Size = inSize;
            } else {
                throw std::bad_alloc();
            }
        }
    }

    void CharBuffer::Grow( size_t inSize ) {
        if ( inSize <= Size ) { return; }
        if( void* mem = realloc( Content, inSize ) ) {
            Content = static_cast<char*>(mem);
            Size = inSize;
        } else {
            throw std::bad_alloc();
        }
    }


    // ============================================================================================ //
    //                                             TOOLS                                            //
    // ============================================================================================ //

    std::vector<char> Tools::GetBinaryFileContents( std::string const &filename ) {                   

        std::ifstream file( filename, std::ios::binary );
        if( file.fail() ) {
        throw std::runtime_error( std::string( "Could not open \"" + filename + "\" file!" ).c_str() );
        }

        std::streampos begin, end;
        begin = file.tellg();
        file.seekg( 0, std::ios::end );
        end = file.tellg();

        std::vector<char> result( static_cast<size_t>(end - begin) );
        file.seekg( 0, std::ios::beg );
        file.read( &result[0], end - begin );
        file.close();

        return result;

    }
     void Tools::OpenWebLink( const std::string & inLink ) {
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
    
    std::string Tools::GetStampYMDHMS(){
        time_t t = time(0);
        struct tm * now = localtime( & t );
        char buffer [80];
        strftime (buffer,80,"%y%m%d%H%M%S",now);
        std::string rString(buffer);
        return rString;
    }

    std::vector<uint64_t> Tools::SplitRange( uint64_t inStart, uint64_t inRange, uint32_t inDiv ) {

        uint64_t rmd = inRange % inDiv;
        uint64_t divSize = inRange / inDiv;
        uint64_t index = inStart;
        std::vector<uint64_t> divPoints( inDiv+1 );

        for ( uint32_t i = 0; i <= inDiv; i++ ) {
            divPoints[i] = index; 
            index += divSize;
            if ( rmd > 0) { index++; rmd--; }
        }

        return divPoints;

    }

    std::vector<uint64_t> Tools::SplitRange( uint64_t inStart, uint64_t inRange, uint32_t maxNbDiv, uint64_t prefDivSize ) {
        // Returns the split range boundaries
        // Returns no more than than the max number of divisions requested, but tries to target the prefDivSize.
        uint32_t nDiv = (uint32_t) (inRange / prefDivSize);
        if ( inRange % prefDivSize > 0 ) { nDiv++; }
        if ( nDiv > maxNbDiv ) { nDiv = maxNbDiv; }
        return Tools::SplitRange( inStart, inRange, nDiv );
    }

    // ASCII Map -- CASE INSENSITIVE ( A and a are the same )
    const unsigned char Tools::ascii_map_no_case[256] =  {
        0,      1,      2,      3,      4,      5,      6,      7,      8,      9,      10,     11,     12,     13,     14,     15,         // 0-15
        16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,         // 16-31
        33,     38,     39,     40,     41,     42,     43,     36,     44,     45,     46,     76,     47,     37,     48,     49,         // 32-47
        122,    126,    127,    128,    129,    130,    131,    132,    133,    134,    50,     51,     77,     78,     79,     52,         // 48-63
        53,     135,    144,    145,    147,    149,    154,    156,    157,    158,    164,    165,    166,    167,    168,    170,        // 64-79
        178,    179,    180,    181,    183,    185,    190,    191,    192,    193,    196,    54,     55,     56,     57,     58,         // 80-95
        59,     135,    144,    145,    147,    149,    155,    156,    157,    158,    164,    165,    166,    167,    168,    170,        // 96-111
        178,    179,    180,    181,    183,    185,    190,    191,    192,    193,    196,    60,     61,     62,     63,     32,         // 112-127

        146,    189,    150,    139,    140,    138,    142,    146,    152,    153,    151,    163,    162,    161,    140,    142,        // 128+
        150,    143,    143,    174,    175,    173,    188,    187,    195,    175,    189,    177,    73,     177,    83,     155,
        137,    160,    172,    186,    169,    169,    136,    171,    70,     117,    116,    124,    123,    64,     81,     82,
        111,    112,    113,    88,     100,    137,    139,    138,    115,    101,    89,     93,     97,     72,     75,     92,
        94,     104,    102,    98,     86,     106,    141,    141,    95,     91,     105,    103,    99,     87,     107,    74,
        148,    148,    152,    153,    151,    159,    160,    162,    163,    96,     90,     110,    109,    65,     161,    108,
        172,    182,    174,    173,    176,    176,    119,    184,    184,    186,    188,    187,    194,    194,    67,     68,
        35,     80,     71,     125,    120,    114,    84,     69,     118,    66,     121,    126,    128,    127,    85,     34

    };

    // ASCII Map -- CASE SENSITIVE ( AaBb )
    const unsigned char Tools::ascii_map_with_case[256] =  {
        0,      1,      2,      3,      4,      5,      6,      7,      8,      9,      10,     11,     12,     13,     14,     15,         // 0-15
        16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,         // 16-31
        33,     38,     39,     40,     41,     42,     43,     36,     44,     45,     46,     76,     47,     37,     48,     49,         // 32-47
        122,    126,    128,    130,    132,    133,    134,    135,    136,    137,    50,     51,     77,     78,     79,     52,         // 48-63
        53,     138,    155,    157,    161,    165,    175,    178,    180,    182,    193,    195,    197,    199,    201,    205,        // 64-79
        220,    222,    224,    226,    229,    233,    243,    245,    247,    249,    254,    54,     55,     56,     57,     58,         // 80-95
        59,     139,    156,    158,    162,    166,    176,    179,    181,    183,    194,    196,    198,    200,    202,    206,        // 96-111
        221,    223,    225,    227,    230,    234,    244,    246,    248,    250,    255,    60,     61,     62,     63,     32,         // 112-127

        159,    242,    168,    146,    148,    144,    152,    160,    172,    174,    170,    192,    190,    188,    147,    151,        // 128+
        167,    154,    153,    213,    215,    211,    240,    238,    253,    214,    241,    219,    73,     218,    83,     177,
        142,    186,    209,    236,    204,    203,    140,    207,    70,     117,    116,    124,    123,    64,     81,     82,
        111,    112,    113,    88,     100,    141,    145,    143,    115,    101,    89,     93,     97,     72,     75,     92,
        94,     104,    102,    98,     86,     106,    150,    149,    95,     91,     105,    103,    99,     87,     107,    74,
        164,    163,    171,    173,    169,    184,    185,    189,    191,    96,     90,     110,    109,    65,     187,    108,
        208,    228,    212,    210,    217,    216,    119,    232,    231,    235,    239,    237,    252,    251,    67,     68,
        35,     80,     71,     125,    120,    114,    84,     69,     118,    66,     121,    127,    131,    129,    85,     34
    };


    // ============================================================================================ //
    //                                     FILE ENGINE HELPERS                                      //
    // ============================================================================================ //

// Check for BOM
// https://simple.wikipedia.org/wiki/Byte_order_mark
// EF BB BF	    UTF-8
// FE FF	    UTF-16, big-endian
// FF FE	    UTF-16, little-endian
// 00 00 FE FF	UTF-32, big-endian
// FF FE 00 00	UTF-32, little-endian
#define CHECK_FOR_BOM                                                                           \
    unsigned char * c = Content;                                                                \
    if (c == nullptr) {                                                                         \
        return false;                                                                           \
    }                                                                                           \
    else if ( *c == 0xEF ) {                                                                    \
        if ( FileSize > 2 && c[1] == 0xBB && c[2] == 0xBF ) { Content += 3; FileSize -= 3; }    \
    }                                                                                           \
    else if ( *c == 0xFE ) {                                                                    \
        if ( FileSize > 1 && c[1] == 0xFF ) { Content += 2; FileSize -= 2; }                    \
    }                                                                                           \
    else if ( *c == 0xFF ) {                                                                    \
        if ( FileSize > 1 && c[1] == 0xFE ) {                                                   \
            Content += 2; FileSize -= 2;                                                        \
            if ( FileSize > 3 && c[2] == 0x00 && c[3] == 0x00 ) { Content += 2; FileSize -= 2; }\
        }                                                                                       \
    }                                                                                           \
    else if ( *c == 0x00 ) {                                                                    \
        if ( FileSize > 3 && c[2] == 0xFE && c[3] == 0xFF ) { Content += 4; FileSize -= 4; }    \
    }


    bool IsValidAccessType( std::string inAccess ) {
        if (inAccess != "r" && inAccess != "w") {
            printf("Requested file access, \"%s\" not valid. Valid options: \"r\", \"w\"", inAccess.c_str());
            return false;
        }
        return true;
    }

    // ============================================================================================ //
    //                                     MEMORY MAPPED FILE                                       //
    // ============================================================================================ //

#if defined( _WIN32 )

    MMFile::MMFile() :
        Content( nullptr ),
        FileSize( 0 ),
        ViewOffset(),
        ViewSize(),
        Path(),
        Access(),
        Granularity(),
        FileHandle( NULL ),
        FileMap( NULL ),
        MapView( NULL ), 
        MSFileSize(),
        MapViewOffset(),
        MapViewSize(),
        MapViewBuffer( NULL ) {
        
        SYSTEM_INFO sysinfo = {0};
        GetSystemInfo(&sysinfo);
        Granularity = sysinfo.dwAllocationGranularity;
        
    }

    MMFile::~MMFile() {
        ClearAll();
    }

    MMFile::MMFile( MMFile && o ) :
        Content(o.Content),
        FileSize(std::move(o.FileSize)),
        ViewOffset(std::move(o.ViewOffset)),
        ViewSize(std::move(o.ViewSize)),
        Path(std::move(o.Path)),
        Access(std::move(o.Access)),
        Granularity(std::move(o.Granularity)),
        FileHandle(o.FileHandle),
        FileMap(o.FileMap),
        MapView(o.MapView),
        MSFileSize(std::move(o.MSFileSize)),
        MapViewOffset(std::move(o.MapViewOffset)),
        MapViewSize(o.MapViewSize),
        MapViewBuffer(o.MapViewBuffer) {
            o.Content = nullptr;
            o.FileSize = 0;
            o.FileHandle = NULL;
            o.FileMap = NULL;
            o.MapView = NULL;
            o.MapViewSize = NULL;
            o.MapViewBuffer = NULL;
    }

    MMFile & MMFile::operator=( MMFile && o ) {
        ClearAll();
        Content = o.Content;
        FileSize = std::move(o.FileSize);
        ViewOffset = std::move(o.ViewOffset);
        ViewSize = std::move(o.ViewSize);
        Path = std::move(o.Path);
        Access = std::move(o.Access);
        Granularity = std::move(o.Granularity);
        FileHandle = o.FileHandle;
        FileMap = o.FileMap;
        MapView = o.MapView;
        MSFileSize = std::move(o.MSFileSize);
        MapViewOffset = std::move(o.MapViewOffset);
        MapViewSize = o.MapViewSize;
        MapViewBuffer = o.MapViewBuffer;
            o.Content = nullptr;
            o.FileSize = 0;
            o.FileHandle = NULL;
            o.FileMap = NULL;
            o.MapView = NULL;
            o.MapViewSize = NULL;
            o.MapViewBuffer = NULL;
        return *this;
    }


    void MMFile::ClearAll() {
        ClearView();
        if (FileMap != NULL) { CloseHandle(FileMap); FileMap = NULL; }
        if (FileHandle != NULL) {
            // std::cout << "Closing handle: " << Path << std::endl;
            CloseHandle(FileHandle); FileHandle = NULL; MSFileSize.QuadPart = FileSize = 0; Path = ""; Access="";
        }
    }

    void MMFile::ClearView() {
        if (MapViewBuffer != NULL){
            UnmapViewOfFile(MapViewBuffer);
            MapViewBuffer = NULL;
            Content = nullptr;
            ViewOffset = 0; ViewSize = 0;
            MapViewOffset.QuadPart = 0; MapViewSize = 0;
        }
    }

    bool MMFile::OpenFile( std::string inPath, std::string inAccess ) {
        return OpenFile( inPath.c_str(), inAccess );
    }

    bool MMFile::OpenFile( const char* inPath, std::string inAccess ) {
        // Opens an existing non-empty file for read or write access
        if ( !IsValidAccessType( inAccess ) ) { return false; }
        ClearAll();
        Access = inAccess;

        DWORD desired_access = GENERIC_READ, share_mode = FILE_SHARE_READ, page_protection = PAGE_READONLY;
        if ( Access == "w" ) {
            desired_access = desired_access | GENERIC_WRITE; share_mode = share_mode | FILE_SHARE_WRITE; page_protection = PAGE_READWRITE;
        }
        FileHandle = CreateFile(inPath, desired_access, share_mode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == FileHandle){
            printf("MMFile - OpenFile(\'%s\') failed with system error code %d\n", inPath, GetLastError());
            ClearAll();
            return false;
        }
        Path = inPath;
        // std::cout << "OpenFile: " << Path << std::endl;

        MSFileSize.QuadPart = 0;
        MSFileSize.LowPart = GetFileSize(FileHandle, (DWORD *)&MSFileSize.HighPart);
        if (INVALID_FILE_SIZE == MSFileSize.LowPart){
            printf("MMFile - GetFileSize failed with system error code %d\n", GetLastError());
            ClearAll();
            return false;
        }
        if ( 0 == MSFileSize.QuadPart){
            printf("MMFile - File is empty\n");
            ClearAll();
            return false;
        }
        FileSize = MSFileSize.QuadPart;

        FileMap = CreateFileMapping(FileHandle, NULL, page_protection, 0, 0, NULL);
        if (NULL == FileMap){
            printf("MMFile - CreateFileMapping failed with system error code %d\n", GetLastError());
            ClearAll();
            return false;
        }

        if ( !SetView() ) { return false; }

        CHECK_FOR_BOM

        return true;

    }

    bool MMFile::NewFile( std::string inPath, size_t inSize ) {
        return NewFile( inPath.c_str(), inSize );
    }

    bool MMFile::NewFile( const char* inPath, size_t inSize ) {
        // Creates a new file with write access. This will overwrite any existing file.
        // If no size is specified, then this generates a 1 byte file filled with null values.
        ClearAll();
        Access = "w";
        FileHandle = CreateFile(inPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == FileHandle){
            printf("MMFile - CreateFile(\'%s\') failed with system error code %d\n", inPath, GetLastError());
            ClearAll();
            return false;
        }
        Path = inPath;
        if ( inSize < 1 ) { inSize = 1; }
        return ResizeFile( inSize);
    }

    bool MMFile::SetView() {
        return SetView(0, FileSize);
    }

    bool MMFile::SetView( size_t inOffset, size_t inSize ) {
        // Sets the Content buffer view to the requested offset and size.
        // The view is truncated to the FileSize.
        if ( inOffset == ViewOffset && inSize == ViewSize ) { return true; }    // Requested offset and size is the same as the current view
        ClearView();
        ViewOffset = inOffset;
        ViewSize = ( inOffset + inSize > FileSize ) ? (FileSize - inOffset) : inSize;

        // Align offset with system granularity
        MapViewOffset.QuadPart = Tools::AlignDown(inOffset, (size_t) Granularity);
        size_t shift = inOffset - MapViewOffset.QuadPart;
        MapViewSize = inSize + shift;

        // Read or Write access
        DWORD map_access;
        if ( Access == "r" ) {
            map_access = FILE_MAP_READ;
        } else if ( Access == "w" ) {
            map_access = FILE_MAP_WRITE;
        }
        
        MapViewBuffer = MapViewOfFile(FileMap, map_access, (DWORD) MapViewOffset.HighPart, (DWORD) MapViewOffset.LowPart, MapViewSize);
        if (NULL == MapViewBuffer) {
            printf("MMFile - MapViewOfFile failed with system error code %d\n", GetLastError());
            return false;
        }
        Content = &((unsigned char *)MapViewBuffer)[shift];
        return true;
    }

    bool MMFile::WriteToDisk( const unsigned char * inData, size_t inOffset, size_t inSize ) {
        // Writes the contents of inData to the current open file. If the requested offset and
        // size exceed the current file size, then the file is expanded to fit the data.
        if ( Access != "w" ) { return false; }
        if ( !ResizeFile( inOffset + inSize ) ) { return false; }
        memcpy( &Content[inOffset], inData, ViewSize );
        return FlushView();
    }

    bool MMFile::FlushView() {
        if ( Access != "w" ) { return false; }
        if ( !FlushViewOfFile(MapViewBuffer, 0) ) {
            printf("Could not flush memory to disk (%d).\n", GetLastError()); 
            return false;
        }
        return true;
    }

    bool MMFile::ResizeFile( size_t inSize ) {
        if ( Access != "w" ) { return false; }
        if ( inSize == FileSize ) { return true; }

        if ( inSize < 1 ) { return NewFile( Path ); }

        ClearView();
        if (FileMap != NULL) { CloseHandle(FileMap); FileMap = NULL; }

        MSFileSize.QuadPart = inSize;
        FileSize = inSize;
        if ( INVALID_SET_FILE_POINTER == SetFilePointer( FileHandle, MSFileSize.LowPart, &MSFileSize.HighPart, FILE_BEGIN ) ) {
            printf("MMFile - SetFilePointer failed with system error code %d\n", GetLastError());
            ClearAll();
            return false;
        }
        if ( !SetEndOfFile( FileHandle ) ) {
            printf("MMFile - SetEndOfFile failed with system error code %d\n", GetLastError());
            ClearAll();
            return false;
        }
        return OpenFile( Path, "w");
    }

#elif defined( __linux__ )

    MMFile::MMFile() :
        Content( nullptr ),
        FileSize( 0 ),
        ViewOffset(),
        ViewSize(),
        Path(),
        Access(),
        Granularity(),
        FileHandle( -1 ),
        FileStat(),
        MapViewOffset(),
        MapViewShift(),
        MapViewSize(),
        MapViewBuffer( NULL ) {
        
        Granularity = (size_t) sysconf( _SC_PAGESIZE );
        
    }

    MMFile::~MMFile() {
        ClearAll();
    }

    MMFile::MMFile( MMFile && o ) :
        Content(o.Content),
        FileSize(std::move(o.FileSize)),
        ViewOffset(std::move(o.ViewOffset)),
        ViewSize(std::move(o.ViewSize)),
        Path(std::move(o.Path)),
        Access(std::move(o.Access)),
        Granularity(std::move(o.Granularity)),
        FileHandle(std::move(o.FileHandle)),
        FileStat(std::move(o.FileStat)),
        MapViewOffset(std::move(o.MapViewOffset)),
        MapViewShift(std::move(o.MapViewShift)),
        MapViewSize(std::move(o.MapViewSize)),
        MapViewBuffer(o.MapViewBuffer) {
            o.Content = nullptr;
            o.FileSize = 0;
            o.FileHandle = -1;
            o.MapViewBuffer = NULL;
    }

    MMFile & MMFile::operator=( MMFile && o ) {
        ClearAll();
        Content = o.Content;
        FileSize = std::move(o.FileSize);
        ViewOffset = std::move(o.ViewOffset);
        ViewSize = std::move(o.ViewSize);
        Path = std::move(o.Path);
        Access = std::move(o.Access);
        Granularity = std::move(o.Granularity);
        FileHandle = std::move(o.FileHandle);
        FileStat = std::move(o.FileStat);
        MapViewOffset = std::move(o.MapViewOffset);
        MapViewShift = std::move(o.MapViewShift);
        MapViewSize = std::move(o.MapViewSize);
        MapViewBuffer = o.MapViewBuffer;
            o.Content = nullptr;
            o.FileSize = 0;
            o.FileHandle = -1;
            o.MapViewBuffer = NULL;
        return *this;
    }

    void MMFile::ClearAll() {
        ClearView();
        if ( FileHandle != -1 ) {
            close(FileHandle);
            FileHandle = -1; FileStat = {}; FileSize = 0; Path = ""; Access="";
        }
    }

    void MMFile::ClearView() {
        if (MapViewBuffer != NULL){
            munmap( MapViewBuffer, MapViewSize );
            MapViewBuffer = NULL;
            Content = nullptr;
            ViewOffset = 0; ViewSize = 0;
            MapViewOffset = 0; MapViewShift = 0; MapViewSize = 0;
        }
    }

    bool MMFile::OpenFile( std::string inPath, std::string inAccess ) {
        return OpenFile( inPath.c_str(), inAccess );
    }

    bool MMFile::OpenFile( const char* inPath, std::string inAccess ) {
        // Opens an existing non-empty file for read or write access
        if ( !IsValidAccessType( inAccess ) ) { return false; }
        ClearAll();
        Access = inAccess;

        int flags = ( Access == "w" ) ? O_RDWR : O_RDONLY;
        FileHandle = open64(inPath, flags);
        if ( FileHandle == -1) {
            perror("MMFile - CreateFile failed with error");
            ClearAll();
            return false;
        }
        Path = inPath;

        if ( fstat64( FileHandle, &FileStat ) == -1 ) {
            perror("MMFile - Getting FileStat failed with error");
            ClearAll();
            return false;
        }
        
        if ( 0 == FileStat.st_size ){
            printf("MMFile - File is empty: %s\n", Path.c_str() );
            ClearAll();
            return false;
        }
        FileSize = FileStat.st_size;

        if ( !SetView() ) { return false; }

        CHECK_FOR_BOM

        return true;
    }

    bool MMFile::NewFile( std::string inPath, size_t inSize ) {
        return NewFile( inPath.c_str(), inSize );
    }

    bool MMFile::NewFile( const char* inPath, size_t inSize ) {
        // Creates a new file with write access. This will overwrite any existing file.
        // If no size is specified, then this generates a 1 byte file filled with null values.
        ClearAll();
        Access = "w";

        mode_t modes = S_IRWXU | S_IRWXG | S_IRWXO;       // read/write/execute permissions for user, group, and other
        FileHandle = open64(inPath, O_RDWR | O_CREAT | O_TRUNC, modes);
        if ( FileHandle == -1) {
            perror("MMFile - CreateFile failed with error");
            ClearAll();
            return false;
        }
        Path = inPath;
        if ( inSize < 1 ) { inSize = 1; }
        return ResizeFile( inSize );
    }

    bool MMFile::SetView() {
        return SetView(0, FileSize);
    }

    bool MMFile::SetView( size_t inOffset, size_t inSize ) {
        // Sets the Content buffer view to the requested offset and size.
        // The view is truncated to the FileSize.
        if ( inOffset == ViewOffset && inSize == ViewSize ) { return true; }    // Requested offset and size is the same as the current view
        ClearView();
        ViewOffset = inOffset;
        ViewSize = ( inOffset + inSize > FileSize ) ? (FileSize - inOffset) : inSize;

        // Align offset with system granularity
        MapViewOffset = Tools::AlignDown(inOffset, Granularity);
        MapViewShift = inOffset - MapViewOffset;
        MapViewSize = inSize + MapViewShift;

        // Read or Write access
        int map_access;
        if ( Access == "r" ) {
            map_access = PROT_READ;
        } else if ( Access == "w" ) {
            map_access = PROT_WRITE;
        }

        MapViewBuffer = mmap64( NULL, MapViewSize, map_access, MAP_SHARED, FileHandle, MapViewOffset );
        if ( MapViewBuffer == MAP_FAILED ) {
            perror("MMFile - mmap failed with error");
            return false;
        }
        Content = &((unsigned char *)MapViewBuffer)[MapViewShift];

        return true;
    }

    bool MMFile::WriteToDisk( const unsigned char * inData, size_t inOffset, size_t inSize ) {
        // Writes the contents of inData to the current open file. If the requested offset and
        // size exceed the current file size, then the file is expanded to fit the data.
        if ( Access != "w" ) { return false; }
        if ( !ResizeFile( inOffset + inSize ) ) { return false; }
        memcpy( &Content[inOffset], inData, ViewSize );
        return FlushView();
    }

    bool MMFile::FlushView() {
        if ( Access != "w" ) { return false; }
        if ( msync( MapViewBuffer, MapViewSize, MS_SYNC) == -1 ) {
            perror("Could not flush memory to disk");
            return false;
        }
        return true;
    }

    bool MMFile::ResizeFile( size_t inSize ) {
        if ( Access != "w" ) { return false; }
        if ( inSize == FileSize ) { return true; }

        if ( inSize < 1 ) { return NewFile( Path ); }
        
        if( ftruncate64( FileHandle, (off_t) inSize ) == -1 ) {
            perror("MMFile - ftruncate failed with error");
            return false;
        }
        FileSize = inSize;

        if ( MapViewBuffer == NULL ) { return SetView(); }

        size_t oldsize = MapViewSize;
        MapViewSize = inSize + MapViewShift;

        MapViewBuffer = mremap( MapViewBuffer, oldsize, MapViewSize, MREMAP_MAYMOVE );
        if ( MapViewBuffer == MAP_FAILED ) {
            perror("MMFile - mremap failed with error");
            return false;
        }
        Content = &((unsigned char *)MapViewBuffer)[MapViewShift];

        printf( "Resized file: %s, new size: %i", Path.c_str(), (int) FileSize );
        return true;

    }

#endif

    bool MMFile::ChangeAccess( std::string inAccess ) {
        if ( inAccess == Access ) { return true; }
        return OpenFile( Path, inAccess );
    }

    // ============================================================================================ //
    //                                        C-STYLE FILE                                          //
    // ============================================================================================ //

    BasicFile::BasicFile() :
        Content( nullptr ),
        FileSize(),
        ViewOffset(),
        ViewSize(),
        FileHandle( NULL ),
        Path() {
    }

    BasicFile::~BasicFile() {
        ClearAll();
    }

    BasicFile::BasicFile( BasicFile && o ) :
        Content(o.Content),
        FileSize(std::move(FileSize)),
        ViewOffset(std::move(ViewOffset)),
        ViewSize(std::move(ViewSize)),
        FileHandle(o.FileHandle),
        Path(std::move(Path)),
        ReadViewOffset(std::move(ReadViewOffset)),
        ReadViewSize(std::move(ReadViewSize)) {
            o.Content = nullptr;
            o.FileHandle = NULL;
    }

    BasicFile & BasicFile::operator=( BasicFile && o ) {
        ClearAll();
        Content = o.Content;
        FileSize = std::move(FileSize);
        ViewOffset = std::move(ViewOffset);
        ViewSize = std::move(ViewSize);
        FileHandle = o.FileHandle;
        Path = std::move(Path);
        ReadViewOffset = std::move(ReadViewOffset);
        ReadViewSize = std::move(ReadViewSize);
            o.Content = nullptr;
            o.FileHandle = NULL;
        return *this;
    }


    void BasicFile::ClearAll() {
        if (FileHandle != NULL) { fclose( FileHandle ); FileHandle = NULL; FileSize = 0; Path = ""; }
        ClearView();
    }

    void BasicFile::ClearView() {
        if( Content != nullptr ) { 
            free(Content); 
            Content = nullptr;
            ViewOffset = 0; ViewSize = 0;
        }
    }

    bool BasicFile::OpenFile( std::string inPath, std::string inAccess ) {
        return OpenFile( inPath.c_str(), inAccess );
    }

    bool BasicFile::OpenFile( const char* inPath, std::string inAccess ) {
        // Opens an existing file for read or write access
        if ( !IsValidAccessType( inAccess ) ) { return false; }
        ClearAll();
        Access = inAccess;

        const char * access_mode = ( Access == "r" ) ? "rb" : "rb+";
        FileHandle = fopen (inPath, access_mode);
        if (FileHandle == NULL) { 
            printf("BasicFile - Error opening file %s with error no: %d\n", inPath, errno );
            return false;
        }
        Path = inPath;

        fseek (FileHandle , 0 , SEEK_END);
        FileSize = ftell (FileHandle );
        fseek (FileHandle, 0, SEEK_SET);    // SEEK_SET sets the beginning of the cursor at 0

        if ( !SetView() ) { return false; }

        CHECK_FOR_BOM

        return true;
    }

    bool BasicFile::SetView() {
        return SetView(0, FileSize);
    }

    bool BasicFile::SetView( size_t inOffset, size_t inSize ) {
        // Sets the Content buffer view to the requested offset and size.
        // The view is truncated to the FileSize.
        if ( inOffset == ViewOffset && inSize == ViewSize ) { return true; }    // Requested offset and size is the same as the current view
        ClearView();
        ViewOffset = inOffset;
        ViewSize = ( inOffset + inSize > FileSize ) ? (FileSize - inOffset) : inSize;
        Content = static_cast<unsigned char*>( malloc( ViewSize ) );
        fseek (FileHandle, (long) ViewOffset, SEEK_SET);
        size_t read_size = fread (Content, sizeof(unsigned char), ViewSize, FileHandle);
        if ( ViewSize != read_size) {
            printf("BasicFile - Error reading file, read size: %u\n", (unsigned int) read_size);
            ClearView();
            return false;
        }
        return true;
    }

    bool BasicFile::NewFile( std::string inPath, size_t inSize ) {
        return NewFile( inPath.c_str(), inSize );
    }

    bool BasicFile::NewFile( const char* inPath, size_t inSize ) {
        // Creates a new file with write access. This will overwrite any existing file.
        ClearAll();
        Access = "w";
        FileHandle = fopen (inPath,"wb+");
        if (FileHandle == NULL) { 
            printf("BasicFile - Error creating new file %s\n", inPath );
            return false;
        }
        Path = inPath;
        if ( inSize < 1 ) { inSize = 1; }
        FileSize = inSize;

        ViewOffset = 0;
        ViewSize = FileSize;
        Content = static_cast<unsigned char*>( malloc( ViewSize ) );

        return true;
    }

    bool BasicFile::WriteToDisk( const unsigned char * inData, size_t inOffset, size_t inSize ) {
        // Writes the contents of inData to the current open file and updates the FileSize.
        if ( Access != "w" ) { return false; }
        fseek (FileHandle, (long) inOffset, SEEK_SET);
        size_t outSize = fwrite( inData, sizeof(char), inSize, FileHandle );
        if (outSize != inSize ) {
            printf("BasicFile - Error writing file, characters written: %f\n", (float) outSize);
            ClearAll();
            return false;
        }
        if ( inOffset + inSize > FileSize ) { FileSize = inOffset + inSize; }
        return true;
    }

    bool BasicFile::FlushView() {
        if ( Access != "w" ) { return false; }
        fseek (FileHandle, (long) 0, SEEK_SET);
        size_t outSize = fwrite( Content, ViewSize, 1, FileHandle );
        if ( outSize != 1 ) {
            printf("BasicFile - Error writing file, characters written: %f\n", (float) outSize);
            printf("    outSize: %i, ViewSize: %i \n", (int) outSize, (int) ViewSize );
            if ( feof( FileHandle ) ) { printf("    Hit EOF condition.\n"); } 
            else { perror("    error:"); }
            ClearAll();
            return false;
        }
        return true;
    }

    bool BasicFile::ResizeFile( size_t inSize ) {
        if ( Access != "w" ) { return false; }
        if ( inSize == FileSize ) { return true; }

        if ( inSize < 1 ) { return NewFile( Path, 1 ); }

        if (FileHandle != NULL) { fclose( FileHandle ); }
        FileSize = inSize;
        FileHandle = fopen (Path.c_str(),"wb+");    // This will overwrite any existing file.
        if (FileHandle == NULL) { 
            printf("BasicFile - Error creating new file %s\n", Path.c_str() );
            return false;
        }
        ViewOffset = 0;
        ViewSize = FileSize;
        Content = static_cast<unsigned char*>( realloc( Content, ViewSize ) );
        FlushView();
        return true;
    }

    bool BasicFile::ChangeAccess( std::string inAccess ) {
        if ( inAccess == Access ) { return true; }
        return OpenFile( Path.c_str(), inAccess );
    }

    // ============================================================================================ //
    //                                          FILE ENGINE                                         //
    // ============================================================================================ //

    FileEngine::FileEngine( bool inUseMMF ) :
        useMMF( inUseMMF ),
        RF(),
        MMF(),
        Content( nullptr ) {
    }
    
    FileEngine::~FileEngine() {
        MMF.ClearAll();
        RF.ClearAll();
    }

    FileEngine::FileEngine( FileEngine && o ) :
        Content(o.Content),
        useMMF(std::move(o.useMMF)),
        RF(std::move(o.RF)),
        MMF(std::move(o.MMF)) {
            o.Content = nullptr;
    }

    FileEngine & FileEngine::operator=( FileEngine && o ) {
        Content = o.Content;
        useMMF = std::move(o.useMMF);
        RF = std::move(o.RF);
        MMF = std::move(o.MMF);
            o.Content = nullptr;
        return *this;
    }

    void FileEngine::ClearAll() {
        (useMMF) ? MMF.ClearAll() : RF.ClearAll();
    }

    void FileEngine::ClearView() {
        (useMMF) ? MMF.ClearView() : RF.ClearView();
    }

    bool FileEngine::OpenFile( std::string inPath, std::string inAccess ) {
        return OpenFile( inPath.c_str(), inAccess );
    }

    bool FileEngine::OpenFile( const char* inPath, std::string inAccess ) {
        bool res = (useMMF) ? MMF.OpenFile(inPath, inAccess) : RF.OpenFile(inPath, inAccess);
        Content = (useMMF) ? MMF.Content : RF.Content;
        return res;
    }


    bool FileEngine::NewFile( std::string inPath, size_t inSize) {
        return NewFile( inPath.c_str(), inSize );
    }

    bool FileEngine::NewFile( const char* inPath, size_t inSize) {
        bool res = (useMMF) ? MMF.NewFile( inPath, inSize ) : RF.NewFile( inPath, inSize );
        Content = (useMMF) ? MMF.Content : RF.Content;
        return res;
    }

    bool FileEngine::SetView() {
        bool res = (useMMF) ? MMF.SetView() : RF.SetView();
        Content = (useMMF) ? MMF.Content : RF.Content;
        return res;
    }

    bool FileEngine::SetView( size_t inOffset, size_t inSize ) {
        bool res = (useMMF) ? MMF.SetView(inOffset, inSize) : RF.SetView(inOffset, inSize);\
        Content = (useMMF) ? MMF.Content : RF.Content;
        return res;
    }

    bool FileEngine::WriteToDisk( const unsigned char * inData, size_t inOffset, size_t inSize ) {
        return (useMMF) ? MMF.WriteToDisk( inData, inOffset, inSize ) : RF.WriteToDisk( inData, inOffset, inSize );
    }

    bool FileEngine::FlushView() {
        return ( useMMF ) ? MMF.FlushView() : RF.FlushView();
    }

    bool FileEngine::ResizeFile( size_t inSize ) {
        bool res = ( useMMF ) ? MMF.ResizeFile( inSize ) : RF.ResizeFile( inSize );
        Content = (useMMF) ? MMF.Content : RF.Content;
        return res;
    }

    bool FileEngine::ChangeAccess( std::string inAccess ) {
        bool res = ( useMMF ) ? MMF.ChangeAccess( inAccess ) : RF.ChangeAccess( inAccess );
        Content = (useMMF) ? MMF.Content : RF.Content;
        return res;
    }

    void FileEngine::SetEngine( bool inUseMMF ) {
        useMMF = inUseMMF;
    }

} // namespace AppCore

#endif // CH_TOOLS_CPP