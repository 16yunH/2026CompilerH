// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "parser.tab.hh"


// Unqualified %code blocks.
#line 45 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"

    namespace fdmj 
    {
        template<typename RHS>
        void calcLocation(location_t &current, const RHS &rhs, const std::size_t n);
    }
    
    #define YYLLOC_DEFAULT(Cur, Rhs, N) calcLocation(Cur, Rhs, N)
    #define yylex lexer.yylex
    Pos *p;

#line 58 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 23 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
namespace fdmj {
#line 151 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"

  /// Build a parser object.
  ASTParser::ASTParser (ASTLexer &lexer_yyarg, const bool debug_yyarg, AST_YYSTYPE* result_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      lexer (lexer_yyarg),
      debug (debug_yyarg),
      result (result_yyarg)
  {}

  ASTParser::~ASTParser ()
  {}

  ASTParser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/

  // basic_symbol.
  template <typename Base>
  ASTParser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  ASTParser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  ASTParser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (value_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}


  template <typename Base>
  ASTParser::symbol_kind_type
  ASTParser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  ASTParser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  ASTParser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_kind.
  ASTParser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  ASTParser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  ASTParser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  ASTParser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  void
  ASTParser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  ASTParser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  ASTParser::symbol_kind_type
  ASTParser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  ASTParser::symbol_kind_type
  ASTParser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }



  // by_state.
  ASTParser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  ASTParser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  ASTParser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  ASTParser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  ASTParser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  ASTParser::symbol_kind_type
  ASTParser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  ASTParser::stack_symbol_type::stack_symbol_type ()
  {}

  ASTParser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  ASTParser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  ASTParser::stack_symbol_type&
  ASTParser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }

  ASTParser::stack_symbol_type&
  ASTParser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  ASTParser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    YY_USE (yysym.kind ());
  }

#if YYDEBUG
  template <typename Base>
  void
  ASTParser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  ASTParser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  ASTParser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  ASTParser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  ASTParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  ASTParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  ASTParser::debug_level_type
  ASTParser::debug_level () const
  {
    return yydebug_;
  }

  void
  ASTParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  ASTParser::state_type
  ASTParser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  ASTParser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  ASTParser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  ASTParser::operator() ()
  {
    return parse ();
  }

  int
  ASTParser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
#line 39 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
{
    #if YYDEBUG != 0
        set_debug_level(debug);
    #endif
}

#line 499 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // PROG: MAINMETHOD
#line 77 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  { 
#ifdef DEBUG
    cerr << "Program" << endl;
#endif
    result->root = new Program(p, (yystack_[0].value.mainMethod));
    //$$ = result->root;
  }
#line 643 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 3: // MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' STMLIST '}'
#line 86 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "MainMethod" << endl;
#endif
    (yylhs.value.mainMethod) = new MainMethod(p, (yystack_[1].value.stmList)) ;
  }
#line 654 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 4: // STMLIST: %empty
#line 94 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "STMLIST empty" << endl;
#endif
    (yylhs.value.stmList) = new vector<Stm*>();
  }
#line 665 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 5: // STMLIST: STM STMLIST
#line 102 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "STM STMLIST" << endl;
#endif
    vector<Stm*> *v = (yystack_[0].value.stmList);
    v->push_back((yystack_[1].value.stm));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.stmList) = v;
  }
#line 679 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 6: // STM: ID '=' EXP ';'
#line 113 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Assign" << endl;
#endif
    (yylhs.value.stm) = new Assign(p, (yystack_[3].value.idExp), (yystack_[1].value.exp));
  }
#line 690 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 7: // STM: RETURN EXP ';'
#line 121 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Return" << endl;
#endif
    (yylhs.value.stm) = new Return(p, (yystack_[1].value.exp));
  }
#line 701 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 8: // EXP: '(' EXP ADD EXP ')'
#line 129 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP ADD EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[2].location.sline, yystack_[2].location.scolumn, yystack_[2].location.eline, yystack_[2].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[3].value.exp), new OpExp(p1, "+"), (yystack_[1].value.exp));
  }
#line 713 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 9: // EXP: '(' EXP MINUS EXP ')'
#line 138 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP MINUS EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[2].location.sline, yystack_[2].location.scolumn, yystack_[2].location.eline, yystack_[2].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[3].value.exp), new OpExp(p1, "-"), (yystack_[1].value.exp));
  }
#line 725 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 10: // EXP: '(' EXP TIMES EXP ')'
#line 147 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP TIMES EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[2].location.sline, yystack_[2].location.scolumn, yystack_[2].location.eline, yystack_[2].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[3].value.exp), new OpExp(p1, "*"), (yystack_[1].value.exp));
  }
#line 737 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 11: // EXP: '(' EXP DIVIDE EXP ')'
#line 156 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP DIVIDE EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[2].location.sline, yystack_[2].location.scolumn, yystack_[2].location.eline, yystack_[2].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[3].value.exp), new OpExp(p1, "/"), (yystack_[1].value.exp));
  }
#line 749 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 12: // EXP: NONNEGATIVEINT
#line 165 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "NonNegativeInt: " << (yystack_[0].value.i) << endl;
#endif
    (yylhs.value.exp) = new IntExp(p, (yystack_[0].value.i));
  }
#line 760 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 13: // EXP: '(' MINUS EXP ')'
#line 173 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "- EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[2].location.sline, yystack_[2].location.scolumn, yystack_[2].location.eline, yystack_[2].location.ecolumn);
    (yylhs.value.exp) =  new UnaryOp(p, new OpExp(p1, "-"), (yystack_[1].value.exp));
  }
#line 772 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 14: // EXP: '(' EXP ')'
#line 182 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "( EXP )" << endl;
#endif
    (yylhs.value.exp) = (yystack_[1].value.exp);
  }
#line 783 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 15: // EXP: ID
#line 190 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "ID" << endl;
#endif
    (yylhs.value.exp) = (yystack_[0].value.idExp);
  }
#line 794 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;

  case 16: // ID: IDENTIFIER
#line 198 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Identifier: " << (yystack_[0].value.s) << endl;
#endif
    (yylhs.value.idExp) = new IdExp(p, (yystack_[0].value.s));
  }
#line 805 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"
    break;


#line 809 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  ASTParser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  const char *
  ASTParser::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "PUBLIC", "INT", "MAIN",
  "RETURN", "NONNEGATIVEINT", "IDENTIFIER", "'('", "')'", "'['", "']'",
  "'{'", "'}'", "'='", "','", "';'", "'.'", "ADD", "MINUS", "TIMES",
  "DIVIDE", "EQ", "NE", "LT", "LE", "GT", "GE", "AND", "OR", "$accept",
  "PROG", "MAINMETHOD", "STMLIST", "STM", "EXP", "ID", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  // ASTParser::context.
  ASTParser::context::context (const ASTParser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  ASTParser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
  ASTParser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  ASTParser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char ASTParser::yypact_ninf_ = -17;

  const signed char ASTParser::yytable_ninf_ = -1;

  const signed char
  ASTParser::yypact_[] =
  {
       1,     7,    10,   -17,    13,   -17,    14,     9,    17,    16,
      -6,   -17,    15,    16,    18,   -17,     0,    19,   -17,   -17,
     -17,    -6,    -6,     6,   -17,    20,    21,   -17,    -6,    -6,
      -6,    -6,   -17,   -17,    22,    24,    25,    28,   -17,   -17,
     -17,   -17
  };

  const signed char
  ASTParser::yydefact_[] =
  {
       0,     0,     0,     2,     0,     1,     0,     0,     0,     4,
       0,    16,     0,     4,     0,    12,     0,     0,    15,     3,
       5,     0,     0,     0,     7,     0,     0,    14,     0,     0,
       0,     0,     6,    13,     0,     0,     0,     0,     8,     9,
      10,    11
  };

  const signed char
  ASTParser::yypgoto_[] =
  {
     -17,   -17,   -17,    26,   -17,   -16,     8
  };

  const signed char
  ASTParser::yydefgoto_[] =
  {
       0,     2,     3,    12,    13,    17,    18
  };

  const signed char
  ASTParser::yytable_[] =
  {
      23,    15,    11,    16,     1,    25,    26,    15,    11,    16,
       5,     4,    34,    35,    36,    37,    27,    14,     6,     8,
      22,    14,    10,     7,    11,    28,    29,    30,    31,    19,
       9,    33,    38,    21,    39,    40,    24,    32,    41,    20
  };

  const signed char
  ASTParser::yycheck_[] =
  {
      16,     7,     8,     9,     3,    21,    22,     7,     8,     9,
       0,     4,    28,    29,    30,    31,    10,     9,     5,    10,
      20,    13,     6,     9,     8,    19,    20,    21,    22,    14,
      13,    10,    10,    15,    10,    10,    17,    17,    10,    13
  };

  const signed char
  ASTParser::yystos_[] =
  {
       0,     3,    32,    33,     4,     0,     5,     9,    10,    13,
       6,     8,    34,    35,    37,     7,     9,    36,    37,    14,
      34,    15,    20,    36,    17,    36,    36,    10,    19,    20,
      21,    22,    17,    10,    36,    36,    36,    36,    10,    10,
      10,    10
  };

  const signed char
  ASTParser::yyr1_[] =
  {
       0,    31,    32,    33,    34,    34,    35,    35,    36,    36,
      36,    36,    36,    36,    36,    36,    37
  };

  const signed char
  ASTParser::yyr2_[] =
  {
       0,     2,     1,     8,     0,     2,     4,     3,     5,     5,
       5,     5,     1,     4,     3,     1,     1
  };




#if YYDEBUG
  const unsigned char
  ASTParser::yyrline_[] =
  {
       0,    76,    76,    85,    94,   101,   112,   120,   128,   137,
     146,   155,   164,   172,   181,   189,   197
  };

  void
  ASTParser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  ASTParser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  ASTParser::symbol_kind_type
  ASTParser::yytranslate_ (int t) YY_NOEXCEPT
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       9,    10,     2,     2,    16,     2,    18,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    17,
       2,    15,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    11,     2,    12,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    13,     2,    14,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30
    };
    // Last valid token kind.
    const int code_max = 275;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

#line 23 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"
} // fdmj
#line 1296 "/home/yunhong16/CompilerH/HW1/build/lib/frontend/parser.cc"

#line 206 "/home/yunhong16/CompilerH/HW1/lib/frontend/parser.yy"

/*
void yyerror(char *s) {
  fprintf(stderr, "%s\n",s);
}

int yywrap() {
  return(1);
}
*/

//%code 
namespace fdmj 
{
    template<typename RHS>
    inline void calcLocation(location_t &current, const RHS &rhs, const std::size_t n)
    {
        current = location_t(YYRHSLOC(rhs, 1).sline, YYRHSLOC(rhs, 1).scolumn, 
                                    YYRHSLOC(rhs, n).eline, YYRHSLOC(rhs, n).ecolumn);
        p = new Pos(current.sline, current.scolumn, current.eline, current.ecolumn);
    }
    
    void ASTParser::error(const location_t &location, const std::string &message)
    {
        std::cerr << "Error at lines " << location << ": " << message << std::endl;
    }

  Program* fdmjParser(ifstream &fp, const bool debug) {
    fdmj:AST_YYSTYPE result; 
    result.root = nullptr;
    fdmj::ASTLexer lexer(fp, debug);
    fdmj::ASTParser parser(lexer, debug, &result); //set up the parser
    if (parser() ) { //call the parser
      cout << "Error: parsing failed" << endl;
      return nullptr;
    }
    if (debug) cout << "Parsing successful" << endl;
    return result.root;
  }

  Program*  fdmjParser(const string &filename, const bool debug) {
    std::ifstream fp(filename);
    if (!fp) {
      cout << "Error: cannot open file " << filename << endl;
      return nullptr;
    }
    return fdmjParser(fp, debug);
  }
}
