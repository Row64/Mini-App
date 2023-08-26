#ifndef UILIBEDITOR
#define UILIBEDITOR

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "UILib.h"
#pragma warning(disable:4996)  //  cross-platform so avoid these Microsoft warnings

using namespace std;

// ============================================================================================ //
//                                          BITMASKS                                            //
// ============================================================================================ //

enum bm_groups { // Bitmask groups for detecting a set of chars in one shot
    bm_digit = 1, 	        // 0123456789
    bm_digit_sign = 2, 	    // 0123456789+-
    bm_digit_dot = 4, 	    // 0123456789.
    bm_newline = 8, 	    // \n \r
    bm_gather = 16, 	    // \n \r ,
    bm_column_end = 32 	    // \n \r , NULL
};
static const unsigned char bm_table[256] =
{
    32,	0,	0,	0,	0,	0,	0,	0,	0,	0,	56,	0,	0,	56,	0,	0,		// 0-15
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 16-31
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	2,	48,	2,	4,	0,		// 32-47
    7,	7,	7,	7,	7,	7,	7,	7,	7,	7,	0,	0,	0,	0,	0,	0,		// 48-63
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 64-79
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 80-95
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 96-111
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 112-127
                                                                        
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 128+
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0		
		
};

#define CHECKBM(inC, inGroup) ( bm_table[static_cast<unsigned char>(inC)] & (inGroup) )

// ============================================================================================ //
//                                          MACROS                                              //
// ============================================================================================ //

#define GATHER_TOKEN {\
    if(token.size() > 0){CastKT(cast, token, CL, KTList, inText);}\
    for(int j=0;j<(int)token.size();j++){CL[token[j]].Syntax = cast;}\
    cast=0;\
    token = {};\
}

#define CAST_TOKEN( inType ) {                                          \
    sToken = inText.substr(CL[T[0]].SIndex, T.size());                  \
    bool kMatch = false;                                                \
    for (string keyW:KTL){if(keyW == sToken){kMatch = true;break;}}     \
    if(kMatch){Cast=inType;return;}                                     \
}

// ============================================================================================ //
//                                        DEFINITIONS                                           //
// ============================================================================================ //


namespace UILib {

void ParsePythonSyntax(vector<CharCell> &CL, string &inText, int inNlType){
    
    // assumes NL_TYPE 2 = "\r\n" (Windows new line type)
    
    vector<string> castStr = {"uncast", "op", "delimiter", "int", "float", "unquoted text", "single quoted", "double quoted","comment","keyword","type"};
    int cast = 0;  // code syntax: 0=uncast, 1=op, 2=delimiter, 3=int, 4=float, 5=Unquoted Text, 6=Single Quoted, 7=Double Quoted, 8=Comment,9=Keywork,10=Type
    int nbP = 0; // number of periods in string
    
    //    symbol:                (         {        [         ,       +       ^       -      *       /        <      =       >       &       %       )       }      ]       !       |        ~       @        :
    map<int, int> opStrength = {{40,100},{123,100},{91,100},{44,95},{43,90},{94,80},{45,90},{42,70},{47,70},{60,30},{61,30},{62,30},{38,20},{37,10},{41,5},{125,5},{93,5},{33,62},{124,38},{126,26},{64,26},{58,26}};
    
    vector<int> token = {}; // holds the current word / string / token
    int strength = 0; // strength of string or number, operators have higher strength (defined by opStrength)
    int c; // current char
    int i;int j;int maxI = (int)CL.size()-1;

    vector<string> KTList;  // container for holding Keyword Type Lists as unquoted text is compared
    bool hitendQ;bool multiS;
    for(i=0;i<=maxI;i++){

        startScan:
        c = CL[i].Char;
        //cout << "--- [" << i << "] c: " << c.Char << "  " << (int)c.Char << "\n";
        strength = 0;
        auto it = opStrength.find(c); if (it != opStrength.end()) { strength = it->second; }
        
        if(c==32||c==9||c=='\r'||c=='\n') { GATHER_TOKEN continue;}  // ------------  in space, tab or newline ------------
        else if(c==' ') { cast=0;GATHER_TOKEN continue;}
        else if(c == 45){ // ------- handle negative number sign -------
            int priorChar = 0; // find character before not including spaces - cycle back to skip spaces
            j = (int)i;while(j-->0){if(CL[j].Char != 32 && CL[j].Char != 9){ priorChar = CL[j].Char; break;}}
            int priorStrength = 0;
            if(i == 0){priorStrength = 1000;}
            else{ auto it = opStrength.find(priorChar); if (it != opStrength.end()) { priorStrength = it->second; } }
            if( priorStrength > 0){cast = 4;nbP = 0; } // carry it forward as negative number
            else{  GATHER_TOKEN;cast = 1;token.push_back(i);GATHER_TOKEN;continue;} // treat it as a minus sign operator
        }
        else if(strength > 4){ // ------------  operator ------------
            GATHER_TOKEN; cast = 1;
            if(c==':'){cast=0;token.push_back(i);continue;}
            if(i<=maxI-1){ // multi-chararacter operators
                if(c=='<'){
                    if(CL[i+1].Char=='='){token.push_back(i);i++;c = CL[i].Char;}
                    else if(CL[i+1].Char=='>'){token.push_back(i);i++;c = CL[i].Char;}
                }
                else if(c=='>'){if(CL[i+1].Char=='='){token.push_back(i);i++;c = CL[i].Char;}}
            }
            if(c=='('||c=='{'||c=='['||c==','||c==')'||c=='}'||c==']'){cast=2;}
            token.push_back(i);GATHER_TOKEN;continue;
        }
        else if(c=='#') {  // ------------  single line comment ------------ 
            while(true){
                token.push_back(i);
                i++;if(i>=maxI){cast = 8;c = CL[i].Char;break;}  // HIT EOL
                if(CL[i].Char=='\r'){token.push_back(i);cast = 8;GATHER_TOKEN i+=2;c = CL[i].Char;cast=0;goto startScan;}
                else if(CL[i].Char=='\n'){token.push_back(i);cast = 8;GATHER_TOKEN  i++;c = CL[i].Char;cast=0;goto startScan;}
            }
        }
        else if(c=='"') {  // ------------  double quote ------------ 
            GATHER_TOKEN
            multiS = false;if(i<maxI-3){if(CL[i+1].Char=='"' && CL[i+2].Char=='"'){multiS=true;}}
            if(multiS){ // quick crawl for triple double quote
                token.push_back(i);i++;
                token.push_back(i);i++;
                while(true){
                    token.push_back(i);
                    i++;if(i>=maxI){cast = 7;goto endScan;}  // HIT EOL ERROR - QUOTE NO END QUOTE
                    if(i<maxI-3){
                        if(CL[i].Char=='"'&&CL[i+1].Char=='"'&&CL[i+2].Char=='"'){token.push_back(i);i++;token.push_back(i);i++;token.push_back(i);i++;cast = 7;GATHER_TOKEN i+=2;break;} 
                    }
                }
            }
            else{  // quick crawl for double quote
                hitendQ = false;
                while(true){
                    token.push_back(i);
                    i++;if(i>=maxI){cast = 7;goto endScan;}  // HIT EOL ERROR - QUOTE NO END QUOTE
                    if(CL[i].Char=='"'){token.push_back(i);i++;cast = 7;GATHER_TOKEN hitendQ=true;break;}
                    else if(CL[i].Char=='\r'){cast = 6;GATHER_TOKEN i+=2;c = CL[i].Char;break;} // ERROR - return within single line quote
                    else if(CL[i].Char=='\n'){cast = 6;GATHER_TOKEN break;} // ERROR - return within single line quote
                }
                if(hitendQ){continue;}
            }
        }
        else if(c=='\'') {  // ------------  single quote ------------ 
            GATHER_TOKEN
            multiS = false;if(i<maxI-3){if(CL[i+1].Char=='\'' && CL[i+2].Char=='\''){multiS=true;}}
            if(multiS){ // quick crawl for triple single quote (multi-line comment)
                token.push_back(i);i++;
                token.push_back(i);i++;
                while(true){
                    token.push_back(i);
                    i++;if(i>=maxI){cast = 6;goto endScan;}  // HIT EOL ERROR - QUOTE NO END QUOTE
                    if(i<maxI-3){
                        if(CL[i].Char=='\''&&CL[i+1].Char=='\''&&CL[i+2].Char=='\''){token.push_back(i);i++;token.push_back(i);i++;token.push_back(i);i++;cast = 6;GATHER_TOKEN i+=2;break;}
                        token.push_back(i);
                    }
                }
            }
            else{  // quick crawl for single quote
                while(true){ 
                    token.push_back(i);
                    i++;if(i>=maxI){cast = 6;goto endScan;}  // HIT EOL ERROR - QUOTE NO END QUOTE
                    if(CL[i].Char=='\''){token.push_back(i);i++;cast = 6;GATHER_TOKEN break;}
                    else if(CL[i].Char=='\r'){cast = 6;GATHER_TOKEN i+=2;c = CL[i].Char;break;} // ERROR - return within single line quote
                    else if(CL[i].Char=='\n'){cast = 6;GATHER_TOKEN break;} // ERROR - return within single line quote
                }
            }
        }
        else{ // numbers and unquoted text - 0=uncast, 1=op, 3=int, 4=float, 5=Unquoted Text
            if(cast == 0){       // first character analysis
                if( CHECKBM(c, bm_digit_sign) ){ cast = 3; }else if(c=='.'){ cast = 4; nbP=1; }else{ cast = 5; }
            }
            else if(cast == 3){   // int
                if( CHECKBM(c, bm_digit_dot) ){ if(c=='.'){ nbP++; cast = 4; } }else{ cast= 5; }
            }
            else if(cast == 4){ // float
                if( CHECKBM(c, bm_digit_dot) ){ if(c=='.'){ nbP++; if(nbP>1){ cast = 5; } } }else{ cast=5; }
            }
        }
        token.push_back(i);
    }
    
    endScan:
    if(token.size()>0){
        if(token[0] < (int)CL.size()){
            GATHER_TOKEN
        }
    }
}

void CastKT(int &Cast, vector<int> &T, vector<CharCell> &CL, vector<string> &KTL, string &inText){

    string logT = inText.substr(CL[T[0]].SIndex, T.size()); 
    if(CL[T[0]].Char == 'a'){
        string sToken;KTL.clear();KTL = {"as", "assert","and"}; CAST_TOKEN(9) // Keywords
    }
    else if(CL[T[0]].Char == 'b'){
        string sToken;KTL.clear();KTL = {"break"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"bool","bytearray","bytes","basestring","buffer"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'c'){
        string sToken;KTL.clear();KTL = {"class","continue"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"classmethod","complex"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'd'){
        string sToken;KTL.clear();KTL = {"def","del"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"dict"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'e'){
        string sToken;KTL.clear();KTL = {"elif","else","except", "exec"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"enumerate"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'f'){
        string sToken;KTL.clear();KTL = {"finally","for","from"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"filter","float","frozenset","file"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'g'){
        string sToken;KTL.clear();KTL = {"global"}; CAST_TOKEN(9) // Keywords
    }
    else if(CL[T[0]].Char == 'i'){
        string sToken;KTL.clear();KTL = {"if","import","in", "is"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"int"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'l'){
        string sToken;KTL.clear();KTL = {"lambda"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"list","long"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'm'){
        string sToken;KTL.clear();KTL = {"map", "memoryview"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'n'){
        string sToken;KTL.clear();KTL = {"nonlocal","not"}; CAST_TOKEN(9) // Keywords
    }
    else if(CL[T[0]].Char == 'o'){
        string sToken;KTL.clear();KTL = {"or"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL.clear();KTL = {"object","open"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'p'){
        string sToken;KTL.clear();KTL = {"pass"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"property", "print"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'r'){
        string sToken;KTL.clear();KTL = {"raise","return"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"range","reversed"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 's'){
        string sToken;KTL.clear();KTL = {"set","slice","staticmethod","str","super"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 't'){
        string sToken;KTL.clear();KTL = {"try"}; CAST_TOKEN(9) // Keywords
        KTL.clear();KTL = {"tuple","type"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'u'){
        string sToken;KTL.clear();KTL = {"unicode"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'w'){
        string sToken;KTL.clear();KTL = {"while","with"}; CAST_TOKEN(9) // Keywords
    }
    else if(CL[T[0]].Char == 'x'){
        string sToken;KTL.clear();KTL = {"xrange"}; CAST_TOKEN(10) // Items
    }
    else if(CL[T[0]].Char == 'y'){
        string sToken;KTL.clear();KTL = {"yield"}; CAST_TOKEN(9) // Keywords
    }
    else if(CL[T[0]].Char == 'z'){
        string sToken;KTL.clear();KTL = {"zip"}; CAST_TOKEN(10) // Items
    }
    
}

void UISetScheme( string inScheme ) {
    
    if ( inScheme == "Default" ) {      // default is Jupyter Notebook colors
        UILibCTX->SchemeName = "Default";
        UILibCTX->SyntaxOptions = {
            //-Name-------------Type-----------------FontC---------FontS---Background----Border-------Underline---
            { "UNCAST",         syn_uncast,          0x000000,     0                                              },      // 0
            { "OPERATOR",       syn_op,              0xaa22ff,     1                                              },      // 1
            { "DELIMITER",      syn_delim,           0x000000,     0                                              },      // 2
            { "INT",            syn_int,             0x008800,     0                                              },      // 3
            { "FLOAT",          syn_float,           0x008800,     0                                              },      // 4
            { "UNQUOTED",       syn_unquoted,        0x000000,     0                                              },      // 5
            { "SINGLE QUOTE",   syn_singlequote,     0xba2121,     0                                              },      // 6
            { "DOUBLE QUOTE",   syn_doublequote,     0xba2121,     0                                              },      // 7
            { "COMMENT",        syn_comment,         0x7EAEC5,     2                                              },      // 8
            { "KEYWORD",        syn_keyword,         0x008000,     1                                              },      // 9
            { "TYPE",           syn_type,            0x008000,     0                                              },      // 10
            { "BRACKET",        syn_bracket,         0x2fbb00,     0,       -1,          -1,             -1       },      // 11
            { "FUNCTION NAME",  syn_funcname,        0x0725fa,     1                                              },      // 12
            { "SHEET FORMULA",  syn_sheetformula,    0x000000,     1                                              },      // 13
            { "SHEET FINPUT",   syn_sformulain,      0x797979,     0                                              },      // 14
            { "SHEET FIERROR",  syn_sformulaerr,     0x000000,     0                                              },      // 15
            { "CUSTOM SELECT",  syn_custom,          0x70B945,     1                                              }       // 16
        };
        UILibCTX->EditorOptions = {
            //-Name----------------------Type---------------------------Fill-----------Border------Underline-----
            { "ACTIVE CURSOR",           ed_cursor_active,              0x000000                                },  // 0
            { "INACTIVE CURSOR",         ed_cursor_inactive,            0x000000                                },  // 1
            { "ACTIVE HIGHLIGHT",        ed_highlight_active,           0xCCE1F1                                },  // 2
            { "INACTIVE HIGHLIGHT",      ed_highlight_inactive,         0xd9d9d9                                },  // 3
            { "ACTIVE INSTANCE",         ed_instance_active,            0x7e7ebc                                },  // 4
            { "INACTIVE INSTANCE",       ed_instance_inactive,          0xf2f2f2                                },  // 5
            { "TEXT EDITOR BOX",         ed_editor_box,                 0xf7f7f7,      0xcfcfcf                 },  // 6
            { "TEXT EDITOR ROW",         ed_editor_row,                 0xf7f7f7,      0xdfdfdf                 },  // 7
            { "OUTPUT HIGHLIGHT",        ed_output_highlight,           0x3390ff                                },  // 8
            { "CELL LABEL",              ed_cell_label,                 0x303f9f                                },  // 9
            { "CELL (EDIT MODE)",        ed_cell_edit,                  0xffffff,      0x66bb6a                 },  // 10
            { "CELL (SELECTED MODE)",    ed_cell_select,                0xffffff,      0x42a5f5                 },  // 11
            { "CELL (HIGHLIGHTED)",      ed_cell_highlight,             0xe3f2fd                                },  // 12
            { "NOTEBOOK BACKGROUND",     ed_background,                 0xffffff                                },  // 13
            { "SEARCH TERM",             ed_term,                       0x228BC5                                }   // 14
        };
    }
    else if ( inScheme == "Spreadsheet" ) {
        UILibCTX->SchemeName = "Spreadsheet";
        UILibCTX->SyntaxOptions = {
            //-Name-------------Type-----------------FontC---------FontS---Background----Border-------Underline---
            { "UNCAST",         syn_uncast,          0x000000,     0                                              },      // 0
            { "OPERATOR",       syn_op,              0x861024,     1                                              },      // 1
            { "DELIMITER",      syn_delim,           0x000000,     0                                              },      // 2
            { "INT",            syn_int,             0xce0303,     0                                              },      // 3
            { "FLOAT",          syn_float,           0xce0303,     0                                              },      // 4
            { "UNQUOTED",       syn_unquoted,        0x000000,     0                                              },      // 5
            { "SINGLE QUOTE",   syn_singlequote,     0x62a35c,     0                                              },      // 6
            { "DOUBLE QUOTE",   syn_doublequote,     0x62a35c,     0                                              },      // 7
            { "COMMENT",        syn_comment,         0xa8a8a8,     2                                              },      // 8
            { "KEYWORD",        syn_keyword,         0x0839bd,     1                                              },      // 9
            { "TYPE",           syn_type,            0x3e6bb9,     0                                              },      // 10
            { "BRACKET",        syn_bracket,         0x000000,     0,       -1,          -1,    0x000000          },      // 11
            { "FUNCTION NAME",  syn_funcname,        0x21be51,     0                                              },      // 12
            { "SHEET FORMULA",  syn_sheetformula,    0x4F4F4F,     1                                              },      // 13
            { "SHEET FINPUT",   syn_sformulain,      0x797979,     0                                              },      // 14
            { "SHEET FIERROR",  syn_sformulaerr,     0x000000,     0                                              },      // 15
            { "CUSTOM SELECT",  syn_custom,          0x70B945,     1                                              }       // 16
        };
        UILibCTX->EditorOptions = {
            //-Name----------------------Type---------------------------Fill-----------Border------Underline-----
            { "ACTIVE CURSOR",           ed_cursor_active,              0x000000                                },  // 0
            { "INACTIVE CURSOR",         ed_cursor_inactive,            0x000000                                },  // 1
            { "ACTIVE HIGHLIGHT",        ed_highlight_active,           0xd9dee3                                },  // 2
            { "INACTIVE HIGHLIGHT",      ed_highlight_inactive,         0xaeb6ba                                },  // 3
            { "ACTIVE INSTANCE",         ed_instance_active,            0x909799                                },  // 4
            { "INACTIVE INSTANCE",       ed_instance_inactive,          0xdfe2e3                                },  // 5
            { "TEXT EDITOR BOX",         ed_editor_box,                 0xffffff,      0xcfcfcf                 },  // 6
            { "TEXT EDITOR ROW",         ed_editor_row,                 0xffffff,      0xdfdfdf                 },  // 7
            { "OUTPUT HIGHLIGHT",        ed_output_highlight,           0x3390ff                                },  // 8
            { "CELL LABEL",              ed_cell_label,                 0x939393                                },  // 9
            { "CELL (EDIT MODE)",        ed_cell_edit,                  0xf7f7f7,      0x29ddab                 },  // 10
            { "CELL (SELECTED MODE)",    ed_cell_select,                0xf7f7f7,      0x46b1d3                 },  // 11
            { "CELL (HIGHLIGHTED)",      ed_cell_highlight,             0xe3f2fd                                },  // 12
            { "NOTEBOOK BACKGROUND",     ed_background,                 0xf7f7f7                                },  // 13
            { "SEARCH TERM",             ed_term,                       0x228BC5                                }   // 14
        };
    }
    else if ( inScheme == "Sublime" ) {
        UILibCTX->SchemeName = "Sublime";
        UILibCTX->SyntaxOptions = {
            //-Name-------------Type-----------------FontC---------FontS---Background----Border-------Underline---
            { "UNCAST",         syn_uncast,          0xf8f8f2,     0                                              },      // 0
            { "OPERATOR",       syn_op,              0xf92472,     1                                              },      // 1
            { "DELIMITER",      syn_delim,           0xf8f8f2,     0                                              },      // 2
            { "INT",            syn_int,             0xac80ff,     0                                              },      // 3
            { "FLOAT",          syn_float,           0xac80ff,     0                                              },      // 4
            { "UNQUOTED",       syn_unquoted,        0xf8f8f2,     0                                              },      // 5
            { "SINGLE QUOTE",   syn_singlequote,     0xe7db6a,     0                                              },      // 6
            { "DOUBLE QUOTE",   syn_doublequote,     0xe7db6a,     0                                              },      // 7
            { "COMMENT",        syn_comment,         0x747056,     2                                              },      // 8
            { "KEYWORD",        syn_keyword,         0xf92472,     1                                              },      // 9
            { "TYPE",           syn_type,            0x67d8ef,     0                                              },      // 10
            { "BRACKET",        syn_bracket,         -1,           0,       -1,          -1,          0xf8f8f2    },      // 11
            { "FUNCTION NAME",  syn_funcname,        0xa6e22a,     0                                              },      // 12
            { "SHEET FORMULA",  syn_sheetformula,    0x4F4F4F,     1                                              },      // 13
            { "SHEET FINPUT",   syn_sformulain,      0x797979,     0                                              },      // 14
            { "SHEET FIERROR",  syn_sformulaerr,     0x000000,     0                                              },      // 15
            { "CUSTOM SELECT",  syn_custom,          0x70B945,     1                                              }       // 16
        };
        UILibCTX->EditorOptions = {
            //-Name----------------------Type---------------------------Fill-----------Border------Underline-----
            { "ACTIVE CURSOR",           ed_cursor_active,              0xf8f8f2                                },  // 0
            { "INACTIVE CURSOR",         ed_cursor_inactive,            0xf8f8f2                                },  // 1
            { "ACTIVE HIGHLIGHT",        ed_highlight_active,           0x48473d                                },  // 2
            { "INACTIVE HIGHLIGHT",      ed_highlight_inactive,         0x666459                                },  // 3
            { "ACTIVE INSTANCE",         ed_instance_active,            0xb8b8b1,      0xb8b8b1                 },  // 4
            { "INACTIVE INSTANCE",       ed_instance_inactive,          0xb8b8b1,      0xb8b8b1                 },  // 5
            { "TEXT EDITOR BOX",         ed_editor_box,                 0x282923,      0xd4d4d4                 },  // 6
            { "TEXT EDITOR ROW",         ed_editor_row,                 0x282923,      0xd4d4d4                 },  // 7
            { "OUTPUT HIGHLIGHT",        ed_output_highlight,           0x3390ff                                },  // 8
            { "CELL LABEL",              ed_cell_label,                 0xb8b8b1                                },  // 9
            { "CELL (EDIT MODE)",        ed_cell_edit,                  0x1e1e1a,      0xa6e22a                 },  // 10
            { "CELL (SELECTED MODE)",    ed_cell_select,                0x1e1e1a,      0x67d8ef                 },  // 11
            { "CELL (HIGHLIGHTED)",      ed_cell_highlight,             0xe3f2fd                                },  // 12
            { "NOTEBOOK BACKGROUND",     ed_background,                 0x1e1e1a                                },  // 13
            { "SEARCH TERM",             ed_term,                       0x228BC5                                }   // 14
        };
    }
    else if ( inScheme == "VS Code" ) {
        UILibCTX->SchemeName = "VS Code";
        UILibCTX->SyntaxOptions = {
            //-Name-------------Type-----------------FontC---------FontS---Background----Border-------Underline---
            { "UNCAST",         syn_uncast,          0xd4d4d4,     0                                              },      // 0
            { "OPERATOR",       syn_op,              0xd4d4d4,     1                                              },      // 1
            { "DELIMITER",      syn_delim,           0xd4d4d4,     0                                              },      // 2
            { "INT",            syn_int,             0x569cd6,     0                                              },      // 3
            { "FLOAT",          syn_float,           0x569cd6,     0                                              },      // 4
            { "UNQUOTED",       syn_unquoted,        0xd4d4d4,     0                                              },      // 5
            { "SINGLE QUOTE",   syn_singlequote,     0xce8f70,     0                                              },      // 6
            { "DOUBLE QUOTE",   syn_doublequote,     0xce8f70,     0                                              },      // 7
            { "COMMENT",        syn_comment,         0x6a9955,     2                                              },      // 8
            { "KEYWORD",        syn_keyword,         0xc586c0,     1                                              },      // 9
            { "TYPE",           syn_type,            0xdbd99a,     0                                              },      // 10
            { "BRACKET",        syn_bracket,         -1,           0,       -1,          0xd4d4d4                 },      // 11
            { "FUNCTION NAME",  syn_funcname,        0xdbd99a,     0                                              },      // 12
            { "SHEET FORMULA",  syn_sheetformula,    0x4F4F4F,     1                                              },      // 13
            { "SHEET FINPUT",   syn_sformulain,      0x797979,     0                                              },      // 14
            { "SHEET FIERROR",  syn_sformulaerr,     0x000000,     0                                              },      // 15
            { "CUSTOM SELECT",  syn_custom,          0x70B945,     1                                              }       // 16
        };
        UILibCTX->EditorOptions = {
            //-Name----------------------Type---------------------------Fill-----------Border------Underline-----
            { "ACTIVE CURSOR",           ed_cursor_active,              0xd4d4d4                                },  // 0
            { "INACTIVE CURSOR",         ed_cursor_inactive,            0xd4d4d4                                },  // 1
            { "ACTIVE HIGHLIGHT",        ed_highlight_active,           0x264f78                                },  // 2
            { "INACTIVE HIGHLIGHT",      ed_highlight_inactive,         0x546777                                },  // 3
            { "ACTIVE INSTANCE",         ed_instance_active,            0x326c9b                                },  // 4
            { "INACTIVE INSTANCE",       ed_instance_inactive,          0x326c9b                                },  // 5
            { "TEXT EDITOR BOX",         ed_editor_box,                 0x1e1e1e,      0x6d6e6a                 },  // 6
            { "TEXT EDITOR ROW",         ed_editor_row,                 0x1e1e1e,      0x6d6e6a                 },  // 7
            { "OUTPUT HIGHLIGHT",        ed_output_highlight,           0x3390ff                                },  // 8
            { "CELL LABEL",              ed_cell_label,                 0x858585                                },  // 9
            { "CELL (EDIT MODE)",        ed_cell_edit,                  0x252526,      0x587c0c                 },  // 10
            { "CELL (SELECTED MODE)",    ed_cell_select,                0x252526,      0x0c7d9d                 },  // 11
            { "CELL (HIGHLIGHTED)",      ed_cell_highlight,             0x9cdcf0                                },  // 12
            { "NOTEBOOK BACKGROUND",     ed_background,                 0x252526                                },  // 13
            { "SEARCH TERM",             ed_term,                       0x228BC5                                }   // 14
        };
    }
}

} // end namespace UILib

#endif /* UILIBEDITOR */
