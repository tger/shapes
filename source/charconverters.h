#ifndef charconverters_h
#define charconverters_h


#include "refcount.h"     // Why?!  (You get strange compiler errors if you dont include refcount.h here.)
#include "iconvselect.h"

#include "FontMetrics_decls.h"


namespace MetaPDF
{
  namespace Helpers
  {

    extern const char * theUCS4EncodingName;
    
    iconv_t requireUTF8ToMacRomanConverter( bool cleanup = false );
    iconv_t requireMacRomanToUTF8Converter( bool cleanup = false );

    iconv_t requireUTF8ToASCIIConverter( bool cleanup = false );

    iconv_t requireUTF8ToUCS4Converter( bool cleanup = false );
    iconv_t requireUTF16BEToUCS4Converter( bool cleanup = false );

    iconv_t requireUTF8ToWinANSIConverter( bool cleanup = false );

    const FontMetrics::GlyphList & requireGlyphList( bool cleanup = false );
  }
}


#endif