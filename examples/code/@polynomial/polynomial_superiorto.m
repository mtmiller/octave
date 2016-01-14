## -*- texinfo -*-
## @deftypefn  {Function File} {} polynomial ()
## @deftypefnx {Function File} {} polynomial (@var{a})
## Create a polynomial object representing the polynomial
##
## @example
## a0 + a1 * x + a2 * x^2 + @dots{} + an * x^n
## @end example
##
## @noindent
## from a vector of coefficients [a0 a1 a2 @dots{} an].
## @end deftypefn

function p = polynomial (a)

  if (nargin > 1)
    print_usage ();
  endif

  if (nargin == 0)
    p.poly = [0];
    p = class (p, "polynomial");
  else
    if (strcmp (class (a), "polynomial"))
      p = a;
    elseif (isvector (a) && isreal (a))
      p.poly = a(:).';  # force row vector
      p = class (p, "polynomial");
    else
      error ("@polynomial: expecting real vector");
    endif
  endif

  superiorto ("double");

endfunction
