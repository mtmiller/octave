////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1996-2020 The Octave Project Developers
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

#include "dir-ops.h"
#include "file-ops.h"
#include "lo-error.h"
#include "lo-sysdep.h"
#include "putenv-wrapper.h"
#include "uniconv-wrappers.h"
#include "unistd-wrappers.h"
#include "unsetenv-wrapper.h"

#if defined (OCTAVE_USE_WINDOWS_API)
#  include <windows.h>
#  include <wchar.h>

#  include "lo-hash.h"
#  include "filepos-wrappers.h"
#  include "unwind-prot.h"
#endif

namespace octave
{
  namespace sys
  {
    std::string
    getcwd (void)
    {
      std::string retval;

#if defined (OCTAVE_USE_WINDOWS_API)
      wchar_t *tmp = _wgetcwd (nullptr, 0);

      if (! tmp)
        (*current_liboctave_error_handler) ("unable to find current directory");

      std::wstring tmp_wstr (tmp);
      free (tmp);

      std::string tmp_str = u8_from_wstring (tmp_wstr);

      retval = tmp_str;

#else
      // Using octave_getcwd_wrapper ensures that we have a getcwd that
      // will allocate a buffer as large as necessary if buf and size are
      // both 0.

      char *tmp = octave_getcwd_wrapper (nullptr, 0);

      if (! tmp)
        (*current_liboctave_error_handler) ("unable to find current directory");

      retval = tmp;
      free (tmp);
#endif

      return retval;
    }

    int
    chdir (const std::string& path_arg)
    {
      std::string path = sys::file_ops::tilde_expand (path_arg);

#if defined (OCTAVE_USE_WINDOWS_API)
      if (path.length () == 2 && path[1] == ':')
        path += '\\';
#endif

      return octave_chdir_wrapper (path.c_str ());
    }

    bool
    get_dirlist (const std::string& dirname, string_vector& dirlist,
                 std::string& msg)
    {
      dirlist = "";
      msg = "";
#if defined (OCTAVE_USE_WINDOWS_API)
      _WIN32_FIND_DATAW ffd;

      std::string path_name (dirname);
      if (path_name.empty ())
        return true;

      if (path_name.back () == '\\' || path_name.back () == '/')
        path_name.push_back ('*');
      else
        path_name.append (R"(\*)");

      // Find first file in directory.
      std::wstring wpath_name = u8_to_wstring (path_name);
      HANDLE hFind = FindFirstFileW (wpath_name.c_str (), &ffd);
      if (INVALID_HANDLE_VALUE == hFind)
        {
          DWORD errCode = GetLastError ();
          char *errorText = nullptr;
          FormatMessageA (FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          nullptr, errCode,
                          MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                          reinterpret_cast <char *> (&errorText), 0, nullptr);
          if (errorText != nullptr)
            {
              msg = std::string (errorText);
              LocalFree (errorText);
            }
          return false;
        }

      std::list<std::string> dirlist_str;
      do
        dirlist_str.push_back (u8_from_wstring (ffd.cFileName));
      while (FindNextFileW (hFind, &ffd) != 0);

      FindClose(hFind);

      dirlist = string_vector (dirlist_str);

#else

      dir_entry dir (dirname);

      if (! dir)
        {
          msg = dir.error ();
          return false;
        }

      dirlist = dir.read ();

      dir.close ();
#endif

      return true;
    }

#if defined (OCTAVE_USE_WINDOWS_API)

    static bool check_fseek_ftell_workaround_needed (bool set_nonbuffered_mode)
    {
      // To check whether the workaround is needed:
      //
      //   * Create a tmp file with LF line endings only.
      //
      //   * Open that file for reading in text mode.
      //
      //   * Read a line.
      //
      //   * Use ftello to record the position of the beginning of the
      //     second line.
      //
      //   * Read and save the contents of the second line.
      //
      //   * Use fseeko to return to the saved position.
      //
      //   * Read the second line again and compare to the previously
      //     saved text.
      //
      //   * If the lines are different, we need to set non-buffered
      //     input mode for files opened in text mode.

      std::string tmpname = sys::tempnam ("", "oct-");

      if (tmpname.empty ())
        {
          (*current_liboctave_warning_handler)
            ("fseek/ftell bug check failed (tmp name creation)!");
          return false;
        }

      std::FILE *fptr = std::fopen (tmpname.c_str (), "wb");

      if (! fptr)
        {
          (*current_liboctave_warning_handler)
            ("fseek/ftell bug check failed (opening tmp file for writing)!");
          return false;
        }

      fprintf (fptr, "%s", "foo\nbar\nbaz\n");

      std::fclose (fptr);

      fptr = std::fopen (tmpname.c_str (), "rt");

      if (! fptr)
        {
          (*current_liboctave_warning_handler)
            ("fseek/ftell bug check failed (opening tmp file for reading)!");
          return false;
        }

      unwind_action act ([fptr, tmpname] () {
                           std::fclose (fptr);
                           sys::unlink (tmpname);
                         });

      if (set_nonbuffered_mode)
        ::setvbuf (fptr, nullptr, _IONBF, 0);

      while (true)
        {
          int c = fgetc (fptr);

          if (c == EOF)
            {
              (*current_liboctave_warning_handler)
                ("fseek/ftell bug check failed (skipping first line)!");
              return false;
            }

          if (c == '\n')
            break;
        }

      off_t pos = octave_ftello_wrapper (fptr);

      char buf1[8];
      int i = 0;
      while (true)
        {
          int c = fgetc (fptr);

          if (c == EOF)
            {
              (*current_liboctave_warning_handler)
                ("fseek/ftell bug check failed (reading second line)!");
              return false;
            }

          if (c == '\n')
            break;

          buf1[i++] = static_cast<char> (c);
        }
      buf1[i] = '\0';

      octave_fseeko_wrapper (fptr, pos, SEEK_SET);

      char buf2[8];
      i = 0;
      while (true)
        {
          int c = fgetc (fptr);

          if (c == EOF)
            {
              (*current_liboctave_warning_handler)
                ("fseek/ftell bug check failed (reading after repositioning)!");
              return false;
            }

          if (c == '\n')
            break;

          buf2[i++] = static_cast<char> (c);
        }
      buf2[i] = '\0';

      return strcmp (buf1, buf2);
    }

#endif

    std::FILE *
    fopen (const std::string& filename, const std::string& mode)
    {
#if defined (OCTAVE_USE_WINDOWS_API)

      std::wstring wfilename = u8_to_wstring (filename);
      std::wstring wmode = u8_to_wstring (mode);

      std::FILE *fptr = _wfopen (wfilename.c_str (), wmode.c_str ());

      static bool fseek_ftell_bug_workaround_needed = false;
      static bool fseek_ftell_bug_checked = false;

      if (! fseek_ftell_bug_checked && mode.find ('t') != std::string::npos)
        {
          // FIXME: Is the following workaround needed for all files
          // opened in text mode, or only for files opened for reading?

          // Try to avoid fseek/ftell bug on Windows systems by setting
          // non-buffered input mode for files opened in text mode, but
          // only if it appears that the workaround is needed.  See
          // Octave bug #58055.

          // To check whether the workaround is needed:
          //
          //   * Create a tmp file with LF line endings only.
          //
          //   * Open that file for reading in text mode.
          //
          //   * Read a line.
          //
          //   * Use ftello to record the position of the beginning of
          //     the second line.
          //
          //   * Read and save the contents of the second line.
          //
          //   * Use fseeko to return to the saved position.
          //
          //   * Read the second line again and compare to the
          //     previously saved text.
          //
          //   * If the lines are different, we need to set non-buffered
          //     input mode for files opened in text mode.
          //
          //   * To verify that the workaround solves the problem,
          //     repeat the above test with non-buffered input mode.  If
          //     that fails, warn that there may be trouble with
          //     ftell/fseek when reading files opened in text mode.

          if (check_fseek_ftell_workaround_needed (false))
            {
              if (check_fseek_ftell_workaround_needed (true))
                (*current_liboctave_warning_handler)
                  ("fseek/ftell may fail for files opened in text mode");
              else
                fseek_ftell_bug_workaround_needed = true;
            }

          fseek_ftell_bug_checked = true;
        }

      if (fseek_ftell_bug_workaround_needed
          && mode.find ('t') != std::string::npos)
        ::setvbuf (fptr, nullptr, _IONBF, 0);

      return fptr;

#else
      return std::fopen (filename.c_str (), mode.c_str ());
#endif
    }

    std::fstream
    fstream (const std::string& filename, const std::ios::openmode mode)
    {
#if defined (OCTAVE_USE_WINDOWS_API)

      std::wstring wfilename = u8_to_wstring (filename);

      return std::fstream (wfilename.c_str (), mode);

#else
      return std::fstream (filename.c_str (), mode);
#endif
    }

    std::ifstream
    ifstream (const std::string& filename, const std::ios::openmode mode)
    {
#if defined (OCTAVE_USE_WINDOWS_API)

      std::wstring wfilename = u8_to_wstring (filename);

      return std::ifstream (wfilename.c_str (), mode);

#else
      return std::ifstream (filename.c_str (), mode);
#endif
    }

    std::ofstream
    ofstream (const std::string& filename, const std::ios::openmode mode)
    {
#if defined (OCTAVE_USE_WINDOWS_API)

      std::wstring wfilename = u8_to_wstring (filename);

      return std::ofstream (wfilename.c_str (), mode);

#else
      return std::ofstream (filename.c_str (), mode);
#endif
    }

    void
    putenv_wrapper (const std::string& name, const std::string& value)
    {
      // This function was adapted from xputenv from Karl Berry's kpathsearch
      // library.
      // FIXME: make this do the right thing if we don't have a SMART_PUTENV.

      int new_len = name.length () + value.length () + 2;

      // FIXME: This leaks memory, but so would a call to setenv.
      // Short of extreme measures to track memory, altering the environment
      // always leaks memory, but the saving grace is that the leaks are small.

      char *new_item = static_cast<char *> (std::malloc (new_len));

      if (new_item)
        sprintf (new_item, "%s=%s", name.c_str (), value.c_str ());

      // As far as I can see there's no way to distinguish between the
      // various errors; putenv doesn't have errno values.

#if defined (OCTAVE_USE_WINDOWS_API)
      wchar_t *wnew_item = u8_to_wchar (new_item);
      unwind_protect frame;
      frame.add_fcn (std::free, static_cast<void *> (new_item));
      if (_wputenv (wnew_item) < 0)
        (*current_liboctave_error_handler) ("putenv (%s) failed", new_item);
#else
      if (octave_putenv_wrapper (new_item) < 0)
        (*current_liboctave_error_handler) ("putenv (%s) failed", new_item);
#endif
    }

    std::string
    getenv_wrapper (const std::string& name)
    {
#if defined (OCTAVE_USE_WINDOWS_API)
      std::wstring wname = u8_to_wstring (name);
      wchar_t *env = _wgetenv (wname.c_str ());
      return env ? u8_from_wstring (env) : "";
#else
      char *env = ::getenv (name.c_str ());
      return env ? env : "";
#endif
    }

    int
    unsetenv_wrapper (const std::string& name)
    {
#if defined (OCTAVE_USE_WINDOWS_API)
      putenv_wrapper (name, "");

      std::wstring wname = u8_to_wstring (name);
      return (SetEnvironmentVariableW (wname.c_str (), nullptr) ? 0 : -1);
#else
      return octave_unsetenv_wrapper (name.c_str ());
#endif
    }

    std::wstring
    u8_to_wstring (const std::string& utf8_string)
    {
      size_t srclen = utf8_string.length ();
      const uint8_t *src = reinterpret_cast<const uint8_t *>
                           (utf8_string.c_str ());

      size_t length = 0;
      wchar_t *wchar = reinterpret_cast<wchar_t *>
                       (octave_u8_conv_to_encoding ("wchar_t", src, srclen,
                                                    &length));

      std::wstring retval = L"";
      if (wchar != nullptr)
        {
          retval = std::wstring (wchar, length / sizeof (wchar_t));
          free (static_cast<void *> (wchar));
        }

      return retval;
    }

    std::string
    u8_from_wstring (const std::wstring& wchar_string)
    {
      size_t srclen = wchar_string.length () * sizeof (wchar_t);
      const char *src = reinterpret_cast<const char *> (wchar_string.c_str ());

      size_t length = 0;
      char *mbchar = reinterpret_cast<char *>
                     (octave_u8_conv_from_encoding ("wchar_t", src, srclen,
                                                    &length));

      std::string retval = "";
      if (mbchar != nullptr)
        {
          retval = std::string (mbchar, length);
          free (static_cast<void *> (mbchar));
        }

      return retval;
    }

    // At quite a few places in the code we are passing file names as
    // char arrays to external library functions.

    // When these functions try to locate the corresponding file on the
    // disc, they need to use the wide character API on Windows to
    // correctly open files with non-ASCII characters.

    // But they have no way of knowing which encoding we are using for
    // the passed string.  So they have no way of reliably converting to
    // a wchar_t array.  (I.e. there is no possible fix for these
    // functions with current C or C++.)

    // To solve the dilemma, the function "get_ASCII_filename" first
    // checks whether there are any non-ASCII characters in the passed
    // file name.  If there are not, it returns the original name.

    // Otherwise, it tries to obtain the short file name (8.3 naming
    // scheme) which only consists of ASCII characters and are safe to
    // pass.  However, short file names can be disabled for performance
    // reasons on the file system level with NTFS.  So there is no
    // guarantee that these exist.

    // If short file names are not stored, a hard link to the file is
    // created.  For this the path to the file is split at the deepest
    // possible level that doesn't contain non-ASCII characters.  At
    // that level a hidden folder is created that holds the hard links.
    // That means we need to have write access on that location.  A path
    // to that hard link is returned.

    // If the file system is FAT32, there are no hard links.  But FAT32
    // always stores short file names.  So we are safe.

    // ExFAT that is occasionally used on USB sticks and SD cards stores
    // neither short file names nor does it support hard links.  So for
    // exFAT with this function, there is (currently) no way to generate
    // a file name that is stripped from non-ASCII characters but still
    // is valid.

    // For Unixy systems, this function does nothing.

    std::string
    get_ASCII_filename (const std::string& orig_file_name)
    {
#if defined (OCTAVE_USE_WINDOWS_API)

      // Return file name that only contains ASCII characters that can
      // be used to access the file orig_file_name.  The original file
      // must exist in the file system before calling this function.
      // This is useful for passing file names to functions that are not
      // aware of the character encoding we are using.

      // 1. Check whether filename contains non-ASCII (UTF-8) characters.

      std::string::const_iterator first_non_ASCII
        = std::find_if (orig_file_name.begin (), orig_file_name.end (),
                        [](char c) { return (c < 0 || c >= 128); });

      if (first_non_ASCII == orig_file_name.end ())
        return orig_file_name;

      // 2. Check if file system stores short filenames (always
      // ASCII-only).

      std::wstring w_orig_file_name_str = u8_to_wstring (orig_file_name);
      const wchar_t *w_orig_file_name = w_orig_file_name_str.c_str ();

      // Get full path to file
      wchar_t w_full_file_name[_MAX_PATH];
      if (_wfullpath (w_full_file_name, w_orig_file_name, _MAX_PATH) == nullptr)
        return orig_file_name;

      std::wstring w_full_file_name_str = w_full_file_name;

      // Get short filename (8.3) from UTF-16 filename.

      long length = GetShortPathNameW (w_full_file_name, nullptr, 0);

      if (length > 0)
        {
          // Dynamically allocate the correct size (terminating null char
          // was included in length).

          wchar_t *w_short_file_name = new wchar_t[length];
          GetShortPathNameW (w_full_file_name, w_short_file_name, length);

          std::wstring w_short_file_name_str
            = std::wstring (w_short_file_name, length);
          std::string short_file_name = u8_from_wstring (w_short_file_name_str);

          if (w_short_file_name_str.compare (0, length-1, w_full_file_name_str) != 0)
            return short_file_name;
        }

      // 3. Create hard link with only-ASCII characters.
      // Get longest possible part of path that only contains ASCII chars.

      std::wstring::iterator w_first_non_ASCII
        = std::find_if (w_full_file_name_str.begin (), w_full_file_name_str.end (),
                        [](wchar_t c) { return (c < 0 || c >= 128); });
      std::wstring tmp_substr
        = std::wstring (w_full_file_name_str.begin (), w_first_non_ASCII);

      size_t pos
        = tmp_substr.find_last_of (u8_to_wstring (file_ops::dir_sep_chars ()));

      std::string par_dir
        = u8_from_wstring (w_full_file_name_str.substr (0, pos+1));

      // Create .oct_ascii directory.
      // FIXME: We need to have write permission in this location.

      std::string oct_ascii_dir = par_dir + ".oct_ascii";
      std::string test_dir = canonicalize_file_name (oct_ascii_dir);

      if (test_dir.empty ())
        {
          std::string msg;
          int status = sys::mkdir (oct_ascii_dir, 0777, msg);

          if (status < 0)
            return orig_file_name;

          // Set hidden property.
          SetFileAttributesA (oct_ascii_dir.c_str (), FILE_ATTRIBUTE_HIDDEN);
        }

      // Create file from hash of full filename.
      std::string filename_hash
        = (oct_ascii_dir + file_ops::dir_sep_str ()
           + crypto::hash ("SHA1", orig_file_name));

      std::string abs_filename_hash = canonicalize_file_name (filename_hash);

      if (! abs_filename_hash.empty ())
        sys::unlink (filename_hash);

      wchar_t w_filename_hash[filename_hash.length ()+1] = {0};

      for (size_t i=0; i < filename_hash.length (); i++)
        w_filename_hash[i] = filename_hash.at (i);

      if (CreateHardLinkW (w_filename_hash, w_orig_file_name, nullptr))
        return filename_hash;

#endif

      return orig_file_name;
    }
  }
}
