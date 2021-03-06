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

#include "shapescore.h"
#include "globals.h"

using namespace Shapes;

void
Kernel::registerClasses( Kernel::Environment * env )
{
	env->initDefineClass( Lang::THE_OBJECT );

	env->initDefineClass( Lang::Class::TypeID );

	env->initDefineClass( Lang::Void::TypeID );
	env->initDefineClass( Lang::Symbol::TypeID );
	env->initDefineClass( Lang::Float::TypeID );
	env->initDefineClass( Lang::Length::TypeID );
	env->initDefineClass( Lang::Boolean::TypeID );
	env->initDefineClass( Lang::String::TypeID );

	env->initDefineClass( Lang::FloatPair::TypeID );
	env->initDefineClass( Lang::Coords2D::TypeID );
	env->initDefineClass( Lang::CornerCoords2D::TypeID );
	env->initDefineClass( Lang::Path2D::TypeID );
	env->initDefineClass( Lang::MultiPath2D::TypeID );

	env->initDefineClass( Lang::FloatTriple::TypeID );
	env->initDefineClass( Lang::Coords3D::TypeID );
	env->initDefineClass( Lang::Path3D::TypeID );
	env->initDefineClass( Lang::MultiPath3D::TypeID );

	env->initDefineClass( Lang::Function::TypeID );
	env->initDefineClass( Lang::Transform2D::TypeID );
	env->initDefineClass( Lang::Transform3D::TypeID );

	env->initDefineClass( Lang::Drawable2D::TypeID );
	env->initDefineClass( Lang::Drawable3D::TypeID );
	env->initDefineClass( Lang::Color::TypeID );
}

