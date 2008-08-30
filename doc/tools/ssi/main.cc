#include "ssiscanner.h"

#include <fstream>
#include <iostream>
#include <cstdlib>


int
main( int argc, char ** argv )
{
	bool onlyDependencies = false;
	bool doEndl = false;
	char * inputFilename = 0;
	char * refbase = "";

	--argc;
	++argv;
	while( argc > 0 )
		{
			if( strncmp( *argv, "-d", 2 ) == 0 )
				{
					char * name = *argv + 2;
					char * asgn = strchr( name, '=' );
					if( asgn == 0 )
						{
							std::cerr << "Missing '=' in command line option '-d'." << std::endl ;
							exit( 1 );
						}
					*asgn = '\0';
					setenv( name, asgn + 1, 1 );
					++argv;
					--argc;
				}
			else if( strcmp( *argv, "--deps" ) == 0 )
				{
					onlyDependencies = true;
					++argv;
					--argc;
				}
			else if( strcmp( *argv, "--head" ) == 0 )
				{
					std::cout << *( argv + 1 ) << ":" ;
					doEndl = true;
					argv += 2;
					argc -= 2;
				}
			else if( strcmp( *argv, "--in" ) == 0 )
				{
					inputFilename = argv[ 1 ];
					argv += 2;
					argc -= 2;
				}
			else
				{
					std::cerr << "Illegal command line option: " << *argv << std::endl ;
					exit( 1 );
				}
		}

	if( inputFilename == 0 )
		{
			std::cerr << "The input file was not specified.  Please use \"--in <filename>\"." << std::endl ;
			exit( 1 );
		}
	std::ifstream iFile( inputFilename );
	if( ! iFile.good( ) )
		{
			std::cerr << "Failed to open main file: " << inputFilename << std::endl ;
			exit( 1 );
		}

	std::string initDir;
	{
		/* Make a copy of the input filename, and remove everything after the last slash, or
		 * the whole string, if there is no slash, so that the result can be appended to the
		 * current working directory to construct the full name of the directory containing the input.
		 */
		char * tmp = strdup( inputFilename );
		char * slash = strrchr( tmp, '/' );
		if( slash == 0 )
			{
				slash = tmp;
			}
		else
			{
				++slash;
			}
		*slash = '\0';
		if( tmp[ 0 ] == '/' )
			{
				initDir = tmp;
			}
		else
			{
				char * cwd = getcwd( 0, 0 );
				initDir = cwd + std::string( "/" ) + tmp;
				free( cwd );
			}
		free( tmp );
	}

	SSIScanner scanner( onlyDependencies, initDir, & iFile, & std::cout );
	scanner.yylex( );

	if( doEndl )
		{
			std::cout << std::endl;
		}

	return 0;
}
