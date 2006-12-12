#include "MetaPDF_Helpers_decls.h"

#include "hottypes.h"
#include "globals.h"
#include "continuations.h"
#include "texttypes.h"
#include "fontmetrics.h"
#include "tagtypes.h"
#include "isnan.h"
#include "pdfstructure.h"

#include <sstream>
#include <limits>
#include <sys/resource.h>

using namespace MetaPDF;


Kernel::Warm::~Warm( )
{ }


Lang::Hot::Hot::Hot( )
{ }

Lang::Hot::~Hot( )
{ }

RefCountPtr< const Lang::Class > Lang::Hot::TypeID( new Lang::SystemFinalClass( strrefdup( "Hot" ) ) );
TYPEINFOIMPL( Hot );


Lang::HotTriple::HotTriple( const RefCountPtr< const Lang::Value > & init, RefCountPtr< const Lang::Function > update, RefCountPtr< const Lang::Function > result )
  : init_( init ), update_( update ), result_( result )
{ }

Lang::HotTriple::~HotTriple( )
{ }

Kernel::Warm *
Lang::HotTriple::warm( ) const
{
  return new Kernel::WarmTriple( init_, update_, result_ );
}

void
Lang::HotTriple::gcMark( Kernel::GCMarkedSet & marked )
{
  const_cast< Lang::Value * >( init_.getPtr( ) )->gcMark( marked );
  const_cast< Lang::Function * >( update_.getPtr( ) )->gcMark( marked );
  const_cast< Lang::Function * >( result_.getPtr( ) )->gcMark( marked );
}


Kernel::WarmTriple::WarmTriple( const RefCountPtr< const Lang::Value > & pile, RefCountPtr< const Lang::Function > update, RefCountPtr< const Lang::Function > result )
  : pile_( pile ), update_( update ), result_( result )
{ }

Kernel::WarmTriple::~WarmTriple( )
{ }

void
Kernel::WarmTriple::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  /* This seems dangerous.  I have not verified that pile will still exist when the continuation below is invoked.
   */
  evalState->cont_ = Kernel::ContRef( new Kernel::StmtStoreValueContinuation( & pile_, 
									      evalState->cont_,
									      callLoc ) );
  update_->call( evalState, pile_, piece, callLoc );
}

void
Kernel::WarmTriple::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  /* The right continuation is set by the calling variable.
   */
  result_->call( evalState, pile_, callLoc );
}


Kernel::WarmOstream::WarmOstream( std::ostream & os )
  : os_( os )
{ }

Kernel::WarmOstream::~WarmOstream( )
{ }

void
Kernel::WarmOstream::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  piece->show( os_ );
  Kernel::ContRef cont = evalState->cont_;
  cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
		    evalState );
}

void
Kernel::WarmOstream::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  throw Exceptions::MiscellaneousRequirement( strrefdup( "A warm ostream cannot be frozen." ) );
}


Kernel::Warm_ostringstream::Warm_ostringstream( )
{ }

Kernel::Warm_ostringstream::~Warm_ostringstream( )
{ }

void
Kernel::Warm_ostringstream::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  piece->show( os_ );
  Kernel::ContRef cont = evalState->cont_;
  cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
		    evalState );
}

void
Kernel::Warm_ostringstream::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( Kernel::ValueRef( new Lang::String( strdup( os_.str( ).c_str( ) ) ) ),
		   evalState );
}


Kernel::WarmGroup2D::WarmGroup2D( )
  : pile_( Lang::THE_NULL2D )
{ }

Kernel::WarmGroup2D::~WarmGroup2D( )
{ }

void
Kernel::WarmGroup2D::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  typedef const Lang::Drawable2D ArgType;
  RefCountPtr< ArgType > arg = Helpers::down_cast< ArgType >( piece, callLoc );
  
  {
    /* For objects of user-defined type, we also check that the class inherits from Drawable.
     */
    typedef const Lang::Instance UserType;
    UserType * obj = dynamic_cast< UserType * >( arg.getPtr( ) );
    if( obj != 0 )
      {
	if( ! obj->getClass( )->method_isa( Lang::Drawable2D::TypeID ) )
	  {
	    throw Exceptions::TypeMismatch( callLoc, "2D insertion", piece->getTypeName( ), ArgType::staticTypeName( ) );
	  }
      }
  }
  
  pile_ = RefCountPtr< const Lang::Group2D >( new Lang::GroupPair2D( arg,
								     pile_,
								     dyn->getGraphicsState( ) ) );
  Kernel::ContRef cont = evalState->cont_;
  cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
		    evalState );
}

void
Kernel::WarmGroup2D::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( pile_,
		   evalState );
}


Kernel::WarmGroup3D::WarmGroup3D( )
  : pile_( Lang::THE_NULL3D )
{ }

Kernel::WarmGroup3D::~WarmGroup3D( )
{ }

void
Kernel::WarmGroup3D::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  pile_ = RefCountPtr< const Lang::Group3D >( new Lang::GroupPair3D( Helpers::down_cast< const Lang::Drawable3D >( piece, callLoc ),
								     pile_,
								     dyn->getGraphicsState( ) ) );
  Kernel::ContRef cont = evalState->cont_;
  cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
		    evalState );
}

void
Kernel::WarmGroup3D::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( pile_,
		   evalState );
}


Kernel::WarmGroupLights::WarmGroupLights( )
  : pile_( Lang::THE_NULL_LIGHTS )
{ }

Kernel::WarmGroupLights::~WarmGroupLights( )
{ }

void
Kernel::WarmGroupLights::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  pile_ = RefCountPtr< const Lang::LightGroup >( new Lang::LightPair( Helpers::down_cast< const Lang::LightSource >( piece, callLoc ),
								      pile_ ) );
  Kernel::ContRef cont = evalState->cont_;
  cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
		    evalState );
}

void
Kernel::WarmGroupLights::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( pile_,
		   evalState );
}


Kernel::WarmZBuf::WarmZBuf( )
{ }

Kernel::WarmZBuf::~WarmZBuf( )
{ }

void
Kernel::WarmZBuf::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  try
    {
      typedef const Lang::Drawable3D ArgType;
      RefCountPtr< ArgType > drawable = Helpers::try_cast_CoreArgument< ArgType >( piece );
      try
	{
	  drawable->polygonize( pile_.getPtr( ), strokePile_.getPtr( ), evalState->dyn_, Lang::THE_3D_IDENTITY, drawable );
	}
      catch( const char * ball )
	{
	  std::ostringstream oss;
	  oss << "Conversion to polygon failed with: " << ball ;
	  throw Exceptions::OutOfRange( callLoc, strrefdup( oss ) );
	}
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  try
    {
      lightPile_->push_back( Helpers::try_cast_CoreArgument< const Lang::LightSource >( piece ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  throw Exceptions::TypeMismatch( callLoc, piece->getTypeName( ), Helpers::typeSetString( Lang::Drawable3D::staticTypeName( ), Lang::LightSource::staticTypeName( ) ) );
}

void
Kernel::WarmZBuf::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( RefCountPtr< const Lang::Value >( new Lang::ZBuf( pile_, strokePile_, lightPile_, evalState->dyn_->getGraphicsState( ) ) ),
		   evalState );
}


Kernel::WarmZSorter::WarmZSorter( )
{ }

Kernel::WarmZSorter::~WarmZSorter( )
{ }

void
Kernel::WarmZSorter::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  try
    {
      typedef const Lang::Drawable3D ArgType;
      RefCountPtr< ArgType > drawable = Helpers::try_cast_CoreArgument< ArgType >( piece );
      try
	{
	  drawable->polygonize( pile_.getPtr( ), strokePile_.getPtr( ), evalState->dyn_, Lang::THE_3D_IDENTITY, drawable );
	}
      catch( const char * ball )
	{
	  std::ostringstream oss;
	  oss << "Conversion to polygon failed with: " << ball ;
	  throw Exceptions::OutOfRange( callLoc, strrefdup( oss ) );
	}
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  try
    {
      lightPile_->push_back( Helpers::try_cast_CoreArgument< const Lang::LightSource >( piece ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  throw Exceptions::TypeMismatch( callLoc, piece->getTypeName( ), Helpers::typeSetString( Lang::Drawable3D::staticTypeName( ), Lang::LightSource::staticTypeName( ) ) );
}

void
Kernel::WarmZSorter::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( RefCountPtr< const Lang::Value >( new Lang::ZSorter( pile_, strokePile_, lightPile_, evalState->dyn_->getGraphicsState( ) ) ),
		   evalState );
}


Kernel::WarmTimer::WarmTimer( )
{
  rusage ru;
  int res = getrusage( RUSAGE_SELF, &ru );
  if( res != 0 )
    {
      throw Exceptions::InternalError( strrefdup( "grtrusage failed." ) );
    }
  start_ = ru.ru_utime;
}

Kernel::WarmTimer::~WarmTimer( )
{ }

void
Kernel::WarmTimer::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  throw Exceptions::MiscellaneousRequirement( strrefdup( "A warm timer does not accept values.  Please freeze to obtain the number of seconds since creation." ) );
}

void
Kernel::WarmTimer::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  rusage ru;
  int res = getrusage( RUSAGE_SELF, &ru );
  if( res != 0 )
    {
      throw Exceptions::InternalError( strrefdup( "grtrusage failed." ) );
    }
  timeval stop = ru.ru_utime;
  double time1 = start_.tv_usec / 1000000.0 + start_.tv_sec;
  double time2 = stop.tv_usec  / 1000000.0 + stop.tv_sec;

  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( Kernel::ValueRef( new Lang::Float( time2 - time1 ) ),
		   evalState );
}


Kernel::WarmText::WarmText( )
{ }

Kernel::WarmText::~WarmText( )
{ }

void
Kernel::WarmText::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  try
    {
      typedef const Lang::TextOperation ArgType;
      pile_->push_back( Helpers::try_cast_CoreArgument< ArgType >( piece ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  try
    {
      typedef const Lang::String ArgType;
      RefCountPtr< ArgType > str = Helpers::try_cast_CoreArgument< ArgType >( piece );
      Lang::KernedText * text = new Lang::KernedText( evalState->dyn_->getTextState( ), evalState->dyn_->getGraphicsState( ) );
      text->pushString( str );
      pile_->push_back( RefCountPtr< const Lang::TextOperation >( text ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  try
    {
      typedef const Lang::Transform2D ArgType;
      RefCountPtr< ArgType > tf = Helpers::try_cast_CoreArgument< ArgType >( piece );
      pile_->push_back( RefCountPtr< const Lang::TextOperation >( new Lang::TextMoveto( tf ) ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  try
    {
      typedef const Lang::Coords2D ArgType;
      RefCountPtr< ArgType > t = Helpers::try_cast_CoreArgument< ArgType >( piece );
      pile_->push_back( RefCountPtr< const Lang::TextOperation >( new Lang::TextNewline( t->x_.get( ), t->y_.get( ) ) ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  try
    {
      typedef const Lang::FloatPair ArgType;
      RefCountPtr< ArgType > t = Helpers::try_cast_CoreArgument< ArgType >( piece );
      Concrete::Length sz = evalState->dyn_->getTextState( )->size_;
      pile_->push_back( RefCountPtr< const Lang::TextOperation >( new Lang::TextNewline( sz * t->x_, sz * t->y_ ) ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  throw Exceptions::TypeMismatch( callLoc, piece->getTypeName( ), Helpers::typeSetString( Lang::TextOperation::staticTypeName( ), Lang::String::staticTypeName( ) ) );
}

void
Kernel::WarmText::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  if( pile_->size( ) == 0 )
    {
      Kernel::ContRef cont = evalState->cont_;
      cont->takeValue( Lang::THE_NULL2D,
		       evalState );
      return;
    }

  Concrete::Length xmin = Concrete::HUGE_LENGTH;
  Concrete::Length xmax = -Concrete::HUGE_LENGTH;
  Concrete::Length ymin = Concrete::HUGE_LENGTH;
  Concrete::Length ymax = -Concrete::HUGE_LENGTH;

  {
    Lang::Transform2D textMatrix( 1, 0, 0, 1, 0, 0 );
    Lang::Transform2D textLineMatrix( 1, 0, 0, 1, 0, 0 );
    typedef typeof *pile_ ListType;
    for( ListType::const_iterator i = pile_->begin( ); i != pile_->end( ); ++i )
      {
	(*i)->measure( & textMatrix, & textLineMatrix, & xmin, & ymin, & xmax, & ymax );
      }
  }

  Lang::ElementaryPath2D * bbox = new Lang::ElementaryPath2D;

  if( xmin < Concrete::HUGE_LENGTH )
    {
      bbox->push_back( new Concrete::PathPoint2D( xmin, ymin ) );
      bbox->push_back( new Concrete::PathPoint2D( xmin, ymax ) );
      bbox->push_back( new Concrete::PathPoint2D( xmax, ymax ) );
      bbox->push_back( new Concrete::PathPoint2D( xmax, ymin ) );
      bbox->close( );
    }

  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( RefCountPtr< const Lang::Value >( new Lang::Text( evalState->dyn_->getGraphicsState( ), 
								     evalState->dyn_->getTextState( ),
								     pile_,
								     RefCountPtr< const Lang::ElementaryPath2D >( bbox ) ) ),
		   evalState );
}


Kernel::WarmType3Font::WarmType3Font( )
  : metrics_( new FontMetrics::BaseFont( ) ), size_( -1 )
{
  metrics_->capHeight_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->xHeight_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->ascender_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->descender_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->leading_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->stdHW_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->stdVW_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->fontBBoxXMin_ = 0;
  metrics_->fontBBoxXMax_ = -1;

  metrics_->horizontalMetrics_ = RefCountPtr< FontMetrics::WritingDirectionMetrics >( new FontMetrics::WritingDirectionMetrics( ) );
  metrics_->horizontalMetrics_->underlinePosition_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->horizontalMetrics_->underlineThickness_ = std::numeric_limits< double >::signaling_NaN( );
  metrics_->horizontalMetrics_->italicAngleRadians_ = std::numeric_limits< double >::signaling_NaN( );
}

Kernel::WarmType3Font::~WarmType3Font( )
{ }

void
Kernel::WarmType3Font::tackOn( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Value > & piece, const Kernel::PassedDyn & dyn, const Ast::SourceLocation & callLoc )
{
  try
    {
      typedef const Lang::Type3Glyph ArgType;
      glyphs_.push_back( Helpers::try_cast_CoreArgument< ArgType >( piece ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  try
    {
      typedef const Lang::KernedText ArgType;
      kernings_.push_back( Helpers::try_cast_CoreArgument< ArgType >( piece ) );
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  try
    {
      typedef const Lang::TaggedValue2D ArgType;
      RefCountPtr< ArgType > taggedVal = Helpers::try_cast_CoreArgument< ArgType >( piece );

      RefCountPtr< const char > tagMem = Lang::Symbol::nameFromKey( taggedVal->key( ) );
      const char * tag = tagMem.getPtr( );
      RefCountPtr< const Lang::Value > val = taggedVal->val( );

      if( strcmp( tag, "size" ) == 0 )
	{
	  if( size_ > 0 )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::Length ValType;
	  size_ = Helpers::down_cast< ValType >( val, callLoc )->get( );
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "The size must be positive." );
	    }
	}
      else if( strcmp( tag, "FontName" ) == 0 )
	{
	  if( metrics_->fontName_ != NullPtr< const char >( ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::Symbol ValType;
	  metrics_->fontName_ = Helpers::down_cast< ValType >( val, callLoc )->name( );
	}
      else if( strcmp( tag, "FullName" ) == 0 )
	{
	  if( metrics_->fullName_ != NullPtr< const char >( ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::String ValType;
	  metrics_->fullName_ = Helpers::down_cast< ValType >( val, callLoc )->val_;
	}
      else if( strcmp( tag, "FamilyName" ) == 0 )
	{
	  if( metrics_->familyName_ != NullPtr< const char >( ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::Symbol ValType;
	  metrics_->familyName_ = Helpers::down_cast< ValType >( val, callLoc )->name( );
	}
      else if( strcmp( tag, "Weight" ) == 0 )
	{
	  try
	    {
	      typedef const Lang::Symbol ValType;
	      RefCountPtr< const char > typedVal = Helpers::try_cast_CoreArgument< ValType >( val )->name( );
	      if( metrics_->weight_ != NullPtr< const char >( ) )
		{
		  throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
		}
	      static std::set< const char *, charPtrLess > legalStretchValues;
	      if( legalStretchValues.size( ) == 0 )
		{
		  initializeLegalStrechValues( & legalStretchValues );
		}
	      if( legalStretchValues.find( typedVal.getPtr( ) ) == legalStretchValues.end( ) )
		{
		  throw Exceptions::OutOfRange( callLoc, "The 'Weight value is illegal.  Please refer to the PDF specification." );
		}
	      metrics_->weight_ = typedVal;
	      goto OK;
	    }
	  catch( const NonLocalExit::NotThisType & ball )
	    {
	      /* Wrong type; never mind!.. but see below!
	       */
	    }

	  try
	    {
	      typedef const Lang::Integer ValType;
	      RefCountPtr< ValType > val = Helpers::try_cast_CoreArgument< ValType >( val );
	      if( metrics_->weightNumber_ > 0 )
		{
		  throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
		}
	      if( val->val_ % 100 != 0 )
		{
		  throw Exceptions::OutOfRange( callLoc, "The 'Weight number must be divisible by 100." );
		}
	      if( val->val_ < 100 ||val->val_ > 900 )
		{
		  throw Exceptions::OutOfRange( callLoc, "The 'Weight number must be in the range [ 100, 900 ]." );
		}
	      metrics_->weightNumber_ = val->val_;
	      goto OK;
	    }
	  catch( const NonLocalExit::NotThisType & ball )
	    {
	      /* Wrong type; never mind!.. but see below!
	       */
	    }

	  throw Exceptions::TypeMismatch( callLoc, val->getTypeName( ), Helpers::typeSetString( Lang::String::staticTypeName( ), Lang::Integer::staticTypeName( ) ) );
	}
      else if( strcmp( tag, "Version" ) == 0 )
	{
	  if( metrics_->version_ != NullPtr< const char >( ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::String ValType;
	  metrics_->version_ = Helpers::down_cast< ValType >( val, callLoc )->val_;
	}
      else if( strcmp( tag, "Notice" ) == 0 )
	{
	  if( metrics_->notice_ != NullPtr< const char >( ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::String ValType;
	  metrics_->notice_ = Helpers::down_cast< ValType >( val, callLoc )->val_;
	}
      else if( strcmp( tag, "EncodingScheme" ) == 0 )
	{
	  if( metrics_->encodingScheme_ != NullPtr< const char >( ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::Symbol ValType;
	  metrics_->encodingScheme_ = Helpers::down_cast< ValType >( val, callLoc )->name( );
	}
      else if( strcmp( tag, "CharacterSet" ) == 0 )
	{
	  if( metrics_->characterSet_ != NullPtr< const char >( ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::Symbol ValType;
	  metrics_->characterSet_ = Helpers::down_cast< ValType >( val, callLoc )->name( );
	}
      else if( strcmp( tag, "Comment" ) == 0 )
	{
	  typedef const Lang::String ValType;
	  RefCountPtr< ValType > typedVal = Helpers::down_cast< ValType >( val, callLoc );
	  metrics_->comments_.push_back( FontMetrics::AFM::AssortedInfo( tagMem, typedVal->val_ ) );
	}
      else if( strcmp( tag, "CapHeight" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->capHeight_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->capHeight_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "XHeight" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->xHeight_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->xHeight_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "Ascent" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->ascender_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->ascender_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "Descent" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->descender_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->descender_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "Leading" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->leading_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->leading_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "StemH" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->stdHW_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->stdHW_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "StemV" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->stdVW_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->stdVW_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "FontBBox" ) == 0 )
	{
	  if( metrics_->fontBBoxXMax_ >= metrics_->fontBBoxXMin_ )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::ElementaryPath2D ValType;
	  RefCountPtr< ValType > val = Helpers::elementaryPathTry2D( val );
	  Concrete::Coords2D llcorner( 0, 0 );
	  Concrete::Coords2D urcorner( 0, 0 );
	  if( ! val->boundingRectangle( & llcorner, & urcorner ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "The path was empty." );
	    }
	  metrics_->fontBBoxXMin_ = llcorner.x_ / size_;
	  metrics_->fontBBoxYMin_ = llcorner.y_ / size_;
	  metrics_->fontBBoxXMax_ = urcorner.x_ / size_;
	  metrics_->fontBBoxYMax_ = urcorner.y_ / size_;
	}
      else if( strcmp( tag, "UnderlinePosition" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->horizontalMetrics_->underlinePosition_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->horizontalMetrics_->underlinePosition_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "UnderlineThickness" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->horizontalMetrics_->underlineThickness_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  if( ! ( size_ > 0 ) )
	    {
	      throw Exceptions::OutOfRange( callLoc, "Please provide a value for 'size first." );
	    }
	  typedef const Lang::Length ValType;
	  metrics_->horizontalMetrics_->underlineThickness_ = Helpers::down_cast< ValType >( val, callLoc )->get( ) / size_;
	}
      else if( strcmp( tag, "ItalicAngle" ) == 0 )
	{
	  if( ! IS_NAN( metrics_->horizontalMetrics_->italicAngleRadians_ ) )
	    {
	      throw Exceptions::MiscellaneousRequirement( "Multiply specified value." );
	    }
	  typedef const Lang::Float ValType;
	  metrics_->horizontalMetrics_->italicAngleRadians_ = Helpers::down_cast< ValType >( val, callLoc )->val_;
	}
      else
	{
	  typedef const Lang::String ValType;
	  RefCountPtr< ValType > typedVal = Helpers::down_cast< ValType >( val, callLoc );
	  metrics_->assortedGlobalInfo_.push_back( FontMetrics::AFM::AssortedInfo( tagMem, typedVal->val_ ) );
	}

    OK:
      Kernel::ContRef cont = evalState->cont_;
      cont->takeHandle( Kernel::THE_SLOT_VARIABLE,
			evalState );
      return;
    }
  catch( const NonLocalExit::NotThisType & ball )
    {
      /* Wrong type; never mind!.. but see below!
       */
    }

  throw Exceptions::TypeMismatch( callLoc, piece->getTypeName( ), Helpers::typeSetString( Lang::Type3Glyph::staticTypeName( ), Lang::KernedText::staticTypeName( ), Lang::TaggedValue2D::staticTypeName( ) ) );
}

void
Kernel::WarmType3Font::freeze( Kernel::EvalState * evalState, const Ast::SourceLocation & callLoc )
{
  if( glyphs_.size( ) == 0 )
    {
      throw Exceptions::OutOfRange( callLoc, "A font without glyphs?!" );
    }
  if( ! ( size_ > 0 ) )
    {
      throw Exceptions::OutOfRange( callLoc, "Missing 'size." );
    }
  if( strlen( metrics_->fontName_.getPtr( ) ) == 0 )
    {
      throw Exceptions::OutOfRange( callLoc, "Missing 'FontName." );      
    }
  if( metrics_->weightNumber_ == 0 )
    {
      metrics_->weightNumber_ = 400;
    }
  bool findBBox = false;
  if( metrics_->fontBBoxXMax_ < metrics_->fontBBoxXMin_  )
    {
      findBBox = true;
    }

  Concrete::ReciprocalLength invSize = 1 / size_;

  metrics_->charCount_ = glyphs_.size( );
  metrics_->isCIDFont_ = false;

  size_t firstChar = INT_MAX;
  size_t lastChar = 0;
  {
    // In the first scan of the glyphs, only firstChar and lastChar are computed.  The other tasks are performed in the later scan.
    typedef typeof glyphs_ ListType;
    for( ListType::const_iterator i = glyphs_.begin( ); i != glyphs_.end( ); ++i )
      {
	size_t code = (*i)->code( );
	if( code > 0 )
	  {
	    firstChar = std::min( firstChar, code );
	    lastChar = std::max( lastChar, code );
	  }
      }
  }

  RefCountPtr< SimplePDF::PDF_Vector > widths;
  {
    RefCountPtr< SimplePDF::PDF_Object > stdWidth = SimplePDF::PDF_out::newFloat( 0 );
    widths->vec.resize( lastChar - firstChar + 1, stdWidth );
  }
  RefCountPtr< SimplePDF::PDF_Dictionary > charProcs;
  RefCountPtr< SimplePDF::PDF_Resources > resources;
  //  RefCountPtr< SimplePDF::PDF_Stream_out > toUnicode;

  RefCountPtr< FontMetrics::WritingDirectionMetrics > horizontal = metrics_->horizontalMetrics_;
  horizontal->charData_.reserve( glyphs_.size( ) );

  {
    // This is the main loop over all glyphs.
    typedef typeof glyphs_ ListType;
    for( ListType::const_iterator i = glyphs_.begin( ); i != glyphs_.end( ); ++i )
      {
	size_t code = (*i)->code( );
	if( code > 0 )
	  {
	    widths->vec[ code - firstChar ] = SimplePDF::PDF_out::newFloat( (*i)->widthX( ) );
	  }

	RefCountPtr< SimplePDF::PDF_Stream_out > glyphStream;
	(*i)->shipout( glyphStream->data, resources );
	charProcs->dic[ (*i)->name( ).getPtr( ) ] = Kernel::the_pdfo->indirect( glyphStream );

	size_t pos = horizontal->charData_.size( );
	FontMetrics::CharacterMetrics * glyphMetrics = new FontMetrics::CharacterMetrics( pos );
	(*i)->setupMetric( glyphMetrics, invSize );
	horizontal->charData_.push_back( glyphMetrics );
	horizontal->nameMap_[ (*i)->name( ) ] = pos;
	if( code > 0 )
	  {
	    horizontal->codeMap_[ code ] = pos;
	  }

	if( findBBox )
	  {
	    (*i)->enlargeBBox( & metrics_->fontBBoxXMin_, & metrics_->fontBBoxYMin_,  & metrics_->fontBBoxXMax_, & metrics_->fontBBoxYMax_ );
	  }
      }
  }

  RefCountPtr< SimplePDF::PDF_Vector > fontBBox( new SimplePDF::PDF_Vector( metrics_->fontBBoxXMin_, metrics_->fontBBoxYMin_,
									    metrics_->fontBBoxXMax_, metrics_->fontBBoxYMax_ ) );
  RefCountPtr< SimplePDF::PDF_Dictionary > fontDescriptor;
  (*fontDescriptor)[ "Type" ] = SimplePDF::PDF_out::newName( "FontDescriptor" );
  (*fontDescriptor)[ "FontName" ] = SimplePDF::PDF_out::newName( metrics_->fontName_.getPtr( ) );
  if( metrics_->familyName_.getPtr( ) != NullPtr< const char >( ) )
    {
      (*fontDescriptor)[ "FontFamily" ] = SimplePDF::PDF_out::newName( metrics_->familyName_.getPtr( ) );
    }
  if( metrics_->weight_.getPtr( ) != NullPtr< const char >( ) )
    {
      // It has already been asserted that this value is legal.
      (*fontDescriptor)[ "FontStretch" ] = SimplePDF::PDF_out::newName( metrics_->weight_.getPtr( ) );
    }
  if( metrics_->weightNumber_ > 0 )
    {
      // It has already been asserted that this value is legal.
      (*fontDescriptor)[ "FontWeight" ] = SimplePDF::PDF_out::newInt( metrics_->weightNumber_ );
    }
  (*fontDescriptor)[ "Flags" ] = SimplePDF::PDF_out::newInt( (size_t)(1) << ( 6 - 1 ) ); // This is just the "Nonsymbolic" flag.
  (*fontDescriptor)[ "FontBBox" ] = fontBBox;
  if( ! IS_NAN( horizontal->italicAngleRadians_ ) )
    {
      (*fontDescriptor)[ "ItalicAngle" ] = SimplePDF::PDF_out::newFloat( ( 180 / M_PI ) * horizontal->italicAngleRadians_ );
    }
  else
    {
      throw Exceptions::OutOfRange( callLoc, "Missing 'ItalicAngle." );
    }
  if( ! IS_NAN( metrics_->ascender_ ) )
    {
      (*fontDescriptor)[ "Ascent" ] = SimplePDF::PDF_out::newFloat( metrics_->ascender_ );
    }
  if( ! IS_NAN( metrics_->descender_ ) )
    {
      (*fontDescriptor)[ "Descent" ] = SimplePDF::PDF_out::newFloat( metrics_->descender_ );
    }
  if( ! IS_NAN( metrics_->leading_ ) )
    {
      (*fontDescriptor)[ "Leading" ] = SimplePDF::PDF_out::newFloat( metrics_->leading_ );
    }
  if( ! IS_NAN( metrics_->capHeight_ ) )
    {
      (*fontDescriptor)[ "CapHeight" ] = SimplePDF::PDF_out::newFloat( metrics_->capHeight_ );
    }
  if( ! IS_NAN( metrics_->xHeight_ ) )
    {
      (*fontDescriptor)[ "XHeight" ] = SimplePDF::PDF_out::newFloat( metrics_->xHeight_ );
    }
  if( ! IS_NAN( metrics_->stdHW_ ) )
    {
      (*fontDescriptor)[ "StemH" ] = SimplePDF::PDF_out::newFloat( metrics_->stdHW_ );
    }
  if( ! IS_NAN( metrics_->stdVW_ ) )
    {
      (*fontDescriptor)[ "StemV" ] = SimplePDF::PDF_out::newFloat( metrics_->stdVW_ );
    }

  Concrete::Length the1bp( 1 );
  
  RefCountPtr< SimplePDF::PDF_Dictionary > dic;
  RefCountPtr< SimplePDF::PDF_Object > indirection = Kernel::the_pdfo->indirect( dic );
  (*dic)[ "Type" ] = SimplePDF::PDF_out::newName( "Font" );
  (*dic)[ "Subtype" ] = SimplePDF::PDF_out::newName( "Type3" );
  (*dic)[ "Encoding" ] = SimplePDF::PDF_out::newName( "MacRomanEncoding" );
  (*dic)[ "FontBBox" ] = fontBBox;
  (*dic)[ "FontMatrix" ] = RefCountPtr< SimplePDF::PDF_Vector >( new SimplePDF::PDF_Vector( the1bp * invSize, 0, 0, the1bp * invSize, 0, 0 ) );
  (*dic)[ "CharProcs" ] = charProcs;
  (*dic)[ "FirstChar" ] = SimplePDF::PDF_out::newInt( firstChar );
  (*dic)[ "LastChar" ] = SimplePDF::PDF_out::newInt( lastChar );
  (*dic)[ "Widths" ] = widths;
  (*dic)[ "FontDescriptor" ] = fontDescriptor;
  (*dic)[ "Resources" ] = Kernel::the_pdfo->indirect( resources );

//   {
//     static bool shown = false;
//     if( ! shown )
//       {
// 	std::cerr << "Warning: The ToUnicode CMap is not setup by Drool." << std::endl ;
// 	shown = true;
//       }
//   }
  //  (*dic)[ "ToUnicode" ] = toUnicode;
  
  Kernel::ContRef cont = evalState->cont_;
  cont->takeValue( Kernel::ValueRef( new Lang::Font( metrics_->fontName_,
						     indirection,
						     metrics_ ) ),
		   evalState );
}

void
Kernel::WarmType3Font::initializeLegalStrechValues( std::set< const char *, charPtrLess > * legalStretchValues )
{
  legalStretchValues->insert( "UltraCondensed" );
  legalStretchValues->insert( "ExtraCondensed" );
  legalStretchValues->insert( "Condensed" );
  legalStretchValues->insert( "SemiCondensed" );
  legalStretchValues->insert( "Normal" );
  legalStretchValues->insert( "SemiExpanded" );
  legalStretchValues->insert( "Expanded" );
  legalStretchValues->insert( "ExtraExpanded" );
  legalStretchValues->insert( "UltraExpanded" );
}