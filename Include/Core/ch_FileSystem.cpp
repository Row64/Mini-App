#ifndef CH_FILESYSTEM_CPP
#define CH_FILESYSTEM_CPP

#include "ch_FileSystem.h"

#include "miniz.h"

bool FSMoveFile( const fs::path& oldpath, const fs::path& newpath ) {
    // Moves the file from oldpath to newpath. If newpath is an existing file, this will replace it.
    // Returns true if the file is sucessfully moved.
    if ( fs::is_regular_file( oldpath ) ) {
        try {
            fs::rename( oldpath, newpath );
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return false;
}

bool FSCopyFile( const fs::path& src, fs::path& target, FSCopyTypes copy_type ) {
    // Copies the file from src to target. If target is an existing file, it will either
    // overwrite, increment, or do nothing depending on the copy_type.
    // Returns true if the file is sucessfully copied.
    if ( fs::is_regular_file( src ) ) {
        try {
            if ( fs::exists(target) ) {
                if ( copy_type == fs_copy_overwrite ) {         // OVERWRITE - removes the existing file and copies src to target
                    fs::remove(target);
                    fs::copy(src, target);
                    return true;
                }
                else if ( copy_type == fs_copy_increment ) {    // INCREMENT - attaches a suffix "_#" to the file name and updates the target path
                    uint32_t suffix = 1;
                    fs::path parent = target.parent_path();
                    std::string name = target.stem().string();
                    std::string ext = target.extension().string();
                    while ( 1 ) {
                        if ( suffix == UINT32_MAX ) {   // Can't increment anymore, so exit false
                            target = parent / (name + ext );
                            return false;
                        }
                        target = parent / (name + "_" + std::to_string(suffix) + ext );
                        if ( !fs::exists(target) ) {    // Got to a name that is available, so copy & exit true
                            fs::copy(src, target);
                            return true;
                        }
                        suffix++;                       // Otherwise, keep incrementing the suffix
                    }
                }
                else {                                          // DEFAULT - do nothing if there is an existing file with the same name
                    return false;
                }
            }
            fs::path tparent = target.parent_path();
            if ( !fs::exists(tparent) ) { fs::create_directories(tparent); }  // Make the target parent directory if it doesn't exists
            fs::copy(src, target);
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return false;
}

bool FSMoveDirectory( const fs::path& oldpath, const fs::path& newpath ) {
    // Moves the directory from oldpath to newpath. If newpath is an existing directory, this will replace it.
    // Returns true if the directory is sucessfully moved.
    if ( fs::is_directory( oldpath ) ) {
        try {
            if ( fs::exists( newpath ) ) { FSRemoveDirectory( newpath ); }
            fs::rename( oldpath, newpath );
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return false;
}

bool FSCopyDirectory( const fs::path& src, const fs::path& target ) {
    // Copies the directory from src to target. Existing files are overwritten.
    // Returns true if the directory is sucessfully copied.
    if ( fs::is_directory(src) ) {
        try {
            if ( !fs::exists(target) ) { fs::create_directories(target); }
            for ( const auto& dirEntry : fs::recursive_directory_iterator(src) ) {
                const auto& p = dirEntry.path();
                const auto targetPath = target / fs::relative(p, src);
                if ( fs::is_directory(p) ) {            // Create directories in target, if not existing.
                    if ( !fs::exists(targetPath) ) { fs::create_directories(targetPath); }
                }
                else if ( fs::is_regular_file(p) ) {    // Copy files to the target
                    if ( fs::exists(targetPath) ) { fs::remove(targetPath); }
                    fs::copy(p, targetPath);
                }

            }
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return false;
}

bool FSRemoveDirectory( const fs::path& path ) {
    // Removes the directory and its contents, including subdirectories.
    // Returns true if directory is successfully removed or does not exist.
    // NOTE: This will remove the directory and its contents as long as permissions are not denied.
    // So, this will remove the directory if it is open in an explorer window. If any of the directory
    // contents are open and write permissions are not denied, then it will also be removed.
    if ( fs::exists( path ) ) {
        try {
            fs::remove_all( path );
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return true;
}

bool FSZipDirectory( const fs::path& src, const fs::path& target, void(*callback)() ) {

    if ( fs::is_directory(src) ) {
        try {

            mz_zip_archive archive;
            if ( !memset(&archive, 0, sizeof(mz_zip_archive)) ) { return false; }
            if ( !mz_zip_writer_init_file( &archive, CONVERT_PATH(target), 0) ) { return false; }
            for ( const auto& dirEntry : fs::recursive_directory_iterator(src) ) {
                const auto& p = dirEntry.path();
                const auto relPath = fs::relative(p, src);
                if ( fs::is_directory(p) ) { 
                    std::string rp = relPath.string() + "/";    // directories have to end with a slash, otherwise zip library tries to make a file
                    if ( !mz_zip_writer_add_mem( &archive, rp.c_str(), NULL, 0, MZ_DEFAULT_COMPRESSION ) ) {
                        printf( "Zip Error: Unable to add directory: %s\n", CONVERT_PATH(p) );
                        continue;
                    }

                }
                else if ( fs::is_regular_file(p) ) {
                    if ( !mz_zip_writer_add_file( &archive, CONVERT_PATH( relPath ), CONVERT_PATH(p), "", 0, MZ_DEFAULT_COMPRESSION ) ) {
                        printf( "Zip Error: Unable to add file: %s\n", CONVERT_PATH(p) );
                        continue;
                    }
                }
                callback();
            }
            if ( !mz_zip_writer_finalize_archive(&archive) ) { return false; }
            if ( !mz_zip_writer_end(&archive) ) { return false; }
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }

    return true;
}

bool FSUnzipDirectory( const fs::path& src, float & perc, std::string &unzipName) {

    if ( fs::is_regular_file(src) ) {

        // target is a source name without extension
        std::string name = src.stem().string();
        fs::path parent = src.parent_path();
        fs::path target = parent / name;
        
        uint32_t suffix = 1;
        std::string ext = target.extension().string();

        struct ZDataStruct {
            int Count = 0;
            int NbItems = 0;
            float & Perc;
            ZDataStruct( float & inP ) : Perc( inP ) {};
        };

        ZDataStruct zdata( perc );

        auto extract_func = []( const char *filename, void *arg ) {
            ZDataStruct * zd = reinterpret_cast<ZDataStruct*>(arg);
            zd->Count++;
            zd->Perc = (float) zd->Count / (float) zd->NbItems;
            return 0;
        };

        while ( 1 ) {
            if( !(fs::exists(target) || fs::is_directory(target)) ){
                // Got to a name that is available, so unpack & exit true
                unzipName = target.string();
                if ( !fs::create_directory(target) ) {
                    printf( "Unzip Error: Could not make directory: %s\n", CONVERT_PATH(target) );
                    return false;
                }
                mz_zip_archive archive;
                if ( !memset(&archive, 0, sizeof(mz_zip_archive)) ) { return false; }
                mz_zip_archive_file_stat info;
                if ( !memset(&info, 0, sizeof(mz_zip_archive_file_stat)) ) { return false; }
                if ( !mz_zip_reader_init_file( &archive, src.string().c_str(), 0) ) { return false; }
                zdata.NbItems = mz_zip_reader_get_num_files( &archive );
                for ( int i = 0; i < zdata.NbItems; i++ ) {
                    if (!mz_zip_reader_file_stat(&archive, i, &info)) {
                        printf( "Unzip Error: Could not get info about file archive.\n" );
                        return false;
                    }
                    fs::path currPath = (target / info.m_filename).make_preferred();
                    if ( info.m_is_directory ) {
                        if ( !fs::create_directory(currPath) ) {
                            printf( "Unzip Error: Could not make directory: %s\n", CONVERT_PATH(currPath) );
                            return false;
                        }
                    }
                    else {
                        if ( !mz_zip_reader_extract_to_file( &archive, i, CONVERT_PATH(currPath), 0) ) {
                            printf( "Unzip Error: Could not extract to file: %s\n", CONVERT_PATH(currPath) );
                            return false;
                        }
                    }
                    if ( extract_func( CONVERT_PATH(currPath), &zdata ) < 0 ) {
                        printf( "Unzip Error: extract_func\n" );
                        return false;
                    }
                }
                return true;
            }
            if ( suffix == UINT32_MAX ) {   // Can't increment anymore, so exit false
                target = parent / (name + ext );
                return false;
            }
            target = parent / (name + "_" + std::to_string(suffix) + ext );
            suffix++;      // Otherwise, keep incrementing the suffix
        }
    }
    return true;
}

std::vector<fs::path> FSListFilesWithName( const fs::path& inDirectory, const fs::path& inName ) {
// Returns a vector of paths for all files in a directory with a specific name, but different extensions.
    
    if ( fs::is_directory(inDirectory) ) {
        try {
            std::vector<fs::path> outPaths = {};
            for ( const auto& dirEntry : fs::directory_iterator(inDirectory) ) {
                const auto& p = dirEntry.path();
                // printf( "p: %s\n", CONVERT_PATH(p) );
                if ( fs::is_regular_file(p) ) {
                    if ( p.stem() == inName ) {
                        outPaths.push_back(p);
                    }
                }
            }
            return outPaths;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return {};
        }
    }
    return {};
}

std::vector<fs::path> FSListFilesStartingWith( const fs::path& inDirectory, const std::string& inName ) {
    if ( fs::is_directory(inDirectory) ) {
        try {
            std::vector<fs::path> outPaths = {};
            for ( const auto& dirEntry : fs::directory_iterator(inDirectory) ) {
                const auto& p = dirEntry.path();
                // printf( "p: %s\n", CONVERT_PATH(p) );
                if ( fs::is_regular_file(p) ) {
                    std::string stem = p.stem().string();
                    if ( stem.find(inName) == 0 ) {
                        outPaths.push_back(p);
                    }
                }
            }
            return outPaths;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return {};
        }
    }
    return {};
}

std::vector<std::string> FSListDirectoryFolders( const fs::path& inDirectory ) {
    if ( fs::is_directory(inDirectory) ) {
        try {
            std::vector<std::string> out = {};
            for ( const auto& dirEntry : fs::directory_iterator(inDirectory) ) {
                const auto& p = dirEntry.path();
                // printf( "p: %s\n", CONVERT_PATH(p) );
                if ( fs::is_directory(p) ) {
                    out.push_back( p.stem().string() );
                }
            }
            return out;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return {};
        }
    }
    return {};
}

bool FSRemoveFilesStartingWith( const fs::path& inDirectory, const std::string& inName ) {
    if ( fs::is_directory(inDirectory) ) {
        try {
            for ( const auto& dirEntry : fs::directory_iterator(inDirectory) ) {
                const auto& p = dirEntry.path();
                // printf( "p: %s\n", CONVERT_PATH(p) );
                if ( fs::is_regular_file(p) ) {
                    std::string stem = p.stem().string();
                    if ( stem.find(inName) == 0 ) {
                        fs::remove( p );
                    }
                }
            }
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return false;
}

bool FSRenameFilesStartingWith( const fs::path& inDirectory, const std::string& oldName, const std::string& newName ) {
    if ( fs::is_directory(inDirectory) ) {
        try {
            for ( const auto& dirEntry : fs::directory_iterator(inDirectory) ) {
                const auto& p = dirEntry.path();
                // printf( "p: %s\n", CONVERT_PATH(p) );
                if ( fs::is_regular_file(p) ) {
                    std::string fname = p.filename().string();
                    if ( fname.find(oldName) == 0 ) {
                        fname.erase( 0, oldName.size() );
                        fname = newName + fname;
                        fs::path newPath = p.parent_path() / fname;
                        if ( !FSMoveFile( p, newPath ) ) {
                            printf( "Unable to rename file: %s to %s\n", CONVERT_PATH(p), CONVERT_PATH(newPath) );
                        }
                    }
                }
            }
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return false;
}

bool FSRemoveFilesByFunc( const fs::path& inDirectory, std::function<bool(const std::string&,const std::string&)> Func ) {
    if ( fs::is_directory(inDirectory) ) {
        try {
            for ( const auto& dirEntry : fs::directory_iterator(inDirectory) ) {
                const auto& p = dirEntry.path();
                // printf( "p: %s\n", CONVERT_PATH(p) );
                if ( fs::is_regular_file(p) ) {
                    if ( Func( p.stem().string(), p.extension().string() ) ) {
                        fs::remove( p );
                        printf( "removed: %s\n", CONVERT_PATH(p) );
                    }
                }
            }
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return false;
}

bool FSRenameFilesByFunc( const fs::path& inDirectory, std::function<std::string(const std::string&,const std::string&)> Func ) {
    // Takes in a function of the form:   Func( "OldName" , ".ext" ) --> "NewName.ext"

    if ( fs::is_directory(inDirectory) ) {
        try {
            for ( const auto& dirEntry : fs::directory_iterator(inDirectory) ) {
                const auto& p = dirEntry.path();
                // printf( "p: %s\n", CONVERT_PATH(p) );
                if ( fs::is_regular_file(p) ) {
                    fs::path newPath = p.parent_path() / Func( p.stem().string(), p.extension().string() );
                    if ( p != newPath ) {
                        if ( !FSMoveFile( p, newPath ) ) {
                            printf( "Unable to rename file: %s to %s\n", CONVERT_PATH(p), CONVERT_PATH(newPath) );
                        }
                    }
                }
            }
            return true;
        } 
        catch ( fs::filesystem_error & e ) {
            printf( "FileSystem Error: %s\n", e.what() );
            return false;
        }
    }
    return false;
}

#endif /* CH_FILESYSTEM_CPP */
