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

#ifndef ast_h
#define ast_h

#include "Shapes_Ast_decls.h"
#include "Shapes_Kernel_decls.h"

#include "refcount.h"
#include "shapestypes.h"
#include "dynamicenvironment.h"
#include "sourcelocation.h"

#include <list>
#include <iostream>
#include <string>

namespace Shapes
{

	namespace Kernel
	{

		class EvalState
		{
		public:
			Ast::Expression * expr_;
			Kernel::PassedEnv env_;
			Kernel::PassedDyn dyn_;
			Kernel::ContRef cont_;

			EvalState( Ast::Expression * expr, const Kernel::PassedEnv & env, const Kernel::PassedDyn & dyn, const Kernel::ContRef & cont )
				: expr_( expr ), env_( env ), dyn_( dyn ), cont_( cont )
			{ }
		};

		class Continuation
		{
		protected:
			const Ast::SourceLocation & traceLoc_;
		public:
			Continuation( const Ast::SourceLocation & traceLoc )
				: traceLoc_( traceLoc )
			{ }
			virtual ~Continuation( )
			{ }
			virtual void takeHandle( Kernel::VariableHandle val, Kernel::EvalState * evalState, bool callingMyself = false ) const;
			virtual void takeValue( const RefCountPtr< const Lang::Value > & val, Kernel::EvalState * evalState, bool callingMyself = false ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked ) = 0;
			const Ast::SourceLocation & traceLoc( ) const;
			class BackTraceElem
			{
				RefCountPtr< const char > mem_;
			public:
				const Kernel::Continuation * cont_;
				const char * msg_;
				BackTraceElem( const Kernel::Continuation * cont, const char * msg );
				BackTraceElem( const Kernel::Continuation * cont, const RefCountPtr< const char > msg );
				~BackTraceElem( );
			};
			virtual void backTrace( std::list< Kernel::Continuation::BackTraceElem > * trace ) const = 0; // outer continuation before ourselves in the list, that is first push_front ourselves, then recurse.
			void backTrace( std::ostream & os );
		};

		std::ostream & operator << ( std::ostream & os, const Kernel::Continuation::BackTraceElem & elem );

		class ForcedStructureContinuation : public Continuation
		{
		protected:
			const char * continuationName_;
		public:
			ForcedStructureContinuation( const char * continuationName, const Ast::SourceLocation & traceLoc );
			virtual ~ForcedStructureContinuation( );
			virtual void takeValue( const RefCountPtr< const Lang::Value > & val, Kernel::EvalState * evalState, bool dummy ) const;
			virtual void takeStructure( const RefCountPtr< const Lang::Structure > & structure, Kernel::EvalState * evalState ) const = 0;

			static RefCountPtr< const Lang::SingleList > findUnforced( RefCountPtr< const Lang::SingleList > lst );
		};

	}

	namespace Ast
	{

		class Node
		{
		protected:
			Node * parent_;
			const Ast::SourceLocation loc_;
		public:
			/* This flag is related to semantics.	It must be set for any expression whose evaluation may cause side effects.
			 * Evaluation of expressions with this flag set may not be delayed, and shall be evaluated in order of appearance.
			 */
			bool imperative_;

			Node( const Ast::SourceLocation & loc );
			virtual ~Node( );
			virtual void analyze( Ast::Node * parent, const Ast::AnalysisEnvironment * env ) = 0;
			virtual void eval( Kernel::EvalState * evalState ) const = 0;
			const Ast::SourceLocation & loc( ) const;
		};

		class Expression : public Node
		{
		public:
			/* This flag must have no influence on semantics.	It can be used for efficiency purposes only.
			 */
			bool immediate_;

			Expression( const Ast::SourceLocation & loc );
			virtual ~Expression( );
		};

		class BindNode : public Node
		{
		protected:
			Ast::SourceLocation idLoc_;
			const char * id_;
		public:
			BindNode( const Ast::SourceLocation & loc, const Ast::SourceLocation & idLoc, const char * id );
			virtual ~BindNode( );
			const char * id( ) const;
			const Ast::SourceLocation & idLoc( ) const;
		};

		class IdentifierNode
		{
		public:
			IdentifierNode( );
			virtual ~IdentifierNode( );
			virtual RefCountPtr< const char > identifier( Kernel::EvalState * evalState ) const = 0;
		};

		class ErrorExpression : public Expression
		{
		public:
			ErrorExpression( const Ast::SourceLocation & loc );
			virtual ~ErrorExpression( );
			virtual void analyze( Ast::Node * parent, const Ast::AnalysisEnvironment * env );
			virtual void eval( Kernel::EvalState * evalState ) const;
		};

	}

}

#endif
