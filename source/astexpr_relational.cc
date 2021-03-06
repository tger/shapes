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

#include <cmath>

#include "astexprs.h"
#include "shapesexceptions.h"
#include "lighttypes.h"

using namespace Shapes;
using namespace std;


RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Symbol ) arg1, DUMMYANDREF( const Lang::Symbol ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( *arg1 < *arg2 ) );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Float ) arg1, DUMMYANDREF( const Lang::Float ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ < arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Integer ) arg1, DUMMYANDREF( const Lang::Integer ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ < arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Length ) arg1, DUMMYANDREF( const Lang::Length ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->get( ) < arg2->get( ) ) );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Coords2D ) arg1, DUMMYANDREF( const Lang::Coords2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint2D * res = new Lang::PathPoint2D( arg2 );
	res->rear_ = arg1;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Coords2D ) arg1, DUMMYANDREF( const Lang::PathPoint2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint2D * res = new Lang::PathPoint2D( *arg2 );
	if( res->rear_.getPtr( ) != 0 )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Pathpoint already has a rear handle" ) );
		}
	res->rear_ = arg1;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Coords2D ) arg1, DUMMYANDREF( const Lang::PathSlider2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	if( arg2->getRear( ) != NullPtr< const Lang::Value >( ) )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Slider already has a rear handle" ) );
		}
	return Kernel::ValueRef( new Lang::PathSlider2D_rear( *arg2, arg1 ) );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::PolarHandleBase ) arg1, DUMMYANDREF( const Lang::Coords2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint2D * res = new Lang::PathPoint2D( arg2 );
	res->rear_ = arg1;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::PolarHandleBase ) arg1, DUMMYANDREF( const Lang::PathPoint2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint2D * res = new Lang::PathPoint2D( *arg2 );
	if( res->rear_.getPtr( ) != 0 )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Pathpoint already has a rear handle" ) );
		}
	res->rear_ = arg1;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::PolarHandleBase ) arg1, DUMMYANDREF( const Lang::PathSlider2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	if( arg2->getRear( ) != NullPtr< const Lang::Value >( ) )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Slider already has a rear handle" ) );
		}
	return Kernel::ValueRef( new Lang::PathSlider2D_rear( *arg2, arg1 ) );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::CornerCoords2D ) arg1, DUMMYANDREF( const Lang::Coords2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	throw Exceptions::MiscellaneousRequirement( strrefdup( "A handle mustn't have a default angle." ) );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::CornerCoords2D ) arg1, DUMMYANDREF( const Lang::PathPoint2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	throw Exceptions::MiscellaneousRequirement( strrefdup( "A handle mustn't have a default angle." ) );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Coords3D ) arg1, DUMMYANDREF( const Lang::Coords3D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint3D * res = new Lang::PathPoint3D( arg2 );
	res->rear_ = arg1;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Coords3D ) arg1, DUMMYANDREF( const Lang::PathPoint3D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint3D * res = new Lang::PathPoint3D( *arg2 );
	if( res->rear_.getPtr( ) != 0 )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Pathpoint already has a rear handle" ) );
		}
	res->rear_ = arg1;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::LessExpr::impl( DUMMYANDREF( const Lang::Coords3D ) arg1, DUMMYANDREF( const Lang::PathSlider3D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	if( arg2->getRear( ) != NullPtr< const Lang::Value >( ) )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Slider already has a rear handle" ) );
		}
	return Kernel::ValueRef( new Lang::PathSlider3D_rear( *arg2, arg1 ) );
}


RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::Symbol ) arg1, DUMMYANDREF( const Lang::Symbol ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( *arg1 > *arg2 ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::Float ) arg1, DUMMYANDREF( const Lang::Float ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ > arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::Integer ) arg1, DUMMYANDREF( const Lang::Integer ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ > arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::Length ) arg1, DUMMYANDREF( const Lang::Length ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->get( ) > arg2->get( ) ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::Coords2D ) arg1, DUMMYANDREF( const Lang::Coords2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint2D * res = new Lang::PathPoint2D( arg1 );
	res->front_ = arg2;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::PathPoint2D ) arg1, DUMMYANDREF( const Lang::Coords2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint2D * res = new Lang::PathPoint2D( *arg1 );
	if( res->front_.getPtr( ) != 0 )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Pathpoint already has a front handle" ) );
		}
	res->front_ = arg2;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::PathSlider2D ) arg1, DUMMYANDREF( const Lang::Coords2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	if( arg1->getFront( ) != NullPtr< const Lang::Value >( ) )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Slider already has a front handle" ) );
		}
	return Kernel::ValueRef( new Lang::PathSlider2D_front( *arg1, arg2 ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::Coords2D ) arg1, DUMMYANDREF( const Lang::PolarHandleBase ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint2D * res = new Lang::PathPoint2D( arg1 );
	res->front_ = arg2;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::PathPoint2D ) arg1, DUMMYANDREF( const Lang::PolarHandleBase ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint2D * res = new Lang::PathPoint2D( *arg1 );
	if( res->front_.getPtr( ) != 0 )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Pathpoint already has a front handle" ) );
		}
	res->front_ = arg2;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::PathSlider2D ) arg1, DUMMYANDREF( const Lang::PolarHandleBase ) arg2, const Kernel::PassedDyn & dyn ) const
{
	if( arg1->getFront( ) != NullPtr< const Lang::Value >( ) )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Slider already has a front handle" ) );
		}
	return Kernel::ValueRef( new Lang::PathSlider2D_front( *arg1, arg2 ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::Coords2D ) arg1, DUMMYANDREF( const Lang::CornerCoords2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	throw Exceptions::MiscellaneousRequirement( strrefdup( "A handle mustn't have a default angle." ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::PathPoint2D ) arg1, DUMMYANDREF( const Lang::CornerCoords2D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	throw Exceptions::MiscellaneousRequirement( strrefdup( "A handle mustn't have a default angle." ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::Coords3D ) arg1, DUMMYANDREF( const Lang::Coords3D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint3D * res = new Lang::PathPoint3D( arg1 );
	res->front_ = arg2;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::PathPoint3D ) arg1, DUMMYANDREF( const Lang::Coords3D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	Lang::PathPoint3D * res = new Lang::PathPoint3D( *arg1 );
	if( res->front_.getPtr( ) != 0 )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Pathpoint already has a front handle" ) );
		}
	res->front_ = arg2;
	return Kernel::ValueRef( res );
}

RefCountPtr< const Lang::Value >
Ast::GreaterExpr::impl( DUMMYANDREF( const Lang::PathSlider3D ) arg1, DUMMYANDREF( const Lang::Coords3D ) arg2, const Kernel::PassedDyn & dyn ) const
{
	if( arg1->getFront( ) != NullPtr< const Lang::Value >( ) )
		{
			throw Exceptions::OutOfRange( this->loc( ), strrefdup( "Slider already has a front handle" ) );
		}
	return Kernel::ValueRef( new Lang::PathSlider3D_front( *arg1, arg2 ) );
}


RefCountPtr< const Lang::Value >
Ast::EqualExpr::impl( DUMMYANDREF( const Lang::Symbol ) arg1, DUMMYANDREF( const Lang::Symbol ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( *arg1 == *arg2 ) );
}

RefCountPtr< const Lang::Value >
Ast::EqualExpr::impl( DUMMYANDREF( const Lang::Float ) arg1, DUMMYANDREF( const Lang::Float ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ == arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::EqualExpr::impl( DUMMYANDREF( const Lang::Integer ) arg1, DUMMYANDREF( const Lang::Integer ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ == arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::EqualExpr::impl( DUMMYANDREF( const Lang::Length ) arg1, DUMMYANDREF( const Lang::Length ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->get( ) == arg2->get( ) ) );
}

RefCountPtr< const Lang::Value >
Ast::EqualExpr::impl( DUMMYANDREF( const Lang::Class ) arg1, DUMMYANDREF( const Lang::Class ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1 == arg2 ) );
}


RefCountPtr< const Lang::Value >
Ast::NotEqualExpr::impl( DUMMYANDREF( const Lang::Symbol ) arg1, DUMMYANDREF( const Lang::Symbol ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( *arg1 != *arg2 ) );
}

RefCountPtr< const Lang::Value >
Ast::NotEqualExpr::impl( DUMMYANDREF( const Lang::Float ) arg1, DUMMYANDREF( const Lang::Float ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ != arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::NotEqualExpr::impl( DUMMYANDREF( const Lang::Integer ) arg1, DUMMYANDREF( const Lang::Integer ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ != arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::NotEqualExpr::impl( DUMMYANDREF( const Lang::Length ) arg1, DUMMYANDREF( const Lang::Length ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->get( ) != arg2->get( ) ) );
}

RefCountPtr< const Lang::Value >
Ast::NotEqualExpr::impl( DUMMYANDREF( const Lang::Class ) arg1, DUMMYANDREF( const Lang::Class ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1 != arg2 ) );
}


RefCountPtr< const Lang::Value >
Ast::LessEqualExpr::impl( DUMMYANDREF( const Lang::Symbol ) arg1, DUMMYANDREF( const Lang::Symbol ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( *arg1 <= *arg2 ) );
}

RefCountPtr< const Lang::Value >
Ast::LessEqualExpr::impl( DUMMYANDREF( const Lang::Float ) arg1, DUMMYANDREF( const Lang::Float ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ <= arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::LessEqualExpr::impl( DUMMYANDREF( const Lang::Integer ) arg1, DUMMYANDREF( const Lang::Integer ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ <= arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::LessEqualExpr::impl( DUMMYANDREF( const Lang::Length ) arg1, DUMMYANDREF( const Lang::Length ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->get( ) <= arg2->get( ) ) );
}


RefCountPtr< const Lang::Value >
Ast::GreaterEqualExpr::impl( DUMMYANDREF( const Lang::Symbol ) arg1, DUMMYANDREF( const Lang::Symbol ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( *arg1 >= *arg2 ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterEqualExpr::impl( DUMMYANDREF( const Lang::Float ) arg1, DUMMYANDREF( const Lang::Float ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ >= arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterEqualExpr::impl( DUMMYANDREF( const Lang::Integer ) arg1, DUMMYANDREF( const Lang::Integer ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ >= arg2->val_ ) );
}

RefCountPtr< const Lang::Value >
Ast::GreaterEqualExpr::impl( DUMMYANDREF( const Lang::Length ) arg1, DUMMYANDREF( const Lang::Length ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->get( ) >= arg2->get( ) ) );
}


RefCountPtr< const Lang::Value >
Ast::AndExpr::impl( DUMMYANDREF( const Lang::Boolean ) arg1, DUMMYANDREF( const Lang::Boolean ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ && arg2->val_ ) );
}


RefCountPtr< const Lang::Value >
Ast::OrExpr::impl( DUMMYANDREF( const Lang::Boolean ) arg1, DUMMYANDREF( const Lang::Boolean ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ || arg2->val_ ) );
}


RefCountPtr< const Lang::Value >
Ast::XorExpr::impl( DUMMYANDREF( const Lang::Boolean ) arg1, DUMMYANDREF( const Lang::Boolean ) arg2, const Kernel::PassedDyn & dyn ) const
{
	return RefCountPtr< const Lang::Value >( new Lang::Boolean( arg1->val_ ^ arg2->val_ ) );
}
