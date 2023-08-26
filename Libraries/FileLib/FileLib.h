#ifndef FILELIB_H
#define FILELIB_H


#include <iostream>

namespace FLib {

    // --------------------------------------------------------------------- //
    // Declarations                                                          //
    // --------------------------------------------------------------------- //

    // Statistical Functions ----------------------------------------------- //

    // template<typename T> void MLinearReg( const std::vector<T> & yData, const std::vector<T> & xData, double & b0, double & b1 );

    void PreparePath(std::string &s);

    void FLOpenPythonLink( const std::string & inLink );
    void FLOpenTextLink( const std::string & inLink );
    void FLOpenWebLink( const std::string & inLink );
    void FLOpenPythonTerminal( const std::string & inPyPath, const std::string & inScriptsPath);

}



#endif /* FILELIB_H */
