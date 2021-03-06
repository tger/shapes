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

#ifndef facettypes_h
#define facettypes_h

#include "Shapes_Ast_decls.h"
#include "Shapes_Kernel_decls.h"
#include "Shapes_Lang_decls.h"
#include "Shapes_Computation_decls.h"

#include "ptrowner.h"
#include "refcount.h"
#include "environment.h"
#include "statetypes.h"
#include "elementarylength.h"
#include "concretecolors.h"

#include <list>
#include <iostream>
#include <stack>
#include <set>


namespace Shapes
{
	namespace Lang
	{

		class ReflectionsBinding : public DynamicBindings
		{
			Ast::SourceLocation loc_;
			RefCountPtr< const Lang::SpecularReflection > reflections_;
			const char * id_;
		public:
			ReflectionsBinding( const char * id, const Ast::SourceLocation & loc, const RefCountPtr< const Lang::SpecularReflection > & reflections );
			virtual ~ReflectionsBinding( );
			virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
			virtual void show( std::ostream & os ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked );
		};

		class AutoIntensityBinding : public DynamicBindings
		{
			Ast::SourceLocation loc_;
			RefCountPtr< const Lang::Color > color_;
			const char * id_;
		public:
			AutoIntensityBinding( const char * id, const Ast::SourceLocation & loc, const RefCountPtr< const Lang::Color > & color );
			virtual ~AutoIntensityBinding( );
			virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
			virtual void show( std::ostream & os ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked );
		};

		class AutoScatteringBinding : public DynamicBindings
		{
			Ast::SourceLocation loc_;
			RefCountPtr< const Lang::SpecularReflection > reflections_;
			const char * id_;
		public:
			AutoScatteringBinding( const char * id, const Ast::SourceLocation & loc, const RefCountPtr< const Lang::SpecularReflection > & reflections );
			virtual ~AutoScatteringBinding( );
			virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
			virtual void show( std::ostream & os ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked );
		};

		class ViewResolutionBinding : public DynamicBindings
		{
			Ast::SourceLocation loc_;
			Concrete::Length resolution_;
			const char * id_;
		public:
			ViewResolutionBinding( const char * id, const Ast::SourceLocation & loc, const Concrete::Length resolution );
			virtual ~ViewResolutionBinding( );
			virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
			virtual void show( std::ostream & os ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked );
		};

		class ShadeOrderBinding : public DynamicBindings
		{
			Ast::SourceLocation loc_;
			Computation::FacetShadeOrder order_;
			const char * id_;
		public:
			ShadeOrderBinding( const char * id, const Ast::SourceLocation & loc, const Computation::FacetShadeOrder order );
			virtual ~ShadeOrderBinding( );
			virtual void bind( MapType & bindings, Kernel::SystemDynamicVariables ** sysBindings ) const;
			virtual void show( std::ostream & os ) const;
			virtual void gcMark( Kernel::GCMarkedSet & marked );
		};

	}

	namespace Kernel
	{

		class ReflectionsDynamicVariableProperties : public Kernel::DynamicVariableProperties
		{
		public:
			ReflectionsDynamicVariableProperties( const char * name );
			virtual ~ReflectionsDynamicVariableProperties( );
			virtual Kernel::VariableHandle fetch( const Kernel::PassedDyn & dyn ) const;
			virtual void makeBinding( Kernel::VariableHandle val, const Ast::SourceLocation & idLoc, const Ast::SourceLocation & exprLoc, Kernel::EvalState * evalState ) const;
		};

		class AutoIntensityDynamicVariableProperties : public Kernel::DynamicVariableProperties
		{
		public:
			AutoIntensityDynamicVariableProperties( const char * name );
			virtual ~AutoIntensityDynamicVariableProperties( );
			virtual Kernel::VariableHandle fetch( const Kernel::PassedDyn & dyn ) const;
			virtual void makeBinding( Kernel::VariableHandle val, const Ast::SourceLocation & idLoc, const Ast::SourceLocation & exprLoc, Kernel::EvalState * evalState ) const;
		};

		class AutoScatteringDynamicVariableProperties : public Kernel::DynamicVariableProperties
		{
		public:
			AutoScatteringDynamicVariableProperties( const char * name );
			virtual ~AutoScatteringDynamicVariableProperties( );
			virtual Kernel::VariableHandle fetch( const Kernel::PassedDyn & dyn ) const;
			virtual void makeBinding( Kernel::VariableHandle val, const Ast::SourceLocation & idLoc, const Ast::SourceLocation & exprLoc, Kernel::EvalState * evalState ) const;
		};

		class ViewResolutionDynamicVariableProperties : public Kernel::DynamicVariableProperties
		{
		public:
			ViewResolutionDynamicVariableProperties( const char * name );
			virtual ~ViewResolutionDynamicVariableProperties( );
			virtual Kernel::VariableHandle fetch( const Kernel::PassedDyn & dyn ) const;
			virtual void makeBinding( Kernel::VariableHandle val, const Ast::SourceLocation & idLoc, const Ast::SourceLocation & exprLoc, Kernel::EvalState * evalState ) const;
		};

		class ShadeOrderDynamicVariableProperties : public Kernel::DynamicVariableProperties
		{
		public:
			ShadeOrderDynamicVariableProperties( const char * name );
			virtual ~ShadeOrderDynamicVariableProperties( );
			virtual Kernel::VariableHandle fetch( const Kernel::PassedDyn & dyn ) const;
			virtual void makeBinding( Kernel::VariableHandle val, const Ast::SourceLocation & idLoc, const Ast::SourceLocation & exprLoc, Kernel::EvalState * evalState ) const;
		};

		class FacetState
		{
		public:
			RefCountPtr< const Lang::SpecularReflection > reflections_;		 // Use SpecularReflectionNumm for no-op, and NullPtr for undefined
			RefCountPtr< const Lang::Color > autoIntensity_;								// Use Gray( -1 ) for no-op, and NullPtr for undefined
			RefCountPtr< const Lang::SpecularReflection > autoScattering_;	// Use SpecularReflectionNumm for no-op, and NullPtr for undefined
			Concrete::Length viewResolution_;															 // Use NaN for undefined
			Computation::FacetShadeOrder shadeOrder_;											 // Use -1 for undefined
		public:
			FacetState( );
			explicit FacetState( const Kernel::FacetState & orig );	 // explicit, since reference counting shall be used in most cases
			FacetState( const Kernel::FacetState & newValues, const Kernel::FacetState & oldValues );
			FacetState( bool setDefaults );
			~FacetState( );
		};

	}

}

#endif
