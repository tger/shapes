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

#ifndef characterencoding_h
#define characterencoding_h


#include "ptrowner.h"
#include "charptrless.h"

#include <vector>
#include <map>
#include <iostream>

// This may not be the appropriate namespace...
namespace FontMetrics
{

	class CharacterEncoding
	{
	public:
		// A character encoding is a primitive thing which can only handle 256 code points.
		typedef unsigned char PositionType;
	private:
		RefCountPtr< const char > encodingName_;

		// This is only a memory, with no particular order of its items.
		PtrOwner_back_Access< std::vector< const char * > > nameMem_;

		// This is idexed by size_t and maps to a name or a null pointer.
		static const size_t TABLE_SIZE = 256;
		std::vector< const char * > namePtrs_;

		// This is the reverse map.
		std::map< const char *, PositionType, charPtrLess > nameMap_;

	public:
		CharacterEncoding( std::istream & iFile );
		~CharacterEncoding( );

		// Returns true on success.
		bool position_to_name( PositionType code, const char ** dst ) const;

		// Returns true on success.
		bool name_to_position( const char * name, PositionType * dst ) const;

	private:
		void readfile( std::istream & iFile );
		static void getToken( std::istream & iFile, std::string * dst );
	};

}

#endif
