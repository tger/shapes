/* This file is part of Shapes.
 *
 * Shapes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Shapes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Shapes.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2008 Henrik Tidefelt
 */

#include "ast.h"
#include "globals.h"

using namespace Shapes;
using namespace std;


void
Kernel::Continuation::takeHandle( Kernel::VariableHandle val, Kernel::EvalState * evalState, bool callingMyself ) const
{
	if( val->isThunk( ) )
		{
			val->force( val, evalState );
		}
	else if( callingMyself )
		{
			throw Exceptions::InternalError( strrefdup( "Continuation is just calling itself..." ) );
		}
	else
		{
			this->takeValue( val->getUntyped( ), evalState, true );
		}
}

void
Kernel::Continuation::takeValue( const RefCountPtr< const Lang::Value > & val, Kernel::EvalState * evalState, bool callingMyself ) const
{
	if( callingMyself )
		{
			throw Exceptions::InternalError( strrefdup( "Continuation is just calling itself..." ) );
		}
	this->takeHandle( Kernel::VariableHandle( new Kernel::Variable( val ) ), evalState, true );
}

const Ast::SourceLocation &
Kernel::Continuation::traceLoc( ) const
{
	return traceLoc_;
}

Kernel::Continuation::BackTraceElem::BackTraceElem( const Kernel::Continuation * cont, const char * msg )
	: mem_( NullPtr< const char >( ) ), cont_( cont ), msg_( msg )
{ }

Kernel::Continuation::BackTraceElem::BackTraceElem( const Kernel::Continuation * cont, const RefCountPtr< const char > msg )
	: mem_( msg ), cont_( cont ), msg_( msg.getPtr( ) )
{ }

Kernel::Continuation::BackTraceElem::~BackTraceElem( )
{ }

std::ostream &
Kernel::operator << ( std::ostream & os, const Kernel::Continuation::BackTraceElem & elem )
{
	os << elem.cont_->traceLoc( ) << "	" << elem.msg_ ;
	return os;
}


void
Kernel::Continuation::backTrace( std::ostream & os )
{
	typedef std::list< Kernel::Continuation::BackTraceElem > ListType;
	ListType trace;
	backTrace( & trace );
	ListType::const_iterator i = trace.begin( );
	ListType::const_iterator next = i;
	++next;
	for( ; i != trace.end( ); ++i, ++next )
		{
			if( next == trace.end( ) ||
					not i->cont_->traceLoc( ).contains( next->cont_->traceLoc( ) ) )
				{
					os << "	" << *i << std::endl ;
				}
		}
}

namespace Shapes
{
	namespace Kernel
	{

		class ForcedStructureContinuationHelper : public Kernel::Continuation
		{
			const ForcedStructureContinuation * cont_;	// This does not have proper memory management...
			Kernel::ContRef contMem_;	// and this is not properly down-casted.
			RefCountPtr< const Lang::Structure > structure_;
			RefCountPtr< const Lang::SingleList > lst_;
		public:
			ForcedStructureContinuationHelper( const ForcedStructureContinuation * cont, Kernel::ContRef contMem, const RefCountPtr< const Lang::Structure > & structure, const RefCountPtr< const Lang::SingleList > & lst, const Ast::SourceLocation & traceLoc )
				: Kernel::Continuation( traceLoc ), cont_( cont ), contMem_( contMem ), structure_( structure ), lst_( lst )
			{ }
			virtual ~ForcedStructureContinuationHelper( )
			{ }
			virtual void takeValue( const RefCountPtr< const Lang::Value > & val, Kernel::EvalState * evalState, bool dummy ) const
			{
				// Getting here means that some value that _we_ don't care about has been forced.

				RefCountPtr< const Lang::SingleList > firstUnforced = Kernel::ForcedStructureContinuation::findUnforced( lst_ );
				if( firstUnforced->isNull( ) )
					{
						cont_->takeStructure( structure_, evalState );
					}
				else
					{
						typedef const Lang::SingleListPair ArgType;
						RefCountPtr< ArgType > p = Helpers::down_cast< ArgType >( firstUnforced, "< internal error: SingleListPair contradicting isNull( )" );
						evalState->cont_ = Kernel::ContRef( new Kernel::ForcedStructureContinuationHelper( cont_, contMem_, structure_, p->cdr_, traceLoc_ ) );
						p->car_->force( const_cast< Kernel::VariableHandle & >( p->car_ ), evalState );
					}

			}
			virtual void backTrace( std::list< Kernel::Continuation::BackTraceElem > * trace ) const
			{
				trace->push_front( Kernel::Continuation::BackTraceElem( this, "< Forcing union >" ) );
				cont_->backTrace( trace );
			}
			virtual void gcMark( Kernel::GCMarkedSet & marked )
			{
				const_cast< Lang::Structure * >( structure_.getPtr( ) )->gcMark( marked );
				const_cast< Lang::SingleList * >( lst_.getPtr( ) )->gcMark( marked );
				contMem_->gcMark( marked );
			}
		};

	}
}


Kernel::ForcedStructureContinuation::ForcedStructureContinuation( const char * continuationName, const Ast::SourceLocation & traceLoc )
	: Kernel::Continuation( traceLoc ), continuationName_( continuationName )
{ }

Kernel::ForcedStructureContinuation::~ForcedStructureContinuation( )
{ }

void
Kernel::ForcedStructureContinuation::takeValue( const RefCountPtr< const Lang::Value > & val, Kernel::EvalState * evalState, bool dummy ) const
{
	typedef const Lang::Structure ArgType;
	RefCountPtr< ArgType > structure = Helpers::down_cast< ArgType >( val, continuationName_ );

	RefCountPtr< const Lang::SingleList > firstUnforced = findUnforced( structure->values_ );
	if( firstUnforced->isNull( ) )
		{
			this->takeStructure( structure, evalState );
		}
	else
		{
			typedef const Lang::SingleListPair ArgType;
			RefCountPtr< ArgType > p = Helpers::down_cast< ArgType >( structure->values_, "< internal error: SingleListPair contradicting isNull( )" );
			evalState->cont_ = Kernel::ContRef( new Kernel::ForcedStructureContinuationHelper( this, evalState->cont_, structure, p->cdr_, traceLoc_ ) );
			p->car_->force( const_cast< Kernel::VariableHandle & >( p->car_ ), evalState );
		}
}

RefCountPtr< const Lang::SingleList >
Kernel::ForcedStructureContinuation::findUnforced( RefCountPtr< const Lang::SingleList > lst )
{
	try
		{
			while( true )
				{
					typedef const Lang::SingleListPair ArgType;
					RefCountPtr< ArgType > p = Helpers::try_cast_CoreArgument< ArgType >( lst );
					if( p->car_->isThunk( ) )
						{
							return lst;
						}
					lst = p->cdr_;
				}
		}
	catch( const NonLocalExit::NotThisType & ball )
		{
			// This means we reached the end of the list.
		}
	return lst;
}



Ast::Node::Node( const Ast::SourceLocation & loc )
	: parent_( 0 ), loc_( loc )
{ }

Ast::Node::~Node( )
{ }

Ast::Expression::Expression( const Ast::SourceLocation & loc )
	: Ast::Node( loc ), immediate_( false )
{ }

Ast::Expression::~Expression( )
{ }


const Ast::SourceLocation &
Ast::Node::loc( ) const
{
	return loc_;
}


Ast::BindNode::BindNode( const Ast::SourceLocation & loc, const Ast::SourceLocation & idLoc, const char * id )
	: Ast::Node( loc ), idLoc_( idLoc ), id_( id )
{ }

Ast::BindNode::~BindNode( )
{
	if( id_ != 0 )
		{
			delete id_;
		}
}

const char *
Ast::BindNode::id( ) const
{
	return id_;
}

const Ast::SourceLocation &
Ast::BindNode::idLoc( ) const
{
	return idLoc_;
}


Ast::IdentifierNode::IdentifierNode( )
{ }

Ast::IdentifierNode::~IdentifierNode( )
{ }


Ast::ErrorExpression::ErrorExpression( const Ast::SourceLocation & loc )
	: Ast::Expression( loc )
{ }

Ast::ErrorExpression::~ErrorExpression( )
{ }

void
Ast::ErrorExpression::analyze( Ast::Node * parent, const Ast::AnalysisEnvironment * env )
{
	/*
		That this is an error should have generated the appropriate messages elsewhere.
	 */
}

void
Ast::ErrorExpression::eval( Kernel::EvalState * evalState ) const
{
	throw Exceptions::InternalError( loc_, "An ErrorExpression was evaluated" );
}

