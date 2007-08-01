#include <cmath>

#include "MetaPDF_Helpers_decls.h"

#include "metapdfcore.h"
#include "continuations.h"
#include "globals.h"
#include "texlabelmanager.h"
#include "metapdfexceptions.h"

using namespace MetaPDF;


namespace MetaPDF
{
  namespace Lang
  {
    
    class Core_TeX : public Lang::CoreFunction
    {
    public:
      Lang::Core_TeX::Core_TeX( const char * title )
	: CoreFunction( title )
      { }
      virtual void
      Lang::Core_TeX::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
      {
	const size_t ARITY = 1;
	CHECK_ARITY( args, ARITY, title_ );
  
	typedef const Lang::String ArgType;
	RefCountPtr< ArgType > arg = Helpers::down_cast_CoreArgument< ArgType >( title_, args, 0, callLoc );
  
	Kernel::ContRef cont = evalState->cont_;
	cont->takeValue( Kernel::theTeXLabelManager.request( std::string( arg->val_.getPtr( ) ), evalState->dyn_ ),
			 evalState );
      }
    };
    
    class Core_coords2D : public Lang::CoreFunction
    {
    public:
      Lang::Core_coords2D::Core_coords2D( const char * title )
	: CoreFunction( title )
      { }
      virtual void
      Lang::Core_coords2D::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
      {
	const size_t ARITY = 2;
	CHECK_ARITY( args, ARITY, title_ );
  
	RefCountPtr< const Lang::Value > xUntyped = args.getValue( 0 );
	RefCountPtr< const Lang::Value > yUntyped = args.getValue( 1 );

	{
	  typedef const Lang::Float ArgType;
	  ArgType * xVal = dynamic_cast< ArgType * >( xUntyped.getPtr( ) );
	  if( xVal != 0 )
	    {
	      ArgType * yVal = dynamic_cast< ArgType * >( yUntyped.getPtr( ) );
	      if( yVal == 0 )
		{
		  if( xVal->val_ == 0 )
		    {
		      /* This is a special case, where 0 is interpreted as a length.
		       */
		      typedef const Lang::Length ArgTypeY;
		      ArgTypeY * yVal = dynamic_cast< ArgTypeY * >( yUntyped.getPtr( ) );
		      if( yVal != 0 )
			{
			  Kernel::ContRef cont = evalState->cont_;
			  cont->takeValue( Kernel::ValueRef( new Lang::Coords2D( Lang::Length( 0 ), *yVal ) ),
					   evalState );
			  return;
			}
		    }
		  throw Exceptions::CoreTypeMismatch( callLoc, "(<>, y )", args, 1, ArgType::staticTypeName( ) );
		}

	      Kernel::ContRef cont = evalState->cont_;
	      cont->takeValue( Kernel::ValueRef( new Lang::FloatPair( xVal->val_, yVal->val_ ) ),
			       evalState );
	      return;
	    }
	}

	{
	  typedef const Lang::Length ArgType;
	  ArgType * xVal = dynamic_cast< ArgType * >( xUntyped.getPtr( ) );
	  if( xVal != 0 )
	    {
	      ArgType * yVal = dynamic_cast< ArgType * >( yUntyped.getPtr( ) );
	      if( yVal == 0 )
		{
		  /* A Float with value 0 is still allowed
		   */
		  typedef const Lang::Float ArgTypeY;
		  ArgTypeY * yVal = dynamic_cast< ArgTypeY * >( yUntyped.getPtr( ) );
		  if( yVal != 0 )
		    {
		      if( yVal->val_ == 0 )
			{
			  Kernel::ContRef cont = evalState->cont_;
			  cont->takeValue( Kernel::ValueRef( new Lang::Coords2D( *xVal, Lang::Length( 0 ) ) ),
					   evalState );
			  return;
			}
		    }
		  throw Exceptions::CoreTypeMismatch( callLoc, "(<>, y )", args, 1, ArgType::staticTypeName( ) );
		}

	      Kernel::ContRef cont = evalState->cont_;
	      cont->takeValue( Kernel::ValueRef( new Lang::Coords2D( *xVal, *yVal ) ),
			       evalState );
	      return;
	    }
	}

	throw Exceptions::CoreTypeMismatch( callLoc, "( x ,<>)", args, 0, Helpers::typeSetString( Lang::Float::staticTypeName( ), Lang::Length::staticTypeName( ) ) );
      }
    };
    
    class Core_cornercoords2D : public Lang::CoreFunction
    {
    public:
      Lang::Core_cornercoords2D::Core_cornercoords2D( const char * title )
	: CoreFunction( title )
      { }
      virtual void
      Lang::Core_cornercoords2D::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
      {
	const size_t ARITY = 3;
	CHECK_ARITY( args, ARITY, title_ );

	Kernel::ContRef cont = evalState->cont_;
	cont->takeValue( Kernel::ValueRef( new Lang::CornerCoords2D( * Helpers::down_cast_CoreArgument< const Lang::Length >( "( x ,<>^<>)", args, 0, callLoc ),
								     * Helpers::down_cast_CoreArgument< const Lang::Length >( "(<>, y ^<>)", args, 1, callLoc ),
								     Helpers::down_cast_CoreArgument< const Lang::Float >( "(<>,<>^ a )", args, 2, callLoc )->val_ ) ),
			 evalState );
      }
    };
    
    class Core_coords3D : public Lang::CoreFunction
    {
    public:
      Lang::Core_coords3D::Core_coords3D( const char * title )
	: CoreFunction( title )
      { }
      virtual void
      Lang::Core_coords3D::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
      {
	const size_t ARITY = 3;
	CHECK_ARITY( args, ARITY, title_ );

	RefCountPtr< const Lang::Value > xUntyped = args.getValue( 0 );
	RefCountPtr< const Lang::Value > yUntyped = args.getValue( 1 );
	RefCountPtr< const Lang::Value > zUntyped = args.getValue( 2 );

	{
	  typedef const Lang::Float ArgType;
	  ArgType * xVal = dynamic_cast< ArgType * >( xUntyped.getPtr( ) );
	  if( xVal != 0 )
	    {
	      Kernel::ContRef cont = evalState->cont_;
	      cont->takeValue( Kernel::ValueRef( new Lang::FloatTriple( xVal->val_,
									Helpers::down_cast_CoreArgument< ArgType >( "(<>, y ,<>)", args, 1, callLoc )->val_,
									Helpers::down_cast_CoreArgument< ArgType >( "(<>,<>, z )", args, 2, callLoc )->val_ ) ),
			       evalState );
	      return;
	    }
	}

	{
	  typedef const Lang::Length ArgType;
	  ArgType * xVal = dynamic_cast< ArgType * >( xUntyped.getPtr( ) );
	  if( xVal != 0 )
	    {
	      Kernel::ContRef cont = evalState->cont_;
	      cont->takeValue( Kernel::ValueRef( new Lang::Coords3D( *xVal,
								     * Helpers::down_cast_CoreArgument< ArgType >( "(<>, y ,<>)", args, 1, callLoc ),
								     * Helpers::down_cast_CoreArgument< ArgType >( "(<>,<>, z )", args, 2, callLoc ) ) ),
			       evalState );
	      return;
	    }
	}

	throw Exceptions::CoreTypeMismatch( callLoc, "( x ,<>,<>)", args, 0, Helpers::typeSetString( Lang::Float::staticTypeName( ), Lang::Length::staticTypeName( ) ) );
      }
    };
    
    class Core_polarHandle2DFree_r : public Lang::CoreFunction
    {
    public:
      Lang::Core_polarHandle2DFree_r::Core_polarHandle2DFree_r( const char * title )
	: CoreFunction( title )
      { }
      virtual void
      Lang::Core_polarHandle2DFree_r::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
      {
	const size_t ARITY = 1;
	CHECK_ARITY( args, ARITY, title_ );

	Kernel::ContRef cont = evalState->cont_;
	cont->takeValue( Kernel::ValueRef( new Lang::PolarHandle2DFree_r( evalState->dyn_->getDefaultUnit( ),
									  Helpers::down_cast_CoreArgument< const Lang::Float >( "(^ a )", args, 0, callLoc )->val_ ) ),
			 evalState );
      }
    };
    
    class Core_polarHandle2DFree_ra : public Lang::CoreFunction
    {
    public:
      Lang::Core_polarHandle2DFree_ra::Core_polarHandle2DFree_ra( const char * title )
	: CoreFunction( title )
      { }
      virtual void
      Lang::Core_polarHandle2DFree_ra::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
      {
	const size_t ARITY = 0;
	CHECK_ARITY( args, ARITY, title_ );

	Kernel::ContRef cont = evalState->cont_;
	cont->takeValue( Kernel::ValueRef( new Lang::PolarHandle2DFree_ra( evalState->dyn_->getDefaultUnit( ) ) ),
			 evalState );
      }
    };

  }
}



RefCountPtr< const Lang::CoreFunction > Ast::THE_FUNCTION_coords2D( new Lang::Core_coords2D( "coords2D" ) );
RefCountPtr< const Lang::CoreFunction > Ast::THE_FUNCTION_cornercoords2D( new Lang::Core_cornercoords2D( "cornercoords2D" ) );
RefCountPtr< const Lang::CoreFunction > Ast::THE_FUNCTION_coords3D( new Lang::Core_coords3D( "coords3D" ) );
RefCountPtr< const Lang::CoreFunction > Ast::THE_FUNCTION_polarHandle2DFree_r( new Lang::Core_polarHandle2DFree_r( "polarHandle2DFree_r" ) );
RefCountPtr< const Lang::CoreFunction > Ast::THE_FUNCTION_polarHandle2DFree_ra( new Lang::Core_polarHandle2DFree_ra( "polarHandle2DFree_ra" ) );
/* This belongs in consts.cc but we must make sure it is initialized before we use it below.  Note that the identifier will actually be destroyed
 * before Ast::THE_FUNCTION_TeX is destroyed, but that should not cause a failure...
 */
RefCountPtr< const char > Lang::TEX_SYNTAX_ID = strrefdup( "teX" );
RefCountPtr< const Lang::CoreFunction > Ast::THE_FUNCTION_TeX( new Lang::Core_TeX( Lang::TEX_SYNTAX_ID.getPtr( ) ) );