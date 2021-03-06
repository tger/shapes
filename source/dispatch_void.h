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

#define UNARYDISPATCHBASEDECL static RefCountPtr< const Lang::Value > unaryDispatch( RefCountPtr< const Lang::Value > self, const ::Shapes::Ast::UnaryExpr * op );

#define UNARYDISPATCHBASEIMPL \
	RefCountPtr< const Lang::Value >																\
	Lang::Value::unaryDispatch( RefCountPtr< const Lang::Value > self, const ::Shapes::Ast::UnaryExpr * op ) \
{\
	return THE_VOID;\
}

#define UNARYDISPATCHDECL

#define UNARYDISPATCHIMPL( T )


#define BINARYDISPATCHBASEDECL static RefCountPtr< const Lang::Value > binaryDispatch1( RefCountPtr< const Lang::Value > self, RefCountPtr< const Lang::Value > other, const ::Shapes::Ast::BinaryInfixExpr * op );

#define BINARYDISPATCHBASEIMPL \
	RefCountPtr< const Lang::Value >																\
	Lang::Value::binaryDispatch1( RefCountPtr< const Lang::Value > self, RefCountPtr< const Lang::Value > other, const ::Shapes::Ast::BinaryInfixExpr * op ) \
{\
	return THE_VOID;\
}

#define BINARYDISPATCHDECL

#define BINARYDISPATCHIMPL( Ts )
