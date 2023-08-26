#ifndef CH_FILESYSTEM_H
#define CH_FILESYSTEM_H

#include <functional>

#include <boost/filesystem.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
namespace fs = boost::filesystem;

#ifndef CONVERT_PATH
    #if defined ( __APPLE__)
        #define CONVERT_PATH( pathvar ) pathvar.c_str()
    #elif defined ( _WIN32 )
        #define CONVERT_PATH( pathvar ) pathvar.string().c_str()
    #else
        #define CONVERT_PATH( pathvar ) pathvar.c_str()
    #endif
#endif

enum FSCopyTypes {
    fs_copy_default = 0,
    fs_copy_overwrite,
    fs_copy_increment
};

bool    FSMoveFile( const fs::path& oldpath, const fs::path& newpath );
bool    FSCopyFile( const fs::path& src, fs::path& target, FSCopyTypes copy_type = fs_copy_default );
bool    FSMoveDirectory( const fs::path& oldpath, const fs::path& newpath );
bool    FSCopyDirectory( const fs::path& src, const fs::path& target );
bool    FSRemoveDirectory( const fs::path& path );

bool    FSZipDirectory( const fs::path& src, const fs::path& target, void(*callback)() = {} );
bool    FSUnzipDirectory( const fs::path& src, float & perc, std::string &unzipName);

std::vector<fs::path> FSListFilesWithName( const fs::path& inDirectory, const fs::path& inName );
std::vector<fs::path> FSListFilesStartingWith( const fs::path& inDirectory, const std::string& inName );
std::vector<std::string> FSListDirectoryFolders( const fs::path& inDirectory );

bool    FSRemoveFilesStartingWith( const fs::path& inDirectory, const std::string& inName );
bool    FSRenameFilesStartingWith( const fs::path& inDirectory, const std::string& oldName, const std::string& newName );

bool    FSRemoveFilesByFunc( const fs::path& inDirectory, std::function<bool(const std::string&,const std::string&)> Func );
bool    FSRenameFilesByFunc( const fs::path& inDirectory, std::function<std::string(const std::string&,const std::string&)> Func );

#endif /* CH_FILESYSTEM_H */
