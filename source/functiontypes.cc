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

#include "shapestypes.h"
#include "shapesexceptions.h"
#include "astexpr.h"
#include "consts.h"
#include "angleselect.h"
#include "astvar.h"
#include "astclass.h"
#include "globals.h"
#include "continuations.h"
#include "methodbase.h"
#include "check.h"

//#include "clapack.h"
//#include "cblas.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <ctype.h>
#include <stack>

#define CHOP_Ltol( x )\
	if( fabs( x ) < Ltol )\
		{\
			x = 0;\
		}\
	else if( fabs( x - 1 ) < Ltol )\
		{\
			x = 1;\
		}\
	else if( fabs( x + 1 ) < Ltol )\
		{\
			x = -1;\
		}

#define CHOP_ptol( x )\
	if( x.abs( ) < ptol )\
		{\
			x = Concrete::ZERO_LENGTH;\
		}

using namespace Shapes;
using namespace std;

void displayArray( std::ostream & os, const double * pr, size_t m, size_t n )
{
	size_t r;
	size_t c;
	char buf[20];
	for( r = 0; r < m; ++r )
		{
			for( c = 0; c < n; ++c )
				{
					sprintf( buf, "%14.5e", *( pr + ( r + c * m ) ) );
					os << buf ;
				}
			os << std::endl ;
		}
}

void displayArray( std::ostream & os, const gsl_matrix * m )
{
	char buf[20];
	for( size_t r = 0; r < m->size1; ++r )
		{
			for( size_t c = 0; c < m->size2; ++c )
				{
					sprintf( buf, "%14.5e", gsl_matrix_get( m, r, c ) );
					os << buf ;
				}
			os << std::endl ;
		}
}

namespace Shapes
{

	namespace Lang
	{

		class Transform2DMethod_chop : public Lang::MethodBase< Lang::Transform2D >
		{
		public:
			Transform2DMethod_chop( RefCountPtr< const Lang::Transform2D > _self, const char * fullMethodID );
			virtual ~Transform2DMethod_chop( );
			virtual void call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const;
			static const char * staticFieldID( ) { return "chop"; }
		};

		class Transform3DMethod_chop : public Lang::MethodBase< Lang::Transform3D >
		{
		public:
			Transform3DMethod_chop( RefCountPtr< const Lang::Transform3D > _self, const char * fullMethodID );
			virtual ~Transform3DMethod_chop( );
			virtual void call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const;
			static const char * staticFieldID( ) { return "chop"; }
		};

	}
}

void
Transform2D_register_methods( Lang::SystemFinalClass * dstClass )
{
	dstClass->registerMethod( new Kernel::MethodFactory< Lang::Transform2D, Lang::Transform2DMethod_chop >( ) );
}

RefCountPtr< const Lang::Class > Lang::Transform2D::TypeID( new Lang::SystemFinalClass( strrefdup( "Transform2D" ), Transform2D_register_methods ) );
TYPEINFOIMPL( Transform2D );

Lang::Transform2D::Transform2D( double xx, double yx, double xy, double yy, Concrete::Length xt, Concrete::Length yt )
	: xx_( xx ), yx_( yx ), xy_( xy ), yy_( yy ), xt_( xt ), yt_( yt )
{ }

Lang::Transform2D::Transform2D( const Lang::Transform2D & tf2, const Lang::Transform2D & tf1 )
	: xx_( tf2.xx_ * tf1.xx_ + tf2.xy_ * tf1.yx_ ),
		yx_( tf2.yx_ * tf1.xx_ + tf2.yy_ * tf1.yx_ ),
		xy_( tf2.xx_ * tf1.xy_ + tf2.xy_ * tf1.yy_ ),
		yy_( tf2.yx_ * tf1.xy_ + tf2.yy_ * tf1.yy_ ),
		xt_( tf2.xx_ * tf1.xt_ + tf2.xy_ * tf1.yt_ + tf2.xt_ ),
		yt_( tf2.yx_ * tf1.xt_ + tf2.yy_ * tf1.yt_ + tf2.yt_ )
{ }

DISPATCHIMPL( Transform2D );

Lang::Transform2D::~Transform2D( )
{ }

Kernel::VariableHandle
Lang::Transform2D::getField( const char * fieldID, const RefCountPtr< const Lang::Value > & selfRef ) const
{
	const size_t N = 2;
	if( strcmp( fieldID, "p" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Coords2D( xt_, yt_ ) );
		}
	if( strcmp( fieldID, "L" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Transform2D( xx_, yx_, xy_, yy_, 0, 0 ) );
		}
	if( strcmp( fieldID, "Lx" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatPair( xx_, yx_ ) );
		}
	if( strcmp( fieldID, "Ly" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatPair( xy_, yy_ ) );
		}
	if( strcmp( fieldID, "xL" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatPair( xx_, xy_ ) );
		}
	if( strcmp( fieldID, "yL" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatPair( yx_, yy_ ) );
		}
	if( strcmp( fieldID, "linear?" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Boolean( xt_ == 0 && yt_ == 0 ) );
		}
	if( strcmp( fieldID, "translation?" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Boolean( isTranslation( ) ) );
		}
	if( strcmp( fieldID, "special?" ) == 0 )
		{
			gsl_matrix * A = gsl_matrix_alloc( N, N );
			gsl_permutation * perm = gsl_permutation_alloc( N );
			int signum;
			write_gsl_matrix( A );
			gsl_linalg_LU_decomp( A, perm, & signum );
			bool res = gsl_linalg_LU_det( A, signum ) > 0;
			gsl_matrix_free( A );
			gsl_permutation_free( perm );
			return Helpers::newValHandle( new Lang::Boolean( res ) );
		}
	if( strcmp( fieldID, "Euclidean?" ) == 0 )
		{
			gsl_matrix * A = gsl_matrix_alloc( N, N );
			gsl_vector * SVD_work = gsl_vector_alloc( N );
			gsl_vector * sigma = gsl_vector_alloc( N );
			gsl_matrix * V = gsl_matrix_alloc( N, N );
			write_gsl_matrix( A );
			gsl_linalg_SV_decomp( A, V, sigma, SVD_work );
			const double tol = 1e-4;
			bool res = 1 - tol < gsl_vector_get( sigma, N - 1 ) && gsl_vector_get( sigma, 0 ) < 1 + tol;
			gsl_matrix_free( V );
			gsl_vector_free( sigma );
			gsl_vector_free( SVD_work );
			gsl_matrix_free( A );
			return Helpers::newValHandle( new Lang::Boolean( res ) );
		}

	return TypeID->getMethod( selfRef, fieldID ); /* This will throw if there is no such method. */
}

Lang::Transform2D *
Lang::Transform2D::clone( ) const
{
	return new Transform2D( xx_, yx_,
													xy_, yy_,
													xt_, yt_ );
}

bool
Lang::Transform2D::isIdentity( ) const
{
	return
		xt_ == Concrete::ZERO_LENGTH && yt_ == Concrete::ZERO_LENGTH &&
		xx_ == 1 && yy_ == 1 &&
		xy_ == 0 && yx_ == 0;
}

bool
Lang::Transform2D::isTranslation( ) const
{
	return
		xx_ == 1 && yy_ == 1 &&
		xy_ == 0 && yx_ == 0;
}

void
Lang::Transform2D::write_gsl_matrix( gsl_matrix * matrix_2_2 ) const
{
	gsl_matrix_set( matrix_2_2, 0, 0, xx_ );
	gsl_matrix_set( matrix_2_2, 1, 0, yx_ );
	gsl_matrix_set( matrix_2_2, 0, 1, xy_ );
	gsl_matrix_set( matrix_2_2, 1, 1, yy_ );
}

void
Lang::Transform2D::write_gsl_vector( gsl_vector * vec_2 ) const
{
	gsl_vector_set( vec_2, 0, Concrete::Length::offtype( xt_ ) );
	gsl_vector_set( vec_2, 1, Concrete::Length::offtype( yt_ ) );
}

void
Lang::Transform2D::shipout( std::ostream & os ) const
{
	os << xx_ << " " << yx_ << " " << xy_ << " " << yy_ << " "
		 << Concrete::Length::offtype( xt_ ) << " " << Concrete::Length::offtype( yt_ ) ;
}

// to be used by text moveto commands
void
Lang::Transform2D::replaceBy( const Lang::Transform2D & newtf )
{
	xx_ = newtf.xx_;
	yx_ = newtf.yx_;
	xy_ = newtf.xy_;
	yy_ = newtf.yy_;
	xt_ = newtf.xt_;
	yt_ = newtf.yt_;
}

// to be used by text newline commands
void
Lang::Transform2D::prependShift( const Concrete::Coords2D & d )
{
	// Think of d as tf1 when composing transforms.

	xt_ += xx_ * d.x_ + xy_ * d.y_;
	yt_ += yx_ * d.x_ + yy_ * d.y_;
}

// to be used by text painting commands
void
Lang::Transform2D::prependXShift( const Concrete::Length & dx )
{
	// Think of d as tf1 when composing transforms.

	xt_ += xx_ * dx;
	yt_ += yx_ * dx;
}

void
Lang::Transform2D::show( std::ostream & os ) const
{
	os << "[ ("
		 << xx_ << ", " << yx_ << ") ("
		 << xy_ << ", " << yy_ << ") ("
		 << Lang::Length( xt_ ) << ", " << Lang::Length( yt_ ) << ") ]" ;
}


void
Transform3D_register_methods( Lang::SystemFinalClass * dstClass )
{
	dstClass->registerMethod( new Kernel::MethodFactory< Lang::Transform3D, Lang::Transform3DMethod_chop >( ) );
}

RefCountPtr< const Lang::Class > Lang::Transform3D::TypeID( new Lang::SystemFinalClass( strrefdup( "Transform3D" ), Transform3D_register_methods ) );
TYPEINFOIMPL( Transform3D );

Lang::Transform3D::Transform3D( double xx, double yx, double zx, double xy, double yy, double zy, double xz, double yz, double zz, Concrete::Length xt, Concrete::Length yt, Concrete::Length zt )
	: planeNormalTransformData_( 0 ),
		xx_( xx ), yx_( yx ), zx_( zx ), xy_( xy ), yy_( yy ), zy_( zy ), xz_( xz ), yz_( yz ), zz_( zz ), xt_( xt ), yt_( yt ), zt_( zt )
{ }

Lang::Transform3D::Transform3D( const gsl_matrix * matrix_3_3, const gsl_vector * vec_3 )
	: planeNormalTransformData_( 0 ),
		xx_( gsl_matrix_get( matrix_3_3, 0, 0 ) ), yx_( gsl_matrix_get( matrix_3_3, 1, 0 ) ), zx_( gsl_matrix_get( matrix_3_3, 2, 0 ) ),
		xy_( gsl_matrix_get( matrix_3_3, 0, 1 ) ), yy_( gsl_matrix_get( matrix_3_3, 1, 1 ) ), zy_( gsl_matrix_get( matrix_3_3, 2, 1 ) ),
		xz_( gsl_matrix_get( matrix_3_3, 0, 2 ) ), yz_( gsl_matrix_get( matrix_3_3, 1, 2 ) ), zz_( gsl_matrix_get( matrix_3_3, 2, 2 ) ),
		xt_( gsl_vector_get( vec_3, 0 ) ), yt_( gsl_vector_get( vec_3, 1 ) ), zt_( gsl_vector_get( vec_3, 2 ) )
{ }



Lang::Transform3D::Transform3D( const Lang::Transform3D & tf2, const Lang::Transform3D & tf1 )
	: planeNormalTransformData_( 0 ),
		xx_( tf2.xx_ * tf1.xx_ + tf2.xy_ * tf1.yx_ + tf2.xz_ * tf1.zx_ ),
		yx_( tf2.yx_ * tf1.xx_ + tf2.yy_ * tf1.yx_ + tf2.yz_ * tf1.zx_ ),
		zx_( tf2.zx_ * tf1.xx_ + tf2.zy_ * tf1.yx_ + tf2.zz_ * tf1.zx_ ),
		xy_( tf2.xx_ * tf1.xy_ + tf2.xy_ * tf1.yy_ + tf2.xz_ * tf1.zy_ ),
		yy_( tf2.yx_ * tf1.xy_ + tf2.yy_ * tf1.yy_ + tf2.yz_ * tf1.zy_ ),
		zy_( tf2.zx_ * tf1.xy_ + tf2.zy_ * tf1.yy_ + tf2.zz_ * tf1.zy_ ),
		xz_( tf2.xx_ * tf1.xz_ + tf2.xy_ * tf1.yz_ + tf2.xz_ * tf1.zz_ ),
		yz_( tf2.yx_ * tf1.xz_ + tf2.yy_ * tf1.yz_ + tf2.yz_ * tf1.zz_ ),
		zz_( tf2.zx_ * tf1.xz_ + tf2.zy_ * tf1.yz_ + tf2.zz_ * tf1.zz_ ),
		xt_( tf2.xx_ * tf1.xt_ + tf2.xy_ * tf1.yt_ + tf2.xz_ * tf1.zt_ + tf2.xt_ ),
		yt_( tf2.yx_ * tf1.xt_ + tf2.yy_ * tf1.yt_ + tf2.yz_ * tf1.zt_ + tf2.yt_ ),
		zt_( tf2.zx_ * tf1.xt_ + tf2.zy_ * tf1.yt_ + tf2.zz_ * tf1.zt_ + tf2.zt_ )
{ }

DISPATCHIMPL( Transform3D );

Lang::Transform3D::~Transform3D( )
{
	if( planeNormalTransformData_ != 0 )
		{
			gsl_matrix_free( planeNormalTransformData_ );
			//			delete planeNormalTransformData_;
		}
}

Kernel::VariableHandle
Lang::Transform3D::getField( const char * fieldID, const RefCountPtr< const Lang::Value > & selfRef ) const
{
	const size_t N = 3;
	if( strcmp( fieldID, "p" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Coords3D( xt_, yt_, zt_ ) );
		}
	if( strcmp( fieldID, "L" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Transform3D( xx_, yx_, zx_, xy_, yy_, zy_, xz_, yz_, zz_, 0, 0, 0 ) );
		}
	if( strcmp( fieldID, "Lx" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatTriple( xx_, yx_, zx_ ) );
		}
	if( strcmp( fieldID, "Ly" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatTriple( xy_, yy_, zy_ ) );
		}
	if( strcmp( fieldID, "Lz" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatTriple( xz_, yz_, zz_ ) );
		}
	if( strcmp( fieldID, "xL" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatTriple( xx_, xy_, xz_ ) );
		}
	if( strcmp( fieldID, "yL" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatTriple( yx_, yy_, yz_ ) );
		}
	if( strcmp( fieldID, "zL" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::FloatTriple( zx_, zy_, zz_ ) );
		}
	if( strcmp( fieldID, "linear?" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Boolean( xt_ == 0 && yt_ == 0 && zt_ == 0 ) );
		}
	if( strcmp( fieldID, "translation?" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Boolean( isTranslation( ) ) );
		}
	if( strcmp( fieldID, "special?" ) == 0 )
		{
			gsl_matrix * A = gsl_matrix_alloc( N, N );
			gsl_permutation * perm = gsl_permutation_alloc( N );
			int signum;
			write_gsl_matrix( A );
			gsl_linalg_LU_decomp( A, perm, & signum );
			bool res = gsl_linalg_LU_det( A, signum ) > 0;
			gsl_matrix_free( A );
			gsl_permutation_free( perm );
			return Helpers::newValHandle( new Lang::Boolean( res ) );
		}
	if( strcmp( fieldID, "Euclidean?" ) == 0 )
		{
			gsl_matrix * A = gsl_matrix_alloc( N, N );
			gsl_vector * SVD_work = gsl_vector_alloc( N );
			gsl_vector * sigma = gsl_vector_alloc( N );
			gsl_matrix * V = gsl_matrix_alloc( N, N );
			write_gsl_matrix( A );
			gsl_linalg_SV_decomp( A, V, sigma, SVD_work );
			const double tol = 1e-4;
			bool res = 1 - tol < gsl_vector_get( sigma, N - 1 ) && gsl_vector_get( sigma, 0 ) < 1 + tol;
			gsl_matrix_free( V );
			gsl_vector_free( sigma );
			gsl_vector_free( SVD_work );
			gsl_matrix_free( A );
			return Helpers::newValHandle( new Lang::Boolean( res ) );
		}

	return TypeID->getMethod( selfRef, fieldID ); /* This will throw if there is no such method. */
}

Lang::Transform3D *
Lang::Transform3D::clone( ) const
{
	return new Transform3D( xx_, yx_, zx_,
													xy_, yy_, zy_,
													xz_, yz_, zz_,
													xt_, yt_, zt_ );
}

bool
Lang::Transform3D::isIdentity( ) const
{
	return
		xt_ == Concrete::ZERO_LENGTH && yt_ == Concrete::ZERO_LENGTH && zt_ == Concrete::ZERO_LENGTH &&
		xx_ == 1 && yy_ == 1 && zz_ == 1 &&
		xy_ == 0 && xz_ == 0 && yx_ == 0 && yz_ == 0 && zx_ == 0 && zy_ == 0;
}

bool
Lang::Transform3D::isTranslation( ) const
{
	return
		xx_ == 1 && yy_ == 1 && zz_ == 1 &&
		xy_ == 0 && xz_ == 0 &&
		yx_ == 0 && yz_ == 0 &&
		zx_ == 0 && zy_ == 0;
}

void
Lang::Transform3D::write_gsl_matrix( gsl_matrix * matrix_3_3 ) const
{
	gsl_matrix_set( matrix_3_3, 0, 0, xx_ );
	gsl_matrix_set( matrix_3_3, 1, 0, yx_ );
	gsl_matrix_set( matrix_3_3, 2, 0, zx_ );
	gsl_matrix_set( matrix_3_3, 0, 1, xy_ );
	gsl_matrix_set( matrix_3_3, 1, 1, yy_ );
	gsl_matrix_set( matrix_3_3, 2, 1, zy_ );
	gsl_matrix_set( matrix_3_3, 0, 2, xz_ );
	gsl_matrix_set( matrix_3_3, 1, 2, yz_ );
	gsl_matrix_set( matrix_3_3, 2, 2, zz_ );
}

void
Lang::Transform3D::write_gsl_vector( gsl_vector * vec_3 ) const
{
	gsl_vector_set( vec_3, 0, Concrete::Length::offtype( xt_ ) );
	gsl_vector_set( vec_3, 1, Concrete::Length::offtype( yt_ ) );
	gsl_vector_set( vec_3, 2, Concrete::Length::offtype( zt_ ) );
}

Concrete::UnitFloatTriple
Lang::Transform3D::transformPlaneUnitNormal( const Concrete::UnitFloatTriple & n ) const
{
	const int N = 3;
	// These statically allocated matrices will leak memory.	They could be removed by using a static automaitc
	// deallocator object.	However, since they shall not be deleted by delete (but by gsl_matrix_free), we cannot
	// us RefCountPtr< gsl_matrix >.
	static gsl_matrix * a = gsl_matrix_alloc( N, N );
	static gsl_matrix * v = gsl_matrix_alloc( N, N );
	static gsl_vector * s = gsl_vector_alloc( N );
	static gsl_vector * work = gsl_vector_alloc( N );


//	 static __CLPK_integer mn = N;
//	 static char jobuvt = 'A';

//	 static double a[ N * N ];
//	 static double u[ N * N ];
//	 static double vt[ N * N ];
//	 __CLPK_integer ldauvt = N;
//	 static double s[ N ];
//	 __CLPK_integer lwork;
//	 __CLPK_integer info;

//	 static __CLPK_doublereal * work = 0;
//	 static RefCountPtr< __CLPK_doublereal > workCleaner;

//	 if( work == 0 )
//		 {
//			 // Then we ask LAPACK how much workspace it wants, and then we assume that this number will not change
//			 // as we call DGESVD with other arguments.

//			 double tmpwork;
//			 lwork = -1;
//			 dgesvd_( & jobuvt, & jobuvt,
//								& mn, & mn,
//								reinterpret_cast< __CLPK_doublereal * >( & a ), & ldauvt,
//								reinterpret_cast< __CLPK_doublereal * >( & s ),
//								reinterpret_cast< __CLPK_doublereal * >( & u ), & ldauvt,
//								reinterpret_cast< __CLPK_doublereal * >( & vt ), & ldauvt,
//								reinterpret_cast< __CLPK_doublereal * >( & tmpwork ), & lwork,
//								& info );
//			 lwork = static_cast< __CLPK_integer >( tmpwork );
//			 work = new __CLPK_doublereal[ lwork ];
//			 workCleaner = RefCountPtr< __CLPK_doublereal >( work );
//		 }


	if( planeNormalTransformData_ == 0 )
		{
			// This is the first time this transform is used to transform a unit plane normal.
			// The linear part of this transform must then be computed, and we use the singular
			// value decomposition for this.

			planeNormalTransformData_ = gsl_matrix_alloc( N, N );

			write_gsl_matrix( a );

//			 std::cerr << "Here's a:" << std::endl ;
//			 displayArray( std::cerr, a );

			{
				int status = gsl_linalg_SV_decomp( a, v, s, work );
				if( status != 0 )
					{
						throw Exceptions::ExternalError( "Gnu Scientific Library SVD routine failed." );
					}
			}

//			 a[ 0 ] = xx_;
//			 a[ 1 ] = yx_;
//			 a[ 2 ] = zx_;
//			 a[ 3 ] = xy_;
//			 a[ 4 ] = yy_;
//			 a[ 5 ] = zy_;
//			 a[ 6 ] = xz_;
//			 a[ 7 ] = yz_;
//			 a[ 8 ] = zz_;

//			 dgesvd_( & jobuvt, & jobuvt,
//								& mn, & mn,
//								reinterpret_cast< __CLPK_doublereal * >( & a ), & ldauvt,
//								reinterpret_cast< __CLPK_doublereal * >( & s ),
//								reinterpret_cast< __CLPK_doublereal * >( & u ), & ldauvt,
//								reinterpret_cast< __CLPK_doublereal * >( & vt ), & ldauvt,
//								reinterpret_cast< __CLPK_doublereal * >( work ), & lwork,
//								& info );

//			 if( info != 0 )
//				 {
//					 throw Exceptions::ExternalError( "LAPACK routine DGESVD failed." );
//				 }

			if( gsl_vector_get( s, 1 ) < 1e-5 )
				{
					throw Exceptions::AffineTransformKillsPlane( gsl_vector_get( s, 1 ) );
				}

//			 std::cerr << "Here's s:" << std::endl ;
//			 gsl_vector_fprintf( stderr, s, "%f" );

			gsl_vector_set( s, 0, gsl_vector_get( s, 2 ) / gsl_vector_get( s, 0 ) );
			gsl_vector_set( s, 1, gsl_vector_get( s, 2 ) / gsl_vector_get( s, 1 ) );
			//			s[ 2 ] = 1;

//			 std::cerr << "Here's the modified s:" << std::endl ;
//			 gsl_vector_fprintf( stderr, s, "%f" );


//			 if( s[ 1 ] < 1e-5 )
//				 {
//					 throw Exceptions::AffineTransformKillsPlane( s[ 1 ] );
//				 }

//			 s[ 0 ] = s[ 2 ] / s[ 0 ];
//			 s[ 1 ] = s[ 2 ] / s[ 1 ];
//			 //			s[ 2 ] = 1;

			// We will now compute " u * diag( s ) * vt ".

			// Note that "u" is stored in a when gsl_linalg_SV_decomp is used.

//			 std::cerr << "Here's u:" << std::endl ;
//			 displayArray( std::cerr, a );

			gsl_matrix_set( a, 0, 0, gsl_matrix_get( a, 0, 0 ) * gsl_vector_get( s, 0 ) );
			gsl_matrix_set( a, 1, 0, gsl_matrix_get( a, 1, 0 ) * gsl_vector_get( s, 0 ) );
			gsl_matrix_set( a, 2, 0, gsl_matrix_get( a, 2, 0 ) * gsl_vector_get( s, 0 ) );
			gsl_matrix_set( a, 0, 1, gsl_matrix_get( a, 0, 1 ) * gsl_vector_get( s, 1 ) );
			gsl_matrix_set( a, 1, 1, gsl_matrix_get( a, 1, 1 ) * gsl_vector_get( s, 1 ) );
			gsl_matrix_set( a, 2, 1, gsl_matrix_get( a, 2, 1 ) * gsl_vector_get( s, 1 ) );
			//	Note that s[ 2 ] == 1

//			 u[ 0 ] *= s[ 0 ];
//			 u[ 1 ] *= s[ 0 ];
//			 u[ 2 ] *= s[ 0 ];
//			 u[ 3 ] *= s[ 1 ];
//			 u[ 4 ] *= s[ 1 ];
//			 u[ 5 ] *= s[ 1 ];
//			 //	Note that s[ 2 ] == 1

//			 std::cerr << "Here's the modified a:" << std::endl ;
//			 displayArray( std::cerr, a );

//			 std::cerr << "Here's v:" << std::endl ;
//			 displayArray( std::cerr, v );

			gsl_blas_dgemm( CblasNoTrans,
											CblasTrans,
											1.,
											a,
											v,
											0.,
											planeNormalTransformData_ );

//			 std::cerr << "Here's the matrix:" << std::endl ;
//			 displayArray( std::cerr, planeNormalTransformData_ );

//			 cblas_dgemm( CblasColMajor, CblasNoTrans, CblasNoTrans,
//										N, N, N,
//										1.,
//										u, N,
//										vt, N,
//										0., planeNormalTransformData_, N );
		}

	// Now that the linear, not too singular, transform has been computed, we just apply it and normalize the result.
	// The normalization is made by the UnitFloatTriple constructor.

	static gsl_vector * _n = gsl_vector_alloc( N );
	static gsl_vector * res = gsl_vector_alloc( N );

	//	static double _n[ N ];
	//	static double res[ N ];

	gsl_vector_set( _n, 0, n.x_ );
	gsl_vector_set( _n, 1, n.y_ );
	gsl_vector_set( _n, 2, n.z_ );

//	 _n[ 0 ] = n.x_;
//	 _n[ 1 ] = n.y_;
//	 _n[ 2 ] = n.z_;

//	 std::cerr << "Input vector: " << std::endl ;
//	 gsl_vector_fprintf( stderr, _n, "%f" );
//	 std::cerr << std::endl ;

	gsl_blas_dgemv( CblasNoTrans,
									1.,
									planeNormalTransformData_,
									_n,
									0.,
									res );

//	 std::cerr << "Result: " << std::endl ;
//	 gsl_vector_fprintf( stderr, res, "%f" );
//	 std::cerr << std::endl ;

//	 cblas_dgemv( CblasColMajor, CblasNoTrans,
//								N, N,
//								1.,
//								planeNormalTransformData_, N,
//								reinterpret_cast< double * >( & _n ), 1,
//								0., reinterpret_cast< double * >( & res ), 1 );

	return Concrete::UnitFloatTriple( gsl_vector_get( res, 0 ),
																		gsl_vector_get( res, 1 ),
																		gsl_vector_get( res, 2 ) );
	//	return Concrete::UnitFloatTriple( res[0], res[1], res[2] );

	/*
			{
				// First we compute a unit vector in the plane
				static const Concrete::UnitFloatTriple xHat( 1, 0, 0 );
				static const Concrete::UnitFloatTriple yHat( 0, 1, 0 );
				Concrete::UnitFloatTriple r1 = Shapes::cross( unitNormal_, xHat );
				Concrete::UnitFloatTriple r2 = Shapes::cross( unitNormal_, yHat );
				Concrete::UnitFloatTriple r( 0, 0, 0 );
				if( r1.norm( ) > r2.norm( ) )
					{
						r = r1.normalized( );
					}
				else
					{
						r = r2.normalized( );
					}

				// Then we find one point that is in the plane
				double ax = fabs( unitNormal_.x_ );
				double ay = fabs( unitNormal_.y_ );
				double az = fabs( unitNormal_.z_ );

				Concrete::Coords3D x0( 0, 0, 0 );
				if( ax >= ay && ax >= az )
					{
						x0 = Concrete::Coords3D( m_ / unitNormal_.x_, 0, 0 );
					}
				else if( ay >= az )
					{
						x0 = Concrete::Coords3D( 0, m_ / unitNormal_.y_, 0 );
					}
				else
					{
						x0 = Concrete::Coords3D( 0, 0, m_ / unitNormal_.z_ );
					}

				// Now it is easy to find two more points that span the plane together with x0
				Concrete::Coords3D x1 = x0 + r;
				Concrete::Coords3D x2 = x0 + Shapes::cross( unitNormal_, r );

				// Now we see where these points go...
				Concrete::Coords3D Tx0 = x0.transformed( tf );
				Concrete::Coords3D Tx1 = x1.transformed( tf );
				Concrete::Coords3D Tx2 = x2.transformed( tf );

				// ... from which the new equation may be computed.
				Concrete::UnitFloatTriple Tnormal = Shapes::cross( Tx1 - Tx0, Tx2 - Tx0 );
				double TnormalNorm = Tnormal.norm( );
				if( TnormalNorm == 0 )
					{
						// A polygon of lower dimension is invisible, so we may just return without pushing any triangles.
						return;
					}
				Concrete::UnitFloatTriple TunitNormal = Tnormal * ( 1 / TnormalNorm );
	*/
}

void
Lang::Transform3D::show( std::ostream & os ) const
{
	os << "[ ("
		 << xx_ << ", " << yx_ << ", " << zx_ << ") ("
		 << xy_ << ", " << yy_ << ", " << zy_ << ") ("
		 << xz_ << ", " << yz_ << ", " << zz_ << ") ("
		 << Lang::Length( xt_ ) << ", " << Lang::Length( yt_ ) << ", " << Lang::Length( zt_ ) << ") ]" ;
}


Kernel::Arguments::Arguments( const Kernel::EvaluatedFormals * formals )
	: formals_( formals ), variables_( new Environment::ValueVector::ValueType ), dst_( 0 ),
		hasSink_( formals_->formals_->hasSink( ) ),
		dstEnd_( formals_->formals_->defaultExprs_.size( ) ),
		isSink_( formals_->isSink_ ),
		sinkArgList_( 0 ), sinkValues_( NullPtr< const Lang::SingleList >( ) ),
		states_( new Environment::StateVector::ValueType ), stateDst_( 0 ),
		mutatorSelf_( 0 )
{
	if( hasSink_ )
		{
			sinkArgList_ = new Ast::ArgListExprs( false ); // This is not an expression-owner.
			sinkValues_ = RefCountPtr< const Lang::SingleList >( new Lang::SingleListNull( ) );
			if( formals_->formals_->argumentOrder_->empty( ) )
				{
					// All arguments go in the sink.
					dst_ = INT_MAX;
				}
		}
	// Thinking of all the evaluated cuts, it actually makes some sense not to reserve memory here.
	//	variables_.reserve( formals_->argumentOrder_->size( ) );
	//	states_.reserve( formals_->stateOrder_->size( ) );
}

Kernel::Arguments::~Arguments( )
{ }

Kernel::Arguments
Kernel::Arguments::clone( ) const
{
	CHECK(
				if( ! states_->empty( ) )
					{
						throw Exceptions::InternalError( "Arguments with states may not be cloned." );
					}
				);

	Kernel::Arguments res( formals_ );

	res.variables_->reserve( variables_->size( ) );
	{
		typedef typeof *variables_ ListType;
		for( ListType::const_iterator i = variables_->begin( ); i != variables_->end( ); ++i )
			{
				res.variables_->push_back( *i );
			}
	}
	res.locations_ = locations_;
	res.dst_ = dst_;
	if( sinkArgList_ != 0 )
		{
			throw Exceptions::NotImplemented( "Cloning of arguments with sink." );
			//res.sinkArgList_ = sinkArgList_->clone( );
			res.sinkValues_ = sinkValues_;
		}

	return res;
}


void
Kernel::Arguments::addOrderedArgument( const Kernel::VariableHandle & arg, Ast::Expression * loc )
{
	/* Recall that if there's a sink, this will be the last argument.
	 */
	if( ! isSink_ &&
			dst_ >= dstEnd_ )
		{
			/* If there is a sink, put it there.	Otherwise, we have detected an arity mismatch, but to generate a good
			 * error message we don't throw the message from here, but rest assured that an error will be delivered in
			 * due time when applyDefaults is invoked.
			 */
			if( hasSink_ )
				{
					sinkArgList_->orderedExprs_->push_back( loc );
					sinkValues_ = RefCountPtr< Lang::SingleList >( new Lang::SingleListPair( arg, sinkValues_ ) );
				}
			else
				{
					variables_->push_back( arg );
					locations_.push_back( loc );
					++dst_; // I'm not sure this is meaningful here...
				}
		}
	else if( dst_ == variables_->size( ) )
		{
			variables_->push_back( arg );
			locations_.push_back( loc );
			++dst_;
		}
	else
		{
			(*variables_)[ dst_ ] = arg;
			locations_[ dst_ ] = loc;
			while( dst_ < variables_->size( ) &&
						 (*variables_)[ dst_ ] != Kernel::THE_SLOT_VARIABLE )
				{
					++dst_;
				}
		}
}

void
Kernel::Arguments::addNamedArgument( const char * id, const Kernel::VariableHandle & arg, Ast::Expression * loc )
{
	if( formals_ == 0 )
		{
			throw Exceptions::CoreNoNamedFormals( "???" );
		}

	typedef typeof *(formals_->formals_->argumentOrder_) FormalsMapType;
	FormalsMapType & formalsMap = *(formals_->formals_->argumentOrder_);

	/* Note that the name of the sink is invisible, so referring to it is just another arguments which is
	 * put in the sink.	This variable happens to have the same name as the sink.
	 */
	FormalsMapType::const_iterator j = formalsMap.find( id );
	if( j == formalsMap.end( ) ||
			( hasSink_ &&
				j->second == dstEnd_ ) )
		{
			if( hasSink_ )
				{
					if( sinkArgList_->namedExprs_->find( id ) != sinkArgList_->namedExprs_->end( ) )
						{
							throw Exceptions::InternalError( "It is a surprise that the sink got a repeated formal." );
						}
					sinkArgList_->namedExprs_->insert( std::pair< const char *, Ast::Expression * >( id, loc ) );
					sinkValues_ = RefCountPtr< Lang::SingleList >( new Lang::SingleListPair( arg, sinkValues_ ) );
					return;
				}
			throw Exceptions::NamedFormalMismatch( formals_->formals_->loc( ), strrefdup( id ), Exceptions::NamedFormalMismatch::VARIABLE );
		}
	size_t pos = j->second;

	if( pos < dst_ )
		{
			throw Exceptions::NamedFormalAlreadySpecified( formals_->formals_->loc( ), strrefdup( id ), pos, Exceptions::NamedFormalAlreadySpecified::VARIABLE );
		}

	if( pos >= variables_->size( ) )
		{
			while( variables_->size( ) < pos )
				{
					variables_->push_back( Kernel::THE_SLOT_VARIABLE );
					locations_.push_back( 0 );
				}
			variables_->push_back( arg );
			locations_.push_back( loc );
		}
	else
		{
			if( (*variables_)[ pos ] != Kernel::THE_SLOT_VARIABLE )
				{
					throw Exceptions::NamedFormalAlreadySpecified( formals_->formals_->loc( ), strrefdup( id ), pos, Exceptions::NamedFormalAlreadySpecified::VARIABLE );
				}
			(*variables_)[ pos ] = arg;
			locations_[ pos ] = loc;
		}
	if( pos == dst_ )
		{
			while( dst_ < variables_->size( ) &&
						 (*variables_)[ dst_ ] != Kernel::THE_SLOT_VARIABLE )
				{
					++dst_;
				}
		}
}

void
Kernel::Arguments::addOrderedState( const Kernel::StateHandle & state, Ast::Node * loc )
{
	if( stateDst_ == states_->size( ) )
		{
			states_->push_back( state );
			stateLocations_.push_back( loc );
			++stateDst_;
		}
	else
		{
			(*states_)[ stateDst_ ] = state;
			stateLocations_[ stateDst_ ] = loc;
			while( stateDst_ < states_->size( ) &&
						 (*states_)[ stateDst_ ] != Kernel::THE_SLOT_STATE )
				{
					++stateDst_;
				}
		}
}

void
Kernel::Arguments::addNamedState( const char * id, const Kernel::StateHandle & state, Ast::Node * loc )
{
	if( formals_ == 0 )
		{
			throw Exceptions::CoreNoNamedFormals( "???" );
		}

	typedef typeof *(formals_->formals_->stateOrder_) FormalsMapType;
	FormalsMapType & formalsMap = *(formals_->formals_->stateOrder_);

	FormalsMapType::const_iterator j = formalsMap.find( id );
	if( j == formalsMap.end( ) )
		{
			throw Exceptions::NamedFormalMismatch( formals_->formals_->loc( ), strrefdup( id ), Exceptions::NamedFormalMismatch::STATE );
		}
	size_t pos = j->second;

	if( pos < stateDst_ )
		{
			throw Exceptions::NamedFormalAlreadySpecified( formals_->formals_->loc( ), strrefdup( id ), pos, Exceptions::NamedFormalAlreadySpecified::STATE );
		}

	if( pos >= states_->size( ) )
		{
			while( states_->size( ) < pos )
				{
					states_->push_back( Kernel::THE_SLOT_STATE );
					stateLocations_.push_back( 0 );
				}
			states_->push_back( state );
			stateLocations_.push_back( loc );
		}
	else
		{
			if( (*states_)[ pos ] != Kernel::THE_SLOT_STATE )
				{
					throw Exceptions::NamedFormalAlreadySpecified( formals_->formals_->loc( ), strrefdup( id ), pos, Exceptions::NamedFormalAlreadySpecified::STATE );
				}
			(*states_)[ pos ] = state;
			stateLocations_[ pos ] = loc;
		}
	if( pos == stateDst_ )
		{
			while( stateDst_ < states_->size( ) &&
						 (*states_)[ stateDst_ ] != Kernel::THE_SLOT_STATE )
				{
					++stateDst_;
				}
		}
}

void
Kernel::Arguments::applyDefaults( )
{
	if( formals_ == 0 )
		{
			return;
		}

	size_t numberOfArguments = variables_->size( );
	size_t formalsSize = formals_->defaults_.size( );

	if( numberOfArguments > formalsSize &&
			! hasSink_ )
		{
			/* The location of the ball must be set by the caller. */
			throw Exceptions::UserArityMismatch( formals_->formals_->loc( ), formalsSize, numberOfArguments, Exceptions::UserArityMismatch::VARIABLE );
		}

	size_t numberOfStates = states_->size( );
	size_t formalsStateSize = formals_->formals_->stateOrder_->size( );
	if( numberOfStates > formalsStateSize )
		{
			/* The location of the ball must be set by the caller. */
			throw Exceptions::UserArityMismatch( formals_->formals_->loc( ), formalsStateSize, numberOfStates, Exceptions::UserArityMismatch::STATE );
		}

	/* First the easy part:	All states must be specified.
	 */
	std::map< size_t, RefCountPtr< const char > > * missingStates = 0;
	{
		size_t pos = 0;
		typedef typeof *states_ ListType;
		for( ListType::const_iterator i = states_->begin( ); i != states_->end( ); ++i, ++pos )
			{
				if( *i == Kernel::THE_SLOT_STATE )
					{
						if( missingStates == 0 )
							{
								missingStates = new typeof *missingStates;
							}
						typedef typeof *(formals_->formals_->stateOrder_) FormalsMapType;
						FormalsMapType & formalsMap = *(formals_->formals_->stateOrder_);
						for( FormalsMapType::const_iterator i = formalsMap.begin( ); ; )
							{
								if( i->second == pos )
									{
										missingStates->insert( missingStates->begin( ), std::pair< size_t, RefCountPtr< const char > >( pos, strrefdup( i->first ) ) );
										break;
									}
								++i;
								if( i == formalsMap.end( ) )
									{
										throw Exceptions::InternalError( "Failed to find position of missing state." );
									}
							}

					}
			}

		if( numberOfStates < formalsStateSize )
			{
				if( missingStates == 0 )
					{
						missingStates = new typeof *missingStates;
					}
				typedef typeof *(formals_->formals_->stateOrder_) FormalsMapType;
				FormalsMapType & formalsMap = *(formals_->formals_->stateOrder_);
				FormalsMapType::const_iterator i = formalsMap.begin( );
				for( size_t j = 0; j < numberOfStates; ++j )
					{
						++i;
					}

				for( ; i != formalsMap.end( ); ++i, ++pos)
					{
						missingStates->insert( missingStates->begin( ), std::pair< size_t, RefCountPtr< const char > >( pos, strrefdup( i->first ) ) );
					}

			}
	}

	/* Allocate positions in the vector for all arguments.
	 */
	variables_->reserve( hasSink_ ? formalsSize + 1 : formalsSize );
	while( variables_->size( ) < formalsSize )
		{
			variables_->push_back( Kernel::THE_SLOT_VARIABLE );
		}
	locations_.resize( formalsSize );

	typedef typeof *variables_ MyListType;
	typedef typeof formals_->defaults_ DefaultListType;
	typedef typeof formals_->locations_ LocationListType;
	DefaultListType::const_iterator src = formals_->defaults_.begin( );
	LocationListType::const_iterator srcLoc = formals_->locations_.begin( );
	std::map< size_t, RefCountPtr< const char > > * missingArgs = 0;
	size_t pos = 0;
	typedef typeof locations_ MyLocationsType;
	for( MyListType::iterator dst = variables_->begin( ); dst != variables_->end( ); ++dst, ++src, ++srcLoc, ++pos )
		{
			if( *dst == Kernel::THE_SLOT_VARIABLE )
				{
					/* Handle error situation.
					 */
					if( *src == Kernel::THE_SLOT_VARIABLE )
						{
							if( missingArgs == 0 )
								{
									missingArgs = new typeof *missingArgs;
								}
							typedef typeof *(formals_->formals_->argumentOrder_) FormalsMapType;
							FormalsMapType & formalsMap = *(formals_->formals_->argumentOrder_);
							for( FormalsMapType::const_iterator i = formalsMap.begin( ); ; )
								{
									if( i->second == pos )
										{
											missingArgs->insert( missingArgs->begin( ), std::pair< size_t, RefCountPtr< const char > >( pos, strrefdup( i->first ) ) );
											break;
										}
									++i;
									if( i == formalsMap.end( ) )
										{
											throw Exceptions::InternalError( "Failed to find position of missing argument." );
										}
								}
						}

					/* Normal case.
					 */
					*dst = *src;
					locations_[ pos ] = *srcLoc;
				}
		}

	if( missingArgs != 0 || missingStates != 0 )
		{
			throw Exceptions::MissingArguments( formals_->formals_->loc( ), missingArgs, missingStates );
		}

	if( hasSink_ )
		{
			variables_->push_back
				( Helpers::newValHandle
					( new Lang::Structure( sinkArgList_,
																 sinkValues_,
																 true ) ) ); // true means that the sinkArgList_ gets owned by the Structure.
			sinkArgList_ = 0;
		}
}


Kernel::VariableHandle &
Kernel::Arguments::getHandle( size_t i )
{
	return (*variables_)[ i ];
}

RefCountPtr< const Lang::Value > &
Kernel::Arguments::getValue( size_t i )
{
	return (*variables_)[ i ]->getUntyped( );
}

const Ast::SourceLocation &
Kernel::Arguments::getLoc( size_t i ) const
{
	return locations_[ i ]->loc( );
}

const Ast::Node *
Kernel::Arguments::getNode( size_t i ) const
{
	return locations_[ i ];
}

Kernel::Thunk *
Kernel::Arguments::getThunk( size_t i )
{
	return (*variables_)[ i ]->copyThunk( );
}

bool
Kernel::Arguments::isSlot( size_t i ) const
{
	return (*variables_)[ i ] == Kernel::THE_SLOT_VARIABLE;
}

Kernel::StateHandle
Kernel::Arguments::getState( size_t i )
{
	return (*states_)[ i ];
}

const Ast::SourceLocation &
Kernel::Arguments::getStateLoc( size_t i ) const
{
	return stateLocations_[ i ]->loc( );
}

size_t
Kernel::Arguments::size( ) const
{
	return variables_->size( );
}

bool
Kernel::Arguments::empty( ) const
{
	return variables_->empty( );
}

void
Kernel::Arguments::setMutatorSelf( Kernel::StateHandle mutatorSelf )
{
	mutatorSelf_ = mutatorSelf;
}

Kernel::StateHandle
Kernel::Arguments::getMutatorSelf( )
{
	if( mutatorSelf_ == 0 )
		{
			throw Exceptions::InternalError( "Kernel::Arguments::getMutatorSelf: self is null." );
		}
	return mutatorSelf_;
}

void
Kernel::Arguments::gcMark( Kernel::GCMarkedSet & marked )
{
	{
		typedef typeof *variables_ ListType;
		for( ListType::const_iterator i = variables_->begin( ); i != variables_->end( ); ++i )
			{
				const_cast< Kernel::Variable * >( i->getPtr( ) )->gcMark( marked );
			}
	}
}

Kernel::Environment::ValueVector
Kernel::Arguments::getVariables( )
{
	return variables_;
}

Kernel::Environment::StateVector
Kernel::Arguments::getStates( )
{
	return states_;
}


namespace Shapes
{
	namespace Kernel
	{

	class FunctionOneHandleCont : public Kernel::Continuation
	{
		RefCountPtr< const Lang::Function > fun_;
		Kernel::PassedDyn dyn_;
		Kernel::ContRef cont_;
	public:
		FunctionOneHandleCont( const RefCountPtr< const Lang::Function > & fun, const Kernel::PassedDyn & dyn, Kernel::ContRef cont, const Ast::SourceLocation & traceLoc )
			: Kernel::Continuation( traceLoc ), fun_( fun ), dyn_( dyn ), cont_( cont )
		{ }
		virtual ~FunctionOneHandleCont( ) { }
		virtual void takeHandle( Kernel::VariableHandle val, Kernel::EvalState * evalState, bool dummy ) const
		{
			/* This continuation really seeks forced arguments, for otherwise a thunk would have been generated directly.
			 * However, this continuation takes handles anyway, since handles is what goes into the argument list.
			 */

			if( val->isThunk( ) )
				{
					val->force( val, evalState );
					return;
				}

			Kernel::Arguments args = fun_->newCurriedArguments( );
			args.addOrderedArgument( val, & Ast::THE_INTERNAL_VALUE_EXPRESSION );

			evalState->dyn_ = dyn_;
			evalState->cont_ = cont_;
			fun_->call( evalState, args, traceLoc_ );
		}
		virtual void backTrace( std::list< Kernel::Continuation::BackTraceElem > * trace ) const
		{
			trace->push_front( Kernel::Continuation::BackTraceElem( this, "internal function call with one handle" ) );
			cont_->backTrace( trace );
		}
		virtual void gcMark( Kernel::GCMarkedSet & marked )
		{
			const_cast< Lang::Function * >( fun_.getPtr( ) )->gcMark( marked );
			dyn_->gcMark( marked );
			cont_->gcMark( marked );
		}
	};

	class FunctionTwoHandlesCont_2 : public Kernel::Continuation
	{
		RefCountPtr< const Lang::Function > fun_;
		Kernel::VariableHandle arg1_;
		Kernel::PassedDyn dyn_;
		Kernel::ContRef cont_;
	public:
		FunctionTwoHandlesCont_2( const RefCountPtr< const Lang::Function > & fun, const Kernel::VariableHandle & arg1, const Kernel::PassedDyn & dyn, Kernel::ContRef cont, const Ast::SourceLocation & traceLoc )
			: Kernel::Continuation( traceLoc ), fun_( fun ), arg1_( arg1 ), dyn_( dyn ), cont_( cont )
		{ }
		virtual ~FunctionTwoHandlesCont_2( ) { }
		virtual void takeHandle( Kernel::VariableHandle arg2, Kernel::EvalState * evalState, bool dummy ) const
		{
			/* This continuation really seeks forced arguments, for otherwise a thunk would have been generated directly.
			 * However, this continuation takes handles anyway, since handles is what goes into the argument list.
			 */

			if( arg2->isThunk( ) )
				{
					arg2->force( arg2, evalState );
					return;
				}

			Kernel::Arguments args = fun_->newCurriedArguments( );
			args.addOrderedArgument( arg1_, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			args.addOrderedArgument( arg2, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			evalState->dyn_ = dyn_;
			evalState->cont_ = cont_;
			fun_->call( evalState, args, traceLoc_ );
		}
		virtual void backTrace( std::list< Kernel::Continuation::BackTraceElem > * trace ) const
		{
			trace->push_front( Kernel::Continuation::BackTraceElem( this, "internal function call with two handles, second" ) );
			cont_->backTrace( trace );
		}
		virtual void gcMark( Kernel::GCMarkedSet & marked )
		{
			const_cast< Lang::Function * >( fun_.getPtr( ) )->gcMark( marked );
			dyn_->gcMark( marked );
			cont_->gcMark( marked );
		}
	};

	class FunctionTwoHandlesCont_1 : public Kernel::Continuation
	{
		RefCountPtr< const Lang::Function > fun_;
		Kernel::VariableHandle arg2_;
		bool forceArg2_;
		Kernel::PassedDyn dyn_;
		Kernel::ContRef cont_;
	public:
		FunctionTwoHandlesCont_1( const RefCountPtr< const Lang::Function > & fun, const Kernel::VariableHandle & arg2, bool forceArg2, const Kernel::PassedDyn & dyn, Kernel::ContRef cont, const Ast::SourceLocation & traceLoc )
			: Kernel::Continuation( traceLoc ), fun_( fun ), arg2_( arg2 ), forceArg2_( forceArg2 ), dyn_( dyn ), cont_( cont )
		{ }
		virtual ~FunctionTwoHandlesCont_1( ) { }
		virtual void takeHandle( Kernel::VariableHandle arg1, Kernel::EvalState * evalState, bool dummy ) const
		{
			/* This continuation really seeks forced arguments, for otherwise a thunk would have been generated directly.
			 * However, this continuation takes handles anyway, since handles is what goes into the argument list.
			 */

			if( arg1->isThunk( ) )
				{
					arg1->force( arg1, evalState );
					return;
				}

			if( forceArg2_ )
				{
					Kernel::ContRef newCont = Kernel::ContRef( new Kernel::FunctionTwoHandlesCont_2( fun_, arg1, dyn_, cont_, traceLoc_ ) );
					evalState->cont_ = newCont;
					newCont->takeHandle( arg2_, evalState );
					return;
				}

			/* The second handle need not be forced
			 */
			Kernel::Arguments args = fun_->newCurriedArguments( );
			args.addOrderedArgument( arg1, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			args.addOrderedArgument( arg2_, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			evalState->dyn_ = dyn_;
			evalState->cont_ = cont_;
			fun_->call( evalState, args, traceLoc_ );
		}
		virtual void backTrace( std::list< Kernel::Continuation::BackTraceElem > * trace ) const
		{
			trace->push_front( Kernel::Continuation::BackTraceElem( this, "internal function call with two handles, first" ) );
			cont_->backTrace( trace );
		}
		virtual void gcMark( Kernel::GCMarkedSet & marked )
		{
			const_cast< Lang::Function * >( fun_.getPtr( ) )->gcMark( marked );
			dyn_->gcMark( marked );
			cont_->gcMark( marked );
		}
	};

	class FunctionTwoHandlesOneStateCont_2 : public Kernel::Continuation
	{
		RefCountPtr< const Lang::Function > fun_;
		Kernel::VariableHandle arg1_;
		Kernel::StateHandle state_;
		Kernel::PassedDyn dyn_;
		Kernel::ContRef cont_;
	public:
		FunctionTwoHandlesOneStateCont_2( const RefCountPtr< const Lang::Function > & fun, const Kernel::VariableHandle & arg1, Kernel::StateHandle state, const Kernel::PassedDyn & dyn, Kernel::ContRef cont, const Ast::SourceLocation & traceLoc )
			: Kernel::Continuation( traceLoc ), fun_( fun ), arg1_( arg1 ), state_( state ), dyn_( dyn ), cont_( cont )
		{ }
		virtual ~FunctionTwoHandlesOneStateCont_2( ) { }
		virtual void takeHandle( Kernel::VariableHandle arg2, Kernel::EvalState * evalState, bool dummy ) const
		{
			/* This continuation really seeks forced arguments, for otherwise a thunk would have been generated directly.
			 * However, this continuation takes handles anyway, since handles is what goes into the argument list.
			 */

			if( arg2->isThunk( ) )
				{
					arg2->force( arg2, evalState );
					return;
				}

			Kernel::Arguments args = fun_->newCurriedArguments( );
			args.addOrderedArgument( arg1_, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			args.addOrderedArgument( arg2, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			args.addOrderedState( state_, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			evalState->dyn_ = dyn_;
			evalState->cont_ = cont_;
			fun_->call( evalState, args, traceLoc_ );
		}
		virtual void backTrace( std::list< Kernel::Continuation::BackTraceElem > * trace ) const
		{
			trace->push_front( Kernel::Continuation::BackTraceElem( this, "internal function call with two handles and one state, second" ) );
			cont_->backTrace( trace );
		}
		virtual void gcMark( Kernel::GCMarkedSet & marked )
		{
			const_cast< Lang::Function * >( fun_.getPtr( ) )->gcMark( marked );
			state_->gcMark( marked );
			dyn_->gcMark( marked );
			cont_->gcMark( marked );
		}
	};

	class FunctionTwoHandlesOneStateCont_1 : public Kernel::Continuation
	{
		RefCountPtr< const Lang::Function > fun_;
		Kernel::VariableHandle arg2_;
		bool forceArg2_;
		Kernel::StateHandle state_;
		Kernel::PassedDyn dyn_;
		Kernel::ContRef cont_;
	public:
		FunctionTwoHandlesOneStateCont_1( const RefCountPtr< const Lang::Function > & fun, const Kernel::VariableHandle & arg2, bool forceArg2, Kernel::StateHandle state, const Kernel::PassedDyn & dyn, Kernel::ContRef cont, const Ast::SourceLocation & traceLoc )
			: Kernel::Continuation( traceLoc ), fun_( fun ), arg2_( arg2 ), forceArg2_( forceArg2 ), state_( state ), dyn_( dyn ), cont_( cont )
		{ }
		virtual ~FunctionTwoHandlesOneStateCont_1( ) { }
		virtual void takeHandle( Kernel::VariableHandle arg1, Kernel::EvalState * evalState, bool dummy ) const
		{
			/* This continuation really seeks forced arguments, for otherwise a thunk would have been generated directly.
			 * However, this continuation takes handles anyway, since handles is what goes into the argument list.
			 */

			if( arg1->isThunk( ) )
				{
					arg1->force( arg1, evalState );
					return;
				}

			if( forceArg2_ )
				{
					Kernel::ContRef newCont = Kernel::ContRef( new Kernel::FunctionTwoHandlesOneStateCont_2( fun_, arg1, state_, dyn_, cont_, traceLoc_ ) );
					evalState->cont_ = newCont;
					newCont->takeHandle( arg2_, evalState );
					return;
				}

			/* The second handle need not be forced
			 */
			Kernel::Arguments args = fun_->newCurriedArguments( );
			args.addOrderedArgument( arg1, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			args.addOrderedArgument( arg2_, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			args.addOrderedState( state_, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
			evalState->dyn_ = dyn_;
			evalState->cont_ = cont_;
			fun_->call( evalState, args, traceLoc_ );
		}
		virtual void backTrace( std::list< Kernel::Continuation::BackTraceElem > * trace ) const
		{
			trace->push_front( Kernel::Continuation::BackTraceElem( this, "internal function call with two handles and one state, first" ) );
			cont_->backTrace( trace );
		}
		virtual void gcMark( Kernel::GCMarkedSet & marked )
		{
			const_cast< Lang::Function * >( fun_.getPtr( ) )->gcMark( marked );
			dyn_->gcMark( marked );
			cont_->gcMark( marked );
			state_->gcMark( marked );
		}
	};

	}
}


Lang::Function::Function( Kernel::EvaluatedFormals * formals )
	: formals_( formals )
{ }

DISPATCHIMPL( Function );

Lang::Function::~Function( )
{
	if( formals_ != 0 )
		{
			delete formals_->formals_;
			delete formals_;
		}
}

Kernel::ValueRef
Lang::Function::transformed( const Lang::Transform2D & tf, Kernel::ValueRef self ) const
{
	return Kernel::ValueRef( new Lang::TransformedFunction2D( tf, self.down_cast< const Lang::Function >( ) ) );
}

bool
Lang::Function::isProcedural( ) const
{
	return false;
}

bool
Lang::Function::inScope( Kernel::PassedEnv env ) const
{
	return true;
}

void
Lang::Function::call( Kernel::EvalState * evalState, const Kernel::ValueRef & arg1, const Ast::SourceLocation & callLoc ) const
{
	Kernel::Arguments args = this->newCurriedArguments( );

	args.addOrderedArgument( Kernel::VariableHandle( new Kernel::Variable( arg1 ) ), & Ast::THE_INTERNAL_VALUE_EXPRESSION );

	this->call( evalState, args, callLoc );
}

void
Lang::Function::analyze( Ast::Node * parent, const Ast::AnalysisEnvironment * env )
{
	throw Exceptions::InternalError( "A syntax function is not overriding the analyze method." );
}

void
Lang::Function::call( Kernel::EvalState * evalState, const Kernel::ValueRef & arg1, const Kernel::ValueRef & arg2, const Ast::SourceLocation & callLoc ) const
{
	Kernel::Arguments args = this->newCurriedArguments( );

	args.addOrderedArgument( Kernel::VariableHandle( new Kernel::Variable( arg1 ) ), & Ast::THE_INTERNAL_VALUE_EXPRESSION );
	args.addOrderedArgument( Kernel::VariableHandle( new Kernel::Variable( arg2 ) ), & Ast::THE_INTERNAL_VALUE_EXPRESSION );

	this->call( evalState, args, callLoc );
}

Ast::ArgListExprs * Lang::Function::oneExprArgList = new Ast::ArgListExprs( static_cast< size_t >( 1 ) );
Ast::ArgListExprs * Lang::Function::twoExprsArgList = new Ast::ArgListExprs( static_cast< size_t >( 2 ) );

void
Lang::Function::call( const RefCountPtr< const Lang::Function > & selfRef, Kernel::EvalState * evalState, const Kernel::VariableHandle & arg1, const Ast::SourceLocation & callLoc ) const
{
	const RefCountPtr< const Kernel::CallContInfo > info = this->newCallContInfo( Lang::Function::oneExprArgList, *evalState );

	if( info->force( 0 ) )
		{
			Kernel::ContRef cont = Kernel::ContRef( new Kernel::FunctionOneHandleCont( selfRef, evalState->dyn_, evalState->cont_, callLoc ) );
			evalState->cont_ = cont;
			cont->takeHandle( arg1, evalState );
			return;
		}

	/* The handle need not be forced
	 */
	Kernel::Arguments args = this->newCurriedArguments( );
	args.addOrderedArgument( arg1, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
	this->call( evalState, args, callLoc );
}

void
Lang::Function::call( const RefCountPtr< const Lang::Function > & selfRef, Kernel::EvalState * evalState, const Kernel::VariableHandle & arg1, const Kernel::VariableHandle & arg2, const Ast::SourceLocation & callLoc ) const
{
	const RefCountPtr< const Kernel::CallContInfo > info = this->newCallContInfo( Lang::Function::twoExprsArgList, *evalState );

	/* Remember that arguments are ordered backwards!
	 */

	if( info->force( 1 ) )
		{
			Kernel::ContRef cont = Kernel::ContRef( new Kernel::FunctionTwoHandlesCont_1( selfRef, arg2, info->force( 0 ), evalState->dyn_, evalState->cont_, callLoc ) );
			evalState->cont_ = cont;
			cont->takeHandle( arg1, evalState );
			return;
		}

	if( info->force( 0 ) )
		{
			Kernel::ContRef cont = Kernel::ContRef( new Kernel::FunctionTwoHandlesCont_2( selfRef, arg1, evalState->dyn_, evalState->cont_, callLoc ) );
			evalState->cont_ = cont;
			cont->takeHandle( arg2, evalState );
			return;
		}

	/* None of the handles need to be forced
	 */
	Kernel::Arguments args = this->newCurriedArguments( );
	args.addOrderedArgument( arg1, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
	args.addOrderedArgument( arg2, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
	this->call( evalState, args, callLoc );
}

void
Lang::Function::call( const RefCountPtr< const Lang::Function > & selfRef, Kernel::EvalState * evalState, const Kernel::VariableHandle & arg1, const Kernel::VariableHandle & arg2, Kernel::StateHandle state, const Ast::SourceLocation & callLoc ) const
{
	/* I'm not quite sure if we should also put a dummy state argument in info...
	 */
	const RefCountPtr< const Kernel::CallContInfo > info = this->newCallContInfo( Lang::Function::twoExprsArgList, *evalState );

	/* Remember that arguments are ordered backwards!
	 */

	if( info->force( 1 ) )
		{
			Kernel::ContRef cont = Kernel::ContRef( new Kernel::FunctionTwoHandlesOneStateCont_1( selfRef, arg2, info->force( 0 ), state, evalState->dyn_, evalState->cont_, callLoc ) );
			evalState->cont_ = cont;
			cont->takeHandle( arg1, evalState );
			return;
		}

	if( info->force( 0 ) )
		{
			Kernel::ContRef cont = Kernel::ContRef( new Kernel::FunctionTwoHandlesOneStateCont_2( selfRef, arg1, state, evalState->dyn_, evalState->cont_, callLoc ) );
			evalState->cont_ = cont;
			cont->takeHandle( arg2, evalState );
			return;
		}

	/* None of the handles need to be forced
	 */
	Kernel::Arguments args = this->newCurriedArguments( );
	args.addOrderedArgument( arg1, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
	args.addOrderedArgument( arg2, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
	args.addOrderedState( state, & Ast::THE_INTERNAL_VALUE_EXPRESSION );
	this->call( evalState, args, callLoc );
}


RefCountPtr< Kernel::CallContInfo >
Lang::Function::newCallContInfo( const Ast::ArgListExprs * argList, const Kernel::EvalState & evalState ) const
{
	return formals_->newCallContInfo( argList, evalState );
}

RefCountPtr< Kernel::CallContInfo >
Lang::Function::newCallContInfo( const Ast::ArgListExprs * argList, const Kernel::EvalState & evalState, const Kernel::Arguments & curryArgs ) const
{
	return formals_->newCallContInfo( argList, evalState, curryArgs );
}

Kernel::Arguments
Lang::Function::newCurriedArguments( ) const
{
	return Kernel::Arguments( formals_ );
}


RefCountPtr< const Lang::Class > Lang::Function::TypeID( new Lang::SystemFinalClass( strrefdup( "Function" ) ) );
TYPEINFOIMPL( Function );

Kernel::EvaluatedFormals::EvaluatedFormals( Kernel::Formals * formals )
	: selectiveForcing_( false ), forceAll_( false ), formals_( formals ), isSink_( true )
{ }

Kernel::EvaluatedFormals::EvaluatedFormals( const char * locationString )
	: selectiveForcing_( true ), formals_( 0 ), isSink_( true )
{
	Kernel::Formals * formals( new Kernel::Formals( ) );
	formals->setLoc( Ast::SourceLocation( locationString ) );
	formals_ = formals;
}

Kernel::EvaluatedFormals::EvaluatedFormals( const char * locationString, bool forceAll)
	: selectiveForcing_( false ), forceAll_( forceAll ), formals_( 0 ), isSink_( true )
{
	Kernel::Formals * formals( new Kernel::Formals( ) );
	formals->setLoc( Ast::SourceLocation( locationString ) );
	formals_ = formals;
}

Kernel::EvaluatedFormals::~EvaluatedFormals( )
{
	/* Don't delete the orderedFormals, since we don't own it.	A case for reference counting? */
}

RefCountPtr< Kernel::CallContInfo >
Kernel::EvaluatedFormals::newCallContInfo( const Ast::ArgListExprs * argList, const Kernel::EvalState & evalState ) const
{
	if( selectiveForcing_ )
		{
			return RefCountPtr< Kernel::CallContInfo >( new Kernel::CallContInfo( argList, evalState, formals_->newArgListForcePos( argList ) ) );
		}
	else
		{
			return RefCountPtr< Kernel::CallContInfo >( new Kernel::CallContInfo( argList, evalState, forceAll_ ) );
		}
}

RefCountPtr< Kernel::CallContInfo >
Kernel::EvaluatedFormals::newCallContInfo( const Ast::ArgListExprs * argList, const Kernel::EvalState & evalState, const Kernel::Arguments & curryArgs ) const
{
	if( selectiveForcing_ )
		{
			return RefCountPtr< Kernel::CallContInfo >( new Kernel::CallContInfo( argList, evalState, formals_->newArgListForcePos( argList, curryArgs ) ) );
		}
	else
		{
			return RefCountPtr< Kernel::CallContInfo >( new Kernel::CallContInfo( argList, evalState, forceAll_ ) );
		}
}

void
Kernel::EvaluatedFormals::appendEvaluatedFormal( const char * id, const Kernel::VariableHandle & defaultVal, const Ast::Node * loc, bool force )
{
	if( ! selectiveForcing_ )
		{
			throw Exceptions::InternalError( "EvaluatedFormals::appendEvaluatedFormal: Function does not have selective forcing." );
		}
	(*(formals_->argumentOrder_))[ id ] = defaults_.size( );
	formals_->forcePos_.push_back( force );
	defaults_.push_back( defaultVal );
	locations_.push_back( loc );
}

void
Kernel::EvaluatedFormals::appendEvaluatedFormal( const char * id, const Kernel::VariableHandle & defaultVal, const Ast::Node * loc )
{
	if( selectiveForcing_ )
		{
			throw Exceptions::InternalError( "EvaluatedFormals::appendEvaluatedFormal: Function requires individual forcing specification." );
		}
	(*(formals_->argumentOrder_))[ id ] = defaults_.size( );
	formals_->forcePos_.push_back( forceAll_ );
	defaults_.push_back( defaultVal );
	locations_.push_back( loc );
}

void
Kernel::EvaluatedFormals::appendEvaluatedCoreFormal( const char * id, const Kernel::VariableHandle & defaultVal, bool force )
{
	appendEvaluatedFormal( id, defaultVal, & Ast::THE_CORE_DEFAULT_VALUE_EXPRESSION, force );
}

void
Kernel::EvaluatedFormals::appendEvaluatedCoreFormal( const char * id, const Kernel::VariableHandle & defaultVal )
{
	appendEvaluatedFormal( id, defaultVal, & Ast::THE_CORE_DEFAULT_VALUE_EXPRESSION );
}

void
Kernel::EvaluatedFormals::appendCoreStateFormal( const char * id )
{
	(*(formals_->stateOrder_))[ id ] = formals_->stateOrder_->size( );
}

void
Kernel::EvaluatedFormals::gcMark( Kernel::GCMarkedSet & marked )
{
	typedef typeof defaults_ ListType;
	for( ListType::const_iterator i = defaults_.begin( ); i != defaults_.end( ); ++i )
		{
			if( *i != NullPtr< Kernel::Variable >( ) )
				{
					const_cast< Kernel::Variable * >( i->getPtr( ) )->gcMark( marked );
				}
		}
}


Lang::CuteFunction::CuteFunction( RefCountPtr< const Lang::Function > callee, const Kernel::Arguments & someArgs )
	: Lang::Function( 0 ), callee_( callee), someArgs_( someArgs.clone( ) )
{ }

Lang::CuteFunction::~CuteFunction( )
{ }

RefCountPtr< Kernel::CallContInfo >
Lang::CuteFunction::newCallContInfo( const Ast::ArgListExprs * argList, const Kernel::EvalState & evalState ) const
{
	return callee_->newCallContInfo( argList, evalState, someArgs_ );
}

RefCountPtr< Kernel::CallContInfo >
Lang::CuteFunction::newCallContInfo( const Ast::ArgListExprs * argList, const Kernel::EvalState & evalState, const Kernel::Arguments & curryArgs ) const
{
	/* when we are the callee of a CuteFunction, our someArgs are part of that CuteFunction's someArgs, and hence curryArgs 
		 contains everything that is to be passed to our callee.
	*/
	return callee_->newCallContInfo( argList, evalState, curryArgs );
}

Kernel::Arguments
Lang::CuteFunction::newCurriedArguments( ) const
{
	return someArgs_.clone( );
}

void
Lang::CuteFunction::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	/* Note that curruying "takes place" in newCurriedArguments.	Other than that, this is the same as the original function.
		 Also note that the total set of arguments being passed to the original function is no more than someArgs and what was passed in the last call.
	 */
	callee_->call( evalState, args, callLoc );
}

bool
Lang::CuteFunction::isTransforming( ) const
{
	return callee_->isTransforming( );
}

void
Lang::CuteFunction::gcMark( Kernel::GCMarkedSet & marked )
{
	const_cast< Lang::Function * >( callee_.getPtr( ) )->gcMark( marked );
	someArgs_.gcMark( marked );
}


Lang::ComposedFunction::ComposedFunction( const RefCountPtr< const Lang::Function > & second, const RefCountPtr< const Lang::Function > & first )
	: Lang::Function( 0 ), second_( second ), first_( first )
{ }

Lang::ComposedFunction::~ComposedFunction( )
{ }

void
Lang::ComposedFunction::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	/* Note that curruying "takes place" in newCurriedArguments.	Other than that, this is the same as the original function.
		 Also note that the total set of arguments being passed to the original function is no more than someArgs and what was passed in the last call.
	 */
	evalState->cont_ = Kernel::ContRef( new Kernel::ComposedFunctionCall_cont( second_, evalState->dyn_, evalState->cont_, callLoc ) );
	first_->call( evalState, args, callLoc );
}

RefCountPtr< Kernel::CallContInfo >
Lang::ComposedFunction::newCallContInfo( const Ast::ArgListExprs * argList, const Kernel::EvalState & evalState ) const
{
	return first_->newCallContInfo( argList, evalState );
}

RefCountPtr< Kernel::CallContInfo >
Lang::ComposedFunction::newCallContInfo( const Ast::ArgListExprs * argList, const Kernel::EvalState & evalState, const Kernel::Arguments & curryArgs ) const
{
	return first_->newCallContInfo( argList, evalState, curryArgs );
}

Kernel::Arguments
Lang::ComposedFunction::newCurriedArguments( ) const
{
	return first_->newCurriedArguments( );
}

bool
Lang::ComposedFunction::isTransforming( ) const
{
	return second_->isTransforming( );
}

void
Lang::ComposedFunction::gcMark( Kernel::GCMarkedSet & marked )
{
	const_cast< Lang::Function * >( second_.getPtr( ) )->gcMark( marked );
	const_cast< Lang::Function * >( first_.getPtr( ) )->gcMark( marked );
}


Lang::UserFunction::UserFunction( Kernel::EvaluatedFormals * formals, Ast::Expression * body, Kernel::PassedEnv env, const Ast::FunctionMode & functionMode )
	: Lang::Function( formals ), body_( body ), env_( env ), functionMode_( functionMode )
{ }

// DISPATCHIMPL( UserFunction );

Lang::UserFunction::~UserFunction( )
{
	// Note that we don't delete the things that we most likely share with other objects
	// Reference counting could be used here, but there will never be more such things than there are function expressions in the source

	// This prevents formals->formals from being deleted by Lang::Function::~Function
	delete formals_;
	formals_ = 0;
}

void
Lang::UserFunction::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	args.applyDefaults( );
	evalState->env_ = new Kernel::Environment( Kernel::theEnvironmentList, env_, formals_->formals_->argumentOrder_, args.getVariables( ), formals_->formals_->stateOrder_, args.getStates( ) );

	if( ! this->isProcedural( ) )
		{
			evalState->env_->activateFunctionBoundary( );
		}

	body_->eval( evalState );
}

bool
Lang::UserFunction::isTransforming( ) const
{
	return ( functionMode_ & Ast::FUNCTION_TRANSFORMING ) != 0;
}

bool
Lang::UserFunction::isProcedural( ) const
{
	return ( functionMode_ & Ast::FUNCTION_PROCEDURAL ) != 0;
}

bool
Lang::UserFunction::inScope( Kernel::PassedEnv env ) const
{
	return ! isProcedural( )
		|| env_->isProceduralParentOf( env );
}

void
Lang::UserFunction::gcMark( Kernel::GCMarkedSet & marked )
{
	env_->gcMark( marked );
}


Lang::TransformedFunction2D::TransformedFunction2D( const Lang::Transform2D & tf, const RefCountPtr< const Lang::Function > & fun )
	: Lang::Function( 0 ), tf_( tf ), fun_( fun )
{ }

Lang::TransformedFunction2D::~TransformedFunction2D( )
{ }

void
Lang::TransformedFunction2D::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	evalState->cont_ = Kernel::ContRef( new Kernel::Transform2DCont( tf_, evalState->cont_, 0 ) );
	fun_->call( evalState, args, callLoc );
}

bool
Lang::TransformedFunction2D::isTransforming( ) const
{
	return fun_->isTransforming( );
}

void
Lang::TransformedFunction2D::gcMark( Kernel::GCMarkedSet & marked )
{
	const_cast< Lang::Function * >( fun_.getPtr( ) )->gcMark( marked );
}


Lang::VectorFunction::VectorFunction( const std::vector< Kernel::ValueRef > * mem )
	: Lang::Function( new Kernel::EvaluatedFormals( "<vector>", true ) ), mem_( mem ),
		memNumeric_( NullPtr< const std::vector< double > >( ) )
{
	formals_->appendEvaluatedCoreFormal( "index", Kernel::THE_SLOT_VARIABLE );
}

// DISPATCHIMPL( VectorFunction );

Lang::VectorFunction::~VectorFunction( )
{ }

Kernel::VariableHandle
Lang::VectorFunction::getField( const char * fieldID, const RefCountPtr< const Lang::Value > & selfRef ) const
{
	if( strcmp( fieldID, "size" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Integer( mem_->size( ) ) );
		}
	throw Exceptions::NonExistentMember( getTypeName( ), fieldID );
}

const char * Lang::VectorFunction::title_ = "<vector>";

void
Lang::VectorFunction::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	const size_t ARITY = 1;
	if( args.size( ) != ARITY )
		{
			throw Exceptions::CoreArityMismatch( title_, ARITY, args.size( ) );
		}

	typedef const Lang::Integer ArgType;
	RefCountPtr< ArgType > arg = Helpers::down_cast_CoreArgument< ArgType >( title_, args, 0, callLoc );

	if( arg->val_ < 0 )
		{
			throw Exceptions::CoreOutOfRange( title_, args, 0, "Index is negative." );
		}

	if( arg->val_ >= static_cast< int >( mem_->size( ) ) )
		{
			throw Exceptions::CoreOutOfRange( title_, args, 0, "Index exceeds vector size." );
		}

	Kernel::ContRef cont = evalState->cont_;
	cont->takeValue( (*mem_)[ arg->val_ ],
									 evalState );
}

bool
Lang::VectorFunction::isTransforming( ) const
{
	return false;
}

void
Lang::VectorFunction::gcMark( Kernel::GCMarkedSet & marked )
{
	for( std::vector< Kernel::ValueRef >::const_iterator i = mem_->begin( ); i != mem_->end( ); ++i )
		{
			const_cast< Lang::Value * >( i->getPtr( ) )->gcMark( marked );
		}
}

RefCountPtr< const std::vector< double > >
Lang::VectorFunction::getNumeric( const Ast::SourceLocation & callLoc ) const
{
	if( memNumeric_ == NullPtr< const std::vector< double > >( ) )
		{
			RefCountPtr< std::vector< double > > res( new std::vector< double > ); // Note that this is not const, so far...
			res->reserve( mem_->size( ) );
			typedef typeof *mem_ SrcType;
			for( SrcType::const_iterator src = mem_->begin( ); src != mem_->end( ); ++src )
				{
					res->push_back( Helpers::down_cast< const Lang::Float >( *src, callLoc )->val_ );
				}

			memNumeric_ = res;
		}
	return memNumeric_;
}


const char * Lang::ColorInterpolator::title_ = "<color-interpolator>";

Lang::ColorInterpolator::ColorInterpolator( const RefCountPtr< KeyContainer > & key,
                                            const RefCountPtr< RGBContainer > & RGBcolor,
                                            const RefCountPtr< GrayContainer > & graycolor,
                                            const RefCountPtr< CMYKContainer > & CMYKcolor,
                                            ColorType colorType)
	: Lang::Function( new Kernel::EvaluatedFormals( Lang::ColorInterpolator::title_, true ) ),
	  key_( key ), RGBcolor_ ( RGBcolor ), graycolor_ ( graycolor ), CMYKcolor_ ( CMYKcolor ), colorType_(colorType)
{
	formals_->appendEvaluatedCoreFormal( "key", Kernel::THE_SLOT_VARIABLE );
}

Lang::ColorInterpolator::~ColorInterpolator( )
{ }

Kernel::VariableHandle
Lang::ColorInterpolator::getField( const char * fieldID, const RefCountPtr< const Lang::Value > & selfRef ) const
{
	if( strcmp( fieldID, "low" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Float( key_->front( ) ) );
		}
	if( strcmp( fieldID, "high" ) == 0 )
		{
			return Helpers::newValHandle( new Lang::Float( key_->back( ) ) );
		}
	throw Exceptions::NonExistentMember( getTypeName( ), fieldID );
}


template <class COLOR_TYPE, class COLOR_CONTAINER>
void
Lang::ColorInterpolator::callHelper( Kernel::EvalState * evalState,
																		 const RefCountPtr< COLOR_CONTAINER > & colorContainer,
																		 double key, KeyContainer::const_iterator keyHi ) const
{
	if( keyHi == key_->end( ) )
		{
			Kernel::ContRef cont = evalState->cont_;
						cont->takeValue( RefCountPtr< const::Lang::Value >( new COLOR_TYPE( colorContainer->back( ) ) ),
						                 evalState );
			return;
		}

	if( keyHi == key_->begin( ) )
		{
			Kernel::ContRef cont = evalState->cont_;
			cont->takeValue( RefCountPtr< const::Lang::Value >( new COLOR_TYPE( colorContainer->front( ) ) ),
											 evalState );
			return;
		}

	KeyContainer::const_iterator keyLo = keyHi - 1;
	double rem = ( key - *keyLo ) / ( *keyHi  - *keyLo );

	Kernel::ContRef cont = evalState->cont_;

	typename COLOR_CONTAINER::const_iterator colorHi = colorContainer->begin( ) + ( keyHi - key_->begin( ) );
	typename COLOR_CONTAINER::const_iterator colorLo = colorHi - 1;

	cont->takeValue( RefCountPtr< const::Lang::Value >( new COLOR_TYPE( colorLo->mulNoCheck( 1 - rem ).addNoCheck( colorHi->mulNoCheck( rem ) ) ) ),
					                 evalState );
}


void
Lang::ColorInterpolator::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	const size_t ARITY = 1;
	if( args.size( ) != ARITY )
		{
			throw Exceptions::CoreArityMismatch( title_, ARITY, args.size( ) );
		}

	typedef const Lang::Float ArgType;
	double key = Helpers::down_cast_CoreArgument< ArgType >( title_, args, 0, callLoc )->val_;

	KeyContainer::const_iterator keyHi = lower_bound( key_->begin( ), key_->end( ), key );
	switch( colorType_ )
		{
			case RGB:
				callHelper< Lang::RGB >( evalState, RGBcolor_, key, keyHi );
				break;
			case GRAY:
				callHelper< Lang::Gray >( evalState, graycolor_, key, keyHi );
				break;
			case CMYK:
				callHelper< Lang::CMYK >( evalState, CMYKcolor_, key, keyHi );
				break;
			case UNDEFINED:
				throw Exceptions::InternalError( "ColorInterpolator::call: Did not expect UNDEFINED in ennum switch." );
		}
}

bool
Lang::ColorInterpolator::isTransforming( ) const
{
	return false;
}

void
Lang::ColorInterpolator::gcMark( Kernel::GCMarkedSet & marked )
{ }


Lang::BinaryOperatorFunction::BinaryOperatorFunction( Ast::BinaryInfixExpr * opExpr, const char * title )
	: Lang::Function( new Kernel::EvaluatedFormals( title, true ) ), opExpr_( opExpr ), title_( title )
{
	formals_->appendEvaluatedCoreFormal( "first", Kernel::THE_SLOT_VARIABLE );
	formals_->appendEvaluatedCoreFormal( "second", Kernel::THE_SLOT_VARIABLE );
}

Lang::BinaryOperatorFunction::~BinaryOperatorFunction( )
{
	delete opExpr_;
}

void
Lang::BinaryOperatorFunction::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	const size_t ARITY = 2;
	CHECK_ARITY( args, ARITY, title_ );
	RefCountPtr< const Lang::Value > arg1 = args.getValue( 0 );
	RefCountPtr< const Lang::Value > arg2 = args.getValue( 1 );

	try
		{
			Kernel::ContRef cont = evalState->cont_;
			cont->takeValue( arg1->binaryDispatch1( arg1, arg2, evalState->dyn_, opExpr_ ),
											 evalState );
		}
	catch( Exceptions::BinaryInfixNotApplicable & ball )
		{
			ball.setOperatorSymbol( title_ );
			throw;
		}
}

bool
Lang::BinaryOperatorFunction::isTransforming( ) const
{
	return false;
}


Lang::UnaryOperatorFunction::UnaryOperatorFunction( Ast::UnaryExpr * opExpr, const char * title )
	: Lang::Function( new Kernel::EvaluatedFormals( title, true ) ), opExpr_( opExpr ), title_( title )
{
	formals_->appendEvaluatedCoreFormal( "only", Kernel::THE_SLOT_VARIABLE );
}

Lang::UnaryOperatorFunction::~UnaryOperatorFunction( )
{
	delete opExpr_;
}

void
Lang::UnaryOperatorFunction::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	const size_t ARITY = 1;
	CHECK_ARITY( args, ARITY, title_ );

	RefCountPtr< const Lang::Value > arg = args.getValue( 0 );

	try
		{
			Kernel::ContRef cont = evalState->cont_;
			cont->takeValue( arg->unaryDispatch( arg, evalState->dyn_, opExpr_ ),
											 evalState );
		}
	catch( Exceptions::UnaryPrefixNotApplicable & ball )
		{
			ball.setOperatorSymbol( title_ );
			throw;
		}
	catch( Exceptions::UnaryPostfixNotApplicable & ball )
		{
			ball.setOperatorSymbol( title_ );
			throw;
		}
}

bool
Lang::UnaryOperatorFunction::isTransforming( ) const
{
	return false;
}


Lang::Transform2DMethod_chop::Transform2DMethod_chop( RefCountPtr< const Lang::Transform2D > self, const char * fullMethodID )
	: Lang::MethodBase< class_type >( self, fullMethodID, false, true )
{
	formals_->appendEvaluatedCoreFormal( "L", Kernel::THE_SLOT_VARIABLE );
	formals_->appendEvaluatedCoreFormal( "p", Kernel::THE_SLOT_VARIABLE );
}

Lang::Transform2DMethod_chop::~Transform2DMethod_chop( )
{ }

void
Lang::Transform2DMethod_chop::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	args.applyDefaults( );

	size_t argsi = 0;
	double Ltol = Helpers::down_cast_CoreArgument< const Lang::Float >( title_, args, argsi, callLoc )->val_;
	if( Ltol < 0 )
		{
			throw Exceptions::CoreOutOfRange( title_, args, argsi, "Tolerances must not be negative." );
		}

	++argsi;
	Concrete::Length ptol = Helpers::down_cast_CoreArgument< const Lang::Length >( title_, args, argsi, callLoc )->get( );
	if( ptol < Concrete::ZERO_LENGTH )
		{
			throw Exceptions::CoreOutOfRange( title_, args, argsi, "Tolerances must not be negative." );
		}

	Lang::Transform2D * res = self_->clone( );

	CHOP_Ltol( res->xx_ );
	CHOP_Ltol( res->yx_ );

	CHOP_Ltol( res->xy_ );
	CHOP_Ltol( res->yy_ );

	CHOP_ptol( res->xt_ );
	CHOP_ptol( res->yt_ );

	Kernel::ContRef cont = evalState->cont_;
	cont->takeValue( RefCountPtr< const Lang::Value >( res ),
									 evalState );
}


Lang::Transform3DMethod_chop::Transform3DMethod_chop( RefCountPtr< const Lang::Transform3D > self, const char * fullMethodID )
	: Lang::MethodBase< class_type >( self, fullMethodID, false, true )
{
	formals_->appendEvaluatedCoreFormal( "L", Kernel::THE_SLOT_VARIABLE );
	formals_->appendEvaluatedCoreFormal( "p", Kernel::THE_SLOT_VARIABLE );
}

Lang::Transform3DMethod_chop::~Transform3DMethod_chop( )
{ }

void
Lang::Transform3DMethod_chop::call( Kernel::EvalState * evalState, Kernel::Arguments & args, const Ast::SourceLocation & callLoc ) const
{
	args.applyDefaults( );

	size_t argsi = 0;
	double Ltol = Helpers::down_cast_CoreArgument< const Lang::Float >( title_, args, argsi, callLoc )->val_;
	if( Ltol < 0 )
		{
			throw Exceptions::CoreOutOfRange( title_, args, argsi, "Tolerances must not be negative." );
		}

	++argsi;
	Concrete::Length ptol = Helpers::down_cast_CoreArgument< const Lang::Length >( title_, args, argsi, callLoc )->get( );
	if( ptol < Concrete::ZERO_LENGTH )
		{
			throw Exceptions::CoreOutOfRange( title_, args, argsi, "Tolerances must not be negative." );
		}

	Lang::Transform3D * res = self_->clone( );

	CHOP_Ltol( res->xx_ );
	CHOP_Ltol( res->yx_ );
	CHOP_Ltol( res->zx_ );

	CHOP_Ltol( res->xy_ );
	CHOP_Ltol( res->yy_ );
	CHOP_Ltol( res->zy_ );

	CHOP_Ltol( res->xz_ );
	CHOP_Ltol( res->yz_ );
	CHOP_Ltol( res->zz_ );

	CHOP_ptol( res->xt_ );
	CHOP_ptol( res->yt_ );
	CHOP_ptol( res->zt_ );

	Kernel::ContRef cont = evalState->cont_;
	cont->takeValue( RefCountPtr< const Lang::Value >( res ),
									 evalState );
}
