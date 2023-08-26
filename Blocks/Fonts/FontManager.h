#ifndef FONTMANAGER_H
#define FONTMANAGER_H

namespace Blocks {

   enum FontEmphasis {
      ftem_none = 0,
      ftem_bold = 1,
      ftem_italic = 2,
      ftem_bolditalic = 3
   };

   void  InitFonts();
   int   GetFontInd(int inType, int inEmphasis, int inScale);
   void  GetFontIndAndScale( int inType, float inSize, int inEmphasis, int & outIndex, float & outScale );
   void  GetFontIndAndScale( int inType, float inSize, bool inB, bool inI, int & outIndex, float & outScale );
   float ConvertStandardFontSize( float ghSize );

   int GetGeneralFont(string inName);
   int BaseFont();
   int BaseFontB();
   int GetCodeFont(string Type);

} // end namespace Blocks


#endif /* FONTMANAGER_H */
