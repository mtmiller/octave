/*

Copyright (C) 1993-2012 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if !defined (octave_token_h)
#define octave_token_h 1

#include <string>

#include "symtab.h"

class
token
{
public:

  enum token_type
    {
      generic_token,
      string_token,
      double_token,
      ettype_token,
      sym_rec_token,
      scls_rec_token,
      meta_rec_token
    };

  enum end_tok_type
    {
      simple_end,
      classdef_end,
      enumeration_end,
      events_end,
      for_end,
      function_end,
      if_end,
      methods_end,
      parfor_end,
      properties_end,
      switch_end,
      while_end,
      try_catch_end,
      unwind_protect_end
    };

  token (int tv, int l = -1, int c = -1);
  token (int tv, const std::string& s, int l = -1, int c = -1);
  token (int tv, double d, const std::string& s = std::string (),
         int l = -1, int c = -1);
  token (int tv, end_tok_type t, int l = -1, int c = -1);
  token (int tv, symbol_table::symbol_record *s, int l = -1, int c = -1);
  token (int tv, symbol_table::symbol_record *cls,
         symbol_table::symbol_record *pkg, int l = -1, int c = -1);
  token (int tv, symbol_table::symbol_record *mth,
         symbol_table::symbol_record *cls,
         symbol_table::symbol_record *pkg, int l = -1, int c = -1);

  ~token (void);

  void mark_trailing_space (void) { tspc = true; }
  bool space_follows_token (void) const { return tspc; }

  int token_value (void) const { return tok_val; }
  bool token_value_is (int tv) const { return tv == tok_val; }

  int line (void) const { return line_num; }
  int column (void) const { return column_num; }

  std::string text (void) const;
  double number (void) const;
  end_tok_type ettype (void) const;
  symbol_table::symbol_record *sym_rec (void);

  symbol_table::symbol_record *method_rec (void);
  symbol_table::symbol_record *class_rec (void);
  symbol_table::symbol_record *package_rec (void);

  symbol_table::symbol_record *meta_class_rec (void);
  symbol_table::symbol_record *meta_package_rec (void);

  std::string text_rep (void);

private:

  // No copying!

  token (const token& tok);

  token& operator = (const token& tok);

  bool tspc;
  int line_num;
  int column_num;
  int tok_val;
  token_type type_tag;
  union
    {
      std::string *str;
      double num;
      end_tok_type et;
      symbol_table::symbol_record *sr;
      struct
        {
          symbol_table::symbol_record *mr;
          symbol_table::symbol_record *cr;
          symbol_table::symbol_record *pr;
        } sc;
      struct
        {
          symbol_table::symbol_record *cr;
          symbol_table::symbol_record *pr;
        } mc;
    };
  std::string orig_text;
};

#endif
