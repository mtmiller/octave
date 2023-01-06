########################################################################
##
## Copyright (C) 2005-2023 The Octave Project Developers
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
## @deftypefn {} {@var{bad_deps} =} get_unsatisfied_deps (@var{desc}, @var{installed_pkgs_lst}, @var{uninstall_flag})
## Undocumented internal function.
## @end deftypefn

function bad_deps = get_unsatisfied_deps (desc, installed_pkgs_lst,
                                          uninstall_flag = false)

  bad_deps = {};

  ## For each dependency.
  for i = 1:length (desc.depends)
    dep = desc.depends{i};

    ## Is the current dependency Octave?
    if (strcmp (dep.package, "octave"))
      if (! compare_versions (OCTAVE_VERSION, dep.version, dep.operator))
        bad_deps{end+1} = dep;
      endif
      ## Is the current dependency not Octave?
    else
      ok = xor (false, uninstall_flag);
      for i = 1:length (installed_pkgs_lst)
        cur_name = installed_pkgs_lst{i}.name;
        cur_version = installed_pkgs_lst{i}.version;
        if (strcmp (dep.package, cur_name)
            && compare_versions (cur_version, dep.version, dep.operator))
          ok = xor (true, uninstall_flag);
          break;
        endif
      endfor
      if (! ok)
        bad_deps{end+1} = dep;
      endif
    endif
  endfor

endfunction
