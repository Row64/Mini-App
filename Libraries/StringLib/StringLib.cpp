#ifndef STRINGLIB_CPP
#define STRINGLIB_CPP

#if defined( _WIN32 )
    #include <windows.h>
#endif

#include "StringLib.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>

using namespace std;

namespace SLib {


    // --------------------------------------------------------------------- //
    // Start and End Operations                                              //
    // --------------------------------------------------------------------- //

    bool STR_StartsWith( const std::string & inStr, const std::string & inTerm ) {
        return ( inStr.find(inTerm) == 0 );
    }

    bool STR_EndsWith( const std::string & inStr, const std::string & inTerm ) {
        if ( inStr.length() >= inTerm.length() ) {
            return ( inStr.compare( inStr.length()-inTerm.length(), inTerm.length(), inTerm) == 0 );
        } else {
            return false;
        }
    }

    bool STR_MatchStart( const std::string & inStr, const std::string & inTerm, const std::string & inIgnore ) {
        bool ignore[255] = {false};
        const char * c = inIgnore.c_str();
        while ( *c != 0 ) { ignore[*c] = true; c++; }
        const char * s = inStr.c_str();
        const char * t = inTerm.c_str();
        while ( *s != 0 && *t != 0 ) {

            // crawl ahead to next non-ignored character
            while ( ignore[*s] ) { s++; }
            while ( ignore[*t] ) { t++; }

            if ( *s == 0 ) {                                        // check if we reached end of string
                if ( *t != 0 ) { return false; }                    // exit FALSE if the string ends before the term
            }
            else if ( *t == 0 ) { break; }                          // check if we reached end of term
            else if ( *s != *t ) { return false; }                  // exit FALSE when there is a non-match
            
            s++; t++;
        }
        return true;
    }

    std::string STR_Trim( const std::string & inStr, const std::string & inChars ) {

        bool mask[255] = {false};
        const unsigned char * tc = (unsigned char *) inChars.c_str();
        while ( *tc != 0 ) { mask[*tc] = true; tc++; }

        const unsigned char * c = (unsigned char *) inStr.c_str();
        size_t nChar = inStr.size();
        size_t startI = 0;
        
        while ( *c != 0 ) {                         // Cycle through to find the index of the first included character
            if ( mask[*c] ) { startI++; c++; }
            else { break; }
        }

        if ( startI >= nChar ) { return ""; }       // Exit because the entire string is dropped characters

        size_t endI = nChar-1;
        for ( endI; endI >= 0; endI-- ) {           // Cycle through to find the index of the last included character
            if ( !mask[ inStr[endI] ] ) { break; }
        }

        assert( startI <= endI && "STR_Trim - startI is greater than endI.");

        std::string newText = "";
        newText.assign( &inStr[startI], endI-startI+1 );

        return std::move(newText);
        
    }
    std::string STR_TrimSpaces( const std::string & inStr ) {
        
        bool mask[255] = {false};
        mask[' '] = true;
        mask['\t'] = true;

        const char * c = inStr.c_str();
        size_t nChar = inStr.size();
        size_t startI = 0;
        
        while ( *c != 0 ) {                         // Cycle through to find the index of the first non-space character
            if ( mask[*c] ) { startI++; c++; }
            else { break; }
        }

        if ( startI >= nChar ) { return ""; }       // Exit because the entire string is spaces

        size_t endI = nChar-1;
        for ( endI; endI >= 0; endI-- ) {           // Cycle through to find the index of the last non-space character
            if ( !mask[ inStr[endI] ] ) { break; }
        }

        assert( startI <= endI && "STR_TrimSpaces - startI is greater than endI.");

        std::string newText = "";
        newText.assign( &inStr[startI], endI-startI+1 );
        
        return std::move(newText);

    }

    std::string STR_RemoveChars( const std::string & inStr, const std::string & inChars ) {

        bool mask[255] = {false};
        const unsigned char * tc = (unsigned char *) inChars.c_str();
        while ( *tc != 0 ) { mask[*tc] = true; tc++; }

        const unsigned char * c = (unsigned char *) inStr.c_str();
        size_t nChar = inStr.size();
        size_t startI = 0;

        std::string newText = "";
        newText.reserve(inStr.size());
        
        while ( *c != 0 ) {                     // Cycle through the string
            if ( mask[*c] ) { startI++; c++; }  // Skip all masked chars
            else { newText += *c; c++; }        // Add all other chars to the new string
        }

        return std::move(newText);

    }
    
    std::string STR_RemoveLines(const std::string & inText, const std::string & inRemoveText) {
       // Removes lines that start with the InRemoveText string.  Is case sensitive
       // NOTE: CURRENTLY ONLY WORKS WITH '\n' SEPERATED TEXT
        vector<string> sList = {};std::string newText = "";
        const char * c= inText.c_str();
        bool rmLine = false;string line;int x=0;
        while( *c != 0){
            rmLine = true;
            for(int i=0;i<inRemoveText.size();i++){
                if(*c==0){sList.push_back(line);break;}
                else if(*c=='\n'){line="";rmLine = false;break;}
                if(inRemoveText[i]!=*c){rmLine = false;break;}
                else{line+=*c;c++;}
            }
            while( *c != 0){
                if(*c=='\n'){if(!rmLine){sList.push_back(line);}line="";break;}
                else if(*c==0){if(!rmLine){sList.push_back(line);}line="";break;}
                line+=*c;c++;if(*c==0){break;}
            }
            c++;
        }
        if(line != ""){sList.push_back(line);}
        string str="";string delim="";
        for(string sline:sList){str+=delim+sline;}
        return str;
    }

    // --------------------------------------------------------------------- //
    // String Manipulation Functions                                         //
    // --------------------------------------------------------------------- //

    std::string STR_JoinList( const std::vector<std::string> & arr, std::string delimiter, bool popback){
        if ( arr.empty() ) return "";
        std::string str;
        for (auto i : arr)
            str += i + delimiter;
        if ( popback ) str = str.substr(0, str.size() - delimiter.size());
        return std::move(str);
    }

    std::string STR_JoinList( const std::vector<std::string> & arr, const char delimiter, bool popback) {
        if ( arr.empty() ) return "";
        std::string str;
        for (auto i : arr)
            str += i + delimiter;
        if ( popback ) str.pop_back();
        return std::move(str);
    }

    std::vector<std::string> STR_SplitByDelimiter( const std::string & inStr, const char delimiter ) {
        std::vector<std::string> output = {};
        const char * c = inStr.c_str();
        std::string str = "";
        while ( *c != 0 ) {
            if ( *c == delimiter ) {
                output.push_back( str );
                str = "";
            }
            else {
                str += *c;
            }
            c++;
        }
        if ( str != "" ) {
            output.push_back( str );
        }
        return std::move( output );
    }

    std::string STR_GetTextLine( int inIndex, const std::string & inText, const std::string & NL ){

        assert( inIndex >= 0 && "STR_GetTextLine - inIndex must be >0.");

        if( NL == "" ) {
            return inIndex < inText.size() ? inText.substr(inIndex,1) : "";
        }
        else if ( NL == "\r\n" ) {
            const char * c = inText.c_str();
            int lineI = 0;
            std::string str = "";
            while( *c != 0){
                if ( *c == '\r' ) { 
                    if ( c[1] == '\n' ) { 
                        if ( lineI == inIndex ) { return str; } 
                        c+=2; lineI++; 
                        continue; 
                    }
                }
                if ( lineI == inIndex ) { str += *c; }
                c++;
            }
            if ( lineI == inIndex ) { return str; }
            return "";
        }
        else if ( NL == "\r" || NL == "\n" ) {
            const char * c = inText.c_str();
            int lineI = 0;
            std::string str = "";
            while( *c != 0){
                if ( *c == NL[0] ) { 
                    if ( lineI == inIndex ) { return str; } 
                    c+=1; lineI++; 
                    continue; 
                }
                if ( lineI == inIndex ) { str += *c; }
                c++;
            }
            if ( lineI == inIndex ) { return str; }
            return "";
        }
        return "";
    }

    std::string STR_ReplaceTextLine( int inIndex, const std::string & inText, const std::string & inReplaceText, const std::string & NL ) {

        assert( inIndex >= 0 && "STR_ReplaceTextLine - inIndex must be >0.");

        if( NL == "" ) {
            std::string newText = inText.substr(0, inIndex);
            newText += inReplaceText;
            newText += inText.substr( inIndex+1 );
            return std::move(newText);
        }
        else if ( NL == "\r\n" ) {
            std::string newText = "";
            const char * c= inText.c_str();
            int lineI = 0; 
            std::string str = "";
            while( *c != 0){
                if(*c=='\r'){
                    if(c[1]=='\n'){
                        if ( lineI == inIndex ) { newText += inReplaceText; }
                        else { newText += str;}
                        newText += "\r\n";
                        c += 2; lineI++; str=""; continue;
                    }
                }
                str += *c; c++;
            }
            if ( lineI == inIndex ) { newText += inReplaceText; }
            else { newText += str; }
            return std::move(newText);
        }
        else if ( NL == "\r" || NL == "\n" ) {
            std::string newText = "";
            const char * c= inText.c_str();
            int lineI = 0; 
            std::string str = "";
            while( *c != 0){
                if(*c==NL[0]){
                    if ( lineI == inIndex ) { newText += inReplaceText; }
                    else { newText += str;}
                    newText += NL;
                    c += 1; lineI++; str=""; continue;
                }
                str += *c; c++;
            }
            if ( lineI == inIndex ) { newText += inReplaceText; }
            else { newText += str; }
            return std::move(newText);
        }
        return inText;
        
    }

    void STR_ToUpper( std::string & inStr ) {
        unsigned char * c = (unsigned char * ) inStr.data();
        while ( *c != 0 ) { *c = std::toupper( *c ); c++; }
    }

    void STR_ToLower( std::string & inStr ) {
        unsigned char * c = (unsigned char * ) inStr.data();
        while ( *c != 0 ) { *c = std::tolower( *c ); c++; }
    }
    
    std::string STR_ReplaceFirst(std::string& s,const std::string& toReplace,const std::string& replaceWith){
        // Replaces the first occurence of a string within another string
        std::size_t pos = s.find(toReplace);
        if (pos == std::string::npos) return s;
        return s.replace(pos, toReplace.length(), replaceWith);
    }
    std::vector<int> STR_LocateMap(const std::vector<std::string>& inWordList, const std::string& inSearchStr){
        // locates first occurance word list and builds a lookup map where -1 means no match
        std::vector<int> result ;
        for( int i = 0 ; i < inWordList.size() ; ++i ){
            int index = (int)inSearchStr.find( inWordList[i] );
            result.push_back(index);
        }
        return result ;
    }
    // --------------------------------------------------------------------- //
    // Language Helper Functions                                             //
    // --------------------------------------------------------------------- //
    std::string STR_DoubleBackSlashes(const std::string &inStr){
        std::string str="";char* c = (char *)inStr.data();
        while( *c != 0){if(*c=='\\'){str+='\\';}str+=*c;++c;}
        return str;
    }
    std::string STR_RemoveDoubleSlashes(const std::string &inStr){
        std::string str="";char* c = (char *)inStr.data();
        while( *c != 0){
            if(*c=='\\' && c[1]=='\\'){++c;}
            else if(*c=='/' && c[1]=='/'){++c;}
            str+=*c;++c;
        }
        return str;
    }

    // --------------------------------------------------------------------- //
    // Numeric String Functions                                              //
    // --------------------------------------------------------------------- //

    bool STR_IsNum( const std::string & inStr ) {
        unsigned char * c = (unsigned char *) inStr.c_str();
        int nbP = 0;
        if ( *c == '-' || *c == '+' ) { c++; }
        else if ( *c == '.' ) { nbP++; c++; }
        else if ( !isdigit(*c) ) { return false; }
        while( *c != 0 ) { 
            if ( isdigit(*c) ) { c++; }
            else if ( *c == '.' ) { 
                if ( nbP > 0 ) return false;
                nbP++; c++;
            }
            else { return false; }
            
        }
        return true;
    }

    bool STR_IsNum( const std::string & inStr, bool & hasDecimal ) {
        unsigned char * c = (unsigned char *) inStr.c_str();
        int nbP = 0; hasDecimal=false;
        if ( *c == '-' || *c == '+' ) { c++; }
        else if ( *c == '.' ) { nbP++; c++; hasDecimal = true; }
        else if ( !isdigit(*c) ) { return false; }
        while( *c != 0 ) { 
            if ( isdigit(*c) ) { c++; }
            else if ( *c == '.' ) { 
                if ( nbP > 0 ) return false;
                nbP++; c++; hasDecimal = true;
            }
            else { return false; }
            
        }
        return true;
    }

    std::string STR_FromNum(int inVal){
    #ifdef SLIB_USE_FMT
        return fmt::to_string(inVal);
    #else
        std::string intStr = "";
        std::ostringstream o;
        o << inVal;
        intStr += o.str();
        return intStr;
    #endif
    }

    std::string STR_FromNum(int64_t inVal){
    #ifdef SLIB_USE_FMT
        return fmt::to_string(inVal);
    #else
        std::string intStr = "";
        std::ostringstream o;
        o << inVal;
        intStr += o.str();
        return intStr;
    #endif
    }

    std::string STR_FromNum(uint64_t inVal){
    #ifdef SLIB_USE_FMT
        return fmt::to_string(inVal);
    #else
        std::string intStr = "";
        std::ostringstream o;
        o << inVal;
        intStr += o.str();
        return intStr;
    #endif
    }

    std::string STR_FromNum(float inVal) {
    #ifdef SLIB_USE_FMT
        std::string str = fmt::format(FMT_COMPILE("{}") , inVal );
        if ( str.size() >= std::numeric_limits<float>::max_digits10 ) { 
            Impl::PrecisionTrim( str, SLIB_MIN_DECIMAL_PLACES ); 
        }
        return std::move(str);
    #else
        std::ostringstream o;
        o.precision( std::numeric_limits<float>::max_digits10-1 );
        o << std::fixed << inVal;
        std::string str = o.str();
        Impl::PrecisionTrim( str, SLIB_MIN_DECIMAL_PLACES );
        return std::move(str);
    #endif
    }

    std::string STR_FromNum(double inVal) {
    #ifdef SLIB_USE_FMT
        std::string str = fmt::format(FMT_COMPILE("{}") , inVal );
        if ( str.size() >= std::numeric_limits<double>::max_digits10 ) { 
            Impl::PrecisionTrim( str, SLIB_MIN_DECIMAL_PLACES ); 
        }
        return std::move(str);
    #else
        std::ostringstream o;
        o.precision( std::numeric_limits<double>::max_digits10-1 );       
        o << std::fixed << inVal;
        std::string str = o.str();
        Impl::PrecisionTrim( str, SLIB_MIN_DECIMAL_PLACES );
        return std::move(str);
    #endif
    }
    std::string STR_FromNumO(double inVal){
        std::ostringstream o;
        o.precision( std::numeric_limits<double>::max_digits10-1 );       
        o << std::fixed << inVal;
        std::string str = o.str();
        Impl::PrecisionTrim( str, SLIB_MIN_DECIMAL_PLACES );
        return std::move(str);
    }

    std::string STR_FromNum(float inVal, int inPrecision) {
    #ifdef SLIB_USE_FMT
        return fmt::format( fmt::format( "{{:.{}f}}", inPrecision), inVal );
    #else
        std::ostringstream o;
        o.precision( inPrecision );
        o << std::fixed << inVal;
        return o.str();
    #endif
    }

    std::string STR_FromNum(double inVal, int inPrecision) {
    #ifdef SLIB_USE_FMT
        return fmt::format( fmt::format( "{{:.{}f}}", inPrecision), inVal );
    #else
        std::ostringstream o;
        o.precision( inPrecision );
        o << std::fixed << inVal;
        return o.str();
    #endif
    }

    void STR_ChangeMinDecimalPlaces( int inCount ) {
        SLIB_MIN_DECIMAL_PLACES = inCount;
    }

    void STR_ResetMinDecimalPlaces() {
        SLIB_MIN_DECIMAL_PLACES = 2;
    }

    void STR_DecimalRound( std::string & inStr, int inDecimalPlace ) {
        // Round the numeric decimal string a the decimal place specified

        size_t dPos = inStr.find('.');
        if ( dPos == std::string::npos ) { return; }    // Exit if there is no decimal point

        size_t pos = dPos + inDecimalPlace;
        if ( pos + 1 >= inStr.size() ) { return; }      // Exit if there is not enough digits for rounding

        unsigned char * c = (unsigned char *) inStr.data() + pos;

        bool addOne = ( c[1] - '0' >= 5 );

        size_t count = 0;
        while ( addOne ) {
            if ( *c == '.' ) {}
            else if ( *c == '9' ) {
                *c = '0';
            }
            else if ( *c >= '0' && *c <= '8' ) {
                *c += 1;
                addOne = false;
                break;
            }

            if ( count == pos ) { break; }
            c--; count++;
        }

        inStr.erase( pos + 1, std::string::npos );
        if ( inStr.back() == '.' ) { inStr.pop_back(); }
        if ( addOne ) { inStr.insert( inStr.begin(), '1' ); }
    }

    void STR_NumTrimZeros( std::string & inStr ) {
        // Removes leading zeros: 
        //      0001000 --> 1000
        //      00.456  --> .456
        // Removes trailing zeros after decimal place:
        //      1.123000 --> 1.123
        // Does not remove the decimal unless it is a trailing decimal:
        //      12345. --> 12345
        //      123.45 --> 123.45
        //      123.00 --> 123
        // Does not remove trailing zeros if no decimal place:
        //      1000 --> 1000

        bool mask[255] = {false};
        mask['0'] = true;

        unsigned char * c = (unsigned char *) inStr.data();
        size_t nChar = inStr.size();
        size_t startI = 0;

        while ( *c != 0 ) {                         // Cycle through to find the index of the first included character
            if ( *c == '0' ) { startI++; c++; }
            else { break; }
        }

        if ( startI >= nChar ) { inStr = ""; return; }  // Exit because the entire string is zeros

        bool hasDot = false;                        // Keep cycling to see if we get a decimal point
        while ( *c != 0 ) {
            if ( *c == '.' ) { hasDot = true; break; }
            c++;
        }

        size_t endI = nChar-1;
        for ( endI; endI >= 0; endI-- ) {           // Cycle through to find the index of the last included character
            if ( inStr[endI] != '0' ) { break; }
        }

        assert( startI <= endI && "STR_Trim - startI is greater than endI.");

        if ( hasDot ) { 
            inStr.erase( endI + 1, std::string::npos );
            if ( inStr.back() == '.' ) { inStr.pop_back(); }
        }
        inStr.erase( 0, startI );

    }

    void STR_NumCommaSep( std::string & inStr ) {
        int64_t pos = (int64_t) inStr.find('.');
        if ( (size_t) pos == std::string::npos ) { pos = (int64_t) inStr.size(); }
        pos -= 3;
        while ( pos > 0 ) {
            inStr.insert( pos, "," );
            pos -= 3;
        }
    }
    
    std::string STR_PadZeros( const std::string & inStr, int nbDigits ) {
        if ( inStr.size() >= nbDigits ) { return inStr; }
        return std::string( nbDigits - inStr.size(), '0' ) + inStr;
    }

    // --------------------------------------------------------------------- //
    // Debug & Logging Functions                                             //
    // --------------------------------------------------------------------- //

    void LogTextDetailed ( const std::string & inStr, const std::string & inDlm ) {
        const char * c = inStr.c_str();
        while( *c != 0 ) {
            if      (*c=='\r') { ConsoleColor(79);  std::cout<<"\\r"; }
            else if (*c=='\n') { ConsoleColor(207); std::cout<<"\\n\n"; }
            else if (*c=='\t') { ConsoleColor(159); std::cout<<"\\t"; }
            else if (*c==0x14) { ConsoleColor(224); std::cout<<"DC4"; }
            else if (*c==0x13) { ConsoleColor(224); std::cout<<"DC3"; }
            else if (*c==0x12) { ConsoleColor(224); std::cout<<"DC2"; }
            else if (*c==0x11) { ConsoleColor(224); std::cout<<"DC1"; }
            else if (*c <32  ) { ConsoleColor(224); std::cout << std::to_string( (int)(*c) ); }
            else               { ConsoleColor(10);  std::cout<<*c; }
            ++c;
            if ( *c != 0 ) { ConsoleColor(7); std::cout << inDlm; }
        }
        ConsoleColor(7); std::cout <<"\n";
    }

    void ConsoleColor(int inClr){
        #if defined( _WIN32 )
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole,inClr);
        #endif
    }
    
    // --------------------------------------------------------------------- //
    // Formula Parsing functions                                             //
    // --------------------------------------------------------------------- //
    string STR_RemoveQuotes(string inStr){ // remove quotes if you don't have them, but don't add if quotes are ' (ex: already a regex)
    
        if(inStr==""){return "";}
        string newStr = inStr;
        newStr.erase(
        std::remove(newStr.begin(), newStr.end(), '\"'), 
        newStr.end());
        return newStr;
    }

    string STR_AddQuotes(string inStr){ // adds quotes if you don't have them, but don't add if quotes are ' (ex: already a regex)
        if(inStr==""){return "\"\"";}
        string newStr = "";if(inStr[0]!='"'){newStr="\"";}newStr+=inStr;
        if(inStr[inStr.size()-1]!='"'){newStr+="\"";}
        return newStr;
    }
    string STR_AddSingleQuotes(string inStr){ // adds quotes if you don't have them, but don't add if quotes are ' (ex: already a regex)
        // also remove double quotes if you have them
        char* c=(char *)inStr.data();string str = "'";
        if(*c=='"'||*c=='\''){c++;}
        while(*c!= 0){if(c[1]==0){if(*c=='"'||*c=='\''){break;}}str+=*c;++c;}
        str += "'";
        return str;
    }
    
    // --------------------------------------------------------------------- //
    // Internal Helper Functions                                             //
    // --------------------------------------------------------------------- //

    namespace Impl {

        void PrecisionTrim( std::string & inStr, int minDecimalPlaces ) {
            // Trims decimal 9's and 0's due to float & double precision artifact to conversion
            // This shows up like: 0.###0000000000000# or 0.###9999999999999#
            // Args:
            //   inStr = numeric string generated by STR_FromNum(...)
            //   inPrecision: the precision used to generate the numeric string
            //   minDecimalPlaces: the minimum number of decimal places to keep when trimming

            int dPos = (int) inStr.find('.');
            if ( dPos == std::string::npos ) { return; }            // Exit if there is no decimal

            int minPos = dPos + minDecimalPlaces;

            // if ( inStr[0] != '0' ) {
                int tPos = (int) inStr.size() - 1;                                      // Start at last char position.
                if ( tPos-1 > dPos ) {                                                  // Then, cycle back 1 char, since the last char can be anything,
                    unsigned char * c = ( unsigned char * ) inStr.data() + tPos - 1;    // and check if that char is a 0 or 9. If so, keep cycling until
                    if ( *c == '9' || *c == '0' ) {                                     // you hit a char that is not 0 or 9.
                        int nT = 1;
                        while ( *c == '9' || *c == '0' ) {
                            c--; nT++;
                        }
                        tPos -= nT;
                        if ( tPos < minPos ) { tPos = minPos; }
                    }
                }
                STR_DecimalRound( inStr, tPos );                    // Truncate the number with rounding applied.
            // }
            
            if ( inStr.size() > minPos ) {                          // Trim trailing zeros and decimal point
                int last = (int) inStr.find_last_not_of('0');
                if ( last < minPos ) { last = minPos; }
                inStr.erase( last+1, std::string::npos );
                if ( inStr.back() == '.' ) { inStr.pop_back(); }
            }

        }

    } // end namespace Impl

} // end namespace SLib

#endif /* STRINGLIB_CPP */
