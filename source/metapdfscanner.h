#ifndef metapdfscanner_h
#define metapdfscanner_h

#include "sourcelocation.h"
#include "charptrless.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stack>
#include <map>
#include <set>
#include <list>
#include <string>

#undef yyFlexLexer
#define yyFlexLexer metapdfFlexLexer
#include <FlexLexer.h>

class MetaPDFScanner : public metapdfFlexLexer
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
  std::stack< ::MetaPDF::Ast::SourceLocation > locStack;
  std::stack< size_t > pathCountStack;
  unsigned int quoteDepth;
  bool moreState;
  int lastleng;
  void more( );
  std::map< const char *, double, charPtrLess > unitTable;
  const char * newUnitName;
  std::set< FileID > neededFiles;
  std::list< std::string > needSearchPath;
  bool showFiles;
  bool randSeedSet;
  std::istream * appendStream_;
 public:
  //  Ast::SourceLocation loc;
  MetaPDFScanner( std::istream * yyin = 0, std::ostream * yyout = 0 );
  virtual ~MetaPDFScanner( );
  void setNameOf_yyin( const char * yyinName );
  void prependStream( std::istream * is );
  void push_backNeedPath( const char * path );
  void setShowFiles( bool _showFiles );
  virtual int yylex( );
  void doBeforeEachAction( );

  double lookupUnitFactor( const char * name ) const;
 private:
  std::string currentNeedFile;
  size_t currentNeedPushCount;
  bool currentNeedIsNeed;
  void push_frontNeedPath( const char * path );
  void pop_frontNeedPath( );
  std::string searchFile( const std::string & suffix ) const;
  void doInclusion( );
};



#endif