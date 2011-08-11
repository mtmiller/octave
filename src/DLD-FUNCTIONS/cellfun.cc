/*

Copyright (C) 2005-2011 Mohamed Kamoun
Copyright (C) 2009 Jaroslav Hajek
Copyright (C) 2010 VZLU Prague

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <vector>
#include <list>
#include <memory>

#include "lo-mappers.h"
#include "oct-locbuf.h"

#include "Cell.h"
#include "oct-map.h"
#include "defun-dld.h"
#include "parse.h"
#include "variables.h"
#include "ov-colon.h"
#include "unwind-prot.h"
#include "gripes.h"
#include "utils.h"

#include "ov-scalar.h"
#include "ov-float.h"
#include "ov-complex.h"
#include "ov-flt-complex.h"
#include "ov-bool.h"
#include "ov-int8.h"
#include "ov-int16.h"
#include "ov-int32.h"
#include "ov-int64.h"
#include "ov-uint8.h"
#include "ov-uint16.h"
#include "ov-uint32.h"
#include "ov-uint64.h"

static octave_value_list
get_output_list (octave_idx_type count, octave_idx_type nargout,
                 const octave_value_list& inputlist,
                 octave_value& func,
                 octave_value& error_handler)
{
  octave_value_list tmp = func.do_multi_index_op (nargout, inputlist);

  if (error_state)
    {
      if (error_handler.is_defined ())
        {
          octave_scalar_map msg;
          msg.assign ("identifier", last_error_id ());
          msg.assign ("message", last_error_message ());
          msg.assign ("index", static_cast<double> (count + static_cast<octave_idx_type>(1)));
          octave_value_list errlist = inputlist;
          errlist.prepend (msg);
          buffer_error_messages--;
          error_state = 0;
          tmp = error_handler.do_multi_index_op (nargout, errlist);
          buffer_error_messages++;

          if (error_state)
            tmp.clear ();
        }
      else
        tmp.clear ();
    }

  return tmp;
}

DEFUN_DLD (cellfun, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {} cellfun (@var{name}, @var{C})\n\
@deftypefnx {Loadable Function} {} cellfun (\"size\", @var{C}, @var{k})\n\
@deftypefnx {Loadable Function} {} cellfun (\"isclass\", @var{C}, @var{class})\n\
@deftypefnx {Loadable Function} {} cellfun (@var{func}, @var{C})\n\
@deftypefnx {Loadable Function} {} cellfun (@var{func}, @var{C}, @var{D})\n\
@deftypefnx {Loadable Function} {[@var{a}, @dots{}] =} cellfun (@dots{})\n\
@deftypefnx {Loadable Function} {} cellfun (@dots{}, 'ErrorHandler', @var{errfunc})\n\
@deftypefnx {Loadable Function} {} cellfun (@dots{}, 'UniformOutput', @var{val})\n\
\n\
Evaluate the function named @var{name} on the elements of the cell array\n\
@var{C}.  Elements in @var{C} are passed on to the named function\n\
individually.  The function @var{name} can be one of the functions\n\
\n\
@table @code\n\
@item isempty\n\
Return 1 for empty elements.\n\
\n\
@item islogical\n\
Return 1 for logical elements.\n\
\n\
@item isreal\n\
Return 1 for real elements.\n\
\n\
@item length\n\
Return a vector of the lengths of cell elements.\n\
\n\
@item ndims\n\
Return the number of dimensions of each element.\n\
\n\
@item numel\n\
@itemx prodofsize\n\
Return the number of elements contained within each cell element.  The\n\
number is the product of the dimensions of the object at each cell element.\n\
\n\
@item size\n\
Return the size along the @var{k}-th dimension.\n\
\n\
@item isclass\n\
Return 1 for elements of @var{class}.\n\
@end table\n\
\n\
Additionally, @code{cellfun} accepts an arbitrary function @var{func}\n\
in the form of an inline function, function handle, or the name of a\n\
function (in a character string).  In the case of a character string\n\
argument, the function must accept a single argument named @var{x}, and\n\
it must return a string value.  The function can take one or more arguments,\n\
with the inputs arguments given by @var{C}, @var{D}, etc.  Equally the\n\
function can return one or more output arguments.  For example:\n\
\n\
@example\n\
@group\n\
cellfun (\"atan2\", @{1, 0@}, @{0, 1@})\n\
     @result{}ans = [1.57080   0.00000]\n\
@end group\n\
@end example\n\
\n\
The number of output arguments of @code{cellfun} matches the number of output\n\
arguments of the function.  The outputs of the function will be collected\n\
into the output arguments of @code{cellfun} like this:\n\
\n\
@example\n\
@group\n\
function [a, b] = twoouts (x)\n\
  a = x;\n\
  b = x*x;\n\
endfunction\n\
[aa, bb] = cellfun(@@twoouts, @{1, 2, 3@})\n\
     @result{}\n\
        aa =\n\
           1 2 3\n\
        bb =\n\
           1 4 9\n\
@end group\n\
@end example\n\
\n\
Note that per default the output argument(s) are arrays of the same size as\n\
the input arguments.  Input arguments that are singleton (1x1) cells will be\n\
automatically expanded to the size of the other arguments.\n\
\n\
If the parameter 'UniformOutput' is set to true (the default), then the\n\
function must return scalars which will be concatenated into the return\n\
array(s).  If 'UniformOutput' is false, the outputs are concatenated into a\n\
cell array (or cell arrays).  For example:\n\
\n\
@example\n\
@group\n\
cellfun (\"tolower\", @{\"Foo\", \"Bar\", \"FooBar\"@},\n\
         \"UniformOutput\",false)\n\
@result{} ans = @{\"foo\", \"bar\", \"foobar\"@}\n\
@end group\n\
@end example\n\
\n\
Given the parameter 'ErrorHandler', then @var{errfunc} defines a function to\n\
call in case @var{func} generates an error.  The form of the function is\n\
\n\
@example\n\
function [@dots{}] = errfunc (@var{s}, @dots{})\n\
@end example\n\
\n\
@noindent\n\
where there is an additional input argument to @var{errfunc} relative to\n\
@var{func}, given by @var{s}.  This is a structure with the elements\n\
'identifier', 'message' and 'index', giving respectively the error\n\
identifier, the error message, and the index into the input arguments\n\
of the element that caused the error.  For example:\n\
\n\
@example\n\
@group\n\
function y = foo (s, x), y = NaN; endfunction\n\
cellfun (\"factorial\", @{-1,2@}, 'ErrorHandler', @@foo)\n\
@result{} ans = [NaN 2]\n\
@end group\n\
@end example\n\
\n\
@seealso{arrayfun, structfun, spfun}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();
  int nargout1 = (nargout < 1 ? 1 : nargout);

  if (nargin < 2)
    {
      error ("cellfun: function requires at least 2 arguments");
      print_usage ();
      return retval;
    }

  octave_value func = args(0);
  bool symbol_table_lookup = false;

  if (! args(1).is_cell ())
    {
      error ("cellfun: C must be a cell array");

      return retval;
    }

  if (func.is_string ())
    {
      const Cell f_args = args(1).cell_value ();

      octave_idx_type k = f_args.numel ();

      std::string name = func.string_value ();

      if (name == "isempty")
        {
          boolNDArray result (f_args.dims ());
          for (octave_idx_type count = 0; count < k ; count++)
            result(count) = f_args.elem(count).is_empty ();
          retval(0) = result;
        }
      else if (name == "islogical")
        {
          boolNDArray result (f_args.dims ());
          for (octave_idx_type  count= 0; count < k ; count++)
            result(count) = f_args.elem(count).is_bool_type ();
          retval(0) = result;
        }
      else if (name == "isreal")
        {
          boolNDArray result (f_args.dims ());
          for (octave_idx_type  count= 0; count < k ; count++)
            result(count) = f_args.elem(count).is_real_type ();
          retval(0) = result;
        }
      else if (name == "length")
        {
          NDArray result (f_args.dims ());
          for (octave_idx_type  count= 0; count < k ; count++)
            result(count) = static_cast<double> (f_args.elem(count).length ());
          retval(0) = result;
        }
      else if (name == "ndims")
        {
          NDArray result (f_args.dims ());
          for (octave_idx_type count = 0; count < k ; count++)
            result(count) = static_cast<double> (f_args.elem(count).ndims ());
          retval(0) = result;
        }
      else if (name == "prodofsize" || name == "numel")
        {
          NDArray result (f_args.dims ());
          for (octave_idx_type count = 0; count < k ; count++)
            result(count) = static_cast<double> (f_args.elem(count).numel ());
          retval(0) = result;
        }
      else if (name == "size")
        {
          if (nargin == 3)
            {
              int d = args(2).nint_value () - 1;

              if (d < 0)
                error ("cellfun: K must be a positive integer");

              if (! error_state)
                {
                  NDArray result (f_args.dims ());
                  for (octave_idx_type count = 0; count < k ; count++)
                    {
                      dim_vector dv = f_args.elem(count).dims ();
                      if (d < dv.length ())
                        result(count) = static_cast<double> (dv(d));
                      else
                        result(count) = 1.0;
                    }
                  retval(0) = result;
                }
            }
          else
            error ("cellfun: not enough arguments for \"size\"");
        }
      else if (name == "isclass")
        {
          if (nargin == 3)
            {
              std::string class_name = args(2).string_value();
              boolNDArray result (f_args.dims ());
              for (octave_idx_type count = 0; count < k ; count++)
                result(count) = (f_args.elem(count).class_name() == class_name);

              retval(0) = result;
            }
          else
            error ("cellfun: not enough arguments for \"isclass\"");
        }
      else
        {
          if (! valid_identifier (name))
            {

              std::string fcn_name = unique_symbol_name ("__cellfun_fcn_");
              std::string fname = "function y = ";
              fname.append (fcn_name);
              fname.append ("(x) y = ");
              octave_function *ptr_func = extract_function (args(0), "cellfun",
                                                            fcn_name, fname, "; endfunction");
              if (ptr_func && ! error_state)
                func = octave_value (ptr_func, true);
            }
          else
            {
              func = symbol_table::find_function (name);
              if (func.is_undefined ())
                error ("cellfun: invalid function NAME: %s", name.c_str ());

              symbol_table_lookup = true;
            }
        }
    }

  if (error_state || ! retval.empty ())
    return retval;

  if (func.is_function_handle () || func.is_inline_function ()
      || func.is_function ())
    {

      // The following is an optimisation because the symbol table can
      // give a more specific function class, so this can result in
      // fewer polymorphic function calls as the function gets called
      // for each value of the array.
      if (! symbol_table_lookup )
        {
          octave_value f = symbol_table::find_function ( func.function_value ()
                                                         -> name ());
          if (f.is_defined ())
            func = f;
        }

      unwind_protect frame;
      frame.protect_var (buffer_error_messages);

      bool uniform_output = true;
      octave_value error_handler;

      while (nargin > 3 && args(nargin-2).is_string())
        {
          std::string arg = args(nargin-2).string_value();

          std::transform (arg.begin (), arg.end (),
                          arg.begin (), tolower);

          if (arg == "uniformoutput")
            uniform_output = args(nargin-1).bool_value();
          else if (arg == "errorhandler")
            {
              if (args(nargin-1).is_function_handle () ||
                  args(nargin-1).is_inline_function ())
                {
                  error_handler = args(nargin-1);
                }
              else if (args(nargin-1).is_string ())
                {
                  std::string err_name = args(nargin-1).string_value ();
                  error_handler = symbol_table::find_function (err_name);
                  if (error_handler.is_undefined ())
                    {
                      error ("cellfun: invalid function NAME: %s", err_name.c_str ());
                      break;
                    }
                }
              else
                {
                  error ("cellfun: invalid value for 'ErrorHandler' function");
                  break;
                }
            }
          else
            {
              error ("cellfun: unrecognized parameter %s",
                     arg.c_str());
              break;
            }

          nargin -= 2;
        }

      nargin -= 1;

      octave_value_list inputlist (nargin, octave_value ());

      OCTAVE_LOCAL_BUFFER (Cell, inputs, nargin);
      OCTAVE_LOCAL_BUFFER (bool, mask, nargin);

      // This is to prevent copy-on-write.
      const Cell *cinputs = inputs;

      octave_idx_type k = 1;

      dim_vector fdims (1, 1);

      if (error_state)
        return octave_value_list ();

      for (int j = 0; j < nargin; j++)
        {
          if (! args(j+1).is_cell ())
            {
              error ("cellfun: arguments must be cells");
              return octave_value_list ();
            }

          inputs[j] = args(j+1).cell_value ();
          mask[j] = inputs[j].numel () != 1;
          if (! mask[j])
            inputlist(j) = cinputs[j](0);
        }

      for (int j = 0; j < nargin; j++)
        {
          if (mask[j])
            {
              fdims = inputs[j].dims ();
              k = inputs[j].numel ();
              for (int i = j+1; i < nargin; i++)
                {
                  if (mask[i] && inputs[i].dims () != fdims)
                    {
                      error ("cellfun: dimensions mismatch");
                      return octave_value_list ();
                    }
                }
              break;
            }
        }

      if (error_handler.is_defined ())
        buffer_error_messages++;

      if (uniform_output)
        {
          std::list<octave_value_list> idx_list (1);
          idx_list.front ().resize (1);
          std::string idx_type = "(";

          OCTAVE_LOCAL_BUFFER (octave_value, retv, nargout1);

          for (octave_idx_type count = 0; count < k ; count++)
            {
              for (int j = 0; j < nargin; j++)
                {
                  if (mask[j])
                    inputlist.xelem (j) = cinputs[j](count);
                }

              const octave_value_list tmp = get_output_list (count, nargout, inputlist,
                                                             func, error_handler);

              if (error_state)
                return retval;

              if (tmp.length () < nargout1)
                {
                  if (tmp.length () < nargout)
                    {
                      error ("cellfun: too many output arguments");
                      return octave_value_list ();
                    }
                  else
                    nargout1 = 0;
                }

              if (count == 0)
                {
                  for (int j = 0; j < nargout1; j++)
                    {
                      octave_value val = tmp(j);

                      if (val.numel () == 1)
                        retv[j] = val.resize (fdims);
                      else
                        {
                          error ("cellfun: all values must be scalars when UniformOutput = true");
                          break;
                        }
                    }
                }
              else
                {
                  for (int j = 0; j < nargout1; j++)
                    {
                      octave_value val = tmp(j);

                      if (! retv[j].fast_elem_insert (count, val))
                        {
                          if (val.numel () == 1)
                            {
                              idx_list.front ()(0) = count + 1.0;
                              retv[j].assign (octave_value::op_asn_eq,
                                              idx_type, idx_list, val);

                              if (error_state)
                                break;
                            }
                          else
                            {
                              error ("cellfun: all values must be scalars when UniformOutput = true");
                              break;
                            }
                        }
                    }
                }

              if (error_state)
                break;
            }

          retval.resize (nargout1);
          for (int j = 0; j < nargout1; j++)
            {
              if (nargout > 0 && retv[j].is_undefined ())
                retval(j) = NDArray (fdims);
              else
                retval(j) = retv[j];
            }
        }
      else
        {
          OCTAVE_LOCAL_BUFFER (Cell, results, nargout1);
          for (int j = 0; j < nargout1; j++)
            results[j].resize (fdims);

          for (octave_idx_type count = 0; count < k ; count++)
            {
              for (int j = 0; j < nargin; j++)
                {
                  if (mask[j])
                    inputlist.xelem (j) = cinputs[j](count);
                }

              const octave_value_list tmp = get_output_list (count, nargout, inputlist,
                                                             func, error_handler);

              if (error_state)
                return retval;

              if (tmp.length () < nargout1)
                {
                  if (tmp.length () < nargout)
                    {
                      error ("cellfun: too many output arguments");
                      return octave_value_list ();
                    }
                  else
                    nargout1 = 0;
                }


              for (int j = 0; j < nargout1; j++)
                results[j](count) = tmp(j);
            }

          retval.resize(nargout1);
          for (int j = 0; j < nargout1; j++)
            retval(j) = results[j];
        }
    }
  else
    error ("cellfun: argument NAME must be a string or function handle");

  return retval;
}

/*

%!test
%!  [a,b] = cellfun (@(x) x, cell (2, 0));
%!  assert (a, zeros (2, 0));
%!  assert (b, zeros (2, 0));

%!test
%!  [a,b] = cellfun (@(x) x, cell (2, 0), "uniformoutput", false);
%!  assert (a, cell (2, 0));
%!  assert (b, cell (2, 0));

%% Test function to check the "Errorhandler" option
%!function [z] = cellfunerror (S, varargin)
%!    z = S;
%!  endfunction

%% First input argument can be a string, an inline function,
%% a function_handle or an anonymous function
%!test
%!  A = cellfun ("islogical", {true, 0.1, false, i*2});
%!  assert (A, [true, false, true, false]);
%!test
%!  A = cellfun (inline ("islogical (x)", "x"), {true, 0.1, false, i*2});
%!  assert (A, [true, false, true, false]);
%!test
%!  A = cellfun (@islogical, {true, 0.1, false, i*2});
%!  assert (A, [true, false, true, false]);
%!test
%!  A = cellfun (@(x) islogical(x), {true, 0.1, false, i*2});
%!  assert (A, [true, false, true, false]);

%% First input argument can be the special string "isreal",
%% "isempty", "islogical", "length", "ndims" or "prodofsize"
%!test
%!  A = cellfun ("isreal", {true, 0.1, {}, i*2, [], "abc"});
%!  assert (A, [true, true, false, false, true, true]);
%!test
%!  A = cellfun ("isempty", {true, 0.1, false, i*2, [], "abc"});
%!  assert (A, [false, false, false, false, true, false]);
%!test
%!  A = cellfun ("islogical", {true, 0.1, false, i*2, [], "abc"});
%!  assert (A, [true, false, true, false, false, false]);
%!test
%!  A = cellfun ("length", {true, 0.1, false, i*2, [], "abc"});
%!  assert (A, [1, 1, 1, 1, 0, 3]);
%!test
%!  A = cellfun ("ndims", {[1, 2; 3, 4]; (cell (1,2,3,4))});
%!  assert (A, [2; 4]);
%!test
%!  A = cellfun ("prodofsize", {[1, 2; 3, 4], (cell (1,2,3,4))});
%!  assert (A, [4, 24]);

%% Number of input and output arguments may not be limited to one
%!test
%!  A = cellfun (@(x,y,z) x + y + z, {1, 1, 1}, {2, 2, 2}, {3, 4, 5});
%!  assert (A, [6, 7, 8]);
%!test
%!  A = cellfun (@(x,y,z) x + y + z, {1, 1, 1}, {2, 2, 2}, {3, 4, 5}, \
%!    "UniformOutput", false);
%!  assert (A, {6, 7, 8});
%!test %% Two input arguments of different types
%!  A = cellfun (@(x,y) islogical (x) && ischar (y), {false, true}, {"a", 3});
%!  assert (A, [true, false]);
%!test %% Pass another variable to the anonymous function
%!  y = true; A = cellfun (@(x) islogical (x) && y, {false, 0.3});
%!  assert (A, [true, false]);
%!test %% Three ouptut arguments of different type
%!  [A, B, C] = cellfun (@find, {10, 11; 0, 12}, "UniformOutput", false);
%!  assert (isequal (A, {true, true; [], true}));
%!  assert (isequal (B, {true, true; [], true}));
%!  assert (isequal (C, {10, 11; [], 12}));

%% Input arguments can be of type cell array of logical
%!test
%!  A = cellfun (@(x,y) x == y, {false, true}, {true, true});
%!  assert (A, [false, true]);
%!test
%!  A = cellfun (@(x,y) x == y, {false; true}, {true; true}, \
%!    "UniformOutput", true);
%!  assert (A, [false; true]);
%!test
%!  A = cellfun (@(x) x, {false, true; false, true}, "UniformOutput", false);
%!  assert (A, {false, true; false, true});
%!test %% Three ouptut arguments of same type
%!  [A, B, C] = cellfun (@find, {true, false; false, true}, \
%!    "UniformOutput", false);
%!  assert (isequal (A, {true, []; [], true}));
%!  assert (isequal (B, {true, []; [], true}));
%!  assert (isequal (C, {true, []; [], true}));
%!test
%!  A = cellfun (@(x,y) cell2str (x,y), {true}, {true}, \
%!    "ErrorHandler", @cellfunerror);
%!  assert (isfield (A, "identifier"), true);
%!  assert (isfield (A, "message"), true);
%!  assert (isfield (A, "index"), true);
%!  assert (isempty (A.message), false);
%!  assert (A.index, 1);
%!test %% Overwriting setting of "UniformOutput" true
%!  A = cellfun (@(x,y) cell2str (x,y), {true}, {true}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  assert (isfield (A, "identifier"), true);
%!  assert (isfield (A, "message"), true);
%!  assert (isfield (A, "index"), true);
%!  assert (isempty (A.message), false);
%!  assert (A.index, 1);

%% Input arguments can be of type cell array of numeric
%!test
%!  A = cellfun (@(x,y) x>y, {1.1, 4.2}, {3.1, 2+3*i});
%!  assert (A, [false, true]);
%!test
%!  A = cellfun (@(x,y) x>y, {1.1, 4.2; 2, 4}, {3.1, 2; 2, 4+2*i}, \
%!    "UniformOutput", true);
%!  assert (A, [false, true; false, false]);
%!test
%!  A = cellfun (@(x,y) x:y, {1.1, 4}, {3.1, 6}, "UniformOutput", false);
%!  assert (isequal (A{1}, [1.1, 2.1, 3.1]));
%!  assert (isequal (A{2}, [4, 5, 6]));
%!test %% Three ouptut arguments of different type
%!  [A, B, C] = cellfun (@find, {10, 11; 0, 12}, "UniformOutput", false);
%!  assert (isequal (A, {true, true; [], true}));
%!  assert (isequal (B, {true, true; [], true}));
%!  assert (isequal (C, {10, 11; [], 12}));
%!test
%!  A = cellfun (@(x,y) cell2str(x,y), {1.1, 4}, {3.1, 6}, \
%!    "ErrorHandler", @cellfunerror);
%!  B = isfield (A(1), "message") && isfield (A(1), "index");
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);
%!test %% Overwriting setting of "UniformOutput" true
%!  A = cellfun (@(x,y) cell2str(x,y), {1.1, 4}, {3.1, 6}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  B = isfield (A(1), "message") && isfield (A(1), "index");
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);

%% Input arguments can be of type cell arrays of character or strings
%!error %% "UniformOutput" false should be used
%!  A = cellfun (@(x,y) x>y, {"ad", "c", "ghi"}, {"cc", "d", "fgh"});
%!test
%!  A = cellfun (@(x,y) x>y, {"a"; "f"}, {"c"; "d"}, "UniformOutput", true);
%!  assert (A, [false; true]);
%!test
%!  A = cellfun (@(x,y) x:y, {"a", "d"}, {"c", "f"}, "UniformOutput", false);
%!  assert (A, {"abc", "def"});
%!test
%!  A = cellfun (@(x,y) cell2str(x,y), {"a", "d"}, {"c", "f"}, \
%!    "ErrorHandler", @cellfunerror);
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);
%!test %% Overwriting setting of "UniformOutput" true
%!  A = cellfun (@(x,y) cell2str(x,y), {"a", "d"}, {"c", "f"}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);

%% Structures cannot be handled by cellfun
%!error
%!  vst1.a = 1.1; vst1.b = 4.2; vst2.a = 3.1; vst2.b = 2;
%!  A = cellfun (@(x,y) (x.a < y.a) && (x.b > y.b), vst1, vst2);

%% Input arguments can be of type cell array of cell arrays
%!test
%!  A = cellfun (@(x,y) x{1} < y{1}, {{1.1}, {4.2}}, {{3.1}, {2}});
%!  assert (A, [1, 0], 1e-16);
%!test
%!  A = cellfun (@(x,y) x{1} < y{1}, {{1.1}; {4.2}}, {{3.1}; {2}}, \
%!    "UniformOutput", true);
%!  assert (A, [1; 0], 1e-16);
%!test
%!  A = cellfun (@(x,y) x{1} < y{1}, {{1.1}, {4.2}}, {{3.1}, {2}}, \
%!    "UniformOutput", false);
%!  assert (A, {true, false});
%!test
%!  A = cellfun (@(x,y) mat2str(x,y), {{1.1}, {4.2}}, {{3.1}, {2}}, \
%!    "ErrorHandler", @cellfunerror);
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);
%!test %% Overwriting setting of "UniformOutput" true
%!  A = cellfun (@(x,y) mat2str(x,y), {{1.1}, {4.2}}, {{3.1}, {2}}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);

%% Input arguments can be of type cell array of structure arrays
%!test
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) (x.a == y.a) && (x.b < y.b), {a}, {b});
%!  assert (A, true);
%!test
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) (x.a == y.a) && (x.b < y.b) , {a}, {b}, \
%!    "UniformOutput", true);
%!  assert (A, true);
%!test
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) (x.a == y.a) && (x.b < y.b) , {a}, {b}, \
%!    "UniformOutput", false);
%!  assert (A, {true});
%!test
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) cell2str (x.a, y.a), {a}, {b}, \
%!    "ErrorHandler", @cellfunerror);
%!  assert (isfield (A, "identifier"), true);
%!  assert (isfield (A, "message"), true);
%!  assert (isfield (A, "index"), true);
%!  assert (isempty (A.message), false);
%!  assert (A.index, 1);
%!test %% Overwriting setting of "UniformOutput" true
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) cell2str (x.a, y.a), {a}, {b}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  assert (isfield (A, "identifier"), true);
%!  assert (isfield (A, "message"), true);
%!  assert (isfield (A, "index"), true);
%!  assert (isempty (A.message), false);
%!  assert (A.index, 1);

%% A lot of other tests
%!error(cellfun(1))
%!error(cellfun('isclass',1))
%!error(cellfun('size',1))
%!error(cellfun(@sin,{[]},'BadParam',false))
%!error(cellfun(@sin,{[]},'UniformOuput'))
%!error(cellfun(@sin,{[]},'ErrorHandler'))
%!assert(cellfun(@sin,{0,1}),sin([0,1]))
%!assert(cellfun(inline('sin(x)'),{0,1}),sin([0,1]))
%!assert(cellfun('sin',{0,1}),sin([0,1]))
%!assert(cellfun('isempty',{1,[]}),[false,true])
%!assert(cellfun('islogical',{false,pi}),[true,false])
%!assert(cellfun('isreal',{1i,1}),[false,true])
%!assert(cellfun('length',{zeros(2,2),1}),[2,1])
%!assert(cellfun('prodofsize',{zeros(2,2),1}),[4,1])
%!assert(cellfun('ndims',{zeros([2,2,2]),1}),[3,2])
%!assert(cellfun('isclass',{zeros([2,2,2]),'test'},'double'),[true,false])
%!assert(cellfun('size',{zeros([1,2,3]),1},1),[1,1])
%!assert(cellfun('size',{zeros([1,2,3]),1},2),[2,1])
%!assert(cellfun('size',{zeros([1,2,3]),1},3),[3,1])
%!assert(cellfun(@atan2,{1,1},{1,2}),[atan2(1,1),atan2(1,2)])
%!assert(cellfun(@atan2,{1,1},{1,2},'UniformOutput',false),{atan2(1,1),atan2(1,2)})
%!assert(cellfun(@sin,{1,2;3,4}),sin([1,2;3,4]))
%!assert(cellfun(@atan2,{1,1;1,1},{1,2;1,2}),atan2([1,1;1,1],[1,2;1,2]))
%!error(cellfun(@factorial,{-1,3}))
%!assert(cellfun(@factorial,{-1,3},'ErrorHandler',@(x,y) NaN),[NaN,6])
%!test
%! [a,b,c]=cellfun(@fileparts,{fullfile("a","b","c.d"),fullfile("e","f","g.h")},'UniformOutput',false);
%! assert(a,{fullfile("a","b"),fullfile("e","f")})
%! assert(b,{'c','g'})
%! assert(c,{'.d','.h'})

*/

static void
do_num2cell_helper (const dim_vector& dv,
                    const Array<int>& dimv,
                    dim_vector& celldv, dim_vector& arraydv,
                    Array<int>& perm)
{
  int dvl = dimv.length ();
  int maxd = dv.length ();
  celldv = dv;
  for (int i = 0; i < dvl; i++)
    maxd = std::max (maxd, dimv(i));
  if (maxd > dv.length ())
    celldv.resize (maxd, 1);
  arraydv = celldv;

  OCTAVE_LOCAL_BUFFER_INIT (bool, sing, maxd, false);

  perm.clear (maxd, 1);
  for (int i = 0; i < dvl; i++)
    {
      int k = dimv(i) - 1;
      if (k < 0)
        {
          error ("num2cell: dimension indices must be positive");
          return;
        }
      else if (i > 0 && k < dimv(i-1) - 1)
        {
          error ("num2cell: dimension indices must be strictly increasing");
          return;
        }

      sing[k] = true;
      perm(i) = k;
    }

  for (int k = 0, i = dvl; k < maxd; k++)
    if (! sing[k])
      perm(i++) = k;

  for (int i = 0; i < maxd; i++)
    if (sing[i])
      celldv(i) = 1;
    else
      arraydv(i) = 1;
}

template<class NDA>
static inline typename NDA::element_type
do_num2cell_elem (const NDA& array, octave_idx_type i)
{ return array(i); }

static inline Cell
do_num2cell_elem (const Cell& array, octave_idx_type i)
{ return Cell (array(i)); }


template<class NDA>
static Cell
do_num2cell (const NDA& array, const Array<int>& dimv)
{
  if (dimv.is_empty ())
    {
      Cell retval (array.dims ());
      octave_idx_type nel = array.numel ();
      for (octave_idx_type i = 0; i < nel; i++)
        retval.xelem (i) = do_num2cell_elem (array, i);

      return retval;
    }
  else
    {
      dim_vector celldv, arraydv;
      Array<int> perm;
      do_num2cell_helper (array.dims (), dimv, celldv, arraydv, perm);
      if (error_state)
        return Cell ();

      NDA parray = array.permute (perm);

      octave_idx_type nela = arraydv.numel (), nelc = celldv.numel ();
      parray = parray.reshape (dim_vector (nela, nelc));

      Cell retval (celldv);
      for (octave_idx_type i = 0; i < nelc; i++)
        {
          retval.xelem (i) = NDA (parray.column (i).reshape (arraydv));
        }

      return retval;
    }
}


DEFUN_DLD (num2cell, args, ,
  "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {@var{C} =} num2cell (@var{A})\n\
@deftypefnx {Loadable Function} {@var{C} =} num2cell (@var{A}, @var{dim})\n\
Convert the numeric matrix @var{A} to a cell array.  If @var{dim} is\n\
defined, the value @var{C} is of dimension 1 in this dimension and the\n\
elements of @var{A} are placed into @var{C} in slices.  For example:\n\
\n\
@example\n\
@group\n\
num2cell([1,2;3,4])\n\
     @result{} ans =\n\
        @{\n\
          [1,1] =  1\n\
          [2,1] =  3\n\
          [1,2] =  2\n\
          [2,2] =  4\n\
        @}\n\
num2cell([1,2;3,4],1)\n\
     @result{} ans =\n\
        @{\n\
          [1,1] =\n\
             1\n\
             3\n\
          [1,2] =\n\
             2\n\
             4\n\
        @}\n\
@end group\n\
@end example\n\
\n\
@seealso{mat2cell}\n\
@end deftypefn")
{
  int nargin =  args.length();
  octave_value retval;

  if (nargin < 1 || nargin > 2)
    print_usage ();
  else
    {
      octave_value array = args(0);
      Array<int> dimv;
      if (nargin > 1)
        dimv = args (1).int_vector_value (true);

      if (error_state)
        ;
      else if (array.is_bool_type ())
        retval = do_num2cell (array.bool_array_value (), dimv);
      else if (array.is_char_matrix ())
        retval = do_num2cell (array.char_array_value (), dimv);
      else if (array.is_numeric_type ())
        {
          if (array.is_integer_type ())
            {
              if (array.is_int8_type ())
                retval = do_num2cell (array.int8_array_value (), dimv);
              else if (array.is_int16_type ())
                retval = do_num2cell (array.int16_array_value (), dimv);
              else if (array.is_int32_type ())
                retval = do_num2cell (array.int32_array_value (), dimv);
              else if (array.is_int64_type ())
                retval = do_num2cell (array.int64_array_value (), dimv);
              else if (array.is_uint8_type ())
                retval = do_num2cell (array.uint8_array_value (), dimv);
              else if (array.is_uint16_type ())
                retval = do_num2cell (array.uint16_array_value (), dimv);
              else if (array.is_uint32_type ())
                retval = do_num2cell (array.uint32_array_value (), dimv);
              else if (array.is_uint64_type ())
                retval = do_num2cell (array.uint64_array_value (), dimv);
            }
          else if (array.is_complex_type ())
            {
              if (array.is_single_type ())
                retval = do_num2cell (array.float_complex_array_value (), dimv);
              else
                retval = do_num2cell (array.complex_array_value (), dimv);
            }
          else
            {
              if (array.is_single_type ())
                retval = do_num2cell (array.float_array_value (), dimv);
              else
                retval = do_num2cell (array.array_value (), dimv);
            }
        }
      else if (array.is_map ())
        retval = do_num2cell (array.map_value (), dimv);
      else if (array.is_cell ())
        retval = do_num2cell (array.cell_value (), dimv);
      else
        gripe_wrong_type_arg ("num2cell", array);
    }

  return retval;
}

/*

%!assert(num2cell([1,2;3,4]),{1,2;3,4})
%!assert(num2cell([1,2;3,4],1),{[1;3],[2;4]})
%!assert(num2cell([1,2;3,4],2),{[1,2];[3,4]})

*/

static bool
mat2cell_mismatch (const dim_vector& dv,
                   const Array<octave_idx_type> *d, int nd)
{
  for (int i = 0; i < nd; i++)
    {
      octave_idx_type s = 0;
      for (octave_idx_type j = 0; j < d[i].length (); j++)
        s += d[i](j);

      octave_idx_type r = i < dv.length () ? dv(i) : 1;

      if (s != r)
        {
          error ("mat2cell: mismatch on %d-th dimension (%d != %d)",
                 i+1, r, s);
          return true;
        }
    }

  return false;
}

template<class container>
static void
prepare_idx (container *idx, int idim, int nd,
             const Array<octave_idx_type>* d)
{
  octave_idx_type nidx = idim < nd ? d[idim].numel () : 1;
  if (nidx == 1)
    idx[0] = idx_vector::colon;
  else
    {
      octave_idx_type l = 0;
      for (octave_idx_type i = 0; i < nidx; i++)
        {
          octave_idx_type u = l + d[idim](i);
          idx[i] = idx_vector (l, u);
          l = u;
        }
    }
}

// 2D specialization, works for Array, Sparse and octave_map.
// Uses 1D or 2D indexing.

template <class Array2D>
static Cell
do_mat2cell_2d (const Array2D& a, const Array<octave_idx_type> *d, int nd)
{
  NoAlias<Cell> retval;
  assert (nd == 1 || nd == 2);
  assert (a.ndims () == 2);

  if (mat2cell_mismatch (a.dims (), d, nd))
    return retval;

  octave_idx_type nridx = d[0].length ();
  octave_idx_type ncidx = nd == 1 ? 1 : d[1].length ();
  retval.clear (nridx, ncidx);

  int ivec = -1;
  if (a.rows () > 1 && a.cols () == 1 && ncidx == 1)
    ivec = 0;
  else if (a.rows () == 1 && nridx == 1 && nd == 2)
    ivec = 1;

  if (ivec >= 0)
    {
      // Vector split. Use 1D indexing.
      octave_idx_type l = 0, nidx = (ivec == 0 ? nridx : ncidx);
      for (octave_idx_type i = 0; i < nidx; i++)
        {
          octave_idx_type u = l + d[ivec](i);
          retval(i) = a.index (idx_vector (l, u));
          l = u;
        }
    }
  else
    {
      // General 2D case. Use 2D indexing.
      OCTAVE_LOCAL_BUFFER (idx_vector, ridx, nridx);
      prepare_idx (ridx, 0, nd, d);

      OCTAVE_LOCAL_BUFFER (idx_vector, cidx, ncidx);
      prepare_idx (cidx, 1, nd, d);

      for (octave_idx_type j = 0; j < ncidx; j++)
        for (octave_idx_type i = 0; i < nridx; i++)
          {
            octave_quit ();

            retval(i,j) = a.index (ridx[i], cidx[j]);
          }
    }

  return retval;
}

// Nd case. Works for Arrays and octave_map.
// Uses Nd indexing.

template <class ArrayND>
Cell
do_mat2cell_nd (const ArrayND& a, const Array<octave_idx_type> *d, int nd)
{
  NoAlias<Cell> retval;
  assert (nd >= 1);

  if (mat2cell_mismatch (a.dims (), d, nd))
    return retval;

  dim_vector rdv = dim_vector::alloc (nd);
  OCTAVE_LOCAL_BUFFER (octave_idx_type, nidx, nd);
  octave_idx_type idxtot = 0;
  for (int i = 0; i < nd; i++)
    {
      rdv(i) = nidx[i] = d[i].length ();
      idxtot += nidx[i];
    }

  retval.clear (rdv);

  OCTAVE_LOCAL_BUFFER (idx_vector, xidx, idxtot);
  OCTAVE_LOCAL_BUFFER (idx_vector *, idx, nd);

  idxtot = 0;
  for (int i = 0; i < nd; i++)
    {
      idx[i] = xidx + idxtot;
      prepare_idx (idx[i], i, nd, d);
      idxtot += nidx[i];
    }

  OCTAVE_LOCAL_BUFFER_INIT (octave_idx_type, ridx, nd, 0);
  NoAlias< Array<idx_vector> > ra_idx
    (dim_vector (1, std::max (nd, a.ndims ())), idx_vector::colon);

  for (octave_idx_type j = 0; j < retval.numel (); j++)
    {
      octave_quit ();

      for (int i = 0; i < nd; i++)
        ra_idx(i) = idx[i][ridx[i]];

      retval(j) = a.index (ra_idx);

      rdv.increment_index (ridx);
    }

  return retval;
}

// Dispatcher.
template <class ArrayND>
Cell
do_mat2cell (const ArrayND& a, const Array<octave_idx_type> *d, int nd)
{
  if (a.ndims () == 2 && nd <= 2)
    return do_mat2cell_2d (a, d, nd);
  else
    return do_mat2cell_nd (a, d, nd);
}

// General case. Works for any class supporting do_index_op.
// Uses Nd indexing.

Cell
do_mat2cell (octave_value& a, const Array<octave_idx_type> *d, int nd)
{
  NoAlias<Cell> retval;
  assert (nd >= 1);

  if (mat2cell_mismatch (a.dims (), d, nd))
    return retval;

  dim_vector rdv = dim_vector::alloc (nd);
  OCTAVE_LOCAL_BUFFER (octave_idx_type, nidx, nd);
  octave_idx_type idxtot = 0;
  for (int i = 0; i < nd; i++)
    {
      rdv(i) = nidx[i] = d[i].length ();
      idxtot += nidx[i];
    }

  retval.clear (rdv);

  OCTAVE_LOCAL_BUFFER (octave_value, xidx, idxtot);
  OCTAVE_LOCAL_BUFFER (octave_value *, idx, nd);

  idxtot = 0;
  for (int i = 0; i < nd; i++)
    {
      idx[i] = xidx + idxtot;
      prepare_idx (idx[i], i, nd, d);
      idxtot += nidx[i];
    }

  OCTAVE_LOCAL_BUFFER_INIT (octave_idx_type, ridx, nd, 0);
  octave_value_list ra_idx (std::max (nd, a.ndims ()),
                            octave_value::magic_colon_t);

  for (octave_idx_type j = 0; j < retval.numel (); j++)
    {
      octave_quit ();

      for (int i = 0; i < nd; i++)
        ra_idx(i) = idx[i][ridx[i]];

      retval(j) = a.do_index_op (ra_idx);

      if (error_state)
        break;

      rdv.increment_index (ridx);
    }

  return retval;
}

DEFUN_DLD (mat2cell, args, ,
  "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {@var{C} =} mat2cell (@var{A}, @var{m}, @var{n})\n\
@deftypefnx {Loadable Function} {@var{C} =} mat2cell (@var{A}, @var{d1}, @var{d2}, @dots{})\n\
@deftypefnx {Loadable Function} {@var{C} =} mat2cell (@var{A}, @var{r})\n\
Convert the matrix @var{A} to a cell array.  If @var{A} is 2-D, then\n\
it is required that @code{sum (@var{m}) == size (@var{A}, 1)} and\n\
@code{sum (@var{n}) == size (@var{A}, 2)}.  Similarly, if @var{A} is\n\
multi-dimensional and the number of dimensional arguments is equal\n\
to the dimensions of @var{A}, then it is required that @code{sum (@var{di})\n\
== size (@var{A}, i)}.\n\
\n\
Given a single dimensional argument @var{r}, the other dimensional\n\
arguments are assumed to equal @code{size (@var{A},@var{i})}.\n\
\n\
An example of the use of mat2cell is\n\
\n\
@example\n\
mat2cell (reshape(1:16,4,4),[3,1],[3,1])\n\
@result{} @{\n\
  [1,1] =\n\
\n\
     1   5   9\n\
     2   6  10\n\
     3   7  11\n\
\n\
  [2,1] =\n\
\n\
     4   8  12\n\
\n\
  [1,2] =\n\
\n\
    13\n\
    14\n\
    15\n\
\n\
  [2,2] = 16\n\
@}\n\
@end example\n\
@seealso{num2cell, cell2mat}\n\
@end deftypefn")
{
  int nargin = args.length();
  octave_value retval;

  if (nargin < 2)
    print_usage ();
  else
    {
      // Prepare indices.
      OCTAVE_LOCAL_BUFFER (Array<octave_idx_type>, d, nargin-1);

      for (int i = 1; i < nargin; i++)
        {
          d[i-1] = args(i).octave_idx_type_vector_value (true);
          if (error_state)
            return retval;
        }

      octave_value a = args(0);
      bool sparse = a.is_sparse_type ();
      if (sparse && nargin > 3)
        {
          error ("mat2cell: sparse arguments only support 2D indexing");
          return retval;
        }

      switch (a.builtin_type ())
        {
        case btyp_double:
          {
            if (sparse)
              retval = do_mat2cell_2d (a.sparse_matrix_value (), d, nargin-1);
            else
              retval = do_mat2cell (a.array_value (), d, nargin - 1);
            break;
          }
        case btyp_complex:
          {
            if (sparse)
              retval = do_mat2cell_2d (a.sparse_complex_matrix_value (), d, nargin-1);
            else
              retval = do_mat2cell (a.complex_array_value (), d, nargin - 1);
            break;
          }
#define BTYP_BRANCH(X,Y) \
        case btyp_ ## X: \
            retval = do_mat2cell (a.Y ## _value (), d, nargin - 1); \
          break

        BTYP_BRANCH (float, float_array);
        BTYP_BRANCH (float_complex, float_complex_array);
        BTYP_BRANCH (bool, bool_array);
        BTYP_BRANCH (char, char_array);

        BTYP_BRANCH (int8,  int8_array);
        BTYP_BRANCH (int16, int16_array);
        BTYP_BRANCH (int32, int32_array);
        BTYP_BRANCH (int64, int64_array);
        BTYP_BRANCH (uint8,  uint8_array);
        BTYP_BRANCH (uint16, uint16_array);
        BTYP_BRANCH (uint32, uint32_array);
        BTYP_BRANCH (uint64, uint64_array);

        BTYP_BRANCH (cell, cell);
        BTYP_BRANCH (struct, map);
#undef BTYP_BRANCH

        case btyp_func_handle:
          gripe_wrong_type_arg ("mat2cell", a);
          break;
        default:
          retval = do_mat2cell (a, d, nargin-1);
        }
    }

  return retval;
}

/*

%!test
%! x = reshape(1:20,5,4);
%! c = mat2cell(x,[3,2],[3,1]);
%! assert(c,{[1,6,11;2,7,12;3,8,13],[16;17;18];[4,9,14;5,10,15],[19;20]})

%!test
%! x = 'abcdefghij';
%! c = mat2cell(x,1,[0,4,2,0,4,0]);
%! empty1by0str = resize('',1,0);
%! assert(c,{empty1by0str,'abcd','ef',empty1by0str,'ghij',empty1by0str})

*/

// FIXME: it would be nice to allow ranges being handled without a conversion.
template <class NDA>
static Cell
do_cellslices_nda (const NDA& array,
                   const Array<octave_idx_type>& lb,
                   const Array<octave_idx_type>& ub,
                   int dim = -1)
{
  octave_idx_type n = lb.length ();
  Cell retval (1, n);
  if (array.is_vector () && (dim == -1
                             || (dim == 0 && array.columns () == 1)
                             || (dim == 1 && array.rows () == 1)))
    {
      for (octave_idx_type i = 0; i < n && ! error_state; i++)
        retval(i) = array.index (idx_vector (lb(i) - 1, ub(i)));
    }
  else
    {
      const dim_vector dv = array.dims ();
      int ndims = dv.length ();
      if (dim < 0)
        dim = dv.first_non_singleton ();
      ndims = std::max (ndims, dim + 1);

      Array<idx_vector> idx (dim_vector (ndims, 1), idx_vector::colon);

      for (octave_idx_type i = 0; i < n && ! error_state; i++)
        {
          idx(dim) = idx_vector (lb(i) - 1, ub(i));
          retval(i) = array.index (idx);
        }
    }

  return retval;
}

DEFUN_DLD (cellslices, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{sl} =} cellslices (@var{x}, @var{lb}, @var{ub}, @var{dim})\n\
Given an array @var{x}, this function produces a cell array of slices from\n\
the array determined by the index vectors @var{lb}, @var{ub}, for lower and\n\
upper bounds, respectively.  In other words, it is equivalent to the\n\
following code:\n\
\n\
@example\n\
@group\n\
n = length (lb);\n\
sl = cell (1, n);\n\
for i = 1:length (lb)\n\
  sl@{i@} = x(:,@dots{},lb(i):ub(i),@dots{},:);\n\
endfor\n\
@end group\n\
@end example\n\
\n\
The position of the index is determined by @var{dim}.  If not specified,\n\
slicing is done along the first non-singleton dimension.\n\
@end deftypefn")
{
  octave_value retval;
  int nargin = args.length ();
  if (nargin == 3 || nargin == 4)
    {
      octave_value x = args(0);
      Array<octave_idx_type> lb = args(1).octave_idx_type_vector_value ();
      Array<octave_idx_type> ub = args(2).octave_idx_type_vector_value ();
      int dim = -1;
      if (nargin == 4)
        {
          dim = args(3).int_value () - 1;
          if (dim < 0)
            error ("cellslices: DIM must be a valid dimension");
        }

      if (! error_state)
        {
          if (lb.length () != ub.length ())
            error ("cellslices: the lengths of LB and UB must match");
          else
            {
              Cell retcell;
              if (! x.is_sparse_type () && x.is_matrix_type ())
                {
                  // specialize for some dense arrays.
                  if (x.is_bool_type ())
                    retcell = do_cellslices_nda (x.bool_array_value (), lb, ub, dim);
                  else if (x.is_char_matrix ())
                    retcell = do_cellslices_nda (x.char_array_value (), lb, ub, dim);
                  else if (x.is_integer_type ())
                    {
                      if (x.is_int8_type ())
                        retcell = do_cellslices_nda (x.int8_array_value (), lb, ub, dim);
                      else if (x.is_int16_type ())
                        retcell = do_cellslices_nda (x.int16_array_value (), lb, ub, dim);
                      else if (x.is_int32_type ())
                        retcell = do_cellslices_nda (x.int32_array_value (), lb, ub, dim);
                      else if (x.is_int64_type ())
                        retcell = do_cellslices_nda (x.int64_array_value (), lb, ub, dim);
                      else if (x.is_uint8_type ())
                        retcell = do_cellslices_nda (x.uint8_array_value (), lb, ub, dim);
                      else if (x.is_uint16_type ())
                        retcell = do_cellslices_nda (x.uint16_array_value (), lb, ub, dim);
                      else if (x.is_uint32_type ())
                        retcell = do_cellslices_nda (x.uint32_array_value (), lb, ub, dim);
                      else if (x.is_uint64_type ())
                        retcell = do_cellslices_nda (x.uint64_array_value (), lb, ub, dim);
                    }
                  else if (x.is_complex_type ())
                    {
                      if (x.is_single_type ())
                        retcell = do_cellslices_nda (x.float_complex_array_value (), lb, ub, dim);
                      else
                        retcell = do_cellslices_nda (x.complex_array_value (), lb, ub, dim);
                    }
                  else
                    {
                      if (x.is_single_type ())
                        retcell = do_cellslices_nda (x.float_array_value (), lb, ub, dim);
                      else
                        retcell = do_cellslices_nda (x.array_value (), lb, ub, dim);
                    }
                }
              else
                {
                  // generic code.
                  octave_idx_type n = lb.length ();
                  retcell = Cell (1, n);
                  const dim_vector dv = x.dims ();
                  int ndims = dv.length ();
                  if (dim < 0)
                    dim = dv.first_non_singleton ();
                  ndims = std::max (ndims, dim + 1);
                  octave_value_list idx (ndims, octave_value::magic_colon_t);
                  for (octave_idx_type i = 0; i < n && ! error_state; i++)
                    {
                      idx(dim) = Range (lb(i), ub(i));
                      retcell(i) = x.do_index_op (idx);
                    }
                }
              if (! error_state)
                retval = retcell;
            }
        }
    }
  else
    print_usage ();

  return retval;
}

/*
%!test
%! m = [1, 2, 3, 4; 5, 6, 7, 8; 9, 10, 11, 12];
%! c = cellslices (m, [1, 2], [2, 3], 2);
%! assert (c, {[1, 2; 5, 6; 9, 10], [2, 3; 6, 7; 10, 11]});
*/

DEFUN_DLD (cellindexmat, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{y} =} cellindexmat (@var{x}, @var{varargin})\n\
Given a cell array of matrices @var{x}, this function computes\n\
\n\
@example\n\
@group\n\
  Y = cell (size (X));\n\
  for i = 1:numel (X)\n\
    Y@{i@} = X@{i@}(varargin@{:@});\n\
  endfor\n\
@end group\n\
@end example\n\
@seealso{cellfun, cellslices}\n\
@end deftypefn")
{
  octave_value retval;
  if (args.length () >= 1)
    {
      if (args(0).is_cell ())
        {
          const Cell x = args(0).cell_value ();
          NoAlias<Cell> y(x.dims ());
          octave_idx_type nel = x.numel ();
          octave_value_list idx = args.slice (1, args.length () - 1);

          for (octave_idx_type i = 0; i < nel; i++)
            {
              octave_quit ();
              octave_value tmp = x(i);
              y(i) = tmp.do_index_op (idx);
              if (error_state)
                break;
            }

          retval = y;
        }
      else
        error ("cellindexmat: X must be a cell");
    }
  else
    print_usage ();

  return retval;
}
