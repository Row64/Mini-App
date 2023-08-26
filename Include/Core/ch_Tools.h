#ifndef CH_TOOLS_H
#define CH_TOOLS_H

#if defined( _WIN32 )
    #include <windows.h>
#elif defined(  __linux__  )
    #include <unistd.h>
    #include <errno.h>
    #include <fcntl.h>
#endif

#include <string>
#include <vector>

namespace AppCore {

    // ============================================================================================ //
    //                                       CHAR BUFFER                                            //
    //                                                                                              //
    // Struct to handle and free malloced char* buffers                                             //
    // ============================================================================================ //

    struct CharBuffer {

        CharBuffer();
        CharBuffer( char* inBuffer, size_t inSize = 0 );
        CharBuffer( unsigned char* inBuffer, size_t inSize = 0 );
        CharBuffer( size_t inSize );
        CharBuffer( size_t inCount, size_t inSize );
        ~CharBuffer();

        // no copy constructor and assignment
        CharBuffer(const CharBuffer&) = delete;
        CharBuffer &operator=(const CharBuffer&) = delete;

        // move constructor and assignment
        CharBuffer( CharBuffer && other );
        CharBuffer & operator=( CharBuffer && other );

        // comparison operators
        bool operator== (const CharBuffer& o) const;
        bool operator!= (const CharBuffer& o) const;

        char*           Content;
        size_t          Size;       // Bytes

        void Clear();
        void Resize( size_t inSize );
        void Grow( size_t inSize );

    };

    // ============================================================================================ //
    //                                             TOOLS                                            //
    // ============================================================================================ //

    struct Tools {
        
        static std::vector<char> GetBinaryFileContents( std::string const &filename );
        static void OpenWebLink( const std::string & inLink );

        


        // Power of 2 alignment functions
        template< typename T > static T AlignUp( T inSize, T inAlign ) { return (inSize + inAlign - 1) & ~(inAlign - 1); };
        template< typename T > static T AlignDown( T inSize, T inAlign ) { return (inSize) & ~(inAlign - 1); };
        template< typename T > static T DivUp( T x, T y ) { return (x + y - 1u) / y; };

        // Timestamp functions
        static std::string GetStampYMDHMS();

        // Splitting functions
        static std::vector<uint64_t> SplitRange( uint64_t inStart, uint64_t inRange, uint32_t inDiv );           // Returns the split range boundaries ( i.e., SplitRange(0, 10, 2) --> { 0, 5, 10 } )
        static std::vector<uint64_t> SplitRange( uint64_t inStart, uint64_t inRange, uint32_t maxNbDiv, uint64_t prefDivSize );

        // Collating sequence tables for sorting
        static const unsigned char ascii_map_no_case[256];    // ASCII Map -- CASE INSENSITIVE ( A and a are the same )
        static const unsigned char ascii_map_with_case[256];  // ASCII Map -- CASE SENSITIVE ( AaBb )
    };

    // ============================================================================================ //
    //                                     MEMORY MAPPED FILE                                       //
    // ============================================================================================ //

    class MMFile {

    public:

        MMFile();
        ~MMFile();

        // no copy constructor and assignment
        MMFile(const MMFile&) = delete;
        MMFile &operator=(const MMFile&) = delete;

        // move constructor and assignment
        MMFile( MMFile && o );
        MMFile & operator=( MMFile && o );

        unsigned char*      Content;
        size_t              FileSize;
        size_t              ViewOffset;      // User requested view offset
        size_t              ViewSize;        // User requested view size in bytes
        std::string         Path;
        std::string         Access;

        void                ClearAll();
        void                ClearView();
        bool                OpenFile( std::string inPath, std::string inAccess = "r" );     // "r" = read, "w" = write
        bool                OpenFile( const char* inPath, std::string inAccess = "r" );
        bool                NewFile( std::string inPath, size_t inSize = 1);
        bool                NewFile( const char* inPath, size_t inSize = 1);
        bool                SetView();
        bool                SetView( size_t inOffset, size_t inSize );
        bool                WriteToDisk( const unsigned char * inData, size_t inOffset, size_t inSize );
        bool                FlushView();
        bool                ResizeFile( size_t inSize );
        bool                ChangeAccess( std::string inAccess );
        
    private:

    #if defined( _WIN32 )

        DWORD               Granularity;
        HANDLE              FileHandle;
        HANDLE              FileMap;
        LPVOID              MapView;
        LARGE_INTEGER       MSFileSize;
        LARGE_INTEGER       MapViewOffset;      // Starting byte of mapped region
        SIZE_T              MapViewSize;

    #elif defined( __linux__ )

        size_t              Granularity;
        int                 FileHandle;
        struct stat64       FileStat;
        size_t              MapViewOffset;
        size_t              MapViewShift;
        size_t              MapViewSize;

    #endif

        void*      MapViewBuffer;
        
    };

    // ============================================================================================ //
    //                                     BASIC C-STYLE FILE                                       //
    // ============================================================================================ //

    class BasicFile {

    public:

        BasicFile();
        ~BasicFile();

        // no copy constructor and assignment
        BasicFile(const BasicFile&) = delete;
        BasicFile &operator=(const BasicFile&) = delete;

        // move constructor and assignment
        BasicFile( BasicFile && o );
        BasicFile & operator=( BasicFile && o );

        unsigned char*      Content;
        size_t              FileSize;
        size_t              ViewOffset;      // User requested view offset
        size_t              ViewSize;        // User requested view size in bytes
        std::string         Path;
        std::string         Access;

        void                ClearAll();
        void                ClearView();
        bool                OpenFile( std::string inPath, std::string inAccess = "r" );     // "r" = read, "w" = write
        bool                OpenFile( const char* inPath, std::string inAccess = "r" );
        bool                NewFile( std::string inPath, size_t inSize );
        bool                NewFile( const char* inPath, size_t inSize );
        bool                SetView();
        bool                SetView( size_t inOffset, size_t inSize );
        bool                WriteToDisk( const unsigned char * inData, size_t inOffset, size_t inSize );
        bool                FlushView();
        bool                ResizeFile( size_t inSize );
        bool                ChangeAccess( std::string inAccess );

    private:

        FILE                *FileHandle;
        size_t              ReadViewOffset;
        size_t              ReadViewSize;

    };

    // ============================================================================================ //
    //                                          FILE ENGINE                                         //
    // ============================================================================================ //

    class FileEngine {
    
    public: 

        FileEngine( bool inUseMMF=false );
        ~FileEngine();

        // no copy constructor and assignment
        FileEngine(const FileEngine&) = delete;
        FileEngine &operator=(const FileEngine&) = delete;

        // move constructor and assignment
        FileEngine( FileEngine && o );
        FileEngine & operator=( FileEngine && o );

        unsigned char*      Content;
        size_t              FileSize() { return (useMMF) ? MMF.FileSize : RF.FileSize; };
        size_t              ViewOffset() { return (useMMF) ? MMF.ViewOffset : RF.ViewOffset; };
        size_t              ViewSize() { return (useMMF) ? MMF.ViewSize : RF.ViewSize; };
        std::string         Path() { return (useMMF) ? MMF.Path : RF.Path; };

        void                ClearAll();
        void                ClearView();
        bool                OpenFile( std::string inPath, std::string inAccess = "r" );
        bool                OpenFile( const char* inPath, std::string inAccess = "r" );
        bool                NewFile( std::string inPath, size_t inSize = 1);
        bool                NewFile( const char* inPath, size_t inSize = 1);
        bool                SetView();
        bool                SetView( size_t inOffset, size_t inSize );
        bool                WriteToDisk( const unsigned char * inData, size_t inOffset, size_t inSize );
        bool                FlushView();
        bool                ResizeFile( size_t inSize );
        bool                ChangeAccess( std::string inAccess );

        void                SetEngine( bool inUseMMF );

    private:

        bool                useMMF;
        BasicFile           RF;       // regular file using C-style fopen
        MMFile              MMF;      // memory mapped file (currently windows only)

    };

} // namespace AppCore


#endif // CH_TOOLS_H