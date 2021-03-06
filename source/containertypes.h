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

#ifndef containertypes_h
#define containertypes_h

#include <list>
#include <iostream>
#include <stack>
#include <set>

#include "ptrowner.h"
#include "refcount.h"
#include "pdfstructure.h"
#include "shapesvalue.h"
#include "environment.h"
#include "charptrless.h"

namespace Shapes
{
	namespace Lang
	{

		class SingleList : public Lang::NoOperatorOverloadValue
		{
		public:
			SingleList( );
			virtual ~SingleList( );
			virtual bool isNull( ) const = 0;
			virtual void foldl( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, const Ast::SourceLocation & callLoc ) const = 0;
			virtual void foldr( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, const Ast::SourceLocation & callLoc ) const = 0;
			virtual void foldsl( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, Kernel::StateHandle state, const Ast::SourceLocation & callLoc ) const = 0;
			virtual void foldsr( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, Kernel::StateHandle state, const Ast::SourceLocation & callLoc ) const = 0;
			virtual Kernel::VariableHandle getField( const char * fieldID, const RefCountPtr< const Lang::Value > & selfRef ) const;
			TYPEINFODECL;
		};

		class SingleListPair : public Lang::SingleList
		{
		public:
			/* The data is provided public becase it is used in function application
			 */
			Kernel::VariableHandle car_;
			RefCountPtr< const Lang::SingleList > cdr_;

			SingleListPair( const Kernel::VariableHandle & car, const RefCountPtr< const Lang::SingleList > & cdr );
			virtual ~SingleListPair( );
			virtual bool isNull( ) const;
			virtual void foldl( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, const Ast::SourceLocation & callLoc ) const;
			virtual void foldr( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, const Ast::SourceLocation & callLoc ) const;
			virtual void foldsl( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, Kernel::StateHandle state, const Ast::SourceLocation & callLoc ) const;
			virtual void foldsr( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, Kernel::StateHandle state, const Ast::SourceLocation & callLoc ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked );
		};

		class SingleListNull : public Lang::SingleList
		{
		public:
			SingleListNull( );
			virtual ~SingleListNull( );
			virtual bool isNull( ) const;
			virtual void foldl( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, const Ast::SourceLocation & callLoc ) const;
			virtual void foldr( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, const Ast::SourceLocation & callLoc ) const;
			virtual void foldsl( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, Kernel::StateHandle state, const Ast::SourceLocation & callLoc ) const;
			virtual void foldsr( Kernel::EvalState * evalState, const RefCountPtr< const Lang::Function > & op, const Kernel::VariableHandle & nullResult, Kernel::StateHandle state, const Ast::SourceLocation & callLoc ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked ){ };
		};

		class Structure : public Lang::NoOperatorOverloadValue
		{
			bool argListOwner_;
		public:
			/* The data is provided public becase it is used in function application
			 */
			const Ast::ArgListExprs * argList_;
			RefCountPtr< const Lang::SingleList > values_;
			Structure( const Ast::ArgListExprs * argList, const RefCountPtr< const Lang::SingleList > & values, bool argListOwner = false );
			virtual ~Structure( );
			virtual Kernel::VariableHandle getField( const char * fieldID, const RefCountPtr< const Lang::Value > & selfRef ) const;
			Kernel::VariableHandle getPosition( size_t pos, const RefCountPtr< const Lang::Value > & selfRef ) const;
			RefCountPtr< const Lang::Structure > getSink( size_t consumedArguments ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked );
			TYPEINFODECL;
		};

		class ConsPair : public Lang::NoOperatorOverloadValue
		{
			Kernel::VariableHandle car_;
			Kernel::VariableHandle cdr_;
		public:
			ConsPair( const Kernel::VariableHandle & car, const Kernel::VariableHandle & cdr );
			virtual ~ConsPair( );
			virtual void show( std::ostream & os ) const;
			virtual Kernel::VariableHandle getField( const char * fieldID, const RefCountPtr< const Lang::Value > & selfRef ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked );
			TYPEINFODECL;
		};

	}

	namespace Kernel
	{
		class StructureFactory
		{
			const Ast::ArgListExprs * argList_;
			std::map< const char *, Kernel::VariableHandle > values_;
		private:
			void init( const std::list< const char * > & fields );
		public:
			StructureFactory( const std::list< const char * > & fields );
			StructureFactory( const char * field1 );
			StructureFactory( const char * field1, const char * field2 );
			StructureFactory( const char * field1, const char * field2, const char * field3 );
			StructureFactory( const char * field1, const char * field2, const char * field3, const char * field4 );
			void clear( );
			void set( const char * field, const Kernel::VariableHandle & value );
			RefCountPtr< const Lang::Structure > build( );
		};
	}
}

#endif
