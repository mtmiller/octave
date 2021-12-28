////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016-2022 The Octave Project Developers
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

#if ! defined (octave_oct_string_h)
#define octave_oct_string_h 1

#include "octave-config.h"

#include "oct-cmplx.h"

namespace octave
{
  //! Octave string utility functions.
  //!
  //! This functions provide a C++ interface to most string functions
  //! available in the Octave interpreter.
  //!
  //! Specializations for Array may consider its dimensions in addition
  //! to the actual string contents.
  //!
  //! @attention
  //! Octave's string comparison functions return true when strings are
  //! are equal, just the opposite of the corresponding C library functions.
  //! In addition, Octave's function only return bool and do not check
  //! lexicographical order.

  namespace string
  {
    //! True if strings are the same.
    //!
    //! ## Specialization for Array<char>
    //!
    //! When comparing whole Array of chars, the actual Array dimensions
    //! are significant.  A column vector and row vector with the same
    //! char array, will still return false.

    template <typename T>
    OCTAVE_API bool
    strcmp (const T& str_a, const T& str_b);

    //! True if string is the same as character sequence.
    //!
    //! Compares a string to the null-terminated character sequence
    //! beginning at the character pointed to by str_b.
    //!
    //! ## Specialization for Array<char>
    //!
    //! For purposes of comparison of dimensions, the character sequence
    //! is considered to be a row vector.

    template <typename T>
    OCTAVE_API bool
    strcmp (const T& str_a, const typename T::value_type *str_b);

    //! True if strings are the same, ignoring case.
    //!
    //! ## Specialization for Array<char>
    //!
    //! When comparing whole Array of chars, the actual Array dimensions
    //! are significant.  A column vector and row vector with the same
    //! char array, will still return false.

    template <typename T>
    OCTAVE_API bool
    strcmpi (const T& str_a, const T& str_b);

    //! True if string is the same as character sequence, ignoring case.
    //!
    //! ## Specialization for Array<char>
    //!
    //! For purposes of comparison of dimensions, the character sequence
    //! is considered to be a row vector.

    template <typename T>
    OCTAVE_API bool
    strcmpi (const T& str_a, const typename T::value_type *str_b);

    //! True if the first N characters are the same.
    //!
    //! ## Specialization for Array<char>
    //!
    //! The comparison is done in the first N characters, the actual
    //! dimensions of the Array are irrelevant.  A row vector and
    //! a column vector of the same still return true.

    template <typename T>
    OCTAVE_API bool
    strncmp (const T& str_a, const T& str_b,
             const typename T::size_type n);

    //! True if the first N characters are the same.
    template <typename T>
    OCTAVE_API bool
    strncmp (const T& str_a, const typename T::value_type *str_b,
             const typename T::size_type n);

    //! True if the first N characters are the same, ignoring case.
    //!
    //! ## Specialization for Array<char>
    //!
    //! The comparison is done in the first N characters, the actual
    //! dimensions of the Array are irrelevant.  A row vector and
    //! a column vector of the same still return true.

    template <typename T>
    OCTAVE_API bool
    strncmpi (const T& str_a, const T& str_b,
              const typename T::size_type n);

    //! True if the first N characters are the same, ignoring case.
    template <typename T>
    OCTAVE_API bool
    strncmpi (const T& str_a, const typename T::value_type *str_b,
              const typename T::size_type n);

    extern OCTAVE_API Complex
    str2double (const std::string& str_arg);

    extern OCTAVE_API std::string
    u8_to_encoding (const std::string& who, const std::string& u8_string,
                    const std::string& encoding);

    extern OCTAVE_API std::string
    u8_from_encoding (const std::string& who, const std::string& native_string,
                      const std::string& encoding);

    enum u8_fallback_type
    {
      U8_REPLACEMENT_CHAR,
      U8_ISO_8859_1
    };

    extern OCTAVE_API unsigned int
    u8_validate (const std::string& who, std::string& in_string,
                 const u8_fallback_type type = U8_REPLACEMENT_CHAR);
  }
}

template <typename T>
extern OCTAVE_API std::string
rational_approx (T val, int len);

#endif
