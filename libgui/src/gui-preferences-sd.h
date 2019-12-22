/*

Copyright (C) 2017-2019 Torsten <ttl-octave@mailbox.org>

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_gui_preferences_sd_h)
#define octave_gui_preferences_sd_h 1

#include "gui-preferences.h"

// Settings dialog

const gui_pref
sd_geometry ("settings/geometry", QVariant ());

const gui_pref
sd_last_tab ("settings/last_tab", QVariant (0));

const gui_pref
sd_last_editor_styles_tab ("settings/last_editor_styles_tab", QVariant (0));

#endif
