////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2016-2020 The Octave Project Developers
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

#include "oct-string.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <string>

#include "Array.h"
#include "lo-ieee.h"
#include "lo-mappers.h"
#include "uniconv-wrappers.h"
#include "unistr-wrappers.h"
#include "unwind-prot.h"

template <typename T>
static bool
str_data_cmp (const typename T::value_type *a, const typename T::value_type *b,
              const typename T::size_type n)
{
  for (typename T::size_type i = 0; i < n; ++i)
    if (a[i] != b[i])
      return false;
  return true;
}

template <typename T>
static bool
str_data_cmpi (const typename T::value_type *a, const typename T::value_type *b,
               const typename T::size_type n)
{
  for (typename T::size_type i = 0; i < n; ++i)
    if (std::tolower (a[i]) != std::tolower (b[i]))
      return false;
  return true;
}


// Templates to handle std::basic_string, std::vector, Array, and char*.
template <typename T>
typename T::size_type
numel (const T& str)
{
  return str.size ();
}

template <>
octave_idx_type
numel (const Array<char>& str)
{
  return str.numel ();
}

template <typename T>
typename T::size_type
strlen (const typename T::value_type *str)
{
  return std::strlen (str);
}

template <typename T>
bool
sizes_cmp (const T& str_a, const T& str_b)
{
  return str_a.size () == str_b.size ();
}

template <>
bool
sizes_cmp (const Array<char>& str_a, const Array<char>& str_b)
{
  return str_a.dims () == str_b.dims ();
}

template <typename T>
bool
sizes_cmp (const T& str_a, const typename T::value_type *str_b)
{
  return str_a.size () == strlen<T> (str_b);
}

template <>
bool
sizes_cmp (const Array<char>& str_a, const char *str_b)
{
  return (str_a.isvector () && str_a.rows () == 1
          && str_a.numel () == strlen<Array<char>> (str_b));
}


template<typename T>
bool
octave::string::strcmp (const T& str_a, const T& str_b)
{
  return (sizes_cmp (str_a, str_b)
          && str_data_cmp<T> (str_a.data (), str_b.data (), numel (str_a)));
}

template<typename T>
bool
octave::string::strcmp (const T& str_a, const typename T::value_type *str_b)
{
  return (sizes_cmp (str_a, str_b)
          && str_data_cmp<T> (str_a.data (), str_b, numel (str_a)));
}


template<typename T>
bool
octave::string::strcmpi (const T& str_a, const T& str_b)
{
  return (sizes_cmp (str_a, str_b)
          && str_data_cmpi<T> (str_a.data (), str_b.data (), numel (str_a)));
}

template<typename T>
bool
octave::string::strcmpi (const T& str_a, const typename T::value_type *str_b)
{
  return (sizes_cmp (str_a, str_b)
          && str_data_cmpi<T> (str_a.data (), str_b, numel (str_a)));
}


template<typename T>
bool
octave::string::strncmp (const T& str_a, const T& str_b,
                         const typename T::size_type n)
{
  typename T::size_type neff;
  auto len_a = numel (str_a);
  auto len_b = numel (str_b);
  neff = std::min (std::max (len_a, len_b), n);

  return (len_a >= neff && len_b >= neff
          && str_data_cmp<T> (str_a.data (), str_b.data (), neff));
}

template<typename T>
bool
octave::string::strncmp (const T& str_a, const typename T::value_type *str_b,
                         const typename T::size_type n)
{
  typename T::size_type neff;
  auto len_a = numel (str_a);
  auto len_b = strlen<T> (str_b);
  neff = std::min (std::max (len_a, len_b), n);

  return (len_a >= neff && len_b >= neff
          && str_data_cmp<T> (str_a.data (), str_b, neff));
}


template<typename T>
bool
octave::string::strncmpi (const T& str_a, const T& str_b,
                          const typename T::size_type n)
{
  typename T::size_type neff;
  auto len_a = numel (str_a);
  auto len_b = numel (str_b);
  neff = std::min (std::max (len_a, len_b), n);

  return (len_a >= neff && len_b >= neff
          && str_data_cmpi<T> (str_a.data (), str_b.data (), neff));
}

template<typename T>
bool
octave::string::strncmpi (const T& str_a, const typename T::value_type *str_b,
                          const typename T::size_type n)
{
  typename T::size_type neff;
  auto len_a = numel (str_a);
  auto len_b = strlen<T> (str_b);
  neff = std::min (std::max (len_a, len_b), n);

  return (len_a >= neff && len_b >= neff
          && str_data_cmpi<T> (str_a.data (), str_b, neff));
}


// Instantiations we need
#define INSTANTIATE_OCTAVE_STRING(T)                                          \
  template bool octave::string::strcmp<T> (const T&, const T&);               \
  template bool octave::string::strcmp<T> (const T&,                          \
                                           const typename T::value_type*);    \
  template bool octave::string::strcmpi<T> (const T&, const T&);              \
  template bool octave::string::strcmpi<T> (const T&,                         \
                                            const typename T::value_type*);   \
  template bool octave::string::strncmp<T> (const T&, const T&,               \
                                            const typename T::size_type);     \
  template bool octave::string::strncmp<T> (const T&,                         \
                                            const typename T::value_type*,    \
                                            const typename T::size_type);     \
  template bool octave::string::strncmpi<T> (const T&, const T&,              \
                                             const typename T::size_type n);  \
  template bool octave::string::strncmpi<T> (const T&,                        \
                                             const typename T::value_type*,   \
                                             const typename T::size_type);

// We could also instantiate std::vector<char> but would it be
// useful for anyone?
INSTANTIATE_OCTAVE_STRING(std::string)
INSTANTIATE_OCTAVE_STRING(Array<char>)

#undef INSTANTIATE_OCTAVE_STRING

static inline bool
is_imag_unit (int c)
{ return c == 'i' || c == 'j'; }

static double
single_num (std::istringstream& is)
{
  double num = 0.0;

  char c = is.peek ();

  // Skip spaces.
  while (isspace (c))
    {
      is.get ();
      c = is.peek ();
    }

  if (std::toupper (c) == 'I')
    {
      // It's infinity.
      is.get ();
      char c1 = is.get ();
      char c2 = is.get ();
      if (std::tolower (c1) == 'n' && std::tolower (c2) == 'f')
        {
          num = octave::numeric_limits<double>::Inf ();
          is.peek (); // May set EOF bit.
        }
      else
        is.setstate (std::ios::failbit); // indicate that read has failed.
    }
  else if (c == 'N')
    {
      // It's NA or NaN
      is.get ();
      char c1 = is.get ();
      if (c1 == 'A')
        {
          num = octave_NA;
          is.peek (); // May set EOF bit.
        }
      else
        {
          char c2 = is.get ();
          if (c1 == 'a' && c2 == 'N')
            {
              num = octave::numeric_limits<double>::NaN ();
              is.peek (); // May set EOF bit.
            }
          else
            is.setstate (std::ios::failbit); // indicate that read has failed.
        }
    }
  else
    is >> num;

  return num;
}

static std::istringstream&
extract_num (std::istringstream& is, double& num, bool& imag, bool& have_sign)
{
  have_sign = imag = false;

  char c = is.peek ();

  // Skip leading spaces.
  while (isspace (c))
    {
      is.get ();
      c = is.peek ();
    }

  bool negative = false;

  // Accept leading sign.
  if (c == '+' || c == '-')
    {
      have_sign = true;
      negative = c == '-';
      is.get ();
      c = is.peek ();
    }

  // Skip spaces after sign.
  while (isspace (c))
    {
      is.get ();
      c = is.peek ();
    }

  // Imaginary number (i*num or just i), or maybe 'inf'.
  if (c == 'i')
    {
      // possible infinity.
      is.get ();
      c = is.peek ();

      if (is.eof ())
        {
          // just 'i' and string is finished.  Return immediately.
          imag = true;
          num = (negative ? -1.0 : 1.0);
          return is;
        }
      else
        {
          if (std::tolower (c) != 'n')
            imag = true;
          is.unget ();
        }
    }
  else if (c == 'j')
    imag = true;

  // It's i*num or just i
  if (imag)
    {
      is.get ();
      c = is.peek ();
      // Skip spaces after imaginary unit.
      while (isspace (c))
        {
          is.get ();
          c = is.peek ();
        }

      if (c == '*')
        {
          // Multiplier follows, we extract it as a number.
          is.get ();
          num = single_num (is);
          if (is.good ())
            c = is.peek ();
        }
      else
        num = 1.0;
    }
  else
    {
      // It's num, num*i, or numi.
      num = single_num (is);
      if (is.good ())
        {
          c = is.peek ();

          // Skip spaces after number.
          while (isspace (c))
            {
              is.get ();
              c = is.peek ();
            }

          if (c == '*')
            {
              is.get ();
              c = is.peek ();

              // Skip spaces after operator.
              while (isspace (c))
                {
                  is.get ();
                  c = is.peek ();
                }

              if (is_imag_unit (c))
                {
                  imag = true;
                  is.get ();
                  c = is.peek ();
                }
              else
                is.setstate (std::ios::failbit); // indicate read has failed.
            }
          else if (is_imag_unit (c))
            {
              imag = true;
              is.get ();
              c = is.peek ();
            }
        }
    }

  if (is.good ())
    {
      // Skip trailing spaces.
      while (isspace (c))
        {
          is.get ();
          c = is.peek ();
        }
    }

  if (negative)
    num = -num;

  return is;
}

static inline void
set_component (Complex& c, double num, bool imag)
{
#if defined (HAVE_CXX_COMPLEX_SETTERS)
  if (imag)
    c.imag (num);
  else
    c.real (num);
#elif defined (HAVE_CXX_COMPLEX_REFERENCE_ACCESSORS)
  if (imag)
    c.imag () = num;
  else
    c.real () = num;
#else
  if (imag)
    c = Complex (c.real (), num);
  else
    c = Complex (num, c.imag ());
#endif
}

Complex
octave::string::str2double (const std::string& str_arg)
{
  Complex val (0.0, 0.0);

  std::string str = str_arg;

  // FIXME: removing all commas doesn't allow actual parsing.
  //        Example: "1,23.45" is wrong, but passes Octave.
  str.erase (std::remove (str.begin (), str.end(), ','), str.end ());
  std::istringstream is (str);

  double num;
  bool i1, i2, s1, s2;

  if (is.eof ())
    val = octave::numeric_limits<double>::NaN ();
  else if (! extract_num (is, num, i1, s1))
    val = octave::numeric_limits<double>::NaN ();
  else
    {
      set_component (val, num, i1);

      if (! is.eof ())
        {
          if (! extract_num (is, num, i2, s2) || i1 == i2 || ! s2)
            val = octave::numeric_limits<double>::NaN ();
          else
            set_component (val, num, i2);
        }
    }

  return val;
}

std::string
octave::string::u8_to_encoding (const std::string& who,
                                const std::string& u8_string,
                                const std::string& encoding)
{
  const uint8_t *src = reinterpret_cast<const uint8_t *>
                       (u8_string.c_str ());
  size_t srclen = u8_string.length ();

  size_t length;
  char *native_str = octave_u8_conv_to_encoding (encoding.c_str (), src,
                                                 srclen, &length);

  if (! native_str)
    {
      if (errno == ENOSYS)
        (*current_liboctave_error_handler)
          ("%s: iconv() is not supported. Installing GNU libiconv and then "
           "re-compiling Octave could fix this.", who.c_str ());
      else
        (*current_liboctave_error_handler)
          ("%s: converting from UTF-8 to codepage '%s' failed: %s",
           who.c_str (), encoding.c_str (), std::strerror (errno));
    }

  octave::unwind_action free_native_str
    ([] (const auto native_str_ptr)
     {
       ::free (native_str_ptr);
     }, static_cast<void *> (native_str));

  std::string retval = std::string (native_str, length);

  return retval;
}

std::string
octave::string::u8_from_encoding (const std::string& who,
                                  const std::string& native_string,
                                  const std::string& encoding)
{
  const char *src = native_string.c_str ();
  size_t srclen = native_string.length ();

  size_t length;
  uint8_t *utf8_str = octave_u8_conv_from_encoding (encoding.c_str (), src,
                                                    srclen, &length);
  if (! utf8_str)
    {
      if (errno == ENOSYS)
        (*current_liboctave_error_handler)
          ("%s: iconv() is not supported. Installing GNU libiconv and then "
           "re-compiling Octave could fix this.", who.c_str ());
      else
        (*current_liboctave_error_handler)
          ("%s: converting from codepage '%s' to UTF-8 failed: %s",
           who.c_str (), encoding.c_str (), std::strerror (errno));
    }

  octave::unwind_action free_utf8_str
    ([] (const auto utf8_str_ptr)
     {
       ::free (utf8_str_ptr);
     }, static_cast<void *> (utf8_str));

  std::string retval = std::string (reinterpret_cast<char *> (utf8_str), length);

  return retval;
}

unsigned int
octave::string::u8_validate (const std::string& who,
                             std::string& in_str,
                             const octave::string::u8_fallback_type type)
{
  std::string out_str;

  unsigned int num_replacements = 0;
  const char *in_chr = in_str.c_str ();
  const char *inv_utf8 = in_chr;
  const char * const in_end = in_chr + in_str.length ();
  while (inv_utf8 && in_chr < in_end)
    {
      inv_utf8 = reinterpret_cast<const char *>
          (octave_u8_check_wrapper (reinterpret_cast<const uint8_t *> (in_chr),
                                    in_end - in_chr));

      if (inv_utf8 == nullptr)
        out_str.append (in_chr, in_end - in_chr);
      else
        {
          num_replacements++;
          out_str.append (in_chr, inv_utf8 - in_chr);
          in_chr = inv_utf8 + 1;

          if (type == U8_REPLACEMENT_CHAR)
            out_str.append ("\xef\xbf\xbd");
          else if (type == U8_ISO_8859_1)
            {
              std::string fallback = "iso-8859-1";
              size_t lengthp;
              uint8_t *val_utf8 = octave_u8_conv_from_encoding
                                    (fallback.c_str (), inv_utf8, 1, &lengthp);

              if (! val_utf8)
                (*current_liboctave_error_handler)
                  ("%s: converting from codepage '%s' to UTF-8 failed: %s",
                   who.c_str (), fallback.c_str (), std::strerror (errno));

              octave::unwind_action free_val_utf8
                ([] (const auto val_utf8_ptr)
                 {
                   ::free (val_utf8_ptr);
                 }, static_cast<void *> (val_utf8));

              out_str.append (reinterpret_cast<const char *> (val_utf8),
                              lengthp);
            }
        }
    }

  in_str = out_str;
  return num_replacements;
}

template <typename T>
std::string
rational_approx (T val, int len)
{
  std::string s;

  if (len <= 0)
    len = 10;

  if (octave::math::isinf (val))
    {
      if (val > 0)
        s = "1/0";
      else
        s = "-1/0";
    }
  else if (octave::math::isnan (val))
    s = "0/0";
  else if (val < std::numeric_limits<int>::min ()
           || val > std::numeric_limits<int>::max ()
           || octave::math::x_nint (val) == val)
    {
      std::ostringstream buf;
      buf.flags (std::ios::fixed);
      buf << std::setprecision (0) << octave::math::round (val);
      s = buf.str ();
    }
  else
    {
      T lastn = 1;
      T lastd = 0;
      T n = octave::math::round (val);
      T d = 1;
      T frac = val - n;
      int m = 0;

      std::ostringstream init_buf;
      init_buf.flags (std::ios::fixed);
      init_buf << std::setprecision (0) << static_cast<int> (n);
      s = init_buf.str ();

      while (true)
        {
          T flip = 1 / frac;
          T step = octave::math::round (flip);
          T nextn = n;
          T nextd = d;

          // Have we converged to 1/intmax ?
          if (std::abs (flip) > static_cast<T> (std::numeric_limits<int>::max ()))
            {
              lastn = n;
              lastd = d;
              break;
            }

          frac = flip - step;
          n = step * n + lastn;
          d = step * d + lastd;
          lastn = nextn;
          lastd = nextd;

          std::ostringstream buf;
          buf.flags (std::ios::fixed);
          buf << std::setprecision (0) << static_cast<int> (n)
              << '/' << static_cast<int> (d);
          m++;

          if (n < 0 && d < 0)
            {
              // Double negative, string can be two characters longer.
              if (buf.str ().length () > static_cast<unsigned int> (len + 2))
                break;
            }
          else
            {
              if (buf.str ().length () > static_cast<unsigned int> (len))
                break;
            }

          if (std::abs (n) > std::numeric_limits<int>::max ()
              || std::abs (d) > std::numeric_limits<int>::max ())
            break;

          s = buf.str ();
        }

      if (lastd < 0)
        {
          // Move negative sign from denominator to numerator
          lastd = - lastd;
          lastn = - lastn;
          std::ostringstream buf;
          buf.flags (std::ios::fixed);
          buf << std::setprecision (0) << static_cast<int> (lastn)
              << '/' << static_cast<int> (lastd);
          s = buf.str ();
        }
    }

  return s;
}

// instantiate the template for float and double
template std::string rational_approx <float> (float val, int len);
template std::string rational_approx <double> (double val, int len);
