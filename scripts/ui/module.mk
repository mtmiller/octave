FCN_FILE_DIRS += ui

ui_FCN_FILES = \
  ui/errordlg.m \
  ui/helpdlg.m \
  ui/inputdlg.m \
  ui/listdlg.m \
  ui/msgbox.m \
  ui/questdlg.m \
  ui/warndlg.m

FCN_FILES += $(ui_FCN_FILES)

PKG_ADD_FILES += ui/PKG_ADD

DIRSTAMP_FILES += ui/$(octave_dirstamp)
