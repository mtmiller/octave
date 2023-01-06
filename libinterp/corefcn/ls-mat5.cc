////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2023 The Octave Project Developers
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

#include <cstring>

#include <iomanip>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "byte-swap.h"
#include "dMatrix.h"
#include "data-conv.h"
#include "file-ops.h"
#include "file-stat.h"
#include "glob-match.h"
#include "lo-mappers.h"
#include "mach-info.h"
#include "oct-env.h"
#include "oct-locbuf.h"
#include "oct-time.h"
#include "quit.h"
#include "str-vec.h"
#include "unistr-wrappers.h"

#include "Cell.h"
#include "defaults.h"
#include "defun.h"
#include "error.h"
#include "errwarn.h"
#include "interpreter-private.h"
#include "interpreter.h"
#include "load-path.h"
#include "load-save.h"
#include "ls-mat5.h"
#include "ls-utils.h"
#include "oct-map.h"
#include "ov-cell.h"
#include "ov-class.h"
#include "ov.h"
#include "ovl.h"
#include "pager.h"
#include "parse.h"
#include "pt-eval.h"
#include "stack-frame.h"
#include "sysdep.h"
#include "unwind-prot.h"
#include "utils.h"
#include "variables.h"
#include "version.h"

#if defined (HAVE_ZLIB)
#  include <zlib.h>
#endif

#define READ_PAD(is_small_data_element, l) ((is_small_data_element) ? 4 : (((l)+7)/8)*8)
#define PAD(l) (((l) > 0 && (l) <= 4) ? 4 : (((l)+7)/8)*8)
#define INT8(l) ((l) == miINT8 || (l) == miUINT8 || (l) == miUTF8)


// The subsystem data block
static octave_value subsys_ov;

// FIXME: the following enum values should be the same as the
// mxClassID values in mexproto.h, but it seems they have also changed
// over time.  What is the correct way to handle this and maintain
// backward compatibility with old MAT files?  For now, use
// "MAT_FILE_" instead of "mx" as the prefix for these names to avoid
// conflict with the mxClassID enum in mexproto.h.

enum arrayclasstype
{
  MAT_FILE_CELL_CLASS=1,              // cell array
  MAT_FILE_STRUCT_CLASS,              // structure
  MAT_FILE_OBJECT_CLASS,              // object
  MAT_FILE_CHAR_CLASS,                // character array
  MAT_FILE_SPARSE_CLASS,              // sparse array
  MAT_FILE_DOUBLE_CLASS,              // double precision array
  MAT_FILE_SINGLE_CLASS,              // single precision floating point
  MAT_FILE_INT8_CLASS,                // 8 bit signed integer
  MAT_FILE_UINT8_CLASS,               // 8 bit unsigned integer
  MAT_FILE_INT16_CLASS,               // 16 bit signed integer
  MAT_FILE_UINT16_CLASS,              // 16 bit unsigned integer
  MAT_FILE_INT32_CLASS,               // 32 bit signed integer
  MAT_FILE_UINT32_CLASS,              // 32 bit unsigned integer
  MAT_FILE_INT64_CLASS,               // 64 bit signed integer
  MAT_FILE_UINT64_CLASS,              // 64 bit unsigned integer
  MAT_FILE_FUNCTION_CLASS,            // Function handle
  MAT_FILE_WORKSPACE_CLASS            // Workspace (undocumented)
};

// Read COUNT elements of data from IS in the format specified by TYPE,
// placing the result in DATA.  If SWAP is TRUE, swap the bytes of
// each element before copying to DATA.  FLT_FMT specifies the format
// of the data if we are reading floating point numbers.

static void
read_mat5_binary_data (std::istream& is, double *data,
                       octave_idx_type  count, bool swap, mat5_data_type type,
                       octave::mach_info::float_format flt_fmt)
{

  switch (type)
    {
    case miINT8:
      read_doubles (is, data, LS_CHAR, count, swap, flt_fmt);
      break;

    case miUTF8:
    case miUINT8:
      read_doubles (is, data, LS_U_CHAR, count, swap, flt_fmt);
      break;

    case miINT16:
      read_doubles (is, data, LS_SHORT, count, swap, flt_fmt);
      break;

    case miUTF16:
    case miUINT16:
      read_doubles (is, data, LS_U_SHORT, count, swap, flt_fmt);
      break;

    case miINT32:
      read_doubles (is, data, LS_INT, count, swap, flt_fmt);
      break;

    case miUTF32:
    case miUINT32:
      read_doubles (is, data, LS_U_INT, count, swap, flt_fmt);
      break;

    case miSINGLE:
      read_doubles (is, data, LS_FLOAT, count, swap, flt_fmt);
      break;

    case miRESERVE1:
      break;

    case miDOUBLE:
      read_doubles (is, data, LS_DOUBLE, count, swap, flt_fmt);
      break;

    case miRESERVE2:
    case miRESERVE3:
      break;

    // FIXME: how are the 64-bit cases supposed to work here?
    case miINT64:
      read_doubles (is, data, LS_LONG, count, swap, flt_fmt);
      break;

    case miUINT64:
      read_doubles (is, data, LS_U_LONG, count, swap, flt_fmt);
      break;

    case miMATRIX:
    default:
      break;
    }
}

static void
read_mat5_binary_data (std::istream& is, float *data,
                       octave_idx_type  count, bool swap, mat5_data_type type,
                       octave::mach_info::float_format flt_fmt)
{

  switch (type)
    {
    case miINT8:
      read_floats (is, data, LS_CHAR, count, swap, flt_fmt);
      break;

    case miUTF8:
    case miUINT8:
      read_floats (is, data, LS_U_CHAR, count, swap, flt_fmt);
      break;

    case miINT16:
      read_floats (is, data, LS_SHORT, count, swap, flt_fmt);
      break;

    case miUTF16:
    case miUINT16:
      read_floats (is, data, LS_U_SHORT, count, swap, flt_fmt);
      break;

    case miINT32:
      read_floats (is, data, LS_INT, count, swap, flt_fmt);
      break;

    case miUTF32:
    case miUINT32:
      read_floats (is, data, LS_U_INT, count, swap, flt_fmt);
      break;

    case miSINGLE:
      read_floats (is, data, LS_FLOAT, count, swap, flt_fmt);
      break;

    case miRESERVE1:
      break;

    case miDOUBLE:
      read_floats (is, data, LS_DOUBLE, count, swap, flt_fmt);
      break;

    case miRESERVE2:
    case miRESERVE3:
      break;

    // FIXME: how are the 64-bit cases supposed to work here?
    case miINT64:
      read_floats (is, data, LS_LONG, count, swap, flt_fmt);
      break;

    case miUINT64:
      read_floats (is, data, LS_U_LONG, count, swap, flt_fmt);
      break;

    case miMATRIX:
    default:
      break;
    }
}

template <typename T>
void
read_mat5_integer_data (std::istream& is, T *m, octave_idx_type count,
                        bool swap, mat5_data_type type)
{

#define READ_INTEGER_DATA(TYPE, swap, data, size, len, stream)          \
  do                                                                    \
    {                                                                   \
      if (len > 0)                                                      \
        {                                                               \
          OCTAVE_LOCAL_BUFFER (TYPE, ptr, len);                         \
          std::streamsize n_bytes = size * static_cast<std::streamsize> (len); \
          stream.read (reinterpret_cast<char *> (ptr), n_bytes);        \
          if (swap)                                                     \
            swap_bytes< size > (ptr, len);                              \
          for (octave_idx_type i = 0; i < len; i++)                     \
            data[i] = ptr[i];                                           \
        }                                                               \
    }                                                                   \
  while (0)

  switch (type)
    {
    case miINT8:
      READ_INTEGER_DATA (int8_t, swap, m, 1, count, is);
      break;

    case miUINT8:
      READ_INTEGER_DATA (uint8_t, swap, m, 1, count, is);
      break;

    case miINT16:
      READ_INTEGER_DATA (int16_t, swap, m, 2, count, is);
      break;

    case miUINT16:
      READ_INTEGER_DATA (uint16_t, swap, m, 2, count, is);
      break;

    case miINT32:
      READ_INTEGER_DATA (int32_t, swap, m, 4, count, is);
      break;

    case miUINT32:
      READ_INTEGER_DATA (uint32_t, swap, m, 4, count, is);
      break;

    case miSINGLE:
    case miRESERVE1:
    case miDOUBLE:
    case miRESERVE2:
    case miRESERVE3:
      break;

    case miINT64:
      READ_INTEGER_DATA (int64_t, swap, m, 8, count, is);
      break;

    case miUINT64:
      READ_INTEGER_DATA (uint64_t, swap, m, 8, count, is);
      break;

    case miMATRIX:
    default:
      break;
    }

#undef READ_INTEGER_DATA

}

template void
read_mat5_integer_data (std::istream& is, octave_int8 *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

template void
read_mat5_integer_data (std::istream& is, octave_int16 *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

template void
read_mat5_integer_data (std::istream& is, octave_int32 *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

template void
read_mat5_integer_data (std::istream& is, octave_int64 *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

template void
read_mat5_integer_data (std::istream& is, octave_uint8 *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

template void
read_mat5_integer_data (std::istream& is, octave_uint16 *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

template void
read_mat5_integer_data (std::istream& is, octave_uint32 *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

template void
read_mat5_integer_data (std::istream& is, octave_uint64 *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

template void
read_mat5_integer_data (std::istream& is, int *m,
                        octave_idx_type count, bool swap,
                        mat5_data_type type);

#define OCTAVE_MAT5_INTEGER_READ(TYP)                                   \
  {                                                                     \
    TYP re (dims);                                                      \
                                                                        \
    std::streampos tmp_pos;                                             \
                                                                        \
    if (read_mat5_tag (is, swap, type, len, is_small_data_element))     \
      error ("load: reading matrix data for '%s'", retval.c_str ());    \
                                                                        \
    octave_idx_type n = re.numel ();                                    \
    tmp_pos = is.tellg ();                                              \
    read_mat5_integer_data (is, re.fortran_vec (), n, swap,             \
                            static_cast<enum mat5_data_type> (type));   \
                                                                        \
    if (! is)                                                           \
      error ("load: reading matrix data for '%s'", retval.c_str ());    \
                                                                        \
    is.seekg (tmp_pos + static_cast<std::streamoff>                     \
              (READ_PAD (is_small_data_element, len)));                 \
                                                                        \
    if (imag)                                                           \
      {                                                                 \
        /* We don't handle imag integer types, convert to an array */   \
        NDArray im (dims);                                              \
                                                                        \
        if (read_mat5_tag (is, swap, type, len, is_small_data_element)) \
          error ("load: reading matrix data for '%s'",                  \
                 retval.c_str ());                                      \
                                                                        \
        n = im.numel ();                                                \
        read_mat5_binary_data (is, im.fortran_vec (), n, swap,          \
                               static_cast<enum mat5_data_type> (type), flt_fmt); \
                                                                        \
        if (! is)                                                       \
          error ("load: reading imaginary matrix data for '%s'",        \
                 retval.c_str ());                                      \
                                                                        \
        ComplexNDArray ctmp (dims);                                     \
                                                                        \
        for (octave_idx_type i = 0; i < n; i++)                         \
          ctmp(i) = Complex (re(i).double_value (), im(i));             \
                                                                        \
        tc = ctmp;                                                      \
      }                                                                 \
    else                                                                \
      tc = re;                                                          \
  }

// Read one element tag from stream IS,
// place the type code in TYPE, the byte count in BYTES and true (false) to
// IS_SMALL_DATA_ELEMENT if the tag is 4 (8) bytes long.
// return nonzero on error
static int
read_mat5_tag (std::istream& is, bool swap, int32_t& type, int32_t& bytes,
               bool& is_small_data_element)
{
  unsigned int upper;
  int32_t temp;

  if (! is.read (reinterpret_cast<char *> (&temp), 4))
    return 1;

  if (swap)
    swap_bytes<4> (&temp);

  upper = (temp >> 16) & 0xffff;
  type = temp & 0xffff;

  if (upper)
    {
      // "compressed" format
      bytes = upper;
      is_small_data_element = true;
    }
  else
    {
      if (! is.read (reinterpret_cast<char *> (&temp), 4))
        return 1;
      if (swap)
        swap_bytes<4> (&temp);
      bytes = temp;
      is_small_data_element = false;
    }

  return 0;
}

static void
read_int (std::istream& is, bool swap, int32_t& val)
{
  is.read (reinterpret_cast<char *> (&val), 4);

  if (swap)
    swap_bytes<4> (&val);
}

// Extract one data element (scalar, matrix, string, etc.) from stream
// IS and place it in TC, returning the name of the variable.
//
// The data is expected to be in Matlab's "Version 5" .mat format,
// though not all the features of that format are supported.
//
// FILENAME is used for error messages.

std::string
read_mat5_binary_element (std::istream& is, const std::string& filename,
                          bool swap, bool& global, octave_value& tc)
{
  std::string retval;

  global = false;

  // NOTE: these are initialized here instead of closer to where they
  // are first used to avoid errors from gcc about goto crossing
  // initialization of variable.

  bool imag;
  bool isclass = false;
  bool logicalvar;
  dim_vector dims;
  enum arrayclasstype arrayclass;
  octave_idx_type nzmax;
  std::string classname;

  bool flt_fmt_is_big_endian
    = (octave::mach_info::native_float_format ()
       == octave::mach_info::flt_fmt_ieee_big_endian);

  // MAT files always use IEEE floating point
  octave::mach_info::float_format flt_fmt = octave::mach_info::flt_fmt_unknown;
  if ((flt_fmt_is_big_endian && ! swap) || (! flt_fmt_is_big_endian && swap))
    flt_fmt = octave::mach_info::flt_fmt_ieee_big_endian;
  else
    flt_fmt = octave::mach_info::flt_fmt_ieee_little_endian;

  // element type, length and small data element flag
  int32_t type = 0;
  int32_t element_length;
  bool is_small_data_element;
  if (read_mat5_tag (is, swap, type, element_length, is_small_data_element))
    return retval;                      // EOF

  octave::interpreter& interp = octave::__get_interpreter__ ();

  if (type == miCOMPRESSED)
    {
#if defined (HAVE_ZLIB)
      // If C++ allowed us direct access to the file descriptor of an
      // ifstream in a uniform way, the code below could be vastly
      // simplified, and additional copies of the data in memory
      // wouldn't be needed.

      OCTAVE_LOCAL_BUFFER (char, inbuf, element_length);
      is.read (inbuf, element_length);

      // We uncompress the first 8 bytes of the header to get the buffer length
      // This will fail with an error Z_MEM_ERROR
      uLongf destLen = 8;
      uLongf elt_len = element_length;
      OCTAVE_LOCAL_BUFFER (unsigned int, tmp, 2);
      if (uncompress2 (reinterpret_cast<Bytef *> (tmp), &destLen,
                       reinterpret_cast<Bytef *> (inbuf), &elt_len)
          == Z_MEM_ERROR)
        error ("load: error probing size of compressed data element");

      // Why should I have to initialize outbuf as I'll just overwrite!!
      if (swap)
        swap_bytes<4> (tmp, 2);

      destLen = tmp[1] + 8;
      std::string outbuf (destLen, ' ');

      // FIXME: find a way to avoid casting away const here!

      elt_len = element_length;
      int err = uncompress2 (reinterpret_cast<Bytef *>
                             (const_cast<char *> (outbuf.c_str ())),
                             &destLen, reinterpret_cast<Bytef *> (inbuf),
                             &elt_len);

      // Ignore buffer error if we have consumed all the input buffer
      // and uncompressing the data generated as many bytes of output as
      // we were expecting given the data element size that was stored
      // in the Matlab data element header.
      if (err == Z_BUF_ERROR && destLen == tmp[1] + 8
          && elt_len == static_cast<uLongf> (element_length))
        err = Z_OK;

      if (err != Z_OK)
        {
          std::string msg;
          switch (err)
            {
            case Z_STREAM_END:
              msg = "stream end";
              break;

            case Z_NEED_DICT:
              msg = "need dict";
              break;

            case Z_ERRNO:
              msg = "errno case";
              break;

            case Z_STREAM_ERROR:
              msg = "stream error";
              break;

            case Z_DATA_ERROR:
              msg = "data error";
              break;

            case Z_MEM_ERROR:
              msg = "mem error";
              break;

            case Z_BUF_ERROR:
              msg = "buf error";
              break;

            case Z_VERSION_ERROR:
              msg = "version error";
              break;
            }

          error ("load: error uncompressing data element (%s from zlib)",
                 msg.c_str ());
        }
      else
        {
          std::istringstream gz_is (outbuf);
          retval = read_mat5_binary_element (gz_is, filename,
                                             swap, global, tc);
        }

      return retval;

#else
      err_disabled_feature ("load", "compressed data elements (zlib)");
#endif
    }

  std::streampos pos;

  if (type != miMATRIX)
    {
      pos = is.tellg ();
      error ("load: invalid element type = %d", type);
    }

  if (element_length == 0)
    {
      tc = Matrix ();
      return retval;
    }

  pos = is.tellg ();

  // array flags subelement
  int32_t len;
  if (read_mat5_tag (is, swap, type, len, is_small_data_element)
      || type != miUINT32 || len != 8 || is_small_data_element)
    error ("load: invalid array flags subelement");

  int32_t flags;
  read_int (is, swap, flags);

  imag = (flags & 0x0800) != 0; // has an imaginary part?

  global = (flags & 0x0400) != 0; // global variable?

  logicalvar = (flags & 0x0200) != 0; // boolean ?

  arrayclass = static_cast<arrayclasstype> (flags & 0xff);

  int32_t tmp_nzmax;
  read_int (is, swap, tmp_nzmax);   // max number of nonzero in sparse
  nzmax = tmp_nzmax;

  // dimensions array subelement
  if (arrayclass != MAT_FILE_WORKSPACE_CLASS)
    {
      int32_t dim_len;

      if (read_mat5_tag (is, swap, type, dim_len, is_small_data_element)
          || type != miINT32)
        error ("load: invalid dimensions array subelement");

      int ndims = dim_len / 4;
      if (ndims == 1)
        {
          // R and Python can create a 1-D object which is really an Nx1 object
          dims.resize (2);
          dims(1) = 1;
        }
      else
        dims.resize (ndims);

      for (int i = 0; i < ndims; i++)
        {
          int32_t n;
          read_int (is, swap, n);
          dims(i) = n;
        }

      std::streampos tmp_pos = is.tellg ();
      is.seekg (tmp_pos + static_cast<std::streamoff>
                (READ_PAD (is_small_data_element, dim_len) - dim_len));
    }
  else
    {
      // Why did mathworks decide to not have dims for a workspace!!!
      dims.resize (2);
      dims(0) = 1;
      dims(1) = 1;
    }

  if (read_mat5_tag (is, swap, type, len, is_small_data_element)
      || ! INT8(type))
    error ("load: invalid array name subelement");

  {
    OCTAVE_LOCAL_BUFFER (char, name, len+1);

    // Structure field subelements have zero-length array name subelements.

    std::streampos tmp_pos = is.tellg ();

    if (len)
      {
        if (! is.read (name, len))
          goto data_read_error;

        is.seekg (tmp_pos + static_cast<std::streamoff>
                  (READ_PAD (is_small_data_element, len)));
      }

    name[len] = '\0';
    retval = name;
  }

  switch (arrayclass)
    {
    case MAT_FILE_CELL_CLASS:
      {
        Cell cell_array (dims);

        octave_idx_type n = cell_array.numel ();

        for (octave_idx_type i = 0; i < n; i++)
          {
            octave_value tc2;

            std::string nm
              = read_mat5_binary_element (is, filename, swap, global, tc2);

            if (! is)
              error ("load: reading cell data for '%s'", nm.c_str ());

            cell_array(i) = tc2;
          }

        tc = cell_array;
      }
      break;

    case MAT_FILE_SPARSE_CLASS:
      {
        octave_idx_type nr = dims(0);
        octave_idx_type nc = dims(1);
        SparseMatrix sm;
        SparseComplexMatrix scm;
        octave_idx_type *ridx;
        octave_idx_type *cidx;
        double *data;

        // Setup return value
        if (imag)
          {
            scm = SparseComplexMatrix (nr, nc, nzmax);
            ridx = scm.ridx ();
            cidx = scm.cidx ();
            data = nullptr;
          }
        else
          {
            sm = SparseMatrix (nr, nc, nzmax);
            ridx = sm.ridx ();
            cidx = sm.cidx ();
            data = sm.data ();
          }

        // row indices
        std::streampos tmp_pos;

        if (read_mat5_tag (is, swap, type, len, is_small_data_element))
          error ("load: reading sparse row data for '%s'", retval.c_str ());

        tmp_pos = is.tellg ();

        read_mat5_integer_data (is, ridx, nzmax, swap,
                                static_cast<enum mat5_data_type> (type));

        if (! is)
          error ("load: reading sparse row data for '%s'", retval.c_str ());

        is.seekg (tmp_pos + static_cast<std::streamoff>
                  (READ_PAD (is_small_data_element, len)));

        // col indices
        if (read_mat5_tag (is, swap, type, len, is_small_data_element))
          error ("load: reading sparse column data for '%s'",
                 retval.c_str ());

        tmp_pos = is.tellg ();

        read_mat5_integer_data (is, cidx, nc + 1, swap,
                                static_cast<enum mat5_data_type> (type));

        if (! is)
          error ("load: reading sparse column data for '%s'",
                 retval.c_str ());

        is.seekg (tmp_pos + static_cast<std::streamoff>
                  (READ_PAD (is_small_data_element, len)));

        // real data subelement
        if (read_mat5_tag (is, swap, type, len, is_small_data_element))
          error ("load: reading sparse matrix data for '%s'",
                 retval.c_str ());

        octave_idx_type nnz = cidx[nc];
        NDArray re;
        if (imag)
          {
            re = NDArray (dim_vector (nnz, 1));
            data = re.fortran_vec ();
          }

        tmp_pos = is.tellg ();
        read_mat5_binary_data (is, data, nnz, swap,
                               static_cast<enum mat5_data_type> (type),
                               flt_fmt);

        if (! is)
          error ("load: reading sparse matrix data for '%s'",
                 retval.c_str ());

        is.seekg (tmp_pos + static_cast<std::streamoff>
                  (READ_PAD (is_small_data_element, len)));

        // imaginary data subelement
        if (imag)
          {
            NDArray im (dim_vector (static_cast<int> (nnz), 1));

            if (read_mat5_tag (is, swap, type, len, is_small_data_element))
              error ("load: reading sparse matrix data for '%s'",
                     retval.c_str ());

            read_mat5_binary_data (is, im.fortran_vec (), nnz, swap,
                                   static_cast<enum mat5_data_type> (type),
                                   flt_fmt);

            if (! is)
              error ("load: reading imaginary sparse matrix data for '%s'",
                     retval.c_str ());

            for (octave_idx_type i = 0; i < nnz; i++)
              scm.xdata (i) = Complex (re (i), im (i));

            tc = scm;
          }
        else
          tc = sm;
      }
      break;

    case MAT_FILE_FUNCTION_CLASS:
      {
        octave_value tc2;
        std::string nm
          = read_mat5_binary_element (is, filename, swap, global, tc2);

        if (! is)
          goto data_read_error;

        // Octave can handle both "/" and "\" as a directory separator
        // and so can ignore the separator field of m0.  I think the
        // sentinel field is also save to ignore.
        octave_scalar_map m0 = tc2.scalar_map_value ();
        octave_scalar_map m1
          = m0.contents ("function_handle").scalar_map_value ();
        std::string ftype = m1.contents ("type").string_value ();
        std::string fname = m1.contents ("function").string_value ();
        std::string fpath = m1.contents ("file").string_value ();

        if (ftype == "simple" || ftype == "scopedfunction")
          {
            if (fpath.empty ())
              {
                octave::tree_evaluator& tw = interp.get_evaluator ();

                // We have a builtin function
                // XXX FCN_HANDLE: SIMPLE/SCOPED
                tc = tw.make_fcn_handle (fname);
              }
            else
              {
                std::string mroot = m0.contents ("matlabroot").string_value ();

                if ((fpath.length () >= mroot.length ())
                    && fpath.substr (0, mroot.length ()) == mroot
                    && octave::config::octave_exec_home () != mroot)
                  {
                    // If fpath starts with matlabroot, and matlabroot
                    // doesn't equal __octave_config_info__ ("exec_prefix")
                    // then the function points to a version of Octave
                    // or Matlab other than the running version.  In that
                    // case we replace with the same function in the
                    // running version of Octave?

                    // First check if just replacing matlabroot is enough
                    std::string str
                      = (octave::config::octave_exec_home ()
                         + fpath.substr (mroot.length ()));
                    octave::sys::file_stat fs (str);

                    if (fs.exists ())
                      {
                        std::size_t xpos
                          = str.find_last_of (octave::sys::file_ops::dir_sep_chars ());

                        std::string dir_name = str.substr (0, xpos);

                        octave_value ov_fcn
                          = octave::load_fcn_from_file (str, dir_name,
                                                        "", "", fname);

                        if (ov_fcn.is_defined ())
                          // XXX FCN_HANDLE: SIMPLE/SCOPED
                          tc = octave_value (new octave_fcn_handle (ov_fcn,
                                             fname));
                      }
                    else
                      {
                        // Next just search for it anywhere in the system path
                        std::list<std::string> names;
                        names.push_back (fname + ".oct");
                        names.push_back (fname + ".mex");
                        names.push_back (fname + ".m");

                        octave::load_path& lp = interp.get_load_path ();

                        octave::directory_path p (lp.system_path ());

                        str = octave::sys::env::make_absolute (p.find_first_of (names));

                        std::size_t xpos
                          = str.find_last_of (octave::sys::file_ops::dir_sep_chars ());

                        std::string dir_name = str.substr (0, xpos);

                        octave_value ov_fcn
                          = octave::load_fcn_from_file (str, dir_name,
                                                        "", "", fname);

                        if (ov_fcn.is_defined ())
                          // XXX FCN_HANDLE: SIMPLE/SCOPED
                          tc = octave_value (new octave_fcn_handle (ov_fcn,
                                             fname));
                        else
                          {
                            warning_with_id ("Octave:load:file-not-found",
                                             "load: can't find the file %s",
                                             fpath.c_str ());
                            goto skip_ahead;
                          }
                      }
                  }
                else
                  {
                    std::size_t xpos
                      = fpath.find_last_of (octave::sys::file_ops::dir_sep_chars ());

                    std::string dir_name = fpath.substr (0, xpos);

                    octave_value ov_fcn
                      = octave::load_fcn_from_file (fpath, dir_name,
                                                    "", "", fname);

                    if (ov_fcn.is_defined ())
                      // XXX FCN_HANDLE: SIMPLE/SCOPED
                      tc = octave_value (new octave_fcn_handle (ov_fcn, fname));
                    else
                      {
                        warning_with_id ("Octave:load:file-not-found",
                                         "load: can't find the file %s",
                                         fpath.c_str ());
                        goto skip_ahead;
                      }
                  }
              }
          }
        else if (ftype == "nested")
          {
            warning_with_id ("Octave:load:unsupported-type",
                             "load: can't load nested function");
            goto skip_ahead;
          }
        else if (ftype == "anonymous")
          {
            octave_scalar_map m2
              = m1.contents ("workspace").scalar_map_value ();
            uint32NDArray MCOS = m2.contents ("MCOS").uint32_array_value ();
            octave_idx_type off
              = static_cast<octave_idx_type> (MCOS(4).double_value ());
            m2 = subsys_ov.scalar_map_value ();
            m2 = m2.contents ("MCOS").scalar_map_value ();
            tc2 = m2.contents ("MCOS").cell_value ()(1 + off).cell_value ()(1);

            octave::stack_frame::local_vars_map local_vars;

            if (! tc2.isempty ())
              {
                m2 = tc2.scalar_map_value ();

                if (m2.nfields () > 0)
                  {
                    octave_value tmp;

                    for (auto p0 = m2.begin (); p0 != m2.end (); p0++)
                      {
                        std::string key = m2.key (p0);
                        octave_value val = m2.contents (p0);

                        local_vars[key] = val;
                      }
                  }
              }

            // Set up temporary scope to use for evaluating the text
            // that defines the anonymous function so that we don't
            // pick up values of random variables that might be in the
            // current scope.

            octave::tree_evaluator& tw = interp.get_evaluator ();
            tw.push_dummy_scope ("read_mat5_binary_element");

            octave::unwind_action act ([&tw] () { tw.pop_scope (); });

            // FIXME: If evaluation of the string gives us an anonymous
            // function handle object, then why extract the function and
            // create a new anonymous function object?  Why not just
            // attach the workspace values to the object returned by
            // eval_string?  This code is also is duplicated in
            // anon_fcn_handle::parse_anon_fcn_handle.

            int parse_status;
            octave_value anon_fcn_handle
              = interp.eval_string (fname.substr (4), true, parse_status);

            if (parse_status != 0)
              error ("load: failed to load anonymous function handle");

            octave_fcn_handle *fh = anon_fcn_handle.fcn_handle_value ();

            if (! fh)
              error ("load: failed to load anonymous function handle");

            // XXX FCN_HANDLE: ANONYMOUS
            tc = octave_value (new octave_fcn_handle (fh->fcn_val (),
                               local_vars));
          }
        else
          error ("load: invalid function handle type");
      }
      break;

    case MAT_FILE_WORKSPACE_CLASS:
      {
        octave_map m (dim_vector (1, 1));
        int n_fields = 2;
        string_vector field (n_fields);

        for (int i = 0; i < n_fields; i++)
          {
            int32_t fn_type;
            int32_t fn_len;
            if (read_mat5_tag (is, swap, fn_type, fn_len, is_small_data_element)
                || ! INT8(fn_type))
              error ("load: invalid field name subelement");

            OCTAVE_LOCAL_BUFFER (char, elname, fn_len + 1);

            std::streampos tmp_pos = is.tellg ();

            if (fn_len)
              {
                if (! is.read (elname, fn_len))
                  goto data_read_error;

                is.seekg (tmp_pos + static_cast<std::streamoff>
                          (READ_PAD (is_small_data_element, fn_len)));
              }

            elname[fn_len] = '\0';

            field(i) = elname;
          }

        std::vector<Cell> elt (n_fields);

        for (octave_idx_type i = 0; i < n_fields; i++)
          elt[i] = Cell (dims);

        octave_idx_type n = dims.numel ();

        // fields subelements
        for (octave_idx_type j = 0; j < n; j++)
          {
            for (octave_idx_type i = 0; i < n_fields; i++)
              {
                if (field(i) == "MCOS")
                  {
                    octave_value fieldtc;
                    read_mat5_binary_element (is, filename, swap, global,
                                              fieldtc);
                    if (! is)
                      goto data_read_error;

                    elt[i](j) = fieldtc;
                  }
                else
                  elt[i](j) = octave_value ();
              }
          }

        for (octave_idx_type i = 0; i < n_fields; i++)
          m.assign (field (i), elt[i]);
        tc = m;
      }
      break;

    case MAT_FILE_OBJECT_CLASS:
      {
        isclass = true;

        if (read_mat5_tag (is, swap, type, len, is_small_data_element)
            || ! INT8(type))
          error ("load: invalid class name");

        {
          OCTAVE_LOCAL_BUFFER (char, name, len+1);

          std::streampos tmp_pos = is.tellg ();

          if (len)
            {
              if (! is.read (name, len))
                goto data_read_error;

              is.seekg (tmp_pos + static_cast<std::streamoff>
                        (READ_PAD (is_small_data_element, len)));
            }

          name[len] = '\0';
          classname = name;
        }
      }
    // Fall-through

    case MAT_FILE_STRUCT_CLASS:
      {
        octave_map m (dims);
        int32_t fn_type;
        int32_t fn_len;
        int32_t field_name_length;

        // field name length subelement -- actually the maximum length
        // of a field name.  The Matlab docs promise this will always
        // be 32.  We read and use the actual value, on the theory
        // that eventually someone will recognize that's a waste of space.
        if (read_mat5_tag (is, swap, fn_type, fn_len, is_small_data_element)
            || fn_type != miINT32)
          error ("load: invalid field name length subelement");

        if (! is.read (reinterpret_cast<char *> (&field_name_length), fn_len))
          goto data_read_error;

        if (swap)
          swap_bytes<4> (&field_name_length);

        // field name subelement.  The length of this subelement tells
        // us how many fields there are.
        if (read_mat5_tag (is, swap, fn_type, fn_len, is_small_data_element)
            || ! INT8(fn_type))
          error ("load: invalid field name subelement");

        octave_idx_type n_fields = fn_len/field_name_length;

        if (n_fields > 0)
          {
            fn_len = READ_PAD (is_small_data_element, fn_len);

            OCTAVE_LOCAL_BUFFER (char, elname, fn_len);

            if (! is.read (elname, fn_len))
              goto data_read_error;

            std::vector<Cell> elt (n_fields);

            for (octave_idx_type i = 0; i < n_fields; i++)
              elt[i] = Cell (dims);

            octave_idx_type n = dims.numel ();

            // fields subelements
            for (octave_idx_type j = 0; j < n; j++)
              {
                for (octave_idx_type i = 0; i < n_fields; i++)
                  {
                    octave_value fieldtc;
                    read_mat5_binary_element (is, filename, swap, global,
                                              fieldtc);
                    elt[i](j) = fieldtc;
                  }
              }

            for (octave_idx_type i = 0; i < n_fields; i++)
              {
                const char *key = elname + i*field_name_length;

                m.assign (key, elt[i]);
              }
          }

        if (isclass)
          {
            octave::cdef_manager& cdm = interp.get_cdef_manager ();

            if (cdm.find_class (classname, false, true).ok ())
              {
                tc = m;
                warning_with_id ("Octave:load:classdef-to-struct",
                                 "load: classdef element has been converted to a struct");
              }
            else
              {
                octave_class *cls
                  = new octave_class (m, classname,
                                      std::list<std::string> ());

                if (cls->reconstruct_exemplar ())
                  {

                    if (! cls->reconstruct_parents ())
                      warning_with_id ("Octave:load:classdef-object-inheritance",
                                       "load: unable to reconstruct object inheritance");

                    tc = cls;

                    octave::load_path& lp = interp.get_load_path ();

                    if (lp.find_method (classname, "loadobj") != "")
                      {
                        try
                          {
                            octave_value_list tmp = octave::feval ("loadobj", tc, 1);

                            tc = tmp(0);
                          }
                        catch (const octave::execution_exception&)
                          {
                            goto data_read_error;
                          }
                      }
                  }
                else
                  {
                    tc = m;
                    warning_with_id ("Octave:load:classdef-to-struct",
                                     "load: element has been converted to a structure");
                  }
              }
          }
        else
          tc = m;
      }
      break;

    case MAT_FILE_INT8_CLASS:
      OCTAVE_MAT5_INTEGER_READ (int8NDArray);
      break;

    case MAT_FILE_UINT8_CLASS:
      {
        OCTAVE_MAT5_INTEGER_READ (uint8NDArray);

        // Logical variables can either be MAT_FILE_UINT8_CLASS or
        // MAT_FILE_DOUBLE_CLASS, so check if we have a logical
        // variable and convert it.

        if (logicalvar)
          {
            uint8NDArray in = tc.uint8_array_value ();
            octave_idx_type nel = in.numel ();
            boolNDArray out (dims);

            for (octave_idx_type i = 0; i < nel; i++)
              out(i) = in(i).bool_value ();

            tc = out;
          }
      }
      break;

    case MAT_FILE_INT16_CLASS:
      OCTAVE_MAT5_INTEGER_READ (int16NDArray);
      break;

    case MAT_FILE_UINT16_CLASS:
      OCTAVE_MAT5_INTEGER_READ (uint16NDArray);
      break;

    case MAT_FILE_INT32_CLASS:
      OCTAVE_MAT5_INTEGER_READ (int32NDArray);
      break;

    case MAT_FILE_UINT32_CLASS:
      OCTAVE_MAT5_INTEGER_READ (uint32NDArray);
      break;

    case MAT_FILE_INT64_CLASS:
      OCTAVE_MAT5_INTEGER_READ (int64NDArray);
      break;

    case MAT_FILE_UINT64_CLASS:
      OCTAVE_MAT5_INTEGER_READ (uint64NDArray);
      break;

    case MAT_FILE_SINGLE_CLASS:
      {
        FloatNDArray re (dims);

        // real data subelement

        std::streampos tmp_pos;

        if (read_mat5_tag (is, swap, type, len, is_small_data_element))
          error ("load: reading matrix data for '%s'", retval.c_str ());

        octave_idx_type n = re.numel ();
        tmp_pos = is.tellg ();
        read_mat5_binary_data (is, re.fortran_vec (), n, swap,
                               static_cast<enum mat5_data_type> (type),
                               flt_fmt);

        if (! is)
          error ("load: reading matrix data for '%s'", retval.c_str ());

        is.seekg (tmp_pos + static_cast<std::streamoff>
                  (READ_PAD (is_small_data_element, len)));

        if (imag)
          {
            // imaginary data subelement

            FloatNDArray im (dims);

            if (read_mat5_tag (is, swap, type, len, is_small_data_element))
              error ("load: reading matrix data for '%s'", retval.c_str ());

            n = im.numel ();
            read_mat5_binary_data (is, im.fortran_vec (), n, swap,
                                   static_cast<enum mat5_data_type> (type),
                                   flt_fmt);

            if (! is)
              error ("load: reading imaginary matrix data for '%s'",
                     retval.c_str ());

            FloatComplexNDArray ctmp (dims);

            for (octave_idx_type i = 0; i < n; i++)
              ctmp(i) = FloatComplex (re(i), im(i));

            tc = ctmp;
          }
        else
          tc = re;
      }
      break;

    case MAT_FILE_CHAR_CLASS:
    // handle as a numerical array to start with

    case MAT_FILE_DOUBLE_CLASS:
    default:
      {
        NDArray re (dims);

        // real data subelement

        std::streampos tmp_pos;

        if (read_mat5_tag (is, swap, type, len, is_small_data_element))
          error ("load: reading matrix data for '%s'", retval.c_str ());

        octave_idx_type n = re.numel ();
        tmp_pos = is.tellg ();
        read_mat5_binary_data (is, re.fortran_vec (), n, swap,
                               static_cast<enum mat5_data_type> (type),
                               flt_fmt);

        if (! is)
          error ("load: reading matrix data for '%s'", retval.c_str ());

        is.seekg (tmp_pos + static_cast<std::streamoff>
                  (READ_PAD (is_small_data_element, len)));

        if (logicalvar)
          {
            // Logical variables can either be MAT_FILE_UINT8_CLASS or
            // MAT_FILE_DOUBLE_CLASS, so check if we have a logical
            // variable and convert it.

            boolNDArray out (dims);

            for (octave_idx_type i = 0; i < n; i++)
              out (i) = static_cast<bool> (re (i));

            tc = out;
          }
        else if (imag)
          {
            // imaginary data subelement

            NDArray im (dims);

            if (read_mat5_tag (is, swap, type, len, is_small_data_element))
              error ("load: reading matrix data for '%s'", retval.c_str ());

            n = im.numel ();
            read_mat5_binary_data (is, im.fortran_vec (), n, swap,
                                   static_cast<enum mat5_data_type> (type),
                                   flt_fmt);

            if (! is)
              error ("load: reading imaginary matrix data for '%s'",
                     retval.c_str ());

            ComplexNDArray ctmp (dims);

            for (octave_idx_type i = 0; i < n; i++)
              ctmp(i) = Complex (re(i), im(i));

            tc = ctmp;
          }
        else if (arrayclass == MAT_FILE_CHAR_CLASS)
          {
            bool converted = false;
            if (re.isvector () && (type == miUTF16 || type == miUINT16))
              {
                uint16NDArray u16 = re;
                const uint16_t *u16_str
                  = reinterpret_cast<const uint16_t *> (u16.data ());

                // Convert to UTF-8.
                std::size_t n8;
                uint8_t *u8_str = octave_u16_to_u8_wrapper (u16_str,
                                  u16.numel (),
                                  nullptr, &n8);
                if (u8_str)
                  {
                    // FIXME: Is there a better way to construct a charMatrix
                    // from a non zero terminated buffer?
                    tc = charMatrix (std::string (reinterpret_cast<char *> (u8_str), n8));
                    free (u8_str);
                    converted = true;
                  }
              }
            else if (re.isvector () && (type == miUTF32 || type == miUINT32))
              {
                uint32NDArray u32 = re;
                const uint32_t *u32_str
                  = reinterpret_cast<const uint32_t *> (u32.data ());

                // Convert to UTF-8.
                std::size_t n8;
                uint8_t *u8_str = octave_u32_to_u8_wrapper (u32_str,
                                  u32.numel (),
                                  nullptr, &n8);
                if (u8_str)
                  {
                    // FIXME: Is there a better way to construct a charMatrix
                    // from a non zero terminated buffer?
                    tc = charMatrix (std::string (reinterpret_cast<char *> (u8_str), n8));
                    free (u8_str);
                    converted = true;
                  }
              }
            else if (type == miUTF8 || type == miUINT8)
              {
                // Octave's internal encoding is UTF-8.  So we should be
                // able to use this natively.
                tc = re;
                tc = tc.convert_to_str (false, true, '\'');
                converted = true;
              }

            if (! converted)
              {
                // Fall back to manually replacing non-ASCII
                // characters by "?".
                bool found_big_char = false;
                for (octave_idx_type i = 0; i < n; i++)
                  {
                    if (re(i) > 127)
                      {
                        re(i) = '?';
                        found_big_char = true;
                      }
                  }

                if (found_big_char)
                  warning_with_id ("Octave:load:unsupported-utf-char",
                                   "load: failed to convert from input to UTF-8; "
                                   "replacing non-ASCII characters with '?'");

                tc = re;
                tc = tc.convert_to_str (false, true, '\'');
              }

          }
        else
          tc = re;
      }
    }

  is.seekg (pos + static_cast<std::streamoff> (element_length));

  if (is.eof ())
    is.clear ();

  return retval;

// FIXME: With short-circuiting error(), no need for goto in code
data_read_error:
  error ("load: trouble reading binary file '%s'", filename.c_str ());

skip_ahead:
  warning_with_id ("Octave:load:skip-unsupported-element",
                   "load: skipping over '%s'", retval.c_str ());
  is.seekg (pos + static_cast<std::streamoff> (element_length));
  return read_mat5_binary_element (is, filename, swap, global, tc);
}

int
read_mat5_binary_file_header (std::istream& is, bool& swap, bool quiet,
                              const std::string& filename)
{
  int16_t version = 0;
  int16_t magic = 0;
  uint64_t subsys_offset;

  is.seekg (116, std::ios::beg);
  is.read (reinterpret_cast<char *> (&subsys_offset), 8);

  is.seekg (124, std::ios::beg);
  is.read (reinterpret_cast<char *> (&version), 2);
  is.read (reinterpret_cast<char *> (&magic), 2);

  if (magic == 0x4d49)
    swap = false;
  else if (magic == 0x494d)
    swap = true;
  else
    {
      if (! quiet)
        error ("load: can't read binary file");

      return -1;
    }

  if (! swap)                   // version number is inverse swapped!
    version = ((version >> 8) & 0xff) + ((version & 0xff) << 8);

  if (version != 1 && ! quiet)
    warning_with_id ("Octave:load:unsupported-version",
                     "load: found version %d binary MAT file, but only prepared for version 1",
                     version);

  if (swap)
    swap_bytes<8> (&subsys_offset, 1);

  if (subsys_offset != UINT64_C (0x2020202020202020)
      && subsys_offset != UINT64_C (0))
    {
      // Read the subsystem data block
      is.seekg (subsys_offset, std::ios::beg);

      octave_value tc;
      bool global;
      read_mat5_binary_element (is, filename, swap, global, tc);

      if (! is)
        return -1;

      if (tc.is_uint8_type ())
        {
          const uint8NDArray itmp = tc.uint8_array_value ();
          octave_idx_type ilen = itmp.numel ();

          // Why should I have to initialize outbuf as just overwrite
          std::string outbuf (ilen - 7, ' ');

          // FIXME: find a way to avoid casting away const here
          char *ctmp = const_cast<char *> (outbuf.c_str ());
          for (octave_idx_type j = 8; j < ilen; j++)
            ctmp[j-8] = itmp(j).char_value ();

          std::istringstream fh_ws (outbuf);

          read_mat5_binary_element (fh_ws, filename, swap, global, subsys_ov);

          if (! is)
            return -1;
        }
      else
        return -1;

      // Reposition to just after the header
      is.seekg (128, std::ios::beg);
    }

  return 0;
}

static int
write_mat5_tag (std::ostream& is, int type, octave_idx_type bytes)
{
  int32_t temp;

  if (bytes > 0 && bytes <= 4)
    temp = (bytes << 16) + type;
  else
    {
      temp = type;
      if (! is.write (reinterpret_cast<char *> (&temp), 4))
        return 1;
      temp = bytes;
    }

  if (! is.write (reinterpret_cast<char *> (&temp), 4))
    return 1;

  return 0;
}

// Have to use copy here to avoid writing over data accessed via
// Matrix::data().

#define MAT5_DO_WRITE(TYPE, data, count, stream)                        \
  do                                                                    \
    {                                                                   \
      OCTAVE_LOCAL_BUFFER (TYPE, ptr, count);                           \
      for (octave_idx_type i = 0; i < count; i++)                       \
        ptr[i] = static_cast<TYPE> (data[i]);                           \
      std::streamsize n_bytes = sizeof (TYPE) * static_cast<std::streamsize> (count); \
      stream.write (reinterpret_cast<char *> (ptr), n_bytes);           \
    }                                                                   \
  while (0)

// write out the numeric values in M to OS,
// preceded by the appropriate tag.
static void
write_mat5_array (std::ostream& os, const NDArray& m, bool save_as_floats)
{
  save_type st = LS_DOUBLE;
  const double *data = m.data ();

  if (save_as_floats)
    {
      if (m.too_large_for_float ())
        {
          warning_with_id ("Octave:save:too-large-for-float",
                           "save: some values too large to save as floats -- saving as doubles instead");
        }
      else
        st = LS_FLOAT;
    }

  double max_val, min_val;
  if (m.all_integers (max_val, min_val))
    st = octave::get_save_type (max_val, min_val);

  mat5_data_type mst;
  int size;
  switch (st)
    {
    default:
    case LS_DOUBLE:  mst = miDOUBLE; size = 8; break;
    case LS_FLOAT:   mst = miSINGLE; size = 4; break;
    case LS_U_CHAR:  mst = miUINT8;  size = 1; break;
    case LS_U_SHORT: mst = miUINT16; size = 2; break;
    case LS_U_INT:   mst = miUINT32; size = 4; break;
    case LS_CHAR:    mst = miINT8;   size = 1; break;
    case LS_SHORT:   mst = miINT16;  size = 2; break;
    case LS_INT:     mst = miINT32;  size = 4; break;
    }

  octave_idx_type nel = m.numel ();
  octave_idx_type len = nel*size;

  write_mat5_tag (os, mst, len);

  {
    switch (st)
      {
      case LS_U_CHAR:
        MAT5_DO_WRITE (uint8_t, data, nel, os);
        break;

      case LS_U_SHORT:
        MAT5_DO_WRITE (uint16_t, data, nel, os);
        break;

      case LS_U_INT:
        MAT5_DO_WRITE (uint32_t, data, nel, os);
        break;

      case LS_U_LONG:
        MAT5_DO_WRITE (uint64_t, data, nel, os);
        break;

      case LS_CHAR:
        MAT5_DO_WRITE (int8_t, data, nel, os);
        break;

      case LS_SHORT:
        MAT5_DO_WRITE (int16_t, data, nel, os);
        break;

      case LS_INT:
        MAT5_DO_WRITE (int32_t, data, nel, os);
        break;

      case LS_LONG:
        MAT5_DO_WRITE (int64_t, data, nel, os);
        break;

      case LS_FLOAT:
        MAT5_DO_WRITE (float, data, nel, os);
        break;

      case LS_DOUBLE: // No conversion necessary.
        os.write (reinterpret_cast<const char *> (data), len);
        break;

      default:
        error ("unrecognized data format requested");
        break;
      }
  }
  if (PAD (len) > len)
    {
      static char buf[9]="\x00\x00\x00\x00\x00\x00\x00\x00";
      os.write (buf, PAD (len) - len);
    }
}

static void
write_mat5_array (std::ostream& os, const FloatNDArray& m, bool)
{
  save_type st = LS_FLOAT;
  const float *data = m.data ();

  float max_val, min_val;
  if (m.all_integers (max_val, min_val))
    st = octave::get_save_type (max_val, min_val);

  mat5_data_type mst;
  int size;
  switch (st)
    {
    default:
    case LS_DOUBLE:  mst = miDOUBLE; size = 8; break;
    case LS_FLOAT:   mst = miSINGLE; size = 4; break;
    case LS_U_CHAR:  mst = miUINT8;  size = 1; break;
    case LS_U_SHORT: mst = miUINT16; size = 2; break;
    case LS_U_INT:   mst = miUINT32; size = 4; break;
    case LS_CHAR:    mst = miINT8;   size = 1; break;
    case LS_SHORT:   mst = miINT16;  size = 2; break;
    case LS_INT:     mst = miINT32;  size = 4; break;
    }

  octave_idx_type nel = m.numel ();
  octave_idx_type len = nel*size;

  write_mat5_tag (os, mst, len);

  {
    switch (st)
      {
      case LS_U_CHAR:
        MAT5_DO_WRITE (uint8_t, data, nel, os);
        break;

      case LS_U_SHORT:
        MAT5_DO_WRITE (uint16_t, data, nel, os);
        break;

      case LS_U_INT:
        MAT5_DO_WRITE (uint32_t, data, nel, os);
        break;

      case LS_U_LONG:
        MAT5_DO_WRITE (uint64_t, data, nel, os);
        break;

      case LS_CHAR:
        MAT5_DO_WRITE (int8_t, data, nel, os);
        break;

      case LS_SHORT:
        MAT5_DO_WRITE (int16_t, data, nel, os);
        break;

      case LS_INT:
        MAT5_DO_WRITE (int32_t, data, nel, os);
        break;

      case LS_LONG:
        MAT5_DO_WRITE (int64_t, data, nel, os);
        break;

      case LS_FLOAT: // No conversion necessary.
        os.write (reinterpret_cast<const char *> (data), len);
        break;

      case LS_DOUBLE:
        MAT5_DO_WRITE (double, data, nel, os);
        break;

      default:
        error ("unrecognized data format requested");
        break;
      }
  }
  if (PAD (len) > len)
    {
      static char buf[9]="\x00\x00\x00\x00\x00\x00\x00\x00";
      os.write (buf, PAD (len) - len);
    }
}

template <typename T>
void
write_mat5_integer_data (std::ostream& os, const T *m, int size,
                         octave_idx_type nel)
{
  mat5_data_type mst;
  unsigned len;

  switch (size)
    {
    case 1:
      mst = miUINT8;
      break;
    case 2:
      mst = miUINT16;
      break;
    case 4:
      mst = miUINT32;
      break;
    case 8:
      mst = miUINT64;
      break;
    case -1:
      mst = miINT8;
      size = - size;
      break;
    case -2:
      mst = miINT16;
      size = - size;
      break;
    case -4:
      mst = miINT32;
      size = - size;
      break;
    case -8:
    default:
      mst = miINT64;
      size = - size;
      break;
    }

  len = nel*size;
  write_mat5_tag (os, mst, len);

  os.write (reinterpret_cast<const char *> (m), len);

  if (PAD (len) > len)
    {
      static char buf[9]="\x00\x00\x00\x00\x00\x00\x00\x00";
      os.write (buf, PAD (len) - len);
    }
}

template void
write_mat5_integer_data (std::ostream& os, const octave_int8 *m,
                         int size, octave_idx_type nel);

template void
write_mat5_integer_data (std::ostream& os, const octave_int16 *m,
                         int size, octave_idx_type nel);

template void
write_mat5_integer_data (std::ostream& os, const octave_int32 *m,
                         int size, octave_idx_type nel);

template void
write_mat5_integer_data (std::ostream& os, const octave_int64 *m,
                         int size, octave_idx_type nel);

template void
write_mat5_integer_data (std::ostream& os, const octave_uint8 *m,
                         int size, octave_idx_type nel);

template void
write_mat5_integer_data (std::ostream& os, const octave_uint16 *m,
                         int size, octave_idx_type nel);

template void
write_mat5_integer_data (std::ostream& os, const octave_uint32 *m,
                         int size, octave_idx_type nel);

template void
write_mat5_integer_data (std::ostream& os, const octave_uint64 *m,
                         int size, octave_idx_type nel);

template void
write_mat5_integer_data (std::ostream& os, const int *m,
                         int size, octave_idx_type nel);

// Write out cell element values in the cell array to OS, preceded by
// the appropriate tag.

static bool
write_mat5_cell_array (std::ostream& os, const Cell& cell,
                       bool mark_global, bool save_as_floats)
{
  octave_idx_type nel = cell.numel ();

  for (octave_idx_type i = 0; i < nel; i++)
    {
      octave_value ov = cell(i);

      if (! save_mat5_binary_element (os, ov, "", mark_global,
                                      false, save_as_floats))
        return false;
    }

  return true;
}

int
save_mat5_array_length (const double *val, octave_idx_type nel,
                        bool save_as_floats)
{
  if (nel > 0)
    {
      int size = 8;

      if (save_as_floats)
        {
          bool too_large_for_float = false;
          for (octave_idx_type i = 0; i < nel; i++)
            {
              double tmp = val[i];

              if (octave::math::isfinite (tmp)
                  && fabs (tmp) > std::numeric_limits<float>::max ())
                {
                  too_large_for_float = true;
                  break;
                }
            }

          if (! too_large_for_float)
            size = 4;
        }

      // The code below is disabled since get_save_type currently
      // doesn't deal with integer types.  This will need to be
      // activated if get_save_type is changed.

      // double max_val = val[0];
      // double min_val = val[0];
      // bool all_integers = true;
      //
      // for (int i = 0; i < nel; i++)
      //   {
      //     double val = val[i];
      //
      //     if (val > max_val)
      //       max_val = val;
      //
      //     if (val < min_val)
      //       min_val = val;
      //
      //     if (octave::math::x_nint (val) != val)
      //       {
      //         all_integers = false;
      //         break;
      //       }
      //   }
      //
      // if (all_integers)
      //   {
      //     if (max_val < 256 && min_val > -1)
      //       size = 1;
      //     else if (max_val < 65536 && min_val > -1)
      //       size = 2;
      //     else if (max_val < 4294967295UL && min_val > -1)
      //       size = 4;
      //     else if (max_val < 128 && min_val >= -128)
      //       size = 1;
      //     else if (max_val < 32768 && min_val >= -32768)
      //       size = 2;
      //     else if (max_val <= 2147483647L && min_val >= -2147483647L)
      //       size = 4;
      //   }

      return 8 + nel * size;
    }
  else
    return 8;
}

int
save_mat5_array_length (const float * /* val */, octave_idx_type nel, bool)
{
  if (nel > 0)
    {
      int size = 4;

      // The code below is disabled since get_save_type currently
      // doesn't deal with integer types.  This will need to be
      // activated if get_save_type is changed.

      // float max_val = val[0];
      // float min_val = val[0];
      // bool all_integers = true;
      //
      // for (int i = 0; i < nel; i++)
      //   {
      //     float val = val[i];
      //
      //     if (val > max_val)
      //       max_val = val;
      //
      //     if (val < min_val)
      //       min_val = val;
      //
      //     if (octave::math::x_nint (val) != val)
      //       {
      //         all_integers = false;
      //         break;
      //       }
      //   }
      //
      // if (all_integers)
      //   {
      //     if (max_val < 256 && min_val > -1)
      //       size = 1;
      //     else if (max_val < 65536 && min_val > -1)
      //       size = 2;
      //     else if (max_val < 4294967295UL && min_val > -1)
      //       size = 4;
      //     else if (max_val < 128 && min_val >= -128)
      //       size = 1;
      //     else if (max_val < 32768 && min_val >= -32768)
      //       size = 2;
      //     else if (max_val <= 2147483647L && min_val >= -2147483647L)
      //       size = 4;
      //

      // Round nel up to nearest even number of elements.
      // Take into account short tags for 4 byte elements.
      return PAD ((nel * size <= 4 ? 4 : 8) + nel * size);
    }
  else
    return 8;
}

int
save_mat5_array_length (const Complex *val, octave_idx_type nel,
                        bool save_as_floats)
{
  int ret;

  OCTAVE_LOCAL_BUFFER (double, tmp, nel);

  for (octave_idx_type i = 1; i < nel; i++)
    tmp[i] = std::real (val[i]);

  ret = save_mat5_array_length (tmp, nel, save_as_floats);

  for (octave_idx_type i = 1; i < nel; i++)
    tmp[i] = std::imag (val[i]);

  ret += save_mat5_array_length (tmp, nel, save_as_floats);

  return ret;
}

int
save_mat5_array_length (const FloatComplex *val, octave_idx_type nel,
                        bool save_as_floats)
{
  int ret;

  OCTAVE_LOCAL_BUFFER (float, tmp, nel);

  for (octave_idx_type i = 1; i < nel; i++)
    tmp[i] = std::real (val[i]);

  ret = save_mat5_array_length (tmp, nel, save_as_floats);

  for (octave_idx_type i = 1; i < nel; i++)
    tmp[i] = std::imag (val[i]);

  ret += save_mat5_array_length (tmp, nel, save_as_floats);

  return ret;
}

static uint16_t *
maybe_convert_to_u16 (const charNDArray& chm, std::size_t& n16_str)
{
  uint16_t *u16_str;
  dim_vector dv = chm.dims ();

  if (chm.ndims () == 2 && dv(0) == 1)
    {
      const uint8_t *u8_str = reinterpret_cast<const uint8_t *> (chm.data ());
      u16_str = octave_u8_to_u16_wrapper (u8_str, chm.numel (),
                                          nullptr, &n16_str);
    }
  else
    u16_str = nullptr;

  return u16_str;
}

int
save_mat5_element_length (const octave_value& tc, const std::string& name,
                          bool save_as_floats, bool mat7_format)
{
  std::size_t max_namelen = 63;
  std::size_t len = name.length ();
  std::string cname = tc.class_name ();
  int ret = 32;

  if (len > 4)
    ret += PAD (len > max_namelen ? max_namelen : len);

  ret += PAD (4 * tc.ndims ());

  if (tc.is_string ())
    {
      charNDArray chm = tc.char_array_value ();
      // convert to UTF-16
      std::size_t n16_str;
      uint16_t *u16_str = maybe_convert_to_u16 (chm, n16_str);
      ret += 8;

      octave_idx_type str_len;
      std::size_t sz_of = 1;
      if (u16_str)
        {
          free (u16_str);
          // Count number of elements in converted string
          str_len = n16_str;
          sz_of = 2;
        }
      else
        str_len = chm.numel ();

      if (str_len > 2)
        ret += PAD (sz_of * str_len);
    }
  else if (tc.issparse ())
    {
      if (tc.iscomplex ())
        {
          const SparseComplexMatrix m = tc.sparse_complex_matrix_value ();
          octave_idx_type nc = m.cols ();
          octave_idx_type nnz = m.nnz ();

          ret += 16 + save_mat5_array_length (m.data (), nnz, save_as_floats);
          if (nnz > 1)
            ret += PAD (nnz * sizeof (int32_t));
          if (nc > 0)
            ret += PAD ((nc + 1) * sizeof (int32_t));
        }
      else
        {
          const SparseMatrix m = tc.sparse_matrix_value ();
          octave_idx_type nc = m.cols ();
          octave_idx_type nnz = m.nnz ();

          ret += 16 + save_mat5_array_length (m.data (), nnz, save_as_floats);
          if (nnz > 1)
            ret += PAD (nnz * sizeof (int32_t));
          if (nc > 0)
            ret += PAD ((nc + 1) * sizeof (int32_t));
        }
    }

#define INT_LEN(nel, size)                      \
  {                                             \
    ret += 8;                                   \
    octave_idx_type sz = nel * size;            \
    if (sz > 4)                                 \
      ret += PAD (sz);                          \
  }

  else if (cname == "int8")
    INT_LEN (tc.int8_array_value ().numel (), 1)
    else if (cname == "int16")
      INT_LEN (tc.int16_array_value ().numel (), 2)
      else if (cname == "int32")
        INT_LEN (tc.int32_array_value ().numel (), 4)
        else if (cname == "int64")
          INT_LEN (tc.int64_array_value ().numel (), 8)
          else if (cname == "uint8")
            INT_LEN (tc.uint8_array_value ().numel (), 1)
            else if (cname == "uint16")
              INT_LEN (tc.uint16_array_value ().numel (), 2)
              else if (cname == "uint32")
                INT_LEN (tc.uint32_array_value ().numel (), 4)
                else if (cname == "uint64")
                  INT_LEN (tc.uint64_array_value ().numel (), 8)
                  else if (tc.islogical ())
                    INT_LEN (tc.bool_array_value ().numel (), 1)
                    else if (tc.is_real_scalar () || tc.is_real_matrix () || tc.is_range ())
                      {
                        if (tc.is_single_type ())
                          {
                            const FloatNDArray m = tc.float_array_value ();
                            ret += save_mat5_array_length (m.data (), m.numel (), save_as_floats);
                          }
                        else
                          {
                            const NDArray m = tc.array_value ();
                            ret += save_mat5_array_length (m.data (), m.numel (), save_as_floats);
                          }
                      }
                    else if (tc.iscell ())
                      {
                        Cell cell = tc.cell_value ();
                        octave_idx_type nel = cell.numel ();

                        for (int i = 0; i < nel; i++)
                          ret += 8 +
                                 save_mat5_element_length (cell (i), "", save_as_floats, mat7_format);
                      }
                    else if (tc.is_complex_scalar () || tc.is_complex_matrix ())
                      {
                        if (tc.is_single_type ())
                          {
                            const FloatComplexNDArray m = tc.float_complex_array_value ();
                            ret += save_mat5_array_length (m.data (), m.numel (), save_as_floats);
                          }
                        else
                          {
                            const ComplexNDArray m = tc.complex_array_value ();
                            ret += save_mat5_array_length (m.data (), m.numel (), save_as_floats);
                          }
                      }
                    else if (tc.isstruct () || tc.is_inline_function () || tc.isobject ())
                      {
                        int fieldcnt = 0;
                        const octave_map m = tc.map_value ();
                        octave_idx_type nel = m.numel ();

                        if (tc.is_inline_function ())
                          ret += 8 + PAD (6);  // length of "inline" is 6
                        else if (tc.isobject ())
                          {
                            std::size_t classlen = tc.class_name ().length ();

                            ret += 8 + PAD (classlen > max_namelen ? max_namelen : classlen);
                          }

                        for (auto i = m.begin (); i != m.end (); i++)
                          fieldcnt++;

                        ret += 16 + fieldcnt * (max_namelen + 1);

                        for (octave_idx_type j = 0; j < nel; j++)
                          {
                            for (auto i = m.begin (); i != m.end (); i++)
                              {
                                const Cell elts = m.contents (i);

                                ret += 8 + save_mat5_element_length (elts(j), "", save_as_floats,
                                                                     mat7_format);
                              }
                          }
                      }
                    else
                      ret = -1;

  return ret;
}

static void
write_mat5_sparse_index_vector (std::ostream& os,
                                const octave_idx_type *idx,
                                octave_idx_type nel)
{
  int tmp = sizeof (int32_t);

  OCTAVE_LOCAL_BUFFER (int32_t, tmp_idx, nel);

  for (octave_idx_type i = 0; i < nel; i++)
    tmp_idx[i] = idx[i];

  write_mat5_integer_data (os, tmp_idx, -tmp, nel);
}

static void
warn_dim_too_large (const std::string& name)
{
  warning_with_id ("Octave:save:dimension-too-large",
                   "save: skipping %s: dimension too large for MAT format",
                   name.c_str ());
}

// save the data from TC along with the corresponding NAME on stream
// OS in the MatLab version 5 binary format.  Return true on success.

bool
save_mat5_binary_element (std::ostream& os,
                          const octave_value& tc, const std::string& name,
                          bool mark_global, bool mat7_format,
                          bool save_as_floats, bool compressing)
{
  int32_t flags = 0;
  int32_t nnz_32 = 0;
  std::string cname = tc.class_name ();
  std::size_t max_namelen = 63;

  dim_vector dv = tc.dims ();
  int nd = tc.ndims ();
  int dim_len = 4*nd;

  static octave_idx_type max_dim_val = std::numeric_limits<int32_t>::max ();

  for (int i = 0; i < nd; i++)
    {
      if (dv(i) > max_dim_val)
        {
          warn_dim_too_large (name);
          return true;  // skip to next
        }
    }

  if (tc.issparse ())
    {
      octave_idx_type nnz;
      octave_idx_type nc;

      if (tc.iscomplex ())
        {
          SparseComplexMatrix scm = tc.sparse_complex_matrix_value ();
          nnz = scm.nzmax ();
          nc = scm.cols ();
        }
      else
        {
          SparseMatrix sm = tc.sparse_matrix_value ();
          nnz = sm.nzmax ();
          nc = sm.cols ();
        }

      if (nnz > max_dim_val || nc + 1 > max_dim_val)
        {
          warn_dim_too_large (name);
          return true;  // skip to next
        }

      nnz_32 = nnz;
    }
  else if (dv.numel () > max_dim_val)
    {
      warn_dim_too_large (name);
      return true;  // skip to next
    }

#if defined (HAVE_ZLIB)

  if (mat7_format && ! compressing)
    {
      bool ret = false;

      std::ostringstream buf;

      // The code seeks backwards in the stream to fix the header.
      // Can't do this with zlib, so use a stringstream.
      ret = save_mat5_binary_element (buf, tc, name, mark_global, true,
                                      save_as_floats, true);

      if (ret)
        {
          // destLen must be at least 0.1% larger than source buffer
          // + 12 bytes.  Reality is it must be larger again than that.
          std::string buf_str = buf.str ();
          uLongf srcLen = buf_str.length ();
          uLongf destLen = compressBound (srcLen);
          OCTAVE_LOCAL_BUFFER (char, out_buf, destLen);

          if (compress (reinterpret_cast<Bytef *> (out_buf), &destLen,
                        reinterpret_cast<const Bytef *> (buf_str.c_str ()),
                        srcLen)
              != Z_OK)
            error ("save: error compressing data element");

          write_mat5_tag (os, miCOMPRESSED,
                          static_cast<octave_idx_type> (destLen));

          os.write (out_buf, destLen);
        }

      return ret;
    }

#else

  octave_unused_parameter (compressing);

#endif

  write_mat5_tag (os, miMATRIX, save_mat5_element_length
                  (tc, name, save_as_floats, mat7_format));

  // array flags subelement
  write_mat5_tag (os, miUINT32, 8);

  if (tc.islogical ())
    flags |= 0x0200;

  if (mark_global)
    flags |= 0x0400;

  if (tc.is_complex_scalar () || tc.is_complex_matrix ())
    flags |= 0x0800;

  if (tc.is_string ())
    flags |= MAT_FILE_CHAR_CLASS;
  else if (cname == "int8")
    flags |= MAT_FILE_INT8_CLASS;
  else if (cname == "int16")
    flags |= MAT_FILE_INT16_CLASS;
  else if (cname == "int32")
    flags |= MAT_FILE_INT32_CLASS;
  else if (cname == "int64")
    flags |= MAT_FILE_INT64_CLASS;
  else if (cname == "uint8" || tc.islogical ())
    flags |= MAT_FILE_UINT8_CLASS;
  else if (cname == "uint16")
    flags |= MAT_FILE_UINT16_CLASS;
  else if (cname == "uint32")
    flags |= MAT_FILE_UINT32_CLASS;
  else if (cname == "uint64")
    flags |= MAT_FILE_UINT64_CLASS;
  else if (tc.issparse ())
    flags |= MAT_FILE_SPARSE_CLASS;
  else if (tc.is_real_scalar () || tc.is_real_matrix () || tc.is_range ()
           || tc.is_complex_scalar () || tc.is_complex_matrix ())
    {
      if (tc.is_single_type ())
        flags |= MAT_FILE_SINGLE_CLASS;
      else
        flags |= MAT_FILE_DOUBLE_CLASS;
    }
  else if (tc.isstruct ())
    flags |= MAT_FILE_STRUCT_CLASS;
  else if (tc.iscell ())
    flags |= MAT_FILE_CELL_CLASS;
  else if (tc.is_inline_function () || tc.isobject ())
    flags |= MAT_FILE_OBJECT_CLASS;
  else
    {
      // FIXME: Should this just error out rather than warn?
      warn_wrong_type_arg ("save", tc);
      error ("save: error while writing '%s' to MAT file", name.c_str ());
    }

  os.write (reinterpret_cast<char *> (&flags), 4);
  // Matlab seems to have trouble reading files that have nzmax == 0 at
  // this point in the file.
  if (nnz_32 == 0)
    nnz_32 = 1;
  os.write (reinterpret_cast<char *> (&nnz_32), 4);

  write_mat5_tag (os, miINT32, dim_len);

  // Strings need to be converted here (or dim-vector will be off).
  charNDArray chm;
  uint16_t *u16_str;
  std::size_t n16_str;
  bool conv_u16 = false;
  if (tc.is_string ())
    {
      chm = tc.char_array_value ();
      u16_str = maybe_convert_to_u16 (chm, n16_str);

      if (u16_str)
        conv_u16 = true;
    }

  if (conv_u16)
    {
      int32_t n = 1;
      os.write (reinterpret_cast<char *> (&n), 4);
      os.write (reinterpret_cast<char *> (&n16_str), 4);
    }
  else
    for (int i = 0; i < nd; i++)
      {
        int32_t n = dv(i);
        os.write (reinterpret_cast<char *> (&n), 4);
      }

  if (PAD (dim_len) > dim_len)
    {
      static char buf[9] = "\x00\x00\x00\x00\x00\x00\x00\x00";
      os.write (buf, PAD (dim_len) - dim_len);
    }

  // array name subelement
  {
    std::size_t namelen = name.length ();

    if (namelen > max_namelen)
      namelen = max_namelen;  // Truncate names if necessary

    int paddedlength = PAD (namelen);

    write_mat5_tag (os, miINT8, namelen);
    OCTAVE_LOCAL_BUFFER (char, paddedname, paddedlength);
    memset (paddedname, 0, paddedlength);
    strncpy (paddedname, name.c_str (), namelen);
    os.write (paddedname, paddedlength);
  }

  // data element
  if (tc.is_string ())
    {
      octave_idx_type len;
      octave_idx_type paddedlength;

      if (conv_u16)
        {
          // converted UTF-16
          len = n16_str*2;
          paddedlength = PAD (len);

          write_mat5_tag (os, miUTF16, len);

          os.write (reinterpret_cast<char *> (u16_str), len);

          free (u16_str);
        }
      else
        {
          // write as UTF-8
          len = chm.numel ();
          paddedlength = PAD (len);

          write_mat5_tag (os, miUTF8, len);

          os.write (chm.data (), len);
        }

      if (paddedlength > len)
        {
          static char padbuf[9] = "\x00\x00\x00\x00\x00\x00\x00\x00";
          os.write (padbuf, paddedlength - len);
        }
    }
  else if (tc.issparse ())
    {
      if (tc.iscomplex ())
        {
          const SparseComplexMatrix m = tc.sparse_complex_matrix_value ();
          octave_idx_type nnz = m.nnz ();
          octave_idx_type nc = m.cols ();

          write_mat5_sparse_index_vector (os, m.ridx (), nnz);
          write_mat5_sparse_index_vector (os, m.cidx (), nc + 1);

          NDArray buf (dim_vector (nnz, 1));

          for (octave_idx_type i = 0; i < nnz; i++)
            buf (i) = std::real (m.data (i));

          write_mat5_array (os, buf, save_as_floats);

          for (octave_idx_type i = 0; i < nnz; i++)
            buf (i) = std::imag (m.data (i));

          write_mat5_array (os, buf, save_as_floats);
        }
      else
        {
          const SparseMatrix m = tc.sparse_matrix_value ();
          octave_idx_type nnz = m.nnz ();
          octave_idx_type nc = m.cols ();

          write_mat5_sparse_index_vector (os, m.ridx (), nnz);
          write_mat5_sparse_index_vector (os, m.cidx (), nc + 1);

          // FIXME
          // Is there a way to easily do without this buffer
          NDArray buf (dim_vector (nnz, 1));

          for (int i = 0; i < nnz; i++)
            buf (i) = m.data (i);

          write_mat5_array (os, buf, save_as_floats);
        }
    }
  else if (cname == "int8")
    {
      int8NDArray m = tc.int8_array_value ();

      write_mat5_integer_data (os, m.data (), -1, m.numel ());
    }
  else if (cname == "int16")
    {
      int16NDArray m = tc.int16_array_value ();

      write_mat5_integer_data (os, m.data (), -2, m.numel ());
    }
  else if (cname == "int32")
    {
      int32NDArray m = tc.int32_array_value ();

      write_mat5_integer_data (os, m.data (), -4, m.numel ());
    }
  else if (cname == "int64")
    {
      int64NDArray m = tc.int64_array_value ();

      write_mat5_integer_data (os, m.data (), -8, m.numel ());
    }
  else if (cname == "uint8")
    {
      uint8NDArray m = tc.uint8_array_value ();

      write_mat5_integer_data (os, m.data (), 1, m.numel ());
    }
  else if (cname == "uint16")
    {
      uint16NDArray m = tc.uint16_array_value ();

      write_mat5_integer_data (os, m.data (), 2, m.numel ());
    }
  else if (cname == "uint32")
    {
      uint32NDArray m = tc.uint32_array_value ();

      write_mat5_integer_data (os, m.data (), 4, m.numel ());
    }
  else if (cname == "uint64")
    {
      uint64NDArray m = tc.uint64_array_value ();

      write_mat5_integer_data (os, m.data (), 8, m.numel ());
    }
  else if (tc.islogical ())
    {
      uint8NDArray m (tc.bool_array_value ());

      write_mat5_integer_data (os, m.data (), 1, m.numel ());
    }
  else if (tc.is_real_scalar () || tc.is_real_matrix () || tc.is_range ())
    {
      if (tc.is_single_type ())
        {
          FloatNDArray m = tc.float_array_value ();

          write_mat5_array (os, m, save_as_floats);
        }
      else
        {
          NDArray m = tc.array_value ();

          write_mat5_array (os, m, save_as_floats);
        }
    }
  else if (tc.iscell ())
    {
      Cell cell = tc.cell_value ();

      if (! write_mat5_cell_array (os, cell, mark_global, save_as_floats))
        error ("save: error while writing '%s' to MAT file", name.c_str ());
    }
  else if (tc.is_complex_scalar () || tc.is_complex_matrix ())
    {
      if (tc.is_single_type ())
        {
          FloatComplexNDArray m_cmplx = tc.float_complex_array_value ();

          write_mat5_array (os, ::real (m_cmplx), save_as_floats);
          write_mat5_array (os, ::imag (m_cmplx), save_as_floats);
        }
      else
        {
          ComplexNDArray m_cmplx = tc.complex_array_value ();

          write_mat5_array (os, ::real (m_cmplx), save_as_floats);
          write_mat5_array (os, ::imag (m_cmplx), save_as_floats);
        }
    }
  else if (tc.isstruct () || tc.is_inline_function () || tc.isobject ())
    {
      if (tc.is_inline_function () || tc.isobject ())
        {
          std::string classname = (tc.isobject () ? tc.class_name ()
                                   : "inline");
          std::size_t namelen = classname.length ();

          if (namelen > max_namelen)
            namelen = max_namelen; // Truncate names if necessary

          int paddedlength = PAD (namelen);

          write_mat5_tag (os, miINT8, namelen);
          OCTAVE_LOCAL_BUFFER (char, paddedname, paddedlength);
          memset (paddedname, 0, paddedlength);
          strncpy (paddedname, classname.c_str (), namelen);
          os.write (paddedname, paddedlength);
        }

      octave_map m;

      octave::load_path& lp = octave::__get_load_path__ ();

      if (tc.isobject ()
          && lp.find_method (tc.class_name (), "saveobj") != "")
        {
          try
            {
              octave_value_list tmp = octave::feval ("saveobj", tc, 1);

              m = tmp(0).map_value ();
            }
          catch (const octave::execution_exception&)
            {
              error ("save: error while writing '%s' to MAT file",
                     name.c_str ());
            }
        }
      else
        m = tc.map_value ();

      // an Octave structure */
      // recursively write each element of the structure
      {
        char buf[64];
        int32_t maxfieldnamelength = max_namelen + 1;

        octave_idx_type nf = m.nfields ();

        write_mat5_tag (os, miINT32, 4);
        os.write (reinterpret_cast<char *> (&maxfieldnamelength), 4);
        write_mat5_tag (os, miINT8, nf*maxfieldnamelength);

        // Iterating over the list of keys will preserve the order of
        // the fields.
        string_vector keys = m.keys ();

        for (octave_idx_type i = 0; i < nf; i++)
          {
            std::string key = keys(i);

            // write the name of each element
            memset (buf, 0, max_namelen + 1);
            // only 31 or 63 char names permitted
            strncpy (buf, key.c_str (), max_namelen);
            os.write (buf, max_namelen + 1);
          }

        octave_idx_type len = m.numel ();

        // Create temporary copy of structure contents to avoid
        // multiple calls of the contents method.
        std::vector<const octave_value *> elts (nf);
        for (octave_idx_type i = 0; i < nf; i++)
          elts[i] = m.contents (keys(i)).data ();

        for (octave_idx_type j = 0; j < len; j++)
          {
            // write the data of each element

            // Iterating over the list of keys will preserve the order
            // of the fields.
            for (octave_idx_type i = 0; i < nf; i++)
              {
                bool retval2 = save_mat5_binary_element (os, elts[i][j], "",
                               mark_global,
                               false,
                               save_as_floats);
                if (! retval2)
                  error ("save: error while writing '%s' to MAT file",
                         name.c_str ());
              }
          }
      }
    }
  else
    // FIXME: Should this just error out rather than warn?
    warn_wrong_type_arg ("save", tc);

  return true;
}
