/* File: metapdfparser.y
 * --------------
 * Yacc input file to generate the parser for the MetaPDF language
 */

%{

/* Just like lex, the text within this first region delimited by %{ and %}
 * is assumed to be C/C++ code and will be copied verbatim to the y.tab.c
 * file ahead of the definitions of the yyparse() function. Add other header
 * file inclusions or C++ variable declarations/prototypes that are needed
 * by your code here.
 */
#include "metapdftypes.h"
#include "metapdfast.h"
#include "metapdfastflow.h"
#include "metapdfastexprs.h"
#include "metapdfastfun.h"
#include "metapdfastvalues.h"
#include "metapdfastvar.h"
#include "metapdfastidentifier.h"
#include "metapdfastclass.h"
#include "metapdfexceptions.h"
#include "consts.h"
#include "charptrless.h"
#include "autoonoff.h"
#include "metapdfcore.h"
#include "texlabelmanager.h"

using namespace MetaPDF;
#include "yyltype.h"
extern YYLTYPE metapdflloc;

#ifdef yylex
  /* This is ugly.
   * Warning! Warning! Warning!
   * We'll soon use that yylex was defined as
   *   #define yylex metapdflex
   * in order to reset it after we're done with the inclusion.
   */
#undef yylex
#include "globals.h"
#include "metapdfscanner.h"
int metapdflex( )
{
  return Ast::theMetaPDFScanner.yylex( );
}
#define yylex metapdflex
#endif

#include "refcount.h"

#include <list>
#include <map>
#include <sstream>

using namespace std;


int metapdflex( );


void metapdferror( RefCountPtr< const char > msg )
{
  throw Exceptions::ParserError( metapdflloc, msg );
}

void metapdferror( char * msg )
{
  metapdferror( strrefdup( msg ) );
}


%}

/*
 * The section before the first %% is the Definitions section of the yacc
 * input file. Here is where you declare tokens and types, add precedence
 * and associativity options, and so on.
 */
 
/*
 * yylval 
 * ------
 * Here we define the type of the yylval global variable that is used by
 * the scanner to store attibute information about the token just scanned
 * and thus communicate that information to the parser. You will need to
 * add new fields to this union as you add different attributes to your
 * non-terminal symbols.
 */

%union {
  Ast::Expression * expr;
  Ast::Node * node;
  Ast::MethodIdExpr * methodId;
  std::list< Ast::Node * > * nodeList;
  std::list< Ast::Expression * > * exprList;
  std::list< RefCountPtr< const char > > * strList;
  std::map< const char *, Ast::Expression *, charPtrLess > * namedExprMap;
  Kernel::Formals * formals;
  std::list< Kernel::Formals * > * formalsList;
  Ast::ArgListExprs * argList;
  int intVal;
  double floatVal;
  bool boolVal;
  char * str;
  int tokenID;
  std::list< const Ast::CallExpr * > * callExprList;
  std::list< Ast::ClassSection * > * classSectionList;
  Ast::ClassSection * classSection;
  Ast::MemberSection * memberSection;
  Ast::MemberDeclaration * memberDeclaration;
  Ast::MemberMode memberMode;
  Ast::ClassMode classMode;
  Ast::FunctionMode functionMode;
}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Yacc will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */

%token <tokenID> T_EOF T_minusminus T_plusplus T_ddot T_dddot T_assign T_eqeq T_eqneq T_flassign T_atat T_projection T_angle
%token <tokenID> T_cycle T_and T_or T_xor T_not T_mapsto T_bindto T_emptybrackets T_compose T_surrounding T_lesseq T_greatereq T_llthan T_declaretype
%token <tokenID> T_let T_letstar T_letrec
%token <tokenID> T_unit T_defaultunit T_tex T_dynamic T_continuation T_continue
%token <tokenID> T_class T_members T_prepare T_abstract T_overrides T_gr__
 // %token <tokenID>  T_letdst T_plusassign T_minusassign T_starassign T_slashassign

%token <intVal> T_int
%token <floatVal> T_float T_length
%token <expr> T_speciallength
%token <boolVal> T_bool
%token <str> T_string T_identifier T_at_identifier T_at_llthan

/* Non-terminal types
 * ------------------
 * In order for yacc to assign/access the correct field of $$, $1, we
 * must to declare which field is appropriate for the non-terminal.
 * As an example, this first type declaration establishes that the DeclList
 * non-terminal uses the field named "declList" in the yylval union. This
 * means that when we are setting $$ for a reduction for DeclList ore reading
 * $n which corresponds to a DeclList nonterminal we are accessing the field
 * of the union named "declList" which is of type List<Decl*>.
 * pp2: You'll need to add many of these of your own.
 */

%type <expr> Program Expr ExprExceptConstStrings DynamicBinding CallExpr CurryCallExpr Function OperatorFunction Class ConstantExceptStrings Coords PolarHandle
%type <expr> NamedLetExpr                 //   LetDestinations LetExpr LetStarExpr LetrecExpr
%type <expr> CodeBracket SuperCall SuperMemberReference
%type <exprList> InsertionSequence
%type <node> GroupElem
%type <nodeList> Group OneOrMoreGroupElems
%type <strList> OrderedFormals OneOrMoreOrderedFormals
%type <formals> Formals OneOrMoreFormalsItems
 //          %type <formalsList> NamedFormalsWithOrder OneOrMoreNamedFormalsWithOrder
%type <argList> ArgList OneOrMoreArgListItems
%type <methodId> MethodIdentifier
%type <callExprList> ListOfParentsWithInitargs;
%type <classSectionList> ClassSections OneOrMoreClassSections
%type <classSection> ClassSection
%type <memberSection> MemberDeclarations OneOrMoreMemberDeclarations
%type <memberSection> MethodDeclarations OneOrMoreMethodDeclarations
%type <memberDeclaration> MemberDeclaration MethodDeclaration
%type <memberMode> MemberAccessList MemberAccessSpecifier
%type <classMode> ClassModeList ClassModeSpecifier OneOrMoreClassModeSpecifiers
%type <functionMode> FunctionModeList OneOrMoreFunctionModeSpecifiers FunctionModeSpecifier

%nonassoc T_assign ':'
%left ']'
%left T_llthan
%nonassoc '!'
%left '|'
%right T_mapsto T_emptybrackets
%left '&'
%nonassoc T_dynamiccolon
%left T_or T_xor
%left T_and
%left T_not
%nonassoc T_eqeq T_eqneq
%left T_plusplus T_minusminus
%left '<' '>'
%nonassoc T_lesseq T_greatereq
%left '+' '-'
%nonassoc T_angle
%left '*' '/' T_projection
%left '~'
%left T_compose
%left '[' '.'
%left '#'
%left T_atat T_surrounding

%nonassoc ')'
%left ','

%start Program

%%
/*
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.
 */

Program
: Group T_EOF
{
  $$ = new Ast::CodeBracket( @1, $1 );
  Ast::theProgram = $$;
  YYACCEPT;
}
| Group error
{
  metapdferror( "Expecting end of file." );
}
;


Coords
: '(' Expr ',' Expr ')'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( $2 );
  args->orderedExprs_->push_back( $4 );
  $$ = new Ast::CallExpr( @$,
			  Ast::THE_FUNCTION_coords2D,
			  args );
}
| '(' Expr ',' Expr '^' Expr ')'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( $2 );
  args->orderedExprs_->push_back( $4 );
  args->orderedExprs_->push_back( $6 );
  $$ = new Ast::CallExpr( @$,
			  Ast::THE_FUNCTION_cornercoords2D,
			  args );
}
| '(' Expr ',' Expr ',' Expr ')'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( $2 );
  args->orderedExprs_->push_back( $4 );
  args->orderedExprs_->push_back( $6 );
  $$ = new Ast::CallExpr( @$,
			  Ast::THE_FUNCTION_coords3D,
			  args );
}
;


PolarHandle
: '(' Expr '^' Expr ')'
{
  $$ = new Ast::PolarHandle2DExpr( @$, $2, $4 );
}
| '(' Expr '^' ')'
{
  $$ = new Ast::PolarHandle2DExprFree_a( @$, $2 );
}
| '(' '^' Expr ')'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( $3 );
  $$ = new Ast::CallExpr( @$,
			  Ast::THE_FUNCTION_polarHandle2DFree_r,
			  args );
}
| '(' '^' ')'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  $$ = new Ast::CallExpr( @$,
			  Ast::THE_FUNCTION_polarHandle2DFree_ra,
			  args );
}
;

ArgList
:
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
}
| OneOrMoreArgListItems
;

OneOrMoreArgListItems
: Expr
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  $$->orderedExprs_->push_back( $1 );
}
| '(' T_identifier T_llthan ')'
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  $$->orderedExprs_->push_back( new Ast::LexiographicVariable( @2, $2, new Kernel::Environment::LexicalKey * ( 0 ), true ) );  // true means warm access
}
| '(' T_at_identifier T_llthan ')'
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  $$->orderedExprs_->push_back( new Ast::DynamicVariable( @2, $2, true ) );  // true means warm access
}
| T_at_llthan
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  $$->orderedExprs_->push_back( new Ast::DynamicVariable( @1, 0, true ) );  // true means warm access
}
| T_identifier ':' Expr
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  (*$$->namedExprs_)[ $1 ] = $3;
}
| T_identifier ':' '(' T_identifier T_llthan ')'
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  (*$$->namedExprs_)[ $1 ] = new Ast::LexiographicVariable( @4, $4, new Kernel::Environment::LexicalKey * ( 0 ), true );   // true means warm access
}
| T_identifier ':' '(' T_at_identifier T_llthan ')'
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  (*$$->namedExprs_)[ $1 ] = new Ast::DynamicVariable( @4, $4, true );   // true means warm access
}
| T_identifier ':' T_at_llthan
{
  $$ = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  (*$$->namedExprs_)[ $1 ] = new Ast::DynamicVariable( @3, 0, true );   // true means warm access
}
| OneOrMoreArgListItems Expr
{
  $$ = $1;
  if( $$->namedExprs_->size( ) != 0 )
    {
      throw Exceptions::ParserError( @2, strrefdup( "Unnamed expressions may not appear among named expressions." ) );
    }
  $$->orderedExprs_->push_back( $2 );
}
| OneOrMoreArgListItems '(' T_identifier T_llthan ')'
{
  $$ = $1;
  if( $$->namedExprs_->size( ) != 0 )
    {
      throw Exceptions::ParserError( @2, strrefdup( "Unnamed expressions may not appear among named expressions." ) );
    }
  $$->orderedExprs_->push_back( new Ast::LexiographicVariable( @3, $3, new Kernel::Environment::LexicalKey * ( 0 ), true ) );  // true means warm access
}
| OneOrMoreArgListItems '(' T_at_identifier T_llthan ')'
{
  $$ = $1;
  if( $$->namedExprs_->size( ) != 0 )
    {
      throw Exceptions::ParserError( @2, strrefdup( "Unnamed expressions may not appear among named expressions." ) );
    }
  $$->orderedExprs_->push_back( new Ast::DynamicVariable( @3, $3, true ) );  // true means warm access
}
| OneOrMoreArgListItems T_at_llthan
{
  $$ = $1;
  if( $$->namedExprs_->size( ) != 0 )
    {
      throw Exceptions::ParserError( @2, strrefdup( "Unnamed expressions may not appear among named expressions." ) );
    }
  $$->orderedExprs_->push_back( new Ast::DynamicVariable( @2, 0, true ) );  // true means warm access
}
| OneOrMoreArgListItems T_identifier ':' Expr
{
  $$ = $1;
  if( $$->namedExprs_->find( $2 ) != $$->namedExprs_->end( ) )
    {
      throw Exceptions::RepeatedFormal( @2, $2 );
    }
  (*$$->namedExprs_)[ $2 ] = $4;
}
| OneOrMoreArgListItems T_identifier ':' '(' T_identifier T_llthan ')'
{
  $$ = $1;
  if( $$->namedExprs_->find( $2 ) != $$->namedExprs_->end( ) )
    {
      throw Exceptions::RepeatedFormal( @2, $2 );
    }
  (*$$->namedExprs_)[ $2 ] = new Ast::LexiographicVariable( @5, $5, new Kernel::Environment::LexicalKey * ( 0 ), true );   // true means warm access
}
| OneOrMoreArgListItems T_identifier ':' '(' T_at_identifier T_llthan ')'
{
  $$ = $1;
  if( $$->namedExprs_->find( $2 ) != $$->namedExprs_->end( ) )
    {
      throw Exceptions::RepeatedFormal( @2, $2 );
    }
  (*$$->namedExprs_)[ $2 ] = new Ast::DynamicVariable( @5, $5, true );   // true means warm access
}
| OneOrMoreArgListItems T_identifier ':' T_at_llthan
{
  $$ = $1;
  if( $$->namedExprs_->find( $2 ) != $$->namedExprs_->end( ) )
    {
      throw Exceptions::RepeatedFormal( @2, $2 );
    }
  (*$$->namedExprs_)[ $2 ] = new Ast::DynamicVariable( @4, 0, true );   // true means warm access
}
;


CallExpr
: '[' Expr ArgList ']'
{
  $$ = new Ast::CallExpr( @$, $2, $3 );
}
| Expr T_emptybrackets Expr
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( $3 );
  $$ = new Ast::CallExpr( @$,
			      $1,
			      args );
}
;


CurryCallExpr
: '[' Expr ArgList T_dddot ']'
{
  $$ = new Ast::CallExpr( @$, $2, $3, true ); /* true means Curry */
}
;


Formals
:
{
  $$ = new Kernel::Formals( );
  $$->setLoc( @$ );
}
| OneOrMoreFormalsItems
{
  $$ = $1;
  $$->setLoc( @$ );
}
;

OneOrMoreFormalsItems
: T_identifier
{
  $$ = new Kernel::Formals( );
  $$->argumentOrder_->insert( std::pair< const char *, size_t >( $1, $$->defaultExprs_.size( ) ) );
  $$->defaultExprs_.push_back( 0 );
}
| T_identifier ':' Expr
{
  $$ = new Kernel::Formals( );
  $$->argumentOrder_->insert( std::pair< const char *, size_t >( $1, $$->defaultExprs_.size( ) ) );
  $$->defaultExprs_.push_back( $3 );
}
| OneOrMoreFormalsItems T_identifier
{
  $$ = $1;
  if( $$->seenDefault_ )
    {
      throw Exceptions::ParserError( @2, strrefdup( "Order-based formals may not appear among named formals." ) );
    } 
  if( $$->argumentOrder_->find( $2 ) != $$->argumentOrder_->end( ) )
    {
      throw Exceptions::RepeatedFormal( @2, $2 );
    }
  $$->argumentOrder_->insert( std::pair< const char *, size_t >( $2, $$->defaultExprs_.size( ) ) );
  $$->defaultExprs_.push_back( 0 );
}
| OneOrMoreFormalsItems T_identifier ':' Expr
{
  $$ = $1;
  $$->seenDefault_ = true;
  if( $$->argumentOrder_->find( $2 ) != $$->argumentOrder_->end( ) )
    {
      throw Exceptions::RepeatedFormal( @2, $2 );
    }
  $$->argumentOrder_->insert( std::pair< const char *, size_t >( $2, $$->defaultExprs_.size( ) ) );
  $$->defaultExprs_.push_back( $4 );
}
;



Function
: '\\' Formals T_mapsto FunctionModeList Expr
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::FunctionFunction * res = new Ast::FunctionFunction( @$, $2, $5, $4 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      RefCountPtr< const Lang::Function >( res ),
			      args );
}
| '(' OperatorFunction ')'
{
  $$ = $2;
}
;

OrderedFormals
:
{
  $$ = new list< RefCountPtr< const char > >( );
}
| OneOrMoreOrderedFormals
;

OneOrMoreOrderedFormals
: T_identifier
{
  $$ = new list< RefCountPtr< const char > >( );
  $$->push_back( strrefdup( $1 ) );
}
| OneOrMoreOrderedFormals T_identifier
{
  $$ = $1;
  $$->push_back( strrefdup( $2 ) );
}
;

OperatorFunction
: T_minusminus
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_MINUSMINUS ) );
}
| T_plusplus
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_PLUSPLUS ) );
}
| '&'
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_AMPERSAND ) );
}
| '+'
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_PLUS ) );
}
| '-'
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_MINUS ) );
}
| '*'
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_STAR ) );
}
| '/'
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_SLASH ) );
}
| T_projection
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_PROJECTION ) );
}
| T_angle
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_ANGLE ) );
}
| '~'
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_NEG ) );
}
| T_compose
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_COMPOSE ) );
}
| '<'
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_LESS ) );
}
| '>'
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_GREATER ) );
}
| T_eqeq
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_EQEQ ) );
}
| T_eqneq
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_EQNEQ ) );
}
| T_lesseq
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_LESSEQ ) );
}
| T_greatereq
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_GREATEREQ ) );
}
| T_not
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_NOT ) );
}
| T_and
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_FUNCTION_AND ) );
}
| T_or
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_FUNCTION_OR ) );
}
| T_xor
{
  $$ = new Ast::Constant( @$, static_cast< RefCountPtr< const Lang::Value > >( Lang::THE_OPERATOR_XOR ) );
}
;

Expr
: ExprExceptConstStrings
| T_string
{
  $$ = new Ast::Constant( @1, new Lang::String( $1 ) );
}
;

ExprExceptConstStrings
: ConstantExceptStrings
| Coords
| PolarHandle
| '[' T_tex ExprExceptConstStrings ']'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( $3 );
  $$ = new Ast::CallExpr( @$,
			  Ast::THE_FUNCTION_TeX,
			  args );
}
| '[' T_tex T_string ']'
{
  Kernel::theTeXLabelManager.announce( string( $3 ) );
  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( new Ast::Constant( @3, new Lang::String( $3 ) ) );
  $$ = new Ast::CallExpr( @$,
			  Ast::THE_FUNCTION_TeX,
			  args );
}
| '!' Expr
{
  $$ = $2;
  $$->immediate_ = true;
}
| CallExpr
| CurryCallExpr
| SuperCall
| SuperMemberReference
| T_speciallength
| '(' Expr ')'
{
  $$ = $2;
}
| '(' '-' Expr ')'
{
  $$ = new Ast::NegExpr( @$, $3 );
}
| '(' '+' Expr ')'
{
  $$ = new Ast::RelativeExpr( @$, $3 );
}
| '(' ')'
{
  $$ = new Ast::EmptyExpression( @$ );
}
| '(' T_identifier T_llthan InsertionSequence ')'
{
  std::list< Ast::Node * > * bracket = new std::list< Ast::Node * >( );
  
  size_t ** pos = new size_t * ( 0 );
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );

  bracket->push_back( new Ast::IntroduceWarm( @3,
					      strdup( Kernel::SEQUENTIAL_EXPR_VAR_ID ),
					      new Ast::LexiographicVariable( @2, $2, new Kernel::Environment::LexicalKey * ( 0 ) ),
					      pos ) );
  for( std::list< Ast::Expression * >::const_iterator i = $4->begin( ); i != $4->end( ); ++i )
    {
      bracket->push_back( new Ast::LexiographicInsertion( @3, strdup( Kernel::SEQUENTIAL_EXPR_VAR_ID ), *i, key ) );
    }
  bracket->push_back( new Ast::Freeze( @3, strdup( Kernel::SEQUENTIAL_EXPR_VAR_ID ), pos ) );
  bracket->push_back( new Ast::LexiographicVariable( @3, strdup( Kernel::SEQUENTIAL_EXPR_VAR_ID ), key ) );
  $$ = new Ast::CodeBracket( @$, bracket );
}
| '(' ':' Expr T_llthan InsertionSequence ')'
{
  std::list< Ast::Node * > * bracket = new std::list< Ast::Node * >( );
  
  size_t ** pos = new size_t * ( 0 );
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );

  bracket->push_back( new Ast::IntroduceWarm( @4,
						  strdup( Kernel::SEQUENTIAL_EXPR_VAR_ID ),
						  $3,
						  pos ) );
  for( std::list< Ast::Expression * >::const_iterator i = $5->begin( ); i != $5->end( ); ++i )
    {
      bracket->push_back( new Ast::LexiographicInsertion( @4, strdup( Kernel::SEQUENTIAL_EXPR_VAR_ID ), *i, key ) );
    }
  bracket->push_back( new Ast::Freeze( @4, strdup( Kernel::SEQUENTIAL_EXPR_VAR_ID ), pos ) );
  bracket->push_back( new Ast::LexiographicVariable( @4, strdup( Kernel::SEQUENTIAL_EXPR_VAR_ID ), key ) );
  $$ = new Ast::CodeBracket( @$, bracket );
}
| T_surrounding Expr
{
  $$ = new Ast::EvalOutsideExpr( @$, $2 );
}
| CodeBracket
| Function
| Class
| T_identifier
{
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
  $$ = new Ast::LexiographicVariable( @$, $1, key );
}
| T_atat Expr
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::EvalSymbolFunction * res = new Ast::EvalSymbolFunction( @$, $2 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      RefCountPtr< const Lang::Function >( res ),
			      args );
}
| T_at_identifier
{
  $$ = new Ast::DynamicVariable( @$, $1 );
}
| Expr '.' T_identifier
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::MemberReferenceFunction * res = new Ast::MemberReferenceFunction( @$, $1, $3 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      RefCountPtr< const Lang::Function >( res ),
			      args );
}
| Expr '.' MethodIdentifier
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::PublicMethodReferenceFunction * res = new Ast::PublicMethodReferenceFunction( @$, $1, $3 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      RefCountPtr< const Lang::Function >( res ),
			      args );
}
| DynamicBinding
| '(' T_continuation T_identifier Expr ')'
{
  $$ = new Ast::LetDynamicECExpr( @$, @3, $3, $4 );
}
| '(' T_continue T_identifier Expr ')'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::ContinueDynamicECFunction * res = new Ast::ContinueDynamicECFunction( @3, $3, $4 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      RefCountPtr< const Lang::Function >( res ),
			      args );
}
| Expr '|' Expr
{
  $$ = new Ast::WithDynamicExpr( @$, $1, $3 );
}
| NamedLetExpr
| Expr T_minusminus T_cycle
{
  $$ = new Ast::CycleExpr( @3, $1 );
}
| Expr T_minusminus Expr
{
  $$ = new Ast::MinusMinusExpr( @2, $1, $3 );
}
| Expr T_plusplus Expr
{
  $$ = new Ast::PlusPlusExpr( @2, $1, $3 );
}
| Expr '&' Expr
{
  $$ = new Ast::AmpersandExpr( @2, $1, $3 );
}
| Expr '+' Expr
{
  $$ = new Ast::PlusExpr( @2, $1, $3 );
}
| Expr '-' Expr
{
  $$ = new Ast::MinusExpr( @2, $1, $3 );
}
| Expr T_angle Expr
{
  $$ = new Ast::AngleExpr( @2, $1, $3 );  
}
| Expr '*' Expr
{
  $$ = new Ast::StarExpr( @2, $1, $3 );  
}
| Expr T_projection Expr
{
  $$ = new Ast::ProjectionExpr( @2, $1, $3 );  
}
| Expr '/' Expr
{
  $$ = new Ast::SlashExpr( @2, $1, $3 );  
}
| '~' Expr
{
  $$ = new Ast::NegExpr( @1, $2 );
}
| Expr T_compose Expr
{
  $$ = new Ast::ComposeExpr( @2, $1, $3 );  
}
| Expr '<' Expr
{
  $$ = new Ast::LessExpr( @2, $1, $3 );
}
| Expr '>' Expr
{
  $$ = new Ast::GreaterExpr( @2, $1, $3 );
}
| Expr T_eqeq Expr
{
  $$ = new Ast::EqualExpr( @2, $1, $3 );
}
| Expr T_eqneq Expr
{
  $$ = new Ast::NotEqualExpr( @2, $1, $3 );
}
| Expr T_lesseq Expr
{
  $$ = new Ast::LessEqualExpr( @2, $1, $3 );
}
| Expr T_greatereq Expr
{
  $$ = new Ast::GreaterEqualExpr( @2, $1, $3 );
}
| T_not Expr
{
  $$ = new Ast::NotExpr( @1, $2 );
}
| Expr T_and Expr
{
  //  $$ = new Ast::AndExpr( @2, $1, $3 );

  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( $1 );
  args->orderedExprs_->push_back( $3 );
  $$ = new Ast::CallExpr( @$,
			  Lang::THE_FUNCTION_AND,
			  args );
}
| Expr T_or Expr
{
  //  $$ = new Ast::OrExpr( @2, $1, $3 );

  Ast::ArgListExprs * args = new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) );
  args->orderedExprs_->push_back( $1 );
  args->orderedExprs_->push_back( $3 );
  $$ = new Ast::CallExpr( @$,
			  Lang::THE_FUNCTION_OR,
			  args );
}
| Expr T_xor Expr
{
  $$ = new Ast::XorExpr( @2, $1, $3 );
}
;


DynamicBinding
: T_at_identifier ':' Expr %prec T_dynamiccolon
{
  $$ = new Ast::DynamicBindingExpression( @$, $1, $3, new Kernel::Environment::LexicalKey * ( 0 ) );
}
| T_at_identifier ':' '(' T_identifier T_llthan ')'  %prec T_dynamiccolon
{
  $$ = new Ast::DynamicBindWarmLexiographicExpr( @$, @1, $1, @4, $4 );
}
| T_at_llthan ':' '(' T_identifier T_llthan ')'  %prec T_dynamiccolon
{
  $$ = new Ast::DynamicBindWarmLexiographicExpr( @$, @1, $1, @4, $4 );
}
| T_at_llthan ':' T_identifier  %prec T_dynamiccolon
{
  $$ = new Ast::DynamicBindWarmLexiographicExpr( @$, @1, $1, @3, $3 );
}
| T_at_identifier ':' '(' T_at_identifier T_llthan ')'  %prec T_dynamiccolon
{
  $$ = new Ast::DynamicBindWarmDynamicExpr( @$, @1, $1, @4, $4 );
}
| T_at_llthan ':' '(' T_at_identifier T_llthan ')'  %prec T_dynamiccolon
{
  $$ = new Ast::DynamicBindWarmDynamicExpr( @$, @1, $1, @4, $4 );
}
| T_at_llthan ':' T_at_identifier  %prec T_dynamiccolon
{
  $$ = new Ast::DynamicBindWarmDynamicExpr( @$, @1, $1, @3, $3 );
}
;

NamedLetExpr
: '[' T_let T_identifier '(' Formals ')' Expr ']'
{
  for( std::vector< Ast::Expression * >::const_iterator i = $5->defaultExprs_.begin( ); i != $5->defaultExprs_.end( ); ++i )
    {
      if( *i == 0 )
	{
	  throw Exceptions::ParserError( @5, strrefdup( "Formals without default value are not allowed in list of let bindings." ) );
	}
    }

  std::list< Ast::Node * > * bracket = new std::list< Ast::Node * >( );

  {  
    Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
    Ast::FunctionFunction * res = new Ast::FunctionFunction( @5, $5, $7, 0 );
    res->push_exprs( args );
    size_t ** pos = new size_t * ( 0 );
    bracket->push_back( new Ast::IntroduceCold( @3,
						    $3,
						    new Ast::CallExpr( @$,
									   RefCountPtr< const Lang::Function >( res ),
									   args ),
						    pos ) );
  }

  {
    Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
    bracket->push_back( new Ast::CallExpr( @$,
					       new Ast::LexiographicVariable( @3, $3, key ),
					       new Ast::ArgListExprs( new list< Ast::Expression * >( ), new std::map< const char *, Ast::Expression *, charPtrLess >( ) ) ) );
  }

  $$ = new Ast::CodeBracket( @$, bracket );
}
;


ConstantExceptStrings
: T_int
{
  $$ = new Ast::Constant( @1, new Lang::Integer( $1 ) );
}
| T_float
{
  $$ = new Ast::Constant( @1, new Lang::Float( $1 ) );
}
| T_length
{
  $$ = new Ast::Constant( @1, new Lang::Length( $1 ) );
}
| T_bool
{
  $$ = new Ast::Constant( @1, new Lang::Boolean( $1 ) );
}
| '\'' T_identifier
{
  $$ = new Ast::Constant( @1, new Lang::Symbol( $2 ) );
}
;

CodeBracket
: '{' Group '}'
{
  $$ = new Ast::CodeBracket( @$, $2 );
}
;

GroupElem
: Expr
{
  $$ = $1;  // Explicit upcast avoids bison warning.
}
| T_identifier ':' Expr
{
  size_t ** pos = new size_t * ( 0 );
  $$ = new Ast::IntroduceCold( @1, $1, $3, pos );
}
| T_identifier ':' Expr T_llthan
{
  size_t ** pos = new size_t * ( 0 );
  $$ = new Ast::IntroduceWarm( @1, $1, $3, pos );
}
| T_identifier ';'
{
  size_t ** pos = new size_t * ( 0 );
  $$ = new Ast::Freeze( @1, $1, pos );
}
| T_dynamic T_at_identifier Expr Expr
{
  $$ = new Ast::DynamicVariableDecl( @$, @2, $2, $3, $4, new size_t * ( 0 ) );
}
| Expr '.' T_identifier T_llthan InsertionSequence
{
  metapdferror( "MemberInsertionSequence not implemented" );
  //  $$ = new Ast::MemberInsertionSequence( @$, $1, $3, $5 );
}
| '(' '#' Expr ')' '.' T_identifier T_llthan InsertionSequence
{
  metapdferror( "ProtectedMemberInsertionSequence not implemented" );
  //  $$ = new Ast::ProtectedMemberInsertionSequence( @$, @2, $3, $6, $8 );
}
;

InsertionSequence
: Expr
{
  $$ = new std::list< Ast::Expression * >( );
  $$->push_back( $1 );
}
| InsertionSequence T_llthan Expr
{
  $$ = $1;
  $$->push_back( $3 );
}
;

OneOrMoreGroupElems
: GroupElem
{
  $$ = new list< Ast::Node * >( );
  $$->push_back( $1 );
}
| OneOrMoreGroupElems GroupElem
{
  $$ = $1;
  $$->push_back( $2 );
}
| T_identifier T_llthan InsertionSequence
{
  $$ = new list< Ast::Node * >( );
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
  for( std::list< Ast::Expression * >::const_iterator i = $3->begin( ); i != $3->end( ); ++i )
    {
      $$->push_back( new Ast::LexiographicInsertion( @1, strdup( $1 ), *i, key ) );
    }
  delete $1;
}
| OneOrMoreGroupElems T_identifier T_llthan InsertionSequence
{
  $$ = $1;
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
  for( std::list< Ast::Expression * >::const_iterator i = $4->begin( ); i != $4->end( ); ++i )
    {
      $$->push_back( new Ast::LexiographicInsertion( @2, strdup( $2 ), *i, key ) );
    }
  delete $2;
}
| T_at_identifier T_llthan InsertionSequence
{
  $$ = new list< Ast::Node * >( );
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
  for( std::list< Ast::Expression * >::const_iterator i = $3->begin( ); i != $3->end( ); ++i )
    {
      $$->push_back( new Ast::DynamicInsertion( @1, strdup( $1 ), *i, key ) );
    }
  delete $1;
}
| OneOrMoreGroupElems T_at_identifier T_llthan InsertionSequence
{
  $$ = $1;
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
  for( std::list< Ast::Expression * >::const_iterator i = $4->begin( ); i != $4->end( ); ++i )
    {
      $$->push_back( new Ast::DynamicInsertion( @2, strdup( $2 ), *i, key ) );
    }
  delete $2;
}
| T_at_llthan InsertionSequence
{
  $$ = new list< Ast::Node * >( );
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
  for( std::list< Ast::Expression * >::const_iterator i = $2->begin( ); i != $2->end( ); ++i )
    {
      $$->push_back( new Ast::DynamicInsertion( @1, 0, *i, key ) );
    }
}
| OneOrMoreGroupElems T_at_llthan InsertionSequence
{
  $$ = $1;
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
  for( std::list< Ast::Expression * >::const_iterator i = $3->begin( ); i != $3->end( ); ++i )
    {
      $$->push_back( new Ast::DynamicInsertion( @2, 0, *i, key ) );
    }
}
;

Group
:
{
  $$ = new list< Ast::Node * >( );
}
| OneOrMoreGroupElems
;


SuperMemberReference
: '(' '#' Expr ')' '.' T_identifier
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::ProtectedMemberReferenceFunction * res = new Ast::ProtectedMemberReferenceFunction( @$, @2, $3, @6, $6 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      RefCountPtr< const Lang::Function >( res ),
			      args );
}
;


MethodIdentifier
: T_identifier '#' T_identifier
{
  Kernel::Environment::LexicalKey ** key = new Kernel::Environment::LexicalKey * ( 0 );
  $$ = new Ast::MethodIdExpr( @$, new Ast::LexiographicVariable( @1, $1, key ), $3 );
}
;


SuperCall
: '[' '(' '#' Expr ')' '.' MethodIdentifier ArgList ']'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::ProtectedMethodReferenceFunction * res = new Ast::ProtectedMethodReferenceFunction( @$, @3, $4, $7 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      new Ast::CallExpr( @3,
						     RefCountPtr< const Lang::Function >( res ),
						     args ),
			      $8 );
}
| '[' '(' '#' ')' '.' MethodIdentifier ArgList ']'
{
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::ProtectedMethodReferenceFunction * res = new Ast::ProtectedMethodReferenceFunction( @$, @3, 0, $6 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      new Ast::CallExpr( @3,
						     RefCountPtr< const Lang::Function >( res ),
						     args ),
			      $7 );
}
;


Class
: '[' T_class '(' Expr Formals ')' T_identifier '(' ListOfParentsWithInitargs ')'
  ClassModeList
  ClassSections
  ']'
{
  DeleteOnExit< char > isaDeleter( $7 );
  if( strcmp( $7, "isa" ) != 0 )
    {
      throw Exceptions::ParserError( @7, strrefdup( "Expected \"isa\"." ) );
    }
  if( ( $11 & Ast::CLASS_MODE_ABSTRACT ) != 0 && ( $11 & Ast::CLASS_MODE_FINAL ) != 0 )
    {
      throw Exceptions::ParserError( @11, strrefdup( "Declaring a class both abstract and final is forbidden." ) );
    }

  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::ClassFunction * res = new Ast::ClassFunction( @$, $4, $5, $9, $11, $12 );
  res->push_exprs( args );
  $$ = new Ast::CallExpr( @$,
			      RefCountPtr< const Lang::Function >( res ),
			      args );
}
;

ListOfParentsWithInitargs
: '(' Expr ArgList ')'
{
  $$ = new std::list< const Ast::CallExpr * >;
  $$->push_back( new Ast::CallExpr( @$, $2, $3 ) );
}
| ListOfParentsWithInitargs '(' Expr ArgList ')'
{
  $$ = $1;
  $$->push_back( new Ast::CallExpr( Ast::SourceLocation( @2, @5 ), $3, $4 ) );
}
;

ClassModeList
:
{
  $$ = 0;
}
| OneOrMoreClassModeSpecifiers
;

OneOrMoreClassModeSpecifiers
: ClassModeSpecifier
| OneOrMoreClassModeSpecifiers ClassModeSpecifier
{
  $$ = $1 | $2;
}
;

ClassModeSpecifier
: T_identifier
{
  DeleteOnExit< char > strDeleter( $1 );
  $$ = 0;
  if( strcmp( $1, "abstract" ) == 0 )
    {
      $$ = Ast::CLASS_MODE_ABSTRACT;
    }
  else if( strcmp( $1, "final" ) == 0 )
    {
      $$ = Ast::CLASS_MODE_FINAL;
    }
  else
    {
      throw Exceptions::ParserError( @$, strrefdup( "This is not a valid class mode specifier" ) );
    }
}
;

ClassSections
:
{
  $$ = new std::list< Ast::ClassSection * >;
}
| OneOrMoreClassSections
;

OneOrMoreClassSections
: ClassSection
{
  $$ = new std::list< Ast::ClassSection * >;
  $$->push_back( $1 );
}
| OneOrMoreClassSections ClassSection
{
  $$ = $1;
  $$->push_back( $2 );
}
;

ClassSection
: '(' T_members MemberDeclarations ')'
{
  $$ = $3;
}
| '(' T_prepare Group ')'
{
  $$ = new Ast::PrepareSection( @$, $3 );
}
| '(' T_identifier MethodDeclarations ')'
{
  DeleteOnExit< char > accessSpecDeleter( $2 );
  unsigned int accessSpec = 0;
  if( strcmp( $2, "__methods__" ) == 0  )
    {
      accessSpec = Ast::MEMBER_ACCESS_PUBLIC_GET | Ast::MEMBER_ACCESS_PROTECTED_GET;
    }
  else if( strcmp( $2, "__abstract__" ) == 0  )
    {
      accessSpec = Ast::MEMBER_ACCESS_PUBLIC_GET | Ast::MEMBER_ACCESS_PROTECTED_GET | Ast::MEMBER_ABSTRACT;
    }
  else if( strcmp( $2, "__final__" ) == 0  )
    {
      accessSpec = Ast::MEMBER_ACCESS_PUBLIC_GET | Ast::MEMBER_ACCESS_PROTECTED_GET | Ast::MEMBER_FINAL;
    }
  else if( strcmp( $2, "__protected__" ) == 0 )
    {
      accessSpec = Ast::MEMBER_ACCESS_PROTECTED_GET;
    }
  else if( strcmp( $2, "__private__" ) == 0 )
    {
      /* OK, no change */
    }
  else
    {
      throw Exceptions::ParserError( @2, strrefdup( "This is not a valid method access specifier." ) );
    }
  $3->addModeBits( accessSpec );
  $$ = $3;
}
| '(' T_abstract OrderedFormals ')'
{
  $$ = new Ast::AbstractSection( @$, $3 );
}
| '(' T_overrides Expr T_gr__ MethodDeclarations ')'
{
  $$ = new Ast::OverridesSection( $3, $5 );
}
;

MemberDeclarations
:
{
  $$ = new Ast::MemberSection;
}
| OneOrMoreMemberDeclarations
;

OneOrMoreMemberDeclarations
: MemberDeclaration
{
  $$ = new Ast::MemberSection;
  $$->push_back( $1 );
}
| OneOrMoreMemberDeclarations MemberDeclaration
{
  $$ = $1;
  $$->push_back( $2 );
}
;

MemberDeclaration
: '(' T_identifier Expr ')'
{
  $$ = new Ast::MemberDeclaration( @$, $2, $3, 0 );
}
| '(' T_identifier Expr MemberAccessList ')'
{
  $$ = new Ast::MemberDeclaration( @$, $2, $3, $4 );
}
;

MemberAccessList
: MemberAccessSpecifier
| MemberAccessList MemberAccessSpecifier
{
  $$ = $1 | $2;
}
;

MemberAccessSpecifier
: '.'
{
  $$ = Ast::MEMBER_ACCESS_PUBLIC_GET | Ast::MEMBER_ACCESS_PROTECTED_GET;
}
| T_llthan
{
  $$ = Ast::MEMBER_ACCESS_PUBLIC_GET | Ast::MEMBER_ACCESS_PUBLIC_INSERT | Ast::MEMBER_ACCESS_PROTECTED_GET | Ast::MEMBER_ACCESS_PROTECTED_INSERT;
}
| '(' '#' '.' ')'
{
  $$ = Ast::MEMBER_ACCESS_PROTECTED_GET;
}
| '(' '#' T_llthan ')'
{
  $$ = Ast::MEMBER_ACCESS_PROTECTED_GET | Ast::MEMBER_ACCESS_PROTECTED_INSERT;
}
| '^'
{
  $$ = Ast::MEMBER_TRANSFORMING;
}
;

MethodDeclarations
:
{
  $$ = new Ast::MemberSection;
}
| OneOrMoreMethodDeclarations
;

OneOrMoreMethodDeclarations
: MethodDeclaration
{
  $$ = new Ast::MemberSection;
  $$->push_back( $1 );
}
| OneOrMoreMethodDeclarations MethodDeclaration
{
  $$ = $1;
  $$->push_back( $2 );
}
;

MethodDeclaration
: '(' T_identifier Expr ')'
{
  $$ = new Ast::MemberDeclaration( @$, $2, $3, Ast::MEMBER_CONST | Ast::MEMBER_METHOD );
}
| '(' '[' T_identifier Formals ']' FunctionModeList GroupElem ')'
{
  Ast::Expression * body = dynamic_cast< Ast::Expression * >( $7 );
  if( body == 0 )
    {
      std::list< Ast::Node * > * bracket = new std::list< Ast::Node * >( );
      bracket->push_back( $7 );
      body = new Ast::CodeBracket( @7, bracket );
    }
  Ast::ArgListExprs * args = new Ast::ArgListExprs( false );
  Ast::FunctionFunction * res = new Ast::FunctionFunction( @$, $4, body, $6 );
  res->push_exprs( args );
  $$ = new Ast::MemberDeclaration( @$, $3, new Ast::CallExpr( @$,
								      RefCountPtr< const Lang::Function >( res ),
												   args ),
				       Ast::MEMBER_CONST | Ast::MEMBER_METHOD | ( (($6 & Ast::FUNCTION_TRANSFORMING) != 0) ? Ast::MEMBER_TRANSFORMING : 0 ) );
}
;

FunctionModeList
:
{
  $$ = 0;
}
| OneOrMoreFunctionModeSpecifiers
;

OneOrMoreFunctionModeSpecifiers
: FunctionModeSpecifier
| OneOrMoreFunctionModeSpecifiers FunctionModeSpecifier
{
  $$ = $1 | $2;
}
;

FunctionModeSpecifier
: '^'
{
  $$ = Ast::FUNCTION_TRANSFORMING;
}
;


%%

/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated y.tab.c file.
 * This section is where you put definitions of helper functions.
 */
