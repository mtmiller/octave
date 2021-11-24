classdef polynomial2
  properties
    poly = 0;
  endproperties

  methods
    function p = polynomial2 (a)
      if (nargin == 1)
        if (isa (a, "polynomial2"))
          p.poly = a.poly;
        elseif (isreal (a) && isvector (a))
          p.poly = a(:).';  # force row vector
        else
          error ("polynomial2: A must be a real vector");
        endif
      endif
    endfunction

    function disp (p)
      a = p.poly;
      first = true;
      for i = 1 : length (a);
        if (a(i) != 0)
          if (first)
            first = false;
          elseif (a(i) > 0 || isnan (a(i)))
            printf (" +");
          endif
          if (a(i) < 0)
            printf (" -");
          endif
          if (i == 1)
            printf (" %.5g", abs (a(i)));
          elseif (abs (a(i)) != 1)
            printf (" %.5g *", abs (a(i)));
          endif
          if (i > 1)
            printf (" X");
          endif
          if (i > 2)
            printf (" ^ %d", i - 1);
          endif
        endif
      endfor

      if (first)
        printf (" 0");
      endif
      printf ("\n");
    endfunction
  endmethods
endclassdef
