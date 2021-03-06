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

#include <iomanip>
#include <stdio.h>
#include <zlib.h>

#include "pdfstructure.h"
#include "simplepdfo.h"
#include "simplepdfi.h"
#include "autoonoff.h"

using namespace std;

using namespace SimplePDF;

/****************/

SimplePDF::PDF_Object::PDF_Object( )
	: complete( false )
{ }
SimplePDF::PDF_Object::~PDF_Object( )
{ }
void
SimplePDF::PDF_Object::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	throw "This function, PDF_Object::writeTo, must not be called.	It is practically purely virtual.";
}
RefCountPtr< PDF_Object >
SimplePDF::PDF_Object::deepCopy( RefCountPtr< PDF_Object > self, size_t * indirectObjectCounter, IndirectRemapType * remap )
{
	return self;
}

/****************/

template< class S >
RefCountPtr< S >
SimplePDF::down_cast_follow( RefCountPtr< PDF_Object > maybeIndirect )
{
	{
		const PDF_Indirect_in * theIndirect( dynamic_cast< const PDF_Indirect_in * >( maybeIndirect.getPtr( ) ) );
		if( theIndirect != 0 )
			{
				return theIndirect->PDFin->follow< S >( maybeIndirect );
			}
	}
	{
		const PDF_Indirect_out * theIndirect( dynamic_cast< const PDF_Indirect_out * >( maybeIndirect.getPtr( ) ) );
		if( theIndirect != 0 )
			{
				return SimplePDF::down_cast_follow< S >( theIndirect->obj );
			}
	}

	return maybeIndirect.down_cast< S >( );
}

namespace SimplePDF
{
	template< >
	RefCountPtr< SimplePDF::PDF_Float >
	down_cast_follow< SimplePDF::PDF_Float >( RefCountPtr< PDF_Object > maybeIndirect )
	{
		{
			const PDF_Indirect_in * theIndirect( dynamic_cast< const PDF_Indirect_in * >( maybeIndirect.getPtr( ) ) );
			if( theIndirect != 0 )
				{
					return theIndirect->PDFin->follow< SimplePDF::PDF_Float >( maybeIndirect );
				}
		}
		{
			const PDF_Indirect_out * theIndirect( dynamic_cast< const PDF_Indirect_out * >( maybeIndirect.getPtr( ) ) );
			if( theIndirect != 0 )
				{
					return SimplePDF::down_cast_follow< SimplePDF::PDF_Float >( theIndirect->obj );
				}
		}
		{
			RefCountPtr< PDF_Int > res( maybeIndirect.down_cast< PDF_Int >( ) );
			if( res != NullPtr< PDF_Int >( ) )
				{
					return RefCountPtr< PDF_Float >( new PDF_Float( res->value( ) ) );
				}
		}
		return maybeIndirect.down_cast< SimplePDF::PDF_Float >( );
	}
}

namespace SimplePDF
{
	template RefCountPtr< SimplePDF::PDF_Vector > down_cast_follow< SimplePDF::PDF_Vector >( RefCountPtr< PDF_Object > );
}

/****************/

SimplePDF::PDF_Null::PDF_Null( )
{ }
SimplePDF::PDF_Null::~PDF_Null( )
{ }

void
SimplePDF::PDF_Null::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	os << "null" ;
}

/****************/

SimplePDF::PDF_Boolean::PDF_Boolean( ValueType _value )
	: my_value( _value )
{ }
SimplePDF::PDF_Boolean::~PDF_Boolean( )
{ }

SimplePDF::PDF_Boolean::ValueType
SimplePDF::PDF_Boolean::value( ) const
{
	return my_value;
}

void
SimplePDF::PDF_Boolean::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	if( my_value )
		{
			os << "true" ;
		}
	else
		{
			os << "false" ;
		}
}

/****************/

SimplePDF::PDF_Int::PDF_Int( PDF_Int::ValueType _value )
	: my_value( _value )
{ }
SimplePDF::PDF_Int::PDF_Int( const char * strvalue )
{
	/* How can it be that end shall be non-const, while strvalue is const? */
	//	const char * end;
	char * end;
	my_value = strtol( strvalue, & end, 10 );
	if( *end != '\0' )
		{
			throw "Internal error: Bad numeric int format";
		}
}
SimplePDF::PDF_Int::~PDF_Int( )
{ }

PDF_Int::ValueType
SimplePDF::PDF_Int::value( ) const
{
	return my_value;
}

void
SimplePDF::PDF_Int::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	os << my_value ;
}

/****************/

SimplePDF::PDF_Float::PDF_Float( PDF_Float::ValueType _value )
	: my_value( _value )
{ }
SimplePDF::PDF_Float::PDF_Float( const char * strvalue )
{
	/* How can it be that end shall be non-const, while strvalue is const? */
	//	const char * end;
	char * end;
	my_value = strtod( strvalue, & end );
	if( *end != '\0' )
		{
			throw( "Internal error: Bad numeric float format" );
		}
}
SimplePDF::PDF_Float::~PDF_Float( )
{ }

SimplePDF::PDF_Float::ValueType PDF_Float::value( ) const
{
	return my_value;
}

void
SimplePDF::PDF_Float::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	os << my_value ;
}

/****************/

SimplePDF::PDF_String::PDF_String( )
{ }
SimplePDF::PDF_String::~PDF_String( )
{ }

/****************/

SimplePDF::PDF_LiteralString::PDF_LiteralString( const string & _str )
	: my_str( _str )
{ }
SimplePDF::PDF_LiteralString::PDF_LiteralString( const char * _str )
	: my_str( _str )
{ }
SimplePDF::PDF_LiteralString::PDF_LiteralString( const list< RefCountPtr< char > > & strs )
{
	size_t length( 0 );
	typedef list< RefCountPtr< char > >::const_iterator I;
	for( I i( strs.begin( ) ); i != strs.end( ); ++i )
		{
			length += strlen( i->getPtr( ) );
		}
	RefCountPtr< char > mem( new char[ length + 1 ] );
	char * dst = mem.getPtr( );
	for( I i( strs.begin( ) ); i != strs.end( ); ++i )
		{
			strcpy( dst, i->getPtr( ) );
			dst += strlen( i->getPtr( ) );
		}
	*dst = '\0';
	my_str = mem.getPtr( );
}
SimplePDF::PDF_LiteralString::~PDF_LiteralString( )
{ }

const string &
SimplePDF::PDF_LiteralString::str( ) const
{
	return my_str;
}

void
SimplePDF::PDF_LiteralString::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	os << "(" ;
	for( const char * src = my_str.c_str( ); *src != '\0'; ++src )
		{
			switch( *src )
				{
				case '(':
					os << "\\(" ;
					break;
				case ')':
					os << "\\)" ;
					break;
				case '\\':
					os << "\\\\" ;
					break;
				default:
					os << *src ;
				}
		}
	os << ")" ;
}

/****************/

SimplePDF::PDF_HexString::PDF_HexString( const string & _hexstr )
	: my_hexstr( _hexstr )
{ }
SimplePDF::PDF_HexString::PDF_HexString( const char * _hexstr )
	: my_hexstr( _hexstr )
{ }
SimplePDF::PDF_HexString::~PDF_HexString( )
{ }

const string &
SimplePDF::PDF_HexString::hexstr( ) const
{
	return my_hexstr;
}

void
SimplePDF::PDF_HexString::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	os << "<" << my_hexstr << ">" ;
}

/****************/

SimplePDF::PDF_Name::PDF_Name( const string & _name )
	: my_name( _name )
{ }
SimplePDF::PDF_Name::~PDF_Name( )
{ }

const string &
SimplePDF::PDF_Name::name( ) const
{
	return my_name;
}

void
SimplePDF::PDF_Name::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	os << "/" << my_name ;
}

ostream & SimplePDF::operator << ( std::ostream & os, const PDF_Name & self )
{
	return os << "/" << self.my_name ;
}


/****************/

SimplePDF::PDF_Vector::PDF_Vector( )
{ }
SimplePDF::PDF_Vector::PDF_Vector( PDF_Float::ValueType c )
{
	vec.push_back( SimplePDF::newFloat( c ) );
}
SimplePDF::PDF_Vector::PDF_Vector( PDF_Float::ValueType c1, PDF_Float::ValueType c2 )
{
	vec.push_back( SimplePDF::newFloat( c1 ) );
	vec.push_back( SimplePDF::newFloat( c2 ) );
}
SimplePDF::PDF_Vector::PDF_Vector( PDF_Float::ValueType c1, PDF_Float::ValueType c2, PDF_Float::ValueType c3 )
{
	vec.push_back( SimplePDF::newFloat( c1 ) );
	vec.push_back( SimplePDF::newFloat( c2 ) );
	vec.push_back( SimplePDF::newFloat( c3 ) );
}
SimplePDF::PDF_Vector::PDF_Vector( PDF_Float::ValueType x1, PDF_Float::ValueType y1,
																	 PDF_Float::ValueType x2, PDF_Float::ValueType y2 )
{
	vec.push_back( SimplePDF::newFloat( x1 ) );
	vec.push_back( SimplePDF::newFloat( y1 ) );
	vec.push_back( SimplePDF::newFloat( x2 ) );
	vec.push_back( SimplePDF::newFloat( y2 ) );
}
SimplePDF::PDF_Vector::PDF_Vector( PDF_Float::ValueType a, PDF_Float::ValueType b,
																	 PDF_Float::ValueType c, PDF_Float::ValueType d,
																	 PDF_Float::ValueType e, PDF_Float::ValueType f )
{
	vec.push_back( SimplePDF::newFloat( a ) );
	vec.push_back( SimplePDF::newFloat( b ) );
	vec.push_back( SimplePDF::newFloat( c ) );
	vec.push_back( SimplePDF::newFloat( d ) );
	vec.push_back( SimplePDF::newFloat( e ) );
	vec.push_back( SimplePDF::newFloat( f ) );
}
SimplePDF::PDF_Vector::PDF_Vector( PDF_Int::ValueType x1, PDF_Int::ValueType y1,
																	 PDF_Int::ValueType x2, PDF_Int::ValueType y2 )
{
	vec.push_back( SimplePDF::newInt( x1 ) );
	vec.push_back( SimplePDF::newInt( y1 ) );
	vec.push_back( SimplePDF::newInt( x2 ) );
	vec.push_back( SimplePDF::newInt( y2 ) );
}
SimplePDF::PDF_Vector::~PDF_Vector( )
{ }

void
SimplePDF::PDF_Vector::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	os << "[" ;
	for( VecType::const_iterator i( vec.begin( ) ); i != vec.end( ); ++i )
		{
			os << " " ;
			(*i)->writeTo( os, xref, *i );
		}
	os << " ]" ;
}

RefCountPtr< PDF_Object >
SimplePDF::PDF_Vector::deepCopy( RefCountPtr< PDF_Object > self, size_t * indirectObjectCounter, IndirectRemapType * remap )
{
	bool allSame( true );
	RefCountPtr< PDF_Vector > res( new PDF_Vector );
	res->vec.reserve( vec.size( ) );
	for( VecType::iterator i( vec.begin( ) ); i != vec.end( ); ++i )
		{
			RefCountPtr< PDF_Object > copy( ::deepCopy( *i, indirectObjectCounter, remap ) );
			res->vec.push_back( copy );
			if( allSame && copy != *i )
				{
					allSame = false;
				}
		}
	if( allSame )
		{
			return self;
		}
	return res;
}

RefCountPtr< SimplePDF::PDF_Vector >
SimplePDF::PDF_Vector::rectangleIntersection( const RefCountPtr< SimplePDF::PDF_Vector > & other ) const
{
	if( vec.size( ) != 4 )
		{
			throw "Internal error: PDF_Vector::rectangleIntersection: self size is not 4.";
		}
	if( other->vec.size( ) != 4 )
		{
			throw "Internal error: PDF_Vector::rectangleIntersection: self size is not 4.";
		}

	RefCountPtr< PDF_Vector > res( new PDF_Vector );
	res->vec.reserve( vec.size( ) );

	typedef typeof vec VectorType;
	typedef VectorType::const_iterator I;
	I src1 = vec.begin( );
	I src2 = other->vec.begin( );
	res->vec.push_back( RefCountPtr< PDF_Float >( new PDF_Float( max( SimplePDF::down_cast_follow< PDF_Float >( *src1 )->value( ), SimplePDF::down_cast_follow< PDF_Float >( *src2 )->value( ) ) ) ) );
	++src1;
	++src2;
	res->vec.push_back( RefCountPtr< PDF_Float >( new PDF_Float( max( SimplePDF::down_cast_follow< PDF_Float >( *src1 )->value( ), SimplePDF::down_cast_follow< PDF_Float >( *src2 )->value( ) ) ) ) );
	++src1;
	++src2;
	res->vec.push_back( RefCountPtr< PDF_Float >( new PDF_Float( min( SimplePDF::down_cast_follow< PDF_Float >( *src1 )->value( ), SimplePDF::down_cast_follow< PDF_Float >( *src2 )->value( ) ) ) ) );
	++src1;
	++src2;
	res->vec.push_back( RefCountPtr< PDF_Float >( new PDF_Float( min( SimplePDF::down_cast_follow< PDF_Float >( *src1 )->value( ), SimplePDF::down_cast_follow< PDF_Float >( *src2 )->value( ) ) ) ) );

	return res;
}


/****************/

SimplePDF::PDF_Dictionary::PDF_Dictionary( )
{ }
SimplePDF::PDF_Dictionary::PDF_Dictionary( const PDF_Dictionary & orig )
	: dic( orig.dic )
{ }
SimplePDF::PDF_Dictionary::~PDF_Dictionary( )
{ }

void
SimplePDF::PDF_Dictionary::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	os << "<< " ;
	for( DicType::const_iterator i( dic.begin( ) ); i != dic.end( ); ++i )
		{
			os << "/" << (i->first) << " " ;
			(i->second)->writeTo( os, xref, i->second );
			os << endl ;
		}
	os << ">>" ;
}

void
SimplePDF::PDF_Dictionary::writeToWithLength( std::ostream & os, SimplePDF::PDF_xref * xref, size_t len ) const
{
	os << "<< " ;
	os << "/Length " << len << endl ;
	for( DicType::const_iterator i( dic.begin( ) ); i != dic.end( ); ++i )
		{
			os << "/" << (i->first) << " " ;
			(i->second)->writeTo( os, xref, i->second );
			os << endl ;
		}
	os << ">>" ;
}

PDF_Int::ValueType
SimplePDF::PDF_Dictionary::getLength( ) const
{
	DicType::const_iterator i( dic.find( string( "Length" ) ) );
	if( i == dic.end( ) )
		{
			throw( "Dictionary[Length]: /Length field missing" );
		}
	RefCountPtr< const PDF_Int > theInt = SimplePDF::down_cast_follow< const PDF_Int >( i->second );
	if( theInt == NullPtr< const PDF_Int >( ) )
		{
			throw( "Dictionary[Length]: /Length is not a PDF_Int" );
		}
	return theInt->value( );
}

PDF_Int::ValueType
SimplePDF::PDF_Dictionary::getCount( ) const
{
	DicType::const_iterator i( dic.find( string( "Count" ) ) );
	if( i == dic.end( ) )
		{
			throw( "Dictionary[Count]: /Count field missing" );
		}
	RefCountPtr< const PDF_Int > theInt = SimplePDF::down_cast_follow< const PDF_Int >( i->second );
	if( theInt == NullPtr< const PDF_Int >( ) )
		{
			throw( "Dictionary[Count]: /Count is not a PDF_Int" );
		}
	return theInt->value( );
}

bool
SimplePDF::PDF_Dictionary::isPages( ) const
{
	DicType::const_iterator i( dic.find( string( "Type" ) ) );
	if( i == dic.end( ) )
		{
			throw( "PDF_Dictionary::isPages: /Type field missing" );
		}
	const PDF_Name * theType( dynamic_cast< const PDF_Name * >( i->second.getPtr( ) ) );
	if( theType == 0 )
		{
			throw( "PDF_Dictionary::isPages: /Type is not a PDF_Name" );
		}
	return theType->name( ) == "Pages";
}

RefCountPtr< SimplePDF::PDF_Object >
SimplePDF::PDF_Dictionary::getInheritable( const char * name ) const
{
	{
		DicType::const_iterator i( dic.find( string( name ) ) );
		if( i != dic.end( ) )
			{
				return i->second;
			}
	}
	{
		DicType::const_iterator i( dic.find( string( "Parent" ) ) );
		if( i != dic.end( ) )
			{
				return SimplePDF::down_cast_follow< SimplePDF::PDF_Dictionary >( i->second )->getInheritable( name );
			}
	}
	return RefCountPtr< SimplePDF::PDF_Object >( NullPtr< SimplePDF::PDF_Object >( ) );
}


RefCountPtr< PDF_Object > &
SimplePDF::PDF_Dictionary::operator [] ( const char * key )
{
	DicType::iterator i( dic.find( key ) );
	if( i == dic.end( ) )
		{
			return ((dic.insert(DicType::value_type( key, RefCountPtr< PDF_Object >( new PDF_Null( ) ) ))).first)->second;
		}
	return i->second;
}

bool
SimplePDF::PDF_Dictionary::hasKey( const char * key ) const
{
	return dic.find( key ) != dic.end( );
}

RefCountPtr< PDF_Object >
SimplePDF::PDF_Dictionary::deepCopy( RefCountPtr< PDF_Object > self, size_t * indirectObjectCounter, IndirectRemapType * remap )
{
	bool allSame( true );
	RefCountPtr< PDF_Dictionary > res( new PDF_Dictionary );
	for( DicType::iterator i( dic.begin( ) ); i != dic.end( ); ++i )
		{
			RefCountPtr< PDF_Object > copy( ::deepCopy( i->second, indirectObjectCounter, remap ) );
			res->dic[ i->first ] = copy;
			if( allSame && copy != i->second )
				{
					allSame = false;
				}
		}
	if( allSame )
		{
			return self;
		}
	return res;
}

/****************/

SimplePDF::PDF_Stream::PDF_Stream( )
{ }
SimplePDF::PDF_Stream::PDF_Stream( const PDF_Dictionary & dic )
	: PDF_Dictionary( dic )
{ }
SimplePDF::PDF_Stream::~PDF_Stream( )
{ }

void
SimplePDF::PDF_Stream::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	throw "PDF_Stream::writeTo called";
}

RefCountPtr< PDF_Object >
SimplePDF::PDF_Stream::deepCopy( RefCountPtr< PDF_Object > self, size_t * indirectObjectCounter, IndirectRemapType * remap )
{
	throw "PDF_Stream::deepCopy must not be called.	(However PDF_Stream_in::deepCopy may be called.)";
}
/****************/

SimplePDF::PDF_Stream_out::PDF_Stream_out( )
{
	data << std::setiosflags( std::ios_base::fixed ) ;
}
SimplePDF::PDF_Stream_out::~PDF_Stream_out( )
{ }

void
SimplePDF::PDF_Stream_out::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	const string & dataStr( data.str( ) );
	PDF_Dictionary::writeToWithLength( os, xref, dataStr.size( ) );
	os << endl << "stream" << endl ;
	os << dataStr << endl ;
	os << "endstream " ;
}

/****************/

SimplePDF::PDF_Stream_in::PDF_Stream_in( PDF_Dictionary * dic, istream * _is, streamoff _dataStart )
	: PDF_Stream( *dic ), is( _is ), dataStart( _dataStart )
{ }
SimplePDF::PDF_Stream_in::PDF_Stream_in( istream * _is, streamoff _dataStart )
	: is( _is ), dataStart( _dataStart )
{ }
SimplePDF::PDF_Stream_in::~PDF_Stream_in( )
{ }

void
SimplePDF::PDF_Stream_in::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	size_t length( PDF_Dictionary::getLength( ) );
	PDF_Dictionary::writeTo( os, xref, self );
	os << endl << "stream" << endl ;
	RefCountPtr< char > buf( new char[ length ] );
	is->seekg( dataStart, ios::beg );
	is->read( buf.getPtr( ), length );
	os.write( buf.getPtr( ), length );
	os << endl << "endstream " ;
}
void
SimplePDF::PDF_Stream_in::writeDataTo( std::ostream & os ) const
{
	size_t length( PDF_Dictionary::getLength( ) );
	RefCountPtr< char > buf( new char[ length ] );
	is->seekg( dataStart, ios::beg );
	is->read( buf.getPtr( ), length );
	os.write( buf.getPtr( ), length );
}
void
SimplePDF::PDF_Stream_in::writeDataDefilteredTo( std::ostream & os ) const
{
	if( dic.find( "Filter" ) == dic.end( ) )
		{
			writeDataTo( os );
			return;
		}

	RefCountPtr< PDF_Object > filterEntry = RefCountPtr< PDF_Object >( new PDF_Null( ) );
	typedef typeof dic DicType;
	DicType::const_iterator i = dic.find( "Filter" );
	if( i != dic.end( ) )
		{
			filterEntry = i->second;
		}
	if( filterEntry.down_cast< PDF_Null >( ) != NullPtr< PDF_Null >( ) )
		{
			writeDataTo( os );
			return;
		}

	RefCountPtr< PDF_Name > filterNameObj( filterEntry.down_cast< PDF_Name >( ) );
	if( filterNameObj == NullPtr< PDF_Name >( ) )
		{
			throw( "The Filter entry was not a name (nor null) in the stream dictionary" );
		}
	const string & filter( filterNameObj->name( ) );

	if( filter == "FlateDecode" )
		{
			size_t inBufSize( PDF_Dictionary::getLength( ) );
			RefCountPtr< char > inBuf( new char[ inBufSize ] );
			is->seekg( dataStart, ios::beg );
			is->read( inBuf.getPtr( ), inBufSize );

			const size_t BUFSIZE( 1024 );
			RefCountPtr< char > outBuf( new char[ BUFSIZE ] );

			z_stream zs;
			zs.next_in = reinterpret_cast< Bytef * >( inBuf.getPtr( ) );
			zs.avail_in = inBufSize;
			zs.next_out = reinterpret_cast< Bytef * >( outBuf.getPtr( ) );
			zs.avail_out = BUFSIZE;
			zs.zalloc = Z_NULL;
			zs.zfree = Z_NULL;
			switch( inflateInit( & zs ) )
				{
				case	Z_OK:
					break;
				default:
					throw( "inflateInit failed" );
				}

			{
				int status;
				while( ( status = inflate( & zs, Z_NO_FLUSH ) ) == Z_OK )
					{
						os.write( outBuf.getPtr( ), BUFSIZE - zs.avail_out );
						zs.next_out = reinterpret_cast< Bytef * >( outBuf.getPtr( ) );
						zs.avail_out = BUFSIZE;
					}
				switch( status )
					{
					case Z_STREAM_END:
						os.write( outBuf.getPtr( ), BUFSIZE - zs.avail_out );
						break;
					default:
						throw( "The inflate cycle did not finish with Z_STREAM_END" );
					}
			}

			switch( inflateEnd( & zs ) )
				{
				case	Z_OK:
					break;
				default:
					throw( "inflateEnd failed" );
				}
		}
	else
		{
			throw( "The filter " + filter + " is not yet supported" );
		}
}

RefCountPtr< PDF_Object >
SimplePDF::PDF_Stream_in::deepCopy( RefCountPtr< PDF_Object > self, size_t * indirectObjectCounter, IndirectRemapType * remap )
{
	RefCountPtr< PDF_Object > dicCopy( PDF_Dictionary::deepCopy( self, indirectObjectCounter, remap ) );
	if( dicCopy.getPtr( ) == this )
		{
			return self;
		}
	/* By increasing the count by 1, we ensure that the memory is not destroyed when this function exits.
	 * We use that this object is newly created, so there are no other references to it.
	 */
	if( *dicCopy.getCounterPtr( ) != 1 )
		{
			throw "I expected to own the only reference to the new PDF_Dictionary";
		}
	++(*dicCopy.getCounterPtr( ));
	return RefCountPtr< PDF_Stream_in >( new PDF_Stream_in( reinterpret_cast< PDF_Dictionary * >( dicCopy.getPtr( ) ), is, dataStart ) );
}
/****************/

SimplePDF::PDF_Indirect::PDF_Indirect( size_t _i, size_t _v )
	: i( _i ), v( _v )
{ }
SimplePDF::PDF_Indirect::~PDF_Indirect( )
{ }
void
SimplePDF::PDF_Indirect_in::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	throw "Internal error: PDF_Indirect_in::writeTo: This object is only good for reading, not for writing!";
	//	os << i << " " << v << " R" ;
}
bool SimplePDF::operator < ( const PDF_Indirect & o1, const PDF_Indirect & o2 )
{
	if( o1.i < o2.i )
		{
			return true;
		}
	if( o1.i > o2.i )
		{
			return false;
		}
	if( o1.v < o2.v )
		{
			return true;
		}
	return false;
}
RefCountPtr< PDF_Object >
SimplePDF::PDF_Indirect::deepCopy( RefCountPtr< PDF_Object > self, size_t * indirectObjectCounter, IndirectRemapType * remap )
{
	throw "PDF_Indirect::deepCopy must not be called.	(However PDF_Indirect_in::deepCopy may be called.)";
}
/****************/

SimplePDF::PDF_Indirect_out::PDF_Indirect_out( RefCountPtr<PDF_Object> _obj, size_t _i, size_t _v )
	: PDF_Indirect( _i, _v ), inUse( true ), obj( _obj )
{ }
SimplePDF::PDF_Indirect_out::~PDF_Indirect_out( )
{ }
void
SimplePDF::PDF_Indirect_out::writeTo( std::ostream & os, SimplePDF::PDF_xref * xref, const RefCountPtr< const PDF_Object > & self ) const
{
	xref->enqueue( self.down_cast< const PDF_Indirect_out >( ) );  /* This shall not fail! */
	os << xref->local( i ) << " " << v << " R" ;  /* We must not call xref->local before xref->enqueue ! */
}
void
SimplePDF::PDF_Indirect_out::writeObject( std::ostream & os, SimplePDF::PDF_xref * xref ) const
{
	if( inUse )
		{
			os << xref->local( i ) << " " << v << " obj" << endl ;
			obj->writeTo( os, xref, obj );
			os << endl << "endobj" << endl ;
		}
}

/****************/

SimplePDF::PDF_Indirect_in::PDF_Indirect_in( size_t _i, size_t _v )
	: PDF_Indirect( _i, _v ), PDFin( 0 )
{ }
SimplePDF::PDF_Indirect_in::~PDF_Indirect_in( )
{ }
RefCountPtr< PDF_Object >
SimplePDF::PDF_Indirect_in::deref( )
{
	if( PDFin == 0 )
		{
			throw( "PDF_Indirect_in::deref: PDFin is not assigned" );
		}
	return PDFin->readObjectNumbered( i, v );
}
RefCountPtr< PDF_Object >
SimplePDF::PDF_Indirect_in::deepCopy( RefCountPtr< PDF_Object > self, size_t * indirectObjectCounter, IndirectRemapType * remap )
{
	{
		IndirectRemapType::iterator i( remap->find( *this ) );
		if( i != remap->end( ) )
			{
				return i->second;
			}
	}
	return remap->insert( IndirectRemapType::value_type( *this, SimplePDF::indirect( ::deepCopy( deref( ), indirectObjectCounter, remap ),
																																									 indirectObjectCounter ) ) ).first->second;
}

/**************/

SimplePDF::PDF_xref::PDF_xref( )
	: size_( 0 )
{
	/* The first position in the xref table is special. */
	localOrder_.push_back( 0 ); /* The first object shall appear in the xref table. */
	localNumbers_.push_back( 0 ); /* It has number 0. */
	byteOffsets_.push_back( 0 ); /* This is interpreted as "not in use" */
	generations_.push_back( 65535 );
}

void
SimplePDF::PDF_xref::enqueue( const RefCountPtr< const::PDF_Indirect_out > & obj )
{
	/* Grow big enough */
	while( byteOffsets_.size( ) <= obj->i )
		{
			localNumbers_.push_back( 0 );
			byteOffsets_.push_back( 0 );
			generations_.push_back( 0 );
		}

	if( byteOffsets_[ obj->i ] == 0 )
		{
			indirectQueue_.push_back( obj );
			localNumbers_[ obj->i ] = localOrder_.size( );
			byteOffsets_[ obj->i ] = 1; /* We will change this later, this just indicates that the object has been put in queue. */
			localOrder_.push_back( obj->i );
		}
}

void
SimplePDF::PDF_xref::writeRecursive( std::ostream & os )
{
	std::streamoff os_start;
	{
		std::streamoff tmp = static_cast< streamoff >( os.tellp( ) );
		os.seekp( 0 );
		os_start = static_cast< streamoff >( os.tellp( ) ); /* Will this always be some kind of zero value? */
		os.seekp( tmp );
	}
	while( ! indirectQueue_.empty( ) )
		{
			RefCountPtr< const SimplePDF::PDF_Indirect_out > i_obj = indirectQueue_.front( );
			indirectQueue_.pop_front( );
			byteOffsets_[ i_obj->i ] = static_cast< streamoff >( os.tellp( ) ) - os_start;
			generations_[ i_obj->i ] = i_obj->v;
			i_obj->writeObject( os, this );
		}
}

size_t
SimplePDF::PDF_xref::local( size_t i ) const
{
	return localNumbers_[ i ];
}

void
SimplePDF::PDF_xref::writeTable( std::ostream & os ) const
{
	if( size_ > 0 )
		{
			throw "Internal error: PDF_xref::writeTable: This table has already been written.";
		}
	size_ = localOrder_.size( );

	os << setfill( '0' ) ;
	os << "xref" << endl
		 << 0 << " " << size_ << endl ;
	typedef typeof localOrder_ ListType;
	for( ListType::const_iterator i = localOrder_.begin( ); i != localOrder_.end( ); ++i )
		{
			size_t v = generations_[ *i ];
			size_t o = byteOffsets_[ *i ];
			os << setw( 10 ) << o << ' ' << setw( 5 ) << v << ' ' << 'n' << ' ' << static_cast< char >( 10 ) ;
		}
}

size_t
SimplePDF::PDF_xref::size( ) const
{
	if( size_ == 0 )
		{
			throw "Internal error: PDF_xref::size: The size of the table is not determined yet.  Call writeTable first.";
		}
	return size_;
}


/**************/

RefCountPtr< PDF_Indirect_out >
SimplePDF::indirect( RefCountPtr< PDF_Object > obj, size_t * i, size_t v )
{
	return RefCountPtr< PDF_Indirect_out >( new PDF_Indirect_out( obj, (*i)++, v ) );
}

RefCountPtr< PDF_Object >
SimplePDF::deepCopy( RefCountPtr< PDF_Object > obj, size_t * indirectObjectCounter, IndirectRemapType * remap )
{
	return obj->deepCopy( obj, indirectObjectCounter, remap );
}
