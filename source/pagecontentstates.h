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

#ifndef pagecontentstates_h
#define pagecontentstates_h


#include "statetypes.h"
#include "texttypes.h"
#include "simplepdfo.h"

namespace Shapes
{
	namespace Kernel
	{

		class PageContentStates
		{
		public:
			Kernel::GraphicsState graphics_;
			Kernel::TextState text_;
			RefCountPtr< SimplePDF::PDF_Resources > resources_;

			PageContentStates( RefCountPtr< SimplePDF::PDF_Resources > & resources );
			PageContentStates( RefCountPtr< SimplePDF::PDF_Resources > & resources, bool setDefaults );
			~PageContentStates( );
		};
 
 }
}


#endif


