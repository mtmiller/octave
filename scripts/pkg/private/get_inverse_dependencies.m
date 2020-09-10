########################################################################
##
## Copyright (C) 2020 The Octave Project Developers
##
## See the file COPYRIGHT.md in the top-level directory of this
## distribution or <https://octave.org/copyright/>.
##
## This file is part of Octave.
##
## Octave is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <https://www.gnu.org/licenses/>.
##
########################################################################

## -*- texinfo -*-
## @deftypefn {} {@var{installed_pkgs_list} =} get_inverse_dependencies (@var{installed_pkgs_lst})
## Find inverse dependencies, if any, for each package, and store in
## the struct field @qcode{"invdeps"}.
##
## @end deftypefn

function installed_pkgs_lst = get_inverse_dependencies (installed_pkgs_lst)

  for i = 1:numel (installed_pkgs_lst)
    installed_pkgs_lst{i}.invdeps = {};  # initialize invdeps field
  endfor

  for i = 1:numel (installed_pkgs_lst)
    pdeps = installed_pkgs_lst{i}.depends;
    for j = 1:numel (pdeps)
      pdep_nm = pdeps{j}.package;
      if (! strcmpi (pdep_nm, "octave"))
        idx = cellfun (@(S) strcmpi (S.name, pdep_nm), installed_pkgs_lst);
        if (any (idx))
          installed_pkgs_lst{idx}.invdeps(end+1) = {installed_pkgs_lst{i}.name};
        endif
      endif
    endfor
  endfor

endfunction
