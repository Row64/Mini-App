#ifndef BLOCK_NFD_H
#define BLOCK_NFD_H

#include <string>

std::string DrawOpenFileDialog( const std::string & extensions = "", const std::string & defaultPath = "" );
std::string DrawSaveFileDialog( const std::string & extension = "", const std::string & defaultPath = "" );

#endif /* BLOCK_NFD_H */
