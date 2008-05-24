#include "Shapes_Kernel_decls.h"

#include "simplepdfi.h"
#include "simplepdfo.h"
#include "globals.h"
#include "consts.h"
#include "shapesexceptions.h"
#include "hottypes.h"
#include "multipage.h"
#include "continuations.h"
#include "charconverters.h"
#include "pagecontentstates.h"
#include "autoonoff.h"
#include "shapesscanner.h"
#include "texlabelmanager.h"
#include "debuglog.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <list>
#include <time.h>
#include <errno.h>
#include <limits>
#include <iconv.h>

int shapesparse( );
extern int shapesdebug;
void printVersion( );

using namespace Shapes;
using namespace SimplePDF;

void argcAssertion( const char * optionSpecifier, int argc, int argcMin );
bool strprefixcmp( char * str, const char * prefix, char ** endp );
bool strtobool( const char * str, const char * containingString, const char * trueLabel = 0, const char * falseLabel = 0 );
std::string callDir;
std::string absoluteFilename( const char * filename );
std::string absoluteDirectory( const char * filename );
void ensureTmpDirectoryExists( const std::string & dirname, bool allowCreate );
RefCountPtr< std::ifstream > performIterativeStartup( const std::string & texJobName );
void abortProcedure( std::ofstream * oFile, const std::string & outputName );
void setupGlobals( );
enum XpdfAction{ XPDF_DEFAULT, XPDF_RAISE, XPDF_RELOAD, XPDF_QUIT, XPDF_NOSERVER };
void xpdfHelper( const std::string & filename, const std::string & server, const XpdfAction & action );
void openHelper( const std::string & filename, const char * application );
void addDefaultNeedPath( );
void addDefaultFontMetricsPath( );
void destroyGlobals( );

namespace Shapes
{
	namespace Interaction
	{
		void systemDebugMessage( const std::string & msg );
	}
}

int
main( int argc, char ** argv )
{
	srand( time(NULL) );

	setupGlobals( );

	bool iterativeMode = true;
	bool useResources = true;

	{
		char * cwd = getcwd( 0, 0 );
		callDir = cwd + std::string( "/" );
		free( cwd );
	}

	std::string outDir;
	std::string tmpDir;
	bool allowCreateTmpDir = false;
	std::string baseName;
	std::string inputName;
	std::string outputName;
	std::string texJobName;
	std::string labelDBName;
	std::string fontmetricsOutputName;

	enum FilenameRequests{ FILENAME_RESOURCE, FILENAME_IN, FILENAME_OUT, FILENAME_TMP, FILENAME_TEXJOB, FILENAME_LABELDB, FILENAME_AFM, FILENAME_TEXINPUTS };

	std::list< int > filenameRequestList;
	std::list< const char * > resourceRequestList;

	bool evalTrace = false;
	bool evalBackTrace = false;
	bool cleanupMemory = true;
	bool memoryStats = false;
	bool launch_xpdf = false;
	SimplePDF::PDF_Version::Version pdfVersion = SimplePDF::PDF_Version::VERSION_UNDEFINED;
	SimplePDF::PDF_Version::Action pdfVersionAction = SimplePDF::PDF_Version::WARN;
	XpdfAction xpdfAction = XPDF_DEFAULT;
	std::string xpdfServer;
	bool do_open = false;
	const char * do_open_application = 0;
	std::ostringstream prependStreamOut;
	bool splitCatalog = false;

	argc -= 1;
	argv += 1;
	while( argc > 0 )
		{
			char * optionSuffix;
			if( strcmp( *argv, "--yydebug" ) == 0 )
				{
					shapesdebug = 1;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--shapes-debug" ) == 0 )
				{
					shapesdebug = 1;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--system-debug" ) == 0 )
				{
					Interaction::debugSystem = true;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--afm-debug" ) == 0 )
				{
					Interaction::fontMetricDebug = true;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--afm-messages" ) == 0 )
				{
					Interaction::fontMetricMessages = true;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--tex-debug" ) == 0 )
				{
					Interaction::pdfLaTeXInteractionTo_stderr = true;
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--bytecolumn=", & optionSuffix ) )
				{
					Interaction::characterColumnInBytes = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--debugstep=", & optionSuffix ) )
				{
					char * endp;
					int tmp = strtol( optionSuffix, & endp, 10 );
					if( tmp < 0 )
						{
							std::cerr << "The --debugstep value must be nonnegative: " << optionSuffix << std::endl ;
							exit( 1 );
						}
					Interaction::debugStep = static_cast< size_t >( tmp );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--debuglog" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					try
						{
							Kernel::theDebugLog.setFilename( *( argv + 1 ) );
						}
					catch( const char * ball )
						{
							std::cerr << ball << std::endl ;
							exit( 1 );
						}
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--debuglog-stderr" ) == 0 )
				{
					try
						{
							Kernel::theDebugLog.setStream( & std::cerr );
						}
					catch( const char * ball )
						{
							std::cerr << ball << std::endl ;
							exit( 1 );
						}
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--debuglog-stdout" ) == 0 )
				{
					try
						{
							Kernel::theDebugLog.setStream( & std::cout );
						}
					catch( const char * ball )
						{
							std::cerr << ball << std::endl ;
							exit( 1 );
						}
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--dtminerror=", & optionSuffix ) )
				{
					Computation::dtMinIsError = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--fmguesserror=", & optionSuffix ) )
				{
					Computation::fontMetricGuessIsError = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--evaltrace" ) == 0 )
				{
					evalTrace = true;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--evalbacktrace" ) == 0 )
				{
					evalTrace = true;
					evalBackTrace = true;
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--backtrace=", & optionSuffix ) )
				{
					Interaction::debugBacktrace = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--iteration=", & optionSuffix ) )
				{
					iterativeMode = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--resources=", & optionSuffix ) )
				{
					useResources = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--stats=", & optionSuffix ) )
				{
					memoryStats = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--memclean=", & optionSuffix ) )
				{
					cleanupMemory = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--showfiles=", & optionSuffix ) )
				{
					Ast::theShapesScanner.setShowFiles( strtobool( optionSuffix, *argv ) );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--pdf-version=", & optionSuffix ) || /* Note that we use that || shortcuts! */
							 strprefixcmp( *argv, "-v", & optionSuffix ) )
				{
					if( pdfVersion != SimplePDF::PDF_Version::VERSION_UNDEFINED )
						{
							std::cerr << "Multiply defined pdf version." << std::endl ;
							exit( 1 );
						}

					switch( *optionSuffix )
						{
						case 'e':
							pdfVersionAction = SimplePDF::PDF_Version::ERROR;
							break;
						case 'w':
							pdfVersionAction = SimplePDF::PDF_Version::WARN;
							break;
						case 's':
							pdfVersionAction = SimplePDF::PDF_Version::SILENT;
							break;
						default:
							std::cerr << "The only allowed action-characters in the pdf version specification are: \"e\" (error), \"w\" (warn), and \"s\" (silent).  You said \"" << *optionSuffix << "\", being the first character in \"" << optionSuffix << "\"." << std::endl ;
							exit( 1 );
						}
					++optionSuffix;
					if( strncmp( optionSuffix, "1.", 2 ) == 0 &&
							'1' <= optionSuffix[2] && optionSuffix[2] <= '6' )
						{
							SimplePDF::PDF_Version::Version versions[] = { SimplePDF::PDF_Version::PDF_1_1, SimplePDF::PDF_Version::PDF_1_2, SimplePDF::PDF_Version::PDF_1_3, SimplePDF::PDF_Version::PDF_1_4, SimplePDF::PDF_Version::PDF_1_5, SimplePDF::PDF_Version::PDF_1_6 };
							pdfVersion = versions[ optionSuffix[2] - '1' ];
						}
					else if( strcmp( optionSuffix, "X" ) == 0 )
						{
							std::cerr << "Restriction to PDF-X is not implemented, please try using a low version number, such as 1.1 instead." << std::endl ;
						}
					else
						{
							std::cerr << "Unsupported pdf version specification: " << optionSuffix << std::endl ;
							exit( 1 );
						}
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--tp=", & optionSuffix ) )
				{
					Kernel::allowTransparency = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--spot-pair=", & optionSuffix ) )
				{
					Kernel::allowSingletonPaths = ! strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--unit=", & optionSuffix ) )
				{
					Interaction::displayUnitName = optionSuffix;

					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--splicingtol=", & optionSuffix ) )
				{
					char * endp;
					Computation::theTrixelizeSplicingTol = strtod( optionSuffix, &endp );
					if( *endp != '\0' )
						{
							std::cerr << "Argument to --splicingtol= was not a float: " << optionSuffix << std::endl ;
							exit( 1 );
						}
					if( Computation::theTrixelizeSplicingTol <= 0 )
						{
							std::cerr << "Argument to --splicingtol not positive: " << Computation::theTrixelizeSplicingTol.offtype< 1, 0 >( ) << std::endl ;
							exit( 1 );
						}

					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--overlaptol=", & optionSuffix ) )
				{
					char * endp;
					Computation::theTrixelizeOverlapTol = strtod( optionSuffix, &endp );
					if( *endp != '\0' )
						{
							std::cerr << "Argument to --overlaptol was not a float: " << optionSuffix << std::endl ;
							exit( 1 );
						}
					if( Computation::theTrixelizeOverlapTol <= 0 )
						{
							std::cerr << "Argument to --overlaptol not positive: " << Computation::theTrixelizeOverlapTol.offtype< 1, 0 >( ) << std::endl ;
							exit( 1 );
						}

					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--needpath" ) == 0 ||
							 strncmp( *argv, "-N", 2 ) == 0 )
				{
					bool longForm = strncmp( *argv, "--", 2 ) == 0;

					const char * pth = 0;
					if( longForm )
						{
							argcAssertion( *argv, argc, 2 );
							pth = *( argv + 1 );
						}
					else
						{
							pth = (*argv) + 2;
						}

					if( strchr( pth, ':' ) != 0 )
						{
							const char * flag = 0;
							const char * shortFlag = "-N";
							if( longForm )
								{
									flag = *argv;
								}
							else
								{
									flag = shortFlag;
								}

							std::cerr << "The path separator ':' is not allowed in the " << flag << " argument.	Consider repeating " << flag ;
							if( longForm )
								{
									std::cerr << " (or " << shortFlag << ")" ;
								}
							std::cerr <<"." << std::endl ;
							exit( 1 );
						}

 					try
						{
							Ast::theShapesScanner.push_backNeedPath( absoluteDirectory( pth ) );
						}
					catch( const Exceptions::Exception & ball )
						{
							std::cout.flush( );
							ball.display( std::cerr );
							exit( 1 );
						}

					if( longForm )
						{
							argv += 2;
							argc -= 2;
						}
					else
						{
							argv += 1;
							argc -= 1;
						}
				}
			else if( strcmp( *argv, "--fontmetricspath" ) == 0 ||
							 strncmp( *argv, "-M", 2 ) == 0 )
				{
					bool longForm = strncmp( *argv, "--", 2 ) == 0;

					const char * pth = 0;
					if( longForm )
						{
							argcAssertion( *argv, argc, 2 );
							pth = *( argv + 1 );
						}
					else
						{
							pth = (*argv) + 2;
						}

					if( strchr( pth, ':' ) != 0 )
						{
							const char * flag = 0;
							const char * shortFlag = "-M";
							if( longForm )
								{
									flag = *argv;
								}
							else
								{
									flag = shortFlag;
								}

							std::cerr << "The path separator ':' is not allowed in the " << flag << " argument.	Consider repeating " << flag ;
							if( longForm )
								{
									std::cerr << " (or " << shortFlag << ")" ;
								}
							std::cerr <<"." << std::endl ;
							exit( 1 );
						}

					Lang::Font::push_backFontMetricsPath( absoluteDirectory( pth ) );

					if( longForm )
						{
							argv += 2;
							argc -= 2;
						}
					else
						{
							argv += 1;
							argc -= 1;
						}
				}
			else if( strprefixcmp( *argv, "--seed=", & optionSuffix ) )
				{
					char * endp;
					long s = strtol( optionSuffix, &endp, 10 );
					if( *endp != '\0' )
						{
							std::cerr << "Argument to --seed= was not an integer: " << optionSuffix << std::endl ;
							exit( 1 );
						}

					srand( s );

					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--arcdelta=", & optionSuffix ) )
				{
					char * endp;
					Computation::the_arcdelta = strtod( optionSuffix, &endp );
					if( *endp != '\0' )
						{
							std::cerr << "Argument to --arcdelta= was not a float: " << optionSuffix << std::endl ;
							exit( 1 );
						}
					if( Computation::the_arcdelta <= 0 )
						{
							std::cerr << "Argument to --arcdelta= not positive: " << Computation::the_arcdelta.offtype< 1, 0 >( ) << std::endl ;
							exit( 1 );
						}

					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--dtmin=", & optionSuffix ) )
				{
					char * endp;
					Computation::the_dtMin = strtod( optionSuffix, &endp );
					if( *endp != '\0' )
						{
							std::cerr << "Argument to --dtmin= was not a float: " << optionSuffix << std::endl ;
							exit( 1 );
						}
					if( Computation::the_dtMin <= 0 )
						{
							std::cerr << "Argument to --dtmin= not positive: " << Computation::the_dtMin << std::endl ;
							exit( 1 );
						}

					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--prepend" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					prependStreamOut << *( argv + 1 ) << std::endl ;
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--base" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( baseName != "" )
						{
							std::cerr << "The name base is multiply specified." << std::endl ;
							exit( 1 );
						}
					baseName = *( argv + 1 );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--which" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					filenameRequestList.push_back( FILENAME_RESOURCE );
					resourceRequestList.push_back( *( argv + 1 ) );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--which-in" ) == 0 )
				{
					filenameRequestList.push_back( FILENAME_IN );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--in" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( inputName != "" )
						{
							std::cerr << "The input file is multiply specified." << std::endl ;
							exit( 1 );
						}
					inputName = absoluteFilename( *( argv + 1 ) );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--which-out" ) == 0 )
				{
					filenameRequestList.push_back( FILENAME_OUT );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--out" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( outputName != "" )
						{
							std::cerr << "The output file is multiply specified." << std::endl ;
							exit( 1 );
						}
					outputName = absoluteFilename( *( argv + 1 ) );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--which-texjob" ) == 0 )
				{
					filenameRequestList.push_back( FILENAME_TEXJOB );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--texjob" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( texJobName != "" )
						{
							std::cerr << "The tex job name is multiply specified." << std::endl ;
							exit( 1 );
						}
					texJobName = *( argv + 1 );
					if( texJobName.find( '/' ) != std::string::npos )
						{
							std::cerr << "The tex job name may not include directory specification.  Please use --tmpdir to set the directory where the tex job is carried out." << std::endl ;
							exit( 1 );
						}
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--which-labeldb" ) == 0 )
				{
					filenameRequestList.push_back( FILENAME_LABELDB );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--labeldb" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( labelDBName != "" )
						{
							std::cerr << "The label database file is multiply specified." << std::endl ;
							exit( 1 );
						}
					labelDBName = absoluteFilename( *( argv + 1 ) );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--which-afmout" ) == 0 )
				{
					filenameRequestList.push_back( FILENAME_AFM );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--which-TEXINPUTS" ) == 0 )
				{
					filenameRequestList.push_back( FILENAME_TEXINPUTS );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--afmout" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( fontmetricsOutputName != "" )
						{
							std::cerr << "The font metrics output name is multiply specified." << std::endl ;
							exit( 1 );
						}
					fontmetricsOutputName = absoluteFilename( *( argv + 1 ) );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--outdir" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( outDir != "" )
						{
							std::cerr << "The output directory is multiply specified." << std::endl ;
							exit( 1 );
						}
					outDir = absoluteDirectory( *( argv + 1 ) );
					argv += 2;
					argc -= 2;
				}
			else if( strprefixcmp( *argv, "--tmp*=", & optionSuffix ) )
				{
					allowCreateTmpDir = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--tmpdir" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( tmpDir != "" )
						{
							std::cerr << "The temporaries directory is multiply specified." << std::endl ;
							exit( 1 );
						}
					tmpDir = absoluteDirectory( *( argv + 1 ) );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--which-tmp" ) == 0 )
				{
					filenameRequestList.push_back( FILENAME_TMP );
					argv += 1;
					argc -= 1;
				}
			else if( strprefixcmp( *argv, "--split=", & optionSuffix ) )
				{
					splitCatalog = strtobool( optionSuffix, *argv );
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--xpdf" ) == 0 )
				{
					launch_xpdf = true;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--xpdf-remote" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					if( xpdfServer != "" )
						{
							std::cerr << "The xpdf server is multiply specified." << std::endl ;
							exit( 1 );
						}
					xpdfServer = *( argv + 1 );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--xpdf-no-server" ) == 0 )
				{
					if( xpdfAction != XPDF_DEFAULT )
						{
							std::cerr << "The xpdf action is multiply specified." << std::endl ;
							exit( 1 );
						}
					xpdfAction = XPDF_NOSERVER;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--xpdf-reload" ) == 0 )
				{
					if( xpdfAction != XPDF_DEFAULT )
						{
							std::cerr << "The xpdf action is multiply specified." << std::endl ;
							exit( 1 );
						}
					xpdfAction = XPDF_RELOAD;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--xpdf-quit" ) == 0 )
				{
					if( xpdfAction != XPDF_DEFAULT )
						{
							std::cerr << "The xpdf action is multiply specified." << std::endl ;
							exit( 1 );
						}
					xpdfAction = XPDF_QUIT;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--open" ) == 0 )
				{
					do_open = true;
					argv += 1;
					argc -= 1;
				}
			else if( strcmp( *argv, "--open-a" ) == 0 )
				{
					argcAssertion( *argv, argc, 2 );
					do_open = true;
					do_open_application = *( argv + 1 );
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--version" ) == 0 )
				{
					printVersion( );
					exit( 0 );
				}
			else if( argc == 1 )
				{
					if( baseName != "" )
						{
							std::cerr << "The name base is multiply specified." << std::endl ;
							exit( 1 );
						}
					struct stat theStat;
					if( stat( *argv, & theStat ) == 0 )
						{
							inputName = *argv;
							char * ext = *argv + strlen( *argv ) - 6;
							if( ext <= *argv )
								{
									std::cerr << "The file name \"" << *argv << "\" is unexpectedly short (it should include the \".shape\" suffix)." << std::endl ;
									exit( 1 );
								}
							if( strcmp( ext, ".shape" ) != 0 )
								{
									std::cerr << "Expected \".shape\" suffix in the file name \"" << *argv << "\"." << std::endl ;
									exit( 1 );
								}
						}
					else
						{
							if( (*argv)[ strlen( *argv ) - 1 ] == '.' )
								{
									inputName = std::string( *argv ) + "shape";
								}
							else
								{
									inputName = std::string( *argv ) + ".shape";
								}
							if( ! stat( inputName.c_str( ), & theStat ) == 0 )
								{
									/* It is not entirely clear what is the best error message here,
									 * as the source file may be specified in several different ways.
									 * This should cause the least confusion.
									 */
									std::cerr << "Failed to locate input file: " << *argv << std::endl ;
									exit( 1 );
								}
						}

					const char * slash = strrchr( inputName.c_str( ), '/' );
					if( slash == 0 )
						{
							slash = inputName.c_str( );
						}
					else
						{
							++slash;
						}
					size_t skipCount = slash - inputName.c_str( );
					baseName = inputName.substr( skipCount, inputName.length( ) - skipCount - 6 );

					argv += 1;
					argc -= 1;
				}
			else
				{
					std::cerr << "Illegal command line option: " << *argv << std::endl ;
					exit( 1 );
				}
		}

	if( outDir == "" )
		{
			outDir = absoluteDirectory( "" );
		}
	if( tmpDir == "" )
		{
			char * start = getenv( "SHAPESTMPDIR" );
			if( start != 0 )
				{
					tmpDir = absoluteDirectory( start );
				}
			else
				{
					tmpDir = absoluteDirectory( "" );
				}
		}
	ensureTmpDirectoryExists( tmpDir, allowCreateTmpDir );

	if( baseName == "" )
		{
			if( texJobName == "" )
				{
					texJobName = "#shapes.labels";
				}
		}
	else
		{
			if( inputName == "" )
				{
					inputName = absoluteFilename( ( baseName + ".shape" ).c_str( ) );
				}
			if( outputName == "" )
				{
					outputName = outDir + baseName + ".pdf";
				}
			if( texJobName == "" )
				{
					texJobName = baseName + ".labels";
				}
			if( labelDBName == "" )
				{
					labelDBName = outDir + baseName + ".labels.pdf";
				}
			if( fontmetricsOutputName == "" )
				{
					fontmetricsOutputName = outDir + baseName + ".afm";
				}
		}

	if( outputName == "" )
		{
			std::cerr << "The output file is undetermined.  Consider specifying it using \"--out <filename>\"." << std::endl ;
			exit( 1 );
		}

	if( labelDBName == "" )
		{
			iterativeMode = false;
		}

	if( ! Kernel::theDebugLog.initialized( ) )
		{
			if( baseName == "" )
				{
					Kernel::theDebugLog.setFilename( "#shapes.log" );
				}
			else
				{
					Kernel::theDebugLog.setFilename( outDir + baseName + ".log" );
				}
		}

	{
		std::string inDir;
		std::string inPath = inputName;
		std::string::size_type slash = inPath.rfind( '/' );
		if( slash == std::string::npos )
			{
				inDir = absoluteDirectory( "" );
			}
		else
			{
				inDir = absoluteDirectory( inPath.substr( 0, slash ).c_str( ) );
			}
		Ast::theShapesScanner.setSourceDir( inDir );
		Kernel::theTeXLabelManager.setup( inDir, tmpDir, texJobName );
	}

	if( Computation::theTrixelizeSplicingTol >= Computation::theTrixelizeOverlapTol )
		{
			std::cerr << "The splicing tolerance (" << Concrete::Length::offtype( Computation::theTrixelizeSplicingTol ) << "bp) must be less than the overlap tolerance (" << Concrete::Length::offtype( Computation::theTrixelizeOverlapTol ) << "bp)." << std::endl ;
			exit( 1 );
		}

	if( xpdfServer == "" )
		{
			xpdfServer = outputName;
		}

	if( xpdfAction == XPDF_DEFAULT )
		{
			xpdfAction = XPDF_RAISE;
		}

	addDefaultNeedPath( );
	addDefaultFontMetricsPath( );

#ifdef RESOURCES_DIR
	if( useResources )
		{
			try
				{
					Ast::theShapesScanner.push_backNeedPath( std::string( RESOURCES_DIR ) + "/extensions/" );
					Lang::Font::push_backFontMetricsPath( std::string( RESOURCES_DIR ) + "/fontmetrics/" );
				}
			catch( const Exceptions::Exception & ball )
				{
					std::cout.flush( );
					ball.display( std::cerr );
					exit( 1 );
				}
		}
#endif

	if( ! filenameRequestList.empty( ) )
		{
			std::list< const char * >::const_iterator resource = resourceRequestList.begin( );
			for( std::list< int >::const_iterator i = filenameRequestList.begin( );
					 i != filenameRequestList.end( );
					 ++i )
				{
					if( i != filenameRequestList.begin( ) )
						{
							std::cout << " " ;
						}
					switch( *i )
						{
						case FILENAME_IN:
							if( inputName == "" )
								{
									std::cout << "<stdin>" ;
								}
							else
								{
									std::cout << inputName ;
								}
							break;
						case FILENAME_OUT:
							if( outputName == "" )
								{
									std::cout << "<stdout>" ;
								}
							else
								{
									std::cout << outputName ;
								}
							break;
						case FILENAME_TMP:
							std::cout << tmpDir ;
							break;
						case FILENAME_TEXJOB:
							std::cout << tmpDir << texJobName ;
							break;
						case FILENAME_LABELDB:
							std::cout << labelDBName ;
							break;
						case FILENAME_AFM:
							std::cout << fontmetricsOutputName ;
							break;
						case FILENAME_TEXINPUTS:
							{
								std::cout << getenv( "TEXINPUTS" ) ;
							}
							break;
						case FILENAME_RESOURCE:
							{
								try
									{
										std::cout << Ast::theShapesScanner.searchFile( *resource ) ;
									}
								catch( const Exceptions::Exception & ball )
									{
										std::cout.flush( );
										ball.display( std::cerr );
										exit( 1 );
									}
								++resource;
							}
							break;
						default:
							std::cerr << "Internal error:	filename request switch in main out of range." << std::endl ;
							exit( 1 );
						}
				}
			exit( 0 );
		}

	if( pdfVersion == SimplePDF::PDF_Version::VERSION_UNDEFINED )
		{
			pdfVersion = SimplePDF::PDF_Version::PDF_1_4;
		}

	Kernel::the_PDF_version.setVersion( pdfVersion );
	Kernel::the_PDF_version.setAction( pdfVersionAction );

	{
		std::ostringstream oss;
		time_t tmp;
		time( &tmp );
		struct tm * now( gmtime( &tmp ) );
		oss << "D:"
				<< std::setfill( '0' )
				<< std::setw(4) << 1900 + now->tm_year
				<< std::setw(2) << 1 + now->tm_mon
				<< std::setw(2) << now->tm_mday
				<< std::setw(2) << now->tm_hour
				<< std::setw(2) << now->tm_min
				<< std::setw(2) << now->tm_sec
				<< "+0000" ;
		(*Kernel::the_pdfo->info_)[ "CreationDate" ] = SimplePDF::PDF_out::newString( oss.str( ).c_str( ) );
	}

	std::ifstream iFile;
	if( inputName == "" )
		{
			(*Kernel::the_pdfo->info_)[ "Title" ] = SimplePDF::PDF_out::newString( "<stdin>" );
			Ast::theShapesScanner.switch_streams( & std::cin, & std::cerr );
			Ast::theShapesScanner.setNameOf_yyin( "stdin" );
		}
	else
		{
			(*Kernel::the_pdfo->info_)[ "Title" ] = SimplePDF::PDF_out::newString( inputName.c_str( ) );
			iFile.open( inputName.c_str( ) );
			if( ! iFile.good( ) || ! iFile.is_open( ) )
				{
					std::cerr << "Failed to open " << inputName << " for input." << std::endl ;
					exit( 1 );
				}
			Ast::theShapesScanner.switch_streams( & iFile, & std::cerr );
			Ast::theShapesScanner.setNameOf_yyin( inputName.c_str( ) );
		}

	std::istringstream prependStreamIn;
	if( ! prependStreamOut.str( ).empty( ) )
		{
			prependStreamIn.str( prependStreamOut.str( ) );
			Ast::theShapesScanner.prependStream( & prependStreamIn );
		}

	std::ofstream oFile;
	oFile.open( outputName.c_str( ) );
	if( ! oFile.good( ) )
		{
			std::cerr << "Failed to open " << outputName << " for output." << std::endl ;
			exit( 1 );
		}

	RefCountPtr< std::ifstream > labelDBFile = RefCountPtr< std::ifstream >( NullPtr< std::ifstream >( ) );

	try
		{
			shapesparse( );
			Kernel::theGlobalEnvironment = new Kernel::Environment( Kernel::theEnvironmentList );
			Kernel::registerGlobals( Kernel::theGlobalEnvironment );
			Kernel::registerDynamic( Kernel::theGlobalEnvironment );
			Kernel::registerHot( Kernel::theGlobalEnvironment );
			Kernel::registerClasses( Kernel::theGlobalEnvironment );
			Kernel::registerCore_elem( Kernel::theGlobalEnvironment );
			Kernel::registerCore_point( Kernel::theGlobalEnvironment );
			Kernel::registerCore_path( Kernel::theGlobalEnvironment );
			Kernel::registerCore_draw( Kernel::theGlobalEnvironment );
			Kernel::registerCore_construct( Kernel::theGlobalEnvironment );
			Kernel::registerCore_font( Kernel::theGlobalEnvironment );
			Kernel::registerCore_misc( Kernel::theGlobalEnvironment );
			Kernel::registerCore_state( Kernel::theGlobalEnvironment );
			Kernel::registerCore_annotation( Kernel::theGlobalEnvironment );
			Ast::theGlobalAnalysisEnvironment = Kernel::theGlobalEnvironment->newAnalysisEnvironment( );
			Ast::theProgram->analyze( 0, Ast::theGlobalAnalysisEnvironment );
			if( ! Ast::theAnalysisErrorsList.empty( ) )
				{
					std::cout.flush( );
					typedef typeof Ast::theAnalysisErrorsList ListType;
					for( ListType::const_iterator i = Ast::theAnalysisErrorsList.begin( ); i != Ast::theAnalysisErrorsList.end( ); ++i )
						{
							{
								typedef const Exceptions::StaticInconsistency ErrorType;
								ErrorType * err = dynamic_cast< ErrorType * >( *i );
								if( err != 0 )
									{
										std::cerr << err->loc( ) << ": " ;
										err->display( std::cerr );
										continue;
									}
							}
							std::cerr << "(Bad exception type)" << ": " ;
							(*i)->display( std::cerr );
						}
					abortProcedure( & oFile, outputName );
				}

			// The display unit is looked up after the input is scanned, so the user may use her own units
			Interaction::displayUnitFactor = Ast::theShapesScanner.lookupUnitFactor( Interaction::displayUnitName );
			if( Interaction::displayUnitFactor <= 0 )
				{
					std::cerr << "Invalid display unit: " << Interaction::displayUnitName << std::endl ;
					abortProcedure( & oFile, outputName );
				}
			labelDBFile = performIterativeStartup( labelDBName );
			RefCountPtr< const Kernel::GraphicsState > graphicsState( new Kernel::GraphicsState( true ) );
			Kernel::PassedDyn baseDyn( new Kernel::DynamicEnvironment( graphicsState ) );

			bool done = false;
			Kernel::EvalState evalState( Ast::theProgram,
																	 Kernel::theGlobalEnvironment,
																	 baseDyn,
																	 Kernel::ContRef( new Kernel::ExitVoidContinuation( & done, Ast::theProgram->loc( ) ) ) );
			try
				{
					while( ! done )
						{
							if( evalTrace )
								{
									if( evalBackTrace )
										{
											evalState.cont_->backTrace( std::cerr );
											std::cerr << "--- Bottom of trace ---" << std::endl ;
										}
									else
										{
											std::cerr << "Eval trace: Cont: " << evalState.cont_->traceLoc( ) << "	Expr: " << evalState.expr_->loc( ) << std::endl ;
										}
								}
							Ast::Expression * expr = evalState.expr_;
							expr->eval( & evalState );
						}
				}
			catch( const Exceptions::StaticInconsistency & ball )
				{
					std::cout.flush( );
					std::cerr << ball.loc( ) << ": " ;
					ball.display( std::cerr );
					abortProcedure( & oFile, outputName );
				}
			catch( Exceptions::Exception & ball )
				{
					std::cout.flush( );
					if( Interaction::debugBacktrace )
						{
							evalState.cont_->backTrace( std::cerr );
						}

					std::cerr << evalState.cont_->traceLoc( ) << Exceptions::Exception::locsep ;
					ball.display( std::cerr );
					abortProcedure( & oFile, outputName );
				}

			Kernel::WarmCatalog * catalog = dynamic_cast< Kernel::WarmCatalog * >( Kernel::theGlobalEnvironment->getStateHandle( Ast::theGlobalAnalysisEnvironment->findLocalStatePosition( Ast::THE_UNKNOWN_LOCATION, Lang::CATALOG_ID ) ) );
			RefCountPtr< const Lang::Group2D > finalPicture = dynamic_cast< Kernel::WarmGroup2D * >( Kernel::theGlobalEnvironment->getStateHandle( Ast::theGlobalAnalysisEnvironment->findLocalStatePosition( Ast::THE_UNKNOWN_LOCATION, Lang::CANVAS_ID ) ) )->getPile( );
			if( catalog->isEmpty( ) && finalPicture->isNull( ) )
				{
					throw Exceptions::EmptyFinalPicture( );
				}
			if( catalog->isEmpty( ) )
				{
					catalog->tackOnPage( baseDyn, finalPicture, Ast::THE_UNKNOWN_LOCATION );
				}

			catalog->shipout( Kernel::the_pdfo );

			if( cleanupMemory )
				{
					Kernel::theGlobalEnvironment->clear( );
				}

			delete Kernel::theGlobalEnvironment;
			delete Ast::theProgram;
		}
	catch( const Exceptions::StaticInconsistency & ball )
		{
			std::cout.flush( );
			std::cerr << ball.loc( ) << ": " ;
			ball.display( std::cerr );
			abortProcedure( & oFile, outputName );
		}
	catch( const Exceptions::Exception & ball )
		{
			std::cout.flush( );
			ball.display( std::cerr );
			abortProcedure( & oFile, outputName );
		}
	catch( const NonLocalExit::DynamicBindingNotFound & ball )
		{
			std::cerr << "Caught DynamicBindingNotFound at top level." << std::endl
					 << "This should really not be possible; it is an internal error." << std::endl ;
			exit( 1 );
		}
	catch( const NonLocalExit::NotThisType & ball )
		{
			std::cerr << "Caught NotThisType at top level." << std::endl
					 << "This should really not be possible; it is an internal error." << std::endl ;
			exit( 1 );
		}
	catch( const NonLocalExit::NonLocalExitBase & ball )
		{
			std::cerr << "Caught an unknown descendant to NonLocalExitBase at top level." << std::endl
					 << "This should really not be possible; it is an internal error." << std::endl ;
			exit( 1 );
		}
	catch( const char * ball )
		{
			std::cerr << "Caught (char*) ball at top level:" << std::endl
					 << "	" << ball << std::endl ;
			std::cerr << "This shit should not be thrown!	Use Exceptions::Exception derivatives!" << std::endl ;
			exit( 1 );
		}
	catch( const std::string & ball )
		{
			std::cerr << "Caught (string) ball at top level:" << std::endl
					 << "	" << ball << std::endl ;
			std::cerr << "This shit should not be thrown!	Use Exceptions::Exception derivatives!" << std::endl ;
			exit( 1 );
		}
	catch( ... )
		{
			std::cerr << "Caught (...) ball at top level." << std::endl ;
			std::cerr << "This shit should not be thrown!	Use Exceptions::Exception derivatives!" << std::endl ;
			exit( 1 );
		}

	if( ! Kernel::thePostCheckErrorsList.empty( ) )
		{
			abortProcedure( & oFile, outputName );
		}

	if( memoryStats )
		{
			std::cerr << "Summary:" << std::endl ;
			std::cerr << "Environments:	alive: " << Kernel::Environment::liveCount << "	of total: " << Kernel::Environment::createdCount
					 << "	(" << 100 * static_cast< double >( Kernel::Environment::liveCount ) / static_cast< double >( Kernel::Environment::createdCount ) << "%)" << std::endl ;
		}

	/* The iterativeFinish must be allowed to write to the labels database file, so the file must be closed.
	 * To make sure it is not in use when we close it, we do the_pdfo->writeData( ) first.
	 */

	Kernel::the_pdfo->writeData( oFile, Kernel::the_PDF_version );

	if( labelDBFile != NullPtr< std::ifstream >( ) )
		{
			if( labelDBFile->is_open( ) )
				{
					labelDBFile->close( );
				}
			labelDBFile = NullPtr< std::ifstream >( ); // Free the reference.
		}

	if( iterativeMode )
		{
			Kernel::theTeXLabelManager.iterativeFinish( labelDBName );
		}

	if( launch_xpdf )
		{
			xpdfHelper( outputName, xpdfServer, xpdfAction );
		}

	if( do_open )
		{
			openHelper( outputName, do_open_application );
		}

	destroyGlobals( );

	return 0;
}


void
argcAssertion( const char * optionSpecifier, int argc, int argcMin )
{
	if( argc < argcMin )
		{
			std::cerr << "The command line option " << optionSpecifier << " requires " << argcMin - 1 << " parameters." << std::endl ;
			exit( 1 );
		}
}

bool
strprefixcmp( char * str, const char * prefix, char ** endp )
{
	int len = strlen( prefix );
	bool res = ( strncmp( str, prefix, len ) == 0 );
	*endp = str + len;
	return res;
}

bool
strtobool( const char * str, const char * containingString, const char * trueLabel, const char * falseLabel )
{
	if( trueLabel != 0 &&
			strcmp( str, trueLabel ) == 0 )
		{
			return true;
		}
	if( falseLabel != 0 &&
			strcmp( str, falseLabel ) == 0 )
		{
			return false;
		}
	if( strcmp( str, "yes" ) == 0 ||
			strcmp( str, "true" ) == 0 ||
			strcmp( str, "on" ) == 0 )
		{
			return true;
		}
	if( strcmp( str, "no" ) == 0 ||
			strcmp( str, "false" ) == 0 ||
			strcmp( str, "off" ) == 0)
		{
			return false;
		}
	std::cerr << "The string \"" << str << "\" in the command line argument \"" << containingString << "\" was not recognized as a boolean value." << std::endl ;
	exit( 1 );
}


RefCountPtr< std::ifstream >
performIterativeStartup( const std::string & labelDBName )
{
	{
		struct stat theStat;
		if( stat( labelDBName.c_str( ), & theStat ) != 0 )
			{
				return RefCountPtr< std::ifstream >( NullPtr< std::ifstream >( ) );
			}
	}
//	 {
//		 ostringstream mvCommand;
//		 mvCommand << "cp '" << oldFilename.str( ) << "' '" << labelDBName.str( ) << "'" ;
//		 Interaction::systemDebugMessage( mvCommand.str( ) );
//		 if( system( mvCommand.str( ).c_str( ) ) != 0 )
//			 {
//				 return RefCountPtr< std::ifstream >( NullPtr< std::ifstream >( ) );
//			 }
//	 }
	RefCountPtr< std::ifstream > labelsFile( new std::ifstream( labelDBName.c_str( ) ) );
	if( ! labelsFile->good( ) )
		{
			return RefCountPtr< std::ifstream >( NullPtr< std::ifstream >( ) );
		}
	try
		{
			Kernel::theTeXLabelManager.iterativeStartup( labelsFile );
			return labelsFile;
		}
	catch( const char * ball )
		{
			std::cerr << "Caught (char*) ball from iterative startup:" << std::endl
					 << "	" << ball << std::endl ;
			exit( 1 );
		}
	catch( const std::string & ball )
		{
			std::cerr << "Caught (string) ball from iterative startup:" << std::endl
					 << "	" << ball << std::endl ;
			exit( 1 );
		}
	catch( const Exceptions::Exception & ball )
		{
			ball.display( std::cerr );
			exit( 1 );
		}
	catch( ... )
		{
			std::cerr << "Caught (...) ball from iterative startup." << std::endl ;
			exit( 1 );
		}
}


void
abortProcedure( std::ofstream * oFile, const std::string & outputName )
{
	if( ! Kernel::thePostCheckErrorsList.empty( ) )
		{
			std::cout.flush( );
			while( ! Kernel::thePostCheckErrorsList.empty( ) )
				{
					Exceptions::Exception * e = Kernel::thePostCheckErrorsList.front( );
					Kernel::thePostCheckErrorsList.pop_front( );
					{
						typedef const Exceptions::PostCondition ErrorType;
						ErrorType * err = dynamic_cast< ErrorType * >( e );
						if( err != 0 )
							{
								std::cerr << err->loc( ) << ": " ;
								err->display( std::cerr );
								continue;
							}
					}
					{
						typedef const Exceptions::RuntimeError ErrorType;
						ErrorType * err = dynamic_cast< ErrorType * >( e );
						if( err != 0 )
							{
								std::cerr << err->getLoc( ) << " (runtime): " ;
								err->display( std::cerr );
								continue;
							}
					}
					std::cerr << "(Bad post-exception type)" << ": " ;
					e->display( std::cerr );
				}
		}
	std::cerr << "Aborting job.	Deleting output file." << std::endl ;
	if( outputName != "" )
		{
			oFile->close( );
		}
	{
		std::ifstream tmpFile( outputName.c_str( ) );
		if( tmpFile.good( ) )
			{
				tmpFile.close( );
				std::string rmCommand = "rm '" + outputName + "'";
				Interaction::systemDebugMessage( rmCommand );
				if( system( rmCommand.c_str( ) ) != 0 )
					{
						/* Never mind; we made a try. */
					}
			}
	}
	exit( 1 );
}

namespace Shapes
{
	namespace Helpers
	{
		void setSelfRef( RefCountPtr< const Lang::Class > cls )
		{
			cls->setSelfRef( cls );
		}
	}
}

void
setupGlobals( )
{
	Lang::ElementaryPath2D * bbox = new Lang::ElementaryPath2D;
	bbox->push_back( new Concrete::PathPoint2D( 0, 0 ) );
	bbox->close( );
	Lang::THE_POINTPICTURE = RefCountPtr< Lang::Drawable2D >( new Lang::BBoxed2D( Lang::THE_NULL2D,
																																								RefCountPtr< Lang::ElementaryPath2D >( bbox ) ) );
	Helpers::setSelfRef( Lang::THE_OBJECT );

	Helpers::setSelfRef( Lang::Void::TypeID );
	Helpers::setSelfRef( Lang::Symbol::TypeID );
	Helpers::setSelfRef( Lang::Float::TypeID );
	Helpers::setSelfRef( Lang::Length::TypeID );
	Helpers::setSelfRef( Lang::Boolean::TypeID );
	Helpers::setSelfRef( Lang::String::TypeID );
	Helpers::setSelfRef( Lang::FloatPair::TypeID );
	Helpers::setSelfRef( Lang::FloatTriple::TypeID );
	Helpers::setSelfRef( Lang::Coords2D::TypeID );
	Helpers::setSelfRef( Lang::CornerCoords2D::TypeID );
	Helpers::setSelfRef( Lang::Coords3D::TypeID );

	Helpers::setSelfRef( Lang::Function::TypeID );
	Helpers::setSelfRef( Lang::Transform2D::TypeID );
	Helpers::setSelfRef( Lang::Transform3D::TypeID );

	Helpers::setSelfRef( Lang::Class::TypeID );

	Helpers::setSelfRef( Lang::TransformedInstance::TypeID );

	{
		Lang::SystemVirtualInterface * tmp = new Lang::SystemVirtualInterface( strrefdup( "Geometric2D" ) );
		Lang::Geometric2D::TypeID = RefCountPtr< const Lang::Class >( tmp );
		Helpers::setSelfRef( Lang::Geometric2D::TypeID );
		/* Note that addVirtual must not be called before the selfRef is set!
		 */
	}
	{
		Lang::SystemVirtualInterface * tmp = new Lang::SystemVirtualInterface( strrefdup( "Geometric3D" ) );
		Lang::Geometric3D::TypeID = RefCountPtr< const Lang::Class >( tmp );
		Helpers::setSelfRef( Lang::Geometric3D::TypeID );
		/* Note that addVirtual must not be called before the selfRef is set!
		 */
	}
	{
		Lang::SystemVirtualInterface * tmp = new Lang::SystemVirtualInterface( strrefdup( "Drawable2D" ) );
		Lang::Drawable2D::TypeID = RefCountPtr< const Lang::Class >( tmp );
		Helpers::setSelfRef( Lang::Drawable2D::TypeID );
		/* Note that addVirtual must not be called before the selfRef is set!
		 */
		tmp->addVirtual( Lang::MESSAGE_DRAWABLE_DRAW_ID );
	}
	{
		Lang::SystemVirtualInterface * tmp = new Lang::SystemVirtualInterface( strrefdup( "Drawable3D" ) );
		Lang::Drawable3D::TypeID = RefCountPtr< const Lang::Class >( tmp );
		Helpers::setSelfRef( Lang::Drawable3D::TypeID );
		/* Note that addVirtual must not be called before the selfRef is set!
		 */
		tmp->addVirtual( Lang::MESSAGE_DRAWABLE_DRAW_ID );
	}
	{
		Lang::SystemVirtualInterface * tmp = new Lang::SystemVirtualInterface( strrefdup( "Color" ) );
		Lang::Color::TypeID = RefCountPtr< const Lang::Class >( tmp );
		Helpers::setSelfRef( Lang::Color::TypeID );
		/* Note that addVirtual must not be called before the selfRef is set!
		 */
		tmp->addVirtual( "stroking" );
		tmp->addVirtual( "nonstroking" );
	}
}

void
xpdfHelper( const std::string & filename, const std::string & server, const XpdfAction & action )
{
	pid_t xpdfProcess = fork( );
	if( xpdfProcess == -1 )
		{
			throw Exceptions::InternalError( strrefdup( "Failed to fork a process for running xpdf." ) );
		}

	if( xpdfProcess == 0 ) /* This is the child */
		{
			/* The exec call below never returns, so the child process never leaves this if clause.
			 * Hence, there is no need to create a special else clasuse below.
			 */
			switch( action )
				{
				case XPDF_RAISE:
					execlp( "xpdf", "xpdf", "-remote", server.c_str( ), "-raise", filename.c_str( ), static_cast< const char * >( 0 ) );
					break;
				case XPDF_RELOAD:
					execlp( "xpdf", "xpdf", "-remote", server.c_str( ), "-reload", static_cast< const char * >( 0 ) );
					break;
				case XPDF_QUIT:
					execlp( "xpdf", "xpdf", "-remote", server.c_str( ), "-quit", static_cast< const char * >( 0 ) );
					break;
				case XPDF_NOSERVER:
					execlp( "xpdf", "xpdf", filename.c_str( ), static_cast< const char * >( 0 ) );
					break;
				default:
					std::cerr << "Internal error: XpdfAction switch out of range." << std::endl ;
					exit( 1 );
				}
			if( errno != 0 )
				{
					std::cerr << "Recieved errno = " << errno << " from execlp call to xpdf." << std::endl ;
					exit( 1 );
				}
			std::cerr << "execlp call to xpdf returned with errno == 0." << std::endl ;
			exit( 1 );
		}

}

void
openHelper( const std::string & filename, const char * application )
{
	pid_t openProcess = fork( );
	if( openProcess == -1 )
		{
			throw Exceptions::InternalError( strrefdup( "Failed to fork a process for running open." ) );
		}

	if( openProcess == 0 ) /* This is the child */
		{
			/* The exec call below never returns, so the child process never leaves this if clause.
			 * Hence, there is no need to create a special else clasuse below.
			 */
			if( application != 0 )
				{
					execlp( "open", "open", "-a", application, filename.c_str( ), static_cast< const char * >( 0 ) );
				}
			else
				{
					execlp( "open", "open", filename.c_str( ), static_cast< const char * >( 0 ) );
				}
			if( errno != 0 )
				{
					std::cerr << "Recieved errno = " << errno << " from execlp call to open." << std::endl ;
					exit( 1 );
				}
			std::cerr << "execlp call to open returned with errno == 0." << std::endl ;
			exit( 1 );
		}
}


void
Interaction::systemDebugMessage( const std::string & msg )
{
	if( Interaction::debugSystem )
		{
			std::cerr << "System command: " << msg << std::endl ;
		}
}

void
addDefaultNeedPath( )
{
	try
		{
			char * start = getenv( "SHAPESINPUTS" );
			if( start == 0 )
				{
					Ast::theShapesScanner.push_backNeedPath( "./" );
					return;
				}
			char * tok = strsep( & start, ":" );
			while( tok != 0 )
				{
					Ast::theShapesScanner.push_backNeedPath( tok );
					tok = strsep( & start, ":" );
				}
		}
	catch( const Exceptions::Exception & ball )
		{
			std::cout.flush( );
			ball.display( std::cerr );
			exit( 1 );
		}
}

void
addDefaultFontMetricsPath( )
{
	char * start = getenv( "SHAPESFONTMETRICS" );
	if( start == 0 )
		{
			return;
		}
	char * tok = strsep( & start, ":" );
	while( tok != 0 )
		{
			Lang::Font::push_backFontMetricsPath( tok );
			tok = strsep( & start, ":" );
		}
}

void
destroyGlobals( )
{
	Helpers::requireUTF8ToMacRomanConverter( true ); // true means "cleanup"
	Helpers::requireMacRomanToUTF8Converter( true ); // true means "cleanup"
	Helpers::requireUTF8ToWinANSIConverter( true );	// true means "cleanup"
	Helpers::requireUTF8ToASCIIConverter( true );	// true means "cleanup"
	Helpers::requireUTF8ToUCS4Converter( true );	// true means "cleanup"
	Helpers::requireUTF16BEToUCS4Converter( true );	// true means "cleanup"
	Helpers::requireGlyphList( true );	// true means "cleanup"
	Helpers::requireMacRomanEncoding( true );	// true means "cleanup"
}

std::string
absoluteFilename( const char * filename )
{
	if( *filename == '/' )
		{
			return filename;
		}
	return callDir + filename;
}

std::string
absoluteDirectory( const char * filename )
{
	if( *filename == '\0' )
		{
			return callDir;
		}
	if( filename[ strlen( filename ) - 1 ] != '/' )
		{
			if( *filename == '/' )
				{
					return filename + std::string( "/" );
				}
			return callDir + filename + "/";
		}
	if( *filename == '/' )
		{
			return filename;
		}
	return callDir + filename;
}

#include <iomanip>

void
ensureTmpDirectoryExists( const std::string & dirname, bool allowCreate )
{
	struct stat theStat;
	if( stat( dirname.c_str( ), & theStat ) == 0 )
		{
			if( ( theStat.st_mode & S_IFDIR ) == 0 )
				{
					std::cerr << "The path " << dirname << " was expected to reference a directory." << std::endl ;
					exit( 1 );
				}
			//				if( ( theStat.st_mode & S_IWOTH ) == 0 )
			//					{
			//						std::cerr << "The directory " << dirname << " was expected have write permission for others." << std::endl ;
			//						exit( 1 );
			//					}
			return;
		}

	if( ! allowCreate )
		{
			std::cerr << "The directory for temporaries, " << dirname << ", does not exist and is not allowed to be created.  Consider using --tmpdir+ instead of --tmpdir ." << std::endl ;
			exit( 1 );
		}

	size_t i2 = 0; /* We know there's a slash at the first position */
	i2 = dirname.find( '/', i2 + 1 );
	bool atRoot = true;
	while( stat( dirname.substr( 0, i2 ).c_str( ), & theStat ) == 0 )
		{
			atRoot = false;
			i2 = dirname.find( '/', i2 + 1 );
		}
	if( atRoot )
		{
			std::cerr << "Shapes will not create directories for temporary files at the root: " << dirname << std::endl ;
			exit( 1 );
		}

	mode_t oldUmask = umask( 0 ); /* We want to be able to create directories with any permissions. */
	while( i2 != std::string::npos )
		{
			if( mkdir( dirname.substr( 0, i2 ).c_str( ), theStat.st_mode & ( S_IRWXU | S_IRWXG | S_IRWXO ) ) != 0 )
				{
					std::cerr << "Failed to create directory for temporary files (errno=" << errno << "): " << dirname.substr( 0, i2 ) << std::endl ;
					exit( 1 );
				}
			i2 = dirname.find( '/', i2 + 1 );
		}
	umask( oldUmask );
}
