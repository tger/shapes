#ifndef texttypes_h
#define texttypes_h

#include "MetaPDF_Ast_decls.h"
#include "MetaPDF_Kernel_decls.h"
#include "MetaPDF_Lang_decls.h"
#include "MetaPDF_Lang_decls.h"

#include "ptrowner.h"
#include "refcount.h"
#include "environment.h"
#include "statetypes.h"
#include "elementarylength.h"
#include "concretecolors.h"
#include "elementarycoords.h"
#include "fontmetrics.h"

#include <list>
#include <iostream>
#include <stack>
#include <set>
#include <vector>


namespace MetaPDF
{
  namespace Lang
  {

    class Font : public Lang::NoOperatorOverloadValue
    {
    private:
      static std::map< RefCountPtr< const char >, RefCountPtr< SimplePDF::PDF_Object >, charRefPtrLess > theFontResourceMap_;
      static std::map< RefCountPtr< const char >, RefCountPtr< const FontMetrics::BaseFont >, charRefPtrLess > theFontMetricsMap_;
      static std::list< std::string > theFontMetricsSearchPath_;

    public:
      static void push_backFontMetricsPath( const char * path );
      static std::string searchGlyphList( );
    private:
      static std::string searchFontMetrics( RefCountPtr< const char > fontName );
      RefCountPtr< const char > fontName_;
      mutable RefCountPtr< SimplePDF::PDF_Object > resource_;
      mutable RefCountPtr< const FontMetrics::BaseFont > metrics_;
    public:
      Font( const RefCountPtr< const char > fontName, RefCountPtr< SimplePDF::PDF_Object > & resource, RefCountPtr< const FontMetrics::BaseFont > metrics = NullPtr< const FontMetrics::BaseFont >( ) );
      Font( const RefCountPtr< const char > builtInFontName );  // It is important that this is not used with fonts that aren't built in.
      virtual ~Font( );
      virtual void gcMark( Kernel::GCMarkedSet & marked );
      const RefCountPtr< SimplePDF::PDF_Object > & resource( ) const;
      RefCountPtr< const char > fontName( ) const;
      RefCountPtr< const FontMetrics::BaseFont > metrics( ) const;
      TYPEINFODECL;
    };

    class TextRenderingMode : public Lang::NoOperatorOverloadValue
    {
    public:
      typedef enum { FILL = 0, STROKE, FILLSTROKE, INVISIBLE, FILLCLIP, STROKECLIP, FILLSTROKECLIP, CLIP, UNDEFINED } ValueType;
      ValueType mode_;
      TextRenderingMode( const ValueType & mode );
      TextRenderingMode( bool fill, bool stroke, bool clip );
      virtual ~TextRenderingMode( );
      virtual void gcMark( Kernel::GCMarkedSet & marked ){ };
      TYPEINFODECL;
    };

    class TextOperation : public Lang::NoOperatorOverloadValue
    {
    public:
      TextOperation();
      virtual ~TextOperation( );

      virtual void shipout( std::ostream & os, Kernel::PageContentStates * pdfState, const Lang::Transform2D & tf ) const = 0;
      virtual void measure( Lang::Transform2D * textMatrix, Lang::Transform2D * textLineMatrix, Concrete::Length * xmin, Concrete::Length * ymin, Concrete::Length * xmax, Concrete::Length * ymax ) const = 0;
      
      TYPEINFODECL;
    };
    
    class KernedText : public Lang::TextOperation
    {
      RefCountPtr< const Kernel::TextState > textState_;
      RefCountPtr< const Kernel::GraphicsState > metaState_;

      // The representation is a bit artificial to be efficient:  NullPtr objects in
      // strings_ mark where kerning values are to be taken from kernings_.  Hence,
      // The number of NullPtr objects in strings_ must match the length of kernings_.
      std::list< RefCountPtr< const Lang::String > > strings_;
      std::list< double > kernings_;
      size_t maxLength_;
    public:
      KernedText( const RefCountPtr< const Kernel::TextState > & textState, const RefCountPtr< const Kernel::GraphicsState > & metaState );
      virtual ~KernedText( );
      void pushString( const RefCountPtr< const Lang::String > & str );
      void pushKerning( double kerning );
      
      virtual void show( std::ostream & os ) const;
      virtual void shipout( std::ostream & os, Kernel::PageContentStates * pdfState, const Lang::Transform2D & tf ) const;
      virtual void measure( Lang::Transform2D * textMatrix, Lang::Transform2D * textLineMatrix, Concrete::Length * xmin, Concrete::Length * ymin, Concrete::Length * xmax, Concrete::Length * ymax ) const;
      
      void push( Lang::KernedText * dst ) const;
      
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };
    
    class TextNewline : public Lang::TextOperation
    {
      Concrete::Coords2D t_;
    public:
      TextNewline( const Concrete::Length tx, const Concrete::Length ty );
      virtual ~TextNewline( );

      virtual void show( std::ostream & os ) const;
      virtual void shipout( std::ostream & os, Kernel::PageContentStates * pdfState, const Lang::Transform2D & tf ) const;
      virtual void measure( Lang::Transform2D * textMatrix, Lang::Transform2D * textLineMatrix, Concrete::Length * xmin, Concrete::Length * ymin, Concrete::Length * xmax, Concrete::Length * ymax ) const;
      
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };

    class TextMoveto : public Lang::TextOperation
    {
      RefCountPtr< const Lang::Transform2D > tf_;
    public:
      TextMoveto( const RefCountPtr< const Lang::Transform2D > & tf );
      virtual ~TextMoveto( );

      virtual void show( std::ostream & os ) const;
      virtual void shipout( std::ostream & os, Kernel::PageContentStates * pdfState, const Lang::Transform2D & tf ) const;
      virtual void measure( Lang::Transform2D * textMatrix, Lang::Transform2D * textLineMatrix, Concrete::Length * xmin, Concrete::Length * ymin, Concrete::Length * xmax, Concrete::Length * ymax ) const;
      
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };

  }

  namespace Lang
  {

    class CharacterSpacingBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      Concrete::Length spacing_;
    public:
      CharacterSpacingBinding( const Ast::SourceLocation & loc, const Concrete::Length spacing );
      virtual ~CharacterSpacingBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };
    
    class WordSpacingBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      Concrete::Length spacing_;
    public:
      WordSpacingBinding( const Ast::SourceLocation & loc, const Concrete::Length spacing );
      virtual ~WordSpacingBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };
        
    class HorizontalScalingBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      double scaling_;
    public:
      HorizontalScalingBinding( const Ast::SourceLocation & loc, const double scaling );
      virtual ~HorizontalScalingBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };
        
    class LeadingBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      Concrete::Length ty_;
      bool isRelative_;
    public:
      LeadingBinding( const Ast::SourceLocation & loc, const Concrete::Length ty );
      LeadingBinding( const Ast::SourceLocation & loc, const double r );
      virtual ~LeadingBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };
        
    class FontBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      RefCountPtr< const Lang::Font > font_;
    public:
      FontBinding( const Ast::SourceLocation & loc, const RefCountPtr< const Lang::Font > & font );
      virtual ~FontBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };
        
    class TextSizeBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      Concrete::Length size_;
    public:
      TextSizeBinding( const Ast::SourceLocation & loc, const Concrete::Length size );
      virtual ~TextSizeBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };

    class TextRenderingModeBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      Lang::TextRenderingMode::ValueType mode_;
    public:
      TextRenderingModeBinding( const Ast::SourceLocation & loc, const Lang::TextRenderingMode::ValueType mode );
      virtual ~TextRenderingModeBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };

    class TextRiseBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      Concrete::Length ty_;
      bool isRelative_;
    public:
      TextRiseBinding( const Ast::SourceLocation & loc, const Concrete::Length ty );
      TextRiseBinding( const Ast::SourceLocation & loc, const double r );
      virtual ~TextRiseBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };
        
    class TextKnockoutBinding : public DynamicBindings
    {
      Ast::SourceLocation loc_;
      bool knockout_;
    public:
      TextKnockoutBinding( const Ast::SourceLocation & loc, const bool knockout );
      virtual ~TextKnockoutBinding( );
      virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
      virtual void gcMark( Kernel::GCMarkedSet & marked );
    };
        
  }

  namespace Kernel
  {

    class CharacterSpacingDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      CharacterSpacingDynamicVariableProperties( const char * name );
      virtual ~CharacterSpacingDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class WordSpacingDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      WordSpacingDynamicVariableProperties( const char * name );
      virtual ~WordSpacingDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class HorizontalScalingDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      HorizontalScalingDynamicVariableProperties( const char * name );
      virtual ~HorizontalScalingDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class LeadingDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      LeadingDynamicVariableProperties( const char * name );
      virtual ~LeadingDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class FontDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      FontDynamicVariableProperties( const char * name );
      virtual ~FontDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class TextSizeDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      TextSizeDynamicVariableProperties( const char * name );
      virtual ~TextSizeDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class TextRenderingModeDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      TextRenderingModeDynamicVariableProperties( const char * name );
      virtual ~TextRenderingModeDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class TextRiseDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      TextRiseDynamicVariableProperties( const char * name );
      virtual ~TextRiseDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class TextKnockoutDynamicVariableProperties : public Kernel::DynamicVariableProperties
    {
    public:
      TextKnockoutDynamicVariableProperties( const char * name );
      virtual ~TextKnockoutDynamicVariableProperties( );
      virtual Kernel::HandleType fetch( const Kernel::PassedDyn & dyn ) const;
      virtual void makeBinding( Kernel::HandleType val, Ast::SourceLocation loc, Kernel::EvalState * evalState ) const;
    };
    
    class TextState
    {
    public:
      static const char KNOCKOUT_FLAG_BIT = 0x01;
      static const char KNOCKOUT_UNDEFINED_BIT = 0x02;
    public:
      Concrete::Length characterSpacing_;                        // Use NaN for undefined.
      Concrete::Length wordSpacing_;                             // Use NaN for undefined.
      double horizontalScaling_;                                 // Use NaN for undefined.
    private:
      Concrete::Length leading_;                                 // Use NaN for undefined.  Offtype in case leadingIsRelative_.
      bool leadingIsRelative_;                                   // This is only defined when leading_ is defined.
      Concrete::Length rise_;                                    // Use NaN for undefined.  Offtype in case riseIsRelative_.
      bool riseIsRelative_;                                      // This is only defined when rise_ is defined.
    public:
      RefCountPtr< const Lang::Font > font_;                     // Use NullPtr for undefined.
      Concrete::Length size_;                                    // Use NaN for undefined.
      Lang::TextRenderingMode::ValueType mode_;                  // Use UNDEFINED for undefined.
      char knockout_;                                            // Set the bit KNOCKOUT_UNDEFINED for undefined.
    public:
      TextState( );
      explicit TextState( const Kernel::TextState & orig );   // explicit, since reference counting shall be used in most cases
      TextState( const Kernel::TextState & newValues, const Kernel::TextState & oldValues );
      TextState( bool setDefaults );    
      ~TextState( );

      void setLeading( const Concrete::Length leading );
      void setLeading( const double relativeLeading );
      bool hasLeading( ) const;
      Concrete::Length leadingConcrete( ) const;
      RefCountPtr< const Lang::Value > leading( ) const;

      void setRise( const Concrete::Length rise );
      void setRise( const double relativeRise );
      bool hasRise( ) const;
      Concrete::Length riseConcrete( ) const;
      RefCountPtr< const Lang::Value > rise( ) const;

      bool synchKnockout( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );
      bool synchAssertKnockout( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );

    private:
      static std::map< bool, RefCountPtr< SimplePDF::PDF_Object > > knockoutNameMap_;

      bool synchCharacterSpacing( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );
      bool synchWordSpacing( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );
      bool synchHorizontalScaling( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );
      bool synchLeading( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );
      bool synchFontAndSize( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );
      bool synchMode( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );
      bool synchRise( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );

      void assertKnockout( const Kernel::TextState * ref );

      bool synchButKnockout( std::ostream & os, const Kernel::TextState * ref, SimplePDF::PDF_Resources * resources, bool force = false );
      
    };

  }
}

#endif