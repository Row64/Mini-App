#ifndef STRINGLIB_H
#define STRINGLIB_H

#include <string>
#include <vector>

// #define SLIB_USE_FMT    // define SLIB_USE_FMT to use {fmt} library
#ifdef SLIB_USE_FMT
#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/format.h>
#endif


namespace SLib {

    // Static Variables

    static int SLIB_MIN_DECIMAL_PLACES = 2;    // Minimum number of decimal places to keep when number to string conversion involves truncating trailing decimal values

    // Start and End Operations
    bool STR_StartsWith( const std::string & inStr, const std::string & inTerm );                                       // Checks if a string starts with the given term
    bool STR_EndsWith ( const std::string & inStr, const std::string & inTerm );                                        // Checks if a string ends with the given term
    bool STR_MatchStart( const std::string & inStr, const std::string & inTerm, const std::string & inIgnore = " " );   // Checks if a string start matches with the given term ignoring specified characters (default ignores spaces) in both string and term
    
    std::string STR_Trim( const std::string & inStr, const std::string & inChars );   // Returns a string with the leading and trailing chars removed.
    std::string STR_TrimSpaces( const std::string & inStr );   // Returns a string with the leading and trailing spaces removed ( removes ' ' and '\t' )

    std::string STR_RemoveChars( const std::string & inStr, const std::string & inChars );   // Returns a string with the chars removed.
    std::string STR_RemoveLines(const std::string & inText, const std::string & inRemoveText); // Removes lines that start with the InRemoveText string.  Is case sensitive
    
    // String Manipulation Functions
    std::string STR_JoinList( const std::vector<std::string> & arr, std::string delimiter, bool popback = true );
    std::string STR_JoinList( const std::vector<std::string> & arr, const char delimiter, bool popback = true );
    std::vector<std::string> STR_SplitByDelimiter( const std::string & inStr, const char delimiter );
    std::string STR_GetTextLine(int inIndex, const std::string & inText, const std::string & NL = "\r\n");                                               // Returns a string for the line of text requested (default newline is \r\n)
    std::string STR_ReplaceTextLine( int inIndex, const std::string & inText, const std::string & inReplaceText, const std::string & NL = "\r\n" );      // Replaces a line in the text with the replacement text (default newline is \r\n)
    std::string STR_ReplaceFirst(std::string& s,const std::string& toReplace,const std::string& replaceWith);
    std::vector<int> STR_LocateMap(const std::vector<std::string>& inWordList, const std::string& inSearchStr);

    void        STR_ToUpper( std::string & inStr );
    void        STR_ToLower( std::string & inStr );

    // String Language Helpers
    std::string     STR_DoubleBackSlashes(const std::string &inStr);
    std::string     STR_RemoveDoubleSlashes(const std::string &inStr);

    // Numeric String Functions
    bool            STR_IsNum( const std::string & inStr );
    bool            STR_IsNum( const std::string & inStr, bool & hasDecimal );
    std::string     STR_FromNum(int inVal);
    std::string     STR_FromNum(int64_t inVal);
    std::string     STR_FromNum(uint64_t inVal);
    
    std::string     STR_FromNum(float  inVal);      // Returns an auto-truncated string representation of the float
    std::string     STR_FromNum(double inVal);      // Returns an auto-truncated string representation of the double
    std::string     STR_FromNumO(double inVal);     // Use ostringstream method to convert double
    std::string     STR_FromNum(float  inVal, int inPrecision);  // Returns a string with explicit number of decimal places given by inPrecision
    std::string     STR_FromNum(double inVal, int inPrecision);  // Returns a string with explicit number of decimal places given by inPrecision
    void            STR_ChangeMinDecimalPlaces( int inCount );
    void            STR_ResetMinDecimalPlaces();
    
    void            STR_DecimalRound( std::string & inStr, int inDecimalPlace = 0 );

    void            STR_NumTrimZeros( std::string & inStr );             // Removes leading and decimal trailing zeros from numeric string
    void            STR_NumCommaSep( std::string & inStr );              // Adds comma as thousands separator

    std::string     STR_PadZeros( const std::string & inStr, int nbDigits );

    // Debug & Logging Functions
    void            LogTextDetailed( const std::string & inStr, const std::string & inDlm = "" );       // Log text in ASCII handling char below 32 so it's easy to see the non-visual codes easily
    void            ConsoleColor( int inClr );                          // Set the console color (currently windows only)

    // Formula Parsing functions
    std::string     STR_RemoveQuotes(std::string inStr);
    std::string     STR_AddQuotes(std::string inStr);
    std::string     STR_AddSingleQuotes(std::string inStr);
    
    
    // Internal Helper functions
    namespace Impl {

    void            PrecisionTrim( std::string & inStr, int minDecimalPlaces );   // Trim trailing 0's and 9's due to float & double precision
    


    
    } // end namespace Impl

} // end namespace StrLib

#endif /* STRINGLIB_H */
