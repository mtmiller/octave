%top {
////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2013-2022 The Octave Project Developers
//
// See the file COPYRIGHT.md in the top-level directory of this
// distribution or <https://octave.org/copyright/>.
//
// This file is part of Octave.
//
// Octave is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Octave is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Octave; see the file COPYING.  If not, see
// <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#if defined (HAVE_PRAGMA_GCC_DIAGNOSTIC)
   // This one needs to be global.
#  pragma GCC diagnostic ignored "-Wunused-function"

   // Disable these warnings for code that is generated by flex, including
   // pattern rules.  Push the current state so we can restore the warning
   // state prior to functions we define at the bottom of the file.
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsign-compare"
#  pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

// Define away the deprecated register storage class specifier to avoid
// potential warnings about it.
#if ! defined (register)
#  define register
#endif

}

%option prefix = "octave_tex_"
%option noyywrap
%option reentrant
%option bison-bridge

%option noyyalloc
%option noyyrealloc
%option noyyfree

%x NUM_MODE
%x MAYBE_NUM_MODE

%{

#include "unistd-wrappers.h"

#include "text-engine.h"

// oct-tex-parser.h must be included after text-engine.h
#include "oct-tex-parser.h"

// FIXME: with bison 3.x, OCTAVE_TEX_STYPE appears in the generated
// oct-parse.h file, but there is no definition for YYSTYPE, which is
// needed by the code that is generated by flex.  I can't seem to find
// a way to tell flex to use OCTAVE_TEX_STYPE instead of YYSTYPE in
// the code it generates, or to tell bison to provide the definition
// of YYSTYPE in the generated oct-parse.h file.

#if defined (OCTAVE_TEX_STYPE_IS_DECLARED) && ! defined YYSTYPE
#  define YYSTYPE OCTAVE_TEX_STYPE
#endif

#define YY_NO_UNISTD_H 1
#define isatty octave_isatty_wrapper
#define yyguts_t octave_tex_yyguts_t

%}

D   [0-9]
NUM (({D}+\.?{D}*)|(\.{D}+))

%%

%{
// Numeric values.
%}

<NUM_MODE>{NUM} {
    int nread = sscanf (yytext, "%lf", &(yylval->num));

    if (nread == 1)
      return NUM;
  }

<NUM_MODE>[, \t]+ { }

<NUM_MODE>"\n"|. {
    yyless (0);
    BEGIN (INITIAL);
  }

<MAYBE_NUM_MODE>"{" {
    BEGIN (NUM_MODE);
    return START;
  }

<MAYBE_NUM_MODE>"\n"|. {
    yyless (0);
    BEGIN (INITIAL);
  }

%{
// Simple commands.
%}

"\\bf" { return BF; }
"\\it" { return IT; }
"\\sl" { return SL; }
"\\rm" { return RM; }

%{
// Generic font commands.
%}

"\\fontname" { return FONTNAME; }

"\\fontsize" {
    BEGIN (MAYBE_NUM_MODE);
    return FONTSIZE;
  }

"\\color[rgb]" {
    BEGIN (MAYBE_NUM_MODE);
    return COLOR_RGB;
  }

"\\color" { return COLOR; }

%{
// Special characters.
%}

"{" { return START; }
"}" { return END; }
"^" { return SUPER; }
"_" { return SUB; }

"\\{"  |
"\\}"  |
"\\^"  |
"\\_"  |
"\\\\" {
    yylval->ch = yytext[1];
    return CH;
  }

%{
// Symbols.
%}

@SYMBOL_RULES@

%{
// Generic character.
%}

"\n" |
.    {
    yylval->ch = yytext[0];
    return CH;
  }

%{
#if defined (HAVE_PRAGMA_GCC_DIAGNOSTIC)
   // Also disable this warning for functions that are generated by flex
   // after the pattern rules.
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
%}

%%

#if defined (HAVE_PRAGMA_GCC_DIAGNOSTIC)
   // Restore prevailing warning state for remainder of the file.
#  pragma GCC diagnostic pop
#endif

void *
octave_tex_alloc (yy_size_t size, yyscan_t)
{
  return malloc (size);
}

void *
octave_tex_realloc (void *ptr, yy_size_t size, yyscan_t)
{
return realloc (ptr, size);
}

void
octave_tex_free (void *ptr, yyscan_t)
{
  free (ptr);
}

OCTAVE_BEGIN_NAMESPACE(octave)

  bool text_parser_tex::init_lexer (const std::string& s)
  {
    if (! m_scanner)
      octave_tex_lex_init (&m_scanner);

    if (m_scanner)
      {
        if (m_buffer_state)
          {
            octave_tex__delete_buffer (reinterpret_cast<YY_BUFFER_STATE> (m_buffer_state),
                                       m_scanner);
            m_buffer_state = nullptr;
          }

        m_buffer_state = octave_tex__scan_bytes (s.data (), s.length (),
                                                 m_scanner);
      }

    return (m_scanner && m_buffer_state);
  }

  void text_parser_tex::destroy_lexer (void)
  {
    if (m_buffer_state)
      {
        octave_tex__delete_buffer (reinterpret_cast<YY_BUFFER_STATE> (m_buffer_state),
                                   m_scanner);
        m_buffer_state = nullptr;
      }

    if (m_scanner)
      {
        octave_tex_lex_destroy (m_scanner);
        m_scanner = nullptr;
      }
  }

OCTAVE_END_NAMESPACE(octave)
