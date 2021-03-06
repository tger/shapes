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

#ifndef shapesscanner_h
#define shapesscanner_h

#include "sourcelocation.h"
#include "charptrless.h"
#include "elementarylength.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stack>
#include <map>
#include <set>
#include <list>
#include <string>

#ifndef FLEXINT_H								// Else *FlexLexer will be defined twice
#	undef yyFlexLexer
#	define yyFlexLexer shapesFlexLexer
#	include <FlexLexer.h>
#endif

class ShapesScanner : public shapesFlexLexer
{
	struct FileID
	{
		dev_t st_dev;
		ino_t st_ino;
		FileID( const struct stat & stat ) : st_dev( stat.st_dev ), st_ino( stat.st_ino ) { }
		bool operator < ( const FileID & other ) const
		{
			if( st_dev < other.st_dev )
				{
					return true;
				}
			if( st_dev > other.st_dev )
				{
					return false;
				}
			return st_ino < other.st_ino;
		}
	};
	std::stack< yy_buffer_state * > stateStack;
	std::stack< ::Shapes::Ast::SourceLocation > locStack;
	std::stack< size_t > pathCountStack;
	unsigned int quoteDepth;
	bool moreState;
	int lastleng;
	void more( );
	std::map< const char *, double, charPtrLess > unitTable;
	const char * newUnitName;
	std::set< FileID > neededFiles;
	std::string sourceDir_;
	std::list< std::string > needSearchPath;
	bool inPrelude_;
	std::istream * preludeFile_;
	std::list< std::string >::const_iterator preludeIterator_;
	std::list< std::pair< std::istream *, const char * > > yyinQueue_;
	bool showFiles;
	bool randSeedSet;
	std::list< std::pair< char *, size_t > > dataStringChunks_;
	size_t dataStringTotalLength_;
 public:
	//	Ast::SourceLocation loc;
	ShapesScanner( );
	virtual ~ShapesScanner( );
	void start( );
	void queueStream( std::istream * is, const char * yyinName );
	void setSourceDir( const std::string & sourceDir );
	void push_backNeedPath( const std::string & path );
	std::string searchFile( const std::string & suffix ) const;
	void setShowFiles( bool _showFiles );
	virtual int yylex( );
	void doBeforeEachAction( );

	double lookupUnitFactor( const char * name ) const;
	Shapes::Concrete::Length strtoLength( const char * str ) const;
 private:
	std::string currentNeedFile;
	size_t currentNeedPushCount;
	bool currentNeedIsNeed;
	void push_frontNeedPath( const std::string & path );
	void pop_frontNeedPath( );
	void doInclusion( );
	void rinseString( );
	void concatenateDataString( );
	std::string needpathWithSuffix( const std::string & needpath, const std::string & suffix ) const;
};



#endif
