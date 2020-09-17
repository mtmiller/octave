////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009-2020 The Octave Project Developers
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

// Both header files are required outside of HAVE_GLP2S_H
#include "errwarn.h"
#include "gl2ps-print.h"

#if defined (HAVE_GL2PS_H) && defined (HAVE_OPENGL)

#include <cstdio>

#include <limits>

#include <gl2ps.h>

#include "file-ops.h"
#include "lo-mappers.h"
#include "oct-locbuf.h"
#include "tmpfile-wrapper.h"
#include "unistd-wrappers.h"
#include "unistr-wrappers.h"
#include "unwind-prot.h"

#include "gl-render.h"
#include "interpreter-private.h"
#include "oct-opengl.h"
#include "sighandlers.h"
#include "sysdep.h"
#include "text-renderer.h"

namespace octave
{
  static void
  safe_pclose (FILE *f)
  {
    if (f)
      octave::pclose (f);
  }

  static void
  safe_fclose (FILE *f)
  {
    if (f)
      std::fclose (f);
  }

  class
  OCTINTERP_API
  gl2ps_renderer : public opengl_renderer
  {
  public:

    gl2ps_renderer (opengl_functions& glfcns, FILE *_fp,
                    const std::string& _term)
      : opengl_renderer (glfcns), fp (_fp), term (_term),
        fontsize (), fontname (), buffer_overflow (false)
    { }

    ~gl2ps_renderer (void) = default;

    // FIXME: should we import the functions from the base class and
    // overload them here, or should we use a different name so we don't
    // have to do this?  Without the using declaration or a name change,
    // the base class functions will be hidden.  That may be OK, but it
    // can also cause some confusion.
    using opengl_renderer::draw;

    void draw (const graphics_object& go, const std::string& print_cmd);

  protected:

    Matrix render_text (const std::string& txt,
                        double x, double y, double z,
                        int halign, int valign, double rotation = 0.0);

    void set_font (const base_properties& props);

    static bool has_alpha (const graphics_handle& h)
    {
      bool retval = false;

      gh_manager& gh_mgr = __get_gh_manager__ ("gl2ps_renderer::has_alpha");

      graphics_object go = gh_mgr.get_object (h);

      if (! go.valid_object ())
        return retval;

      if (go.isa ("axes") || go.isa ("hggroup"))
        {
          Matrix  children = go.get ("children").matrix_value ();
          for (octave_idx_type ii = 0; ii < children.numel (); ii++)
            {
              retval = has_alpha (graphics_handle (children(ii)));
              if (retval)
                break;
            }
        }
      else if (go.isa ("patch") || go.isa ("surface"))
        {
          octave_value fa = go.get ("facealpha");
          if (fa.is_scalar_type () && fa.is_double_type ()
              && fa.double_value () < 1)
            retval = true;
        }
      else if (go.isa ("scatter"))
        {
          octave_value fa = go.get ("markerfacealpha");
          if (fa.is_scalar_type () && fa.is_double_type ()
              && fa.double_value () < 1)
            retval = true;
        }

      return retval;
    }

    void draw_axes (const axes::properties& props)
    {
      // Initialize a sorting tree (viewport) in gl2ps for each axes
      GLint vp[4];
      m_glfcns.glGetIntegerv (GL_VIEWPORT, vp);
      gl2psBeginViewport (vp);


      // Don't remove hidden primitives when some of them are transparent
      GLint opts;
      gl2psGetOptions (&opts);
      if (has_alpha (props.get___myhandle__ ()))
        {
          opts &= ~GL2PS_OCCLUSION_CULL;
          // FIXME: currently the GL2PS_BLEND (which is more an equivalent of
          // GL_ALPHA_TEST than GL_BLEND) is not working on a per primitive
          // basis.  We thus set it once per viewport.
          gl2psEnable (GL2PS_BLEND);
        }
      else
        {
          opts |= GL2PS_OCCLUSION_CULL;
          gl2psDisable (GL2PS_BLEND);
        }

      gl2psSetOptions (opts);

      // Draw and finish () or there may be primitives missing in the gl2ps
      // output.
      opengl_renderer::draw_axes (props);
      finish ();

      // Finalize viewport
      GLint state = gl2psEndViewport ();
      if (state == GL2PS_NO_FEEDBACK && props.is_visible ())
        warning ("gl2ps_renderer::draw_axes: empty feedback buffer and/or nothing else to print");
      else if (state == GL2PS_ERROR)
        error ("gl2ps_renderer::draw_axes: gl2psEndPage returned GL2PS_ERROR");

      buffer_overflow |= (state == GL2PS_OVERFLOW);

      // Don't draw background for subsequent viewports (legends, subplots,
      // etc.)
      gl2psGetOptions (&opts);
      opts &= ~GL2PS_DRAW_BACKGROUND;
      gl2psSetOptions (opts);
    }

    void draw_text (const text::properties& props);

    void draw_image (const image::properties& props);
    void draw_pixels (int w, int h, const float *data);
    void draw_pixels (int w, int h, const uint8_t *data);
    void draw_pixels (int w, int h, const uint16_t *data);

    void init_marker (const std::string& m, double size, float width)
    {
      opengl_renderer::init_marker (m, size, width);

      // FIXME: gl2ps can't handle closed contours so we set linecap/linejoin
      //        round to obtain a better looking result for some markers.
      if (m == "o" || m == "v" || m == "^" || m == ">" || m == "<" || m == "h"
          || m == "hexagram" || m == "p" || m == "pentagram")
        {
          set_linejoin ("round");
          set_linecap ("round");
        }
      else
        {
          set_linejoin ("miter");
          set_linecap ("square");
        }
    }

    void set_linestyle (const std::string& s, bool use_stipple = false,
                        double linewidth = 0.5)
    {
      opengl_renderer::set_linestyle (s, use_stipple, linewidth);

      if (s == "-" && ! use_stipple)
        gl2psDisable (GL2PS_LINE_STIPPLE);
      else
        gl2psEnable (GL2PS_LINE_STIPPLE);
    }

    void set_linecap (const std::string& s)
    {
      opengl_renderer::set_linejoin (s);

#if defined (HAVE_GL2PSLINEJOIN)
      if (s == "butt")
        gl2psLineCap (GL2PS_LINE_CAP_BUTT);
      else if (s == "square")
        gl2psLineCap (GL2PS_LINE_CAP_SQUARE);
      else if (s == "round")
        gl2psLineCap (GL2PS_LINE_CAP_ROUND);
#endif
    }

    void set_linejoin (const std::string& s)
    {
      opengl_renderer::set_linejoin (s);

#if defined (HAVE_GL2PSLINEJOIN)
      if (s == "round")
        gl2psLineJoin (GL2PS_LINE_JOIN_ROUND);
      else if (s == "miter")
        gl2psLineJoin (GL2PS_LINE_JOIN_MITER);
      else if (s == "chamfer")
        gl2psLineJoin (GL2PS_LINE_JOIN_BEVEL);
#endif
    }

    void set_polygon_offset (bool on, float offset = 0.0f)
    {
      if (on)
        {
          opengl_renderer::set_polygon_offset (on, offset);
          gl2psEnable (GL2PS_POLYGON_OFFSET_FILL);
        }
      else
        {
          gl2psDisable (GL2PS_POLYGON_OFFSET_FILL);
          opengl_renderer::set_polygon_offset (on, offset);
        }
    }

    void set_linewidth (float w)
    {
      gl2psLineWidth (w);
    }

  private:

    // Use xform to compute the coordinates of the string list
    // that have been parsed by freetype.
    void fix_strlist_position (double x, double y, double z,
                               Matrix box, double rotation,
                               std::list<text_renderer::string>& lst);

    // Build an svg text element from a list of parsed strings.
    std::string strlist_to_svg (double x, double y, double z, Matrix box,
                                double rotation,
                                std::list<text_renderer::string>& lst);

    // Build a list of postscript commands from a list of parsed strings.
    std::string strlist_to_ps (double x, double y, double z, Matrix box,
                               double rotation,
                               std::list<text_renderer::string>& lst);

    int alignment_to_mode (int ha, int va) const;

    FILE *fp;
    caseless_str term;
    double fontsize;
    std::string fontname;
    bool buffer_overflow;
  };

  static bool
  has_2D_axes (const graphics_handle& h)
  {
    bool retval = true;

    gh_manager& gh_mgr = __get_gh_manager__ ("gl2ps_renderer::has_2D_axes");

    graphics_object go = gh_mgr.get_object (h);

    if (! go.valid_object ())
      return retval;

    if (go.isa ("figure") || go.isa ("uipanel"))
      {
        Matrix  children = go.get ("children").matrix_value ();
        for (octave_idx_type ii = 0; ii < children.numel (); ii++)
          {
            retval = has_2D_axes (graphics_handle (children(ii)));
            if (! retval)
              break;
          }
      }
    else if (go.isa ("axes"))
      {
        axes::properties& ap
          = reinterpret_cast<axes::properties&> (go.get_properties ());
        retval = ap.get_is2D (true);
      }

    return retval;
  }

  void
  gl2ps_renderer::draw (const graphics_object& go, const std::string& print_cmd)
  {
    static bool in_draw = false;
    static std::string old_print_cmd;
    static GLint buffsize;

    if (! in_draw)
      {
        unwind_protect frame;

        frame.protect_var (in_draw);

        in_draw = true;

        GLint gl2ps_term = GL2PS_PS;
        if (term.find ("eps") != std::string::npos)
          gl2ps_term = GL2PS_EPS;
        else if (term.find ("pdf") != std::string::npos)
          gl2ps_term = GL2PS_PDF;
        else if (term.find ("ps") != std::string::npos)
          gl2ps_term = GL2PS_PS;
        else if (term.find ("svg") != std::string::npos)
          gl2ps_term = GL2PS_SVG;
        else if (term.find ("pgf") != std::string::npos)
          gl2ps_term = GL2PS_PGF;
        else if (term.find ("tex") != std::string::npos)
          gl2ps_term = GL2PS_TEX;
        else
          warning ("gl2ps_renderer::draw: Unknown terminal %s, using 'ps'",
                   term.c_str ());

        GLint gl2ps_text = 0;
        if (term.find ("notxt") != std::string::npos)
          gl2ps_text = GL2PS_NO_TEXT;

        // Default sort order optimizes for 3D plots
        GLint gl2ps_sort = GL2PS_BSP_SORT;

        // FIXME: gl2ps does not provide a way to change the sorting algorithm
        // on a viewport basis, we thus disable sorting only if all axes are 2D
        if (has_2D_axes (go.get ("__myhandle__")))
          gl2ps_sort = GL2PS_NO_SORT;

        // Use a temporary file in case an overflow happens
        FILE *tmpf = octave_tmpfile_wrapper ();

        if (! tmpf)
          error ("gl2ps_renderer::draw: couldn't open temporary file for printing");

        frame.add_fcn (safe_fclose, tmpf);

        // Reset buffsize, unless this is 2nd pass of a texstandalone print.
        if (term.find ("tex") == std::string::npos)
          buffsize = 2*1024*1024;
        else
          buffsize /= 2;

        buffer_overflow = true;

        while (buffer_overflow)
          {
            buffer_overflow = false;
            buffsize *= 2;

            std::fseek (tmpf, 0, SEEK_SET);
            octave_ftruncate_wrapper (fileno (tmpf), 0);

            // For LaTeX output the print process uses 2 drawnow() commands.
            // The first one is for the pdf/ps/eps graph to be included.  The
            // print_cmd is saved as old_print_cmd.  Then the second drawnow()
            // outputs the tex-file and the graphic filename to be included is
            // extracted from old_print_cmd.

            std::string include_graph;

            size_t found_redirect = old_print_cmd.find ('>');

            if (found_redirect != std::string::npos)
              include_graph = old_print_cmd.substr (found_redirect + 1);
            else
              include_graph = old_print_cmd;

            size_t n_begin = include_graph.find_first_not_of (R"( "')");

            if (n_begin != std::string::npos)
              {
                // Strip any quote characters characters around filename
                size_t n_end = include_graph.find_last_not_of (R"( "')");
                include_graph = include_graph.substr (n_begin,
                                                      n_end - n_begin + 1);
                // Strip path from filename
                n_begin = include_graph.find_last_of (octave::sys::file_ops::dir_sep_chars ());
                include_graph = include_graph.substr (n_begin + 1);
              }
            else
              include_graph = "foobar-inc";

            // FIXME: workaround gl2ps drawing 2 background planes, the first
            //        eventually being black and producing visual artifacts
            const figure::properties& fprop
              = dynamic_cast<const figure::properties&> (go.get_properties ());
            Matrix c = fprop.get_color_rgb ();
            m_glfcns.glClearColor (c(0), c(1), c(2), 1);

            // Allow figures to be printed at arbitrary resolution
            set_device_pixel_ratio (fprop.get___device_pixel_ratio__ ());

            // GL2PS_SILENT was removed to allow gl2ps to print errors on stderr
            GLint ret = gl2psBeginPage ("gl2ps_renderer figure", "Octave",
                                        nullptr, gl2ps_term, gl2ps_sort,
                                        (GL2PS_BEST_ROOT
                                         | gl2ps_text
                                         | GL2PS_DRAW_BACKGROUND
                                         | GL2PS_NO_PS3_SHADING
                                         | GL2PS_USE_CURRENT_VIEWPORT),
                                        GL_RGBA, 0, nullptr, 0, 0, 0,
                                        buffsize, tmpf, include_graph.c_str ());
            if (ret == GL2PS_ERROR)
              {
                old_print_cmd.clear ();
                error ("gl2ps_renderer::draw: gl2psBeginPage returned GL2PS_ERROR");
              }

            opengl_renderer::draw (go);

            if (buffer_overflow)
              warning ("gl2ps_renderer::draw: retrying with buffer size: %.1E B\n", double (2*buffsize));

            if (! buffer_overflow)
              old_print_cmd = print_cmd;

            // Don't check return value of gl2psEndPage, it is not meaningful.
            // Errors and warnings are checked after gl2psEndViewport in
            // gl2ps_renderer::draw_axes instead.
            gl2psEndPage ();
          }

        // Copy temporary file to pipe
        std::fseek (tmpf, 0, SEEK_SET);
        char str[8192];  // 8 kB is a common kernel buffersize
        size_t nread, nwrite;
        nread = 1;

        // In EPS terminal read the header line by line and insert a
        // new procedure
        const char* fcn = "/SRX  { gsave FCT moveto rotate xshow grestore } BD\n";
        bool header_found = ! (term.find ("eps") != std::string::npos
                               || term.find ("svg") != std::string::npos);

        while (! feof (tmpf) && nread)
          {
            if (! header_found && std::fgets (str, 8192, tmpf))
              nread = strlen (str);
            else
              nread = std::fread (str, 1, 8192, tmpf);

            if (nread)
              {
                if (! header_found && std::strncmp (str, "/SBCR", 5) == 0)
                  {
                    header_found = true;
                    nwrite = std::fwrite (fcn, 1, strlen (fcn), fp);
                    if (nwrite != strlen (fcn))
                      {
                        // FIXME: is this the best thing to do here?
                        respond_to_pending_signals ();
                        error ("gl2ps_renderer::draw: internal pipe error");
                      }
                  }
                else if (! header_found
                         && term.find ("svg") != std::string::npos)
                  {
                    // FIXME: gl2ps uses pixel units for SVG format.
                    //        Modify resulting svg to use points instead.
                    //        Remove this "else if" block, and
                    //        make header_found true for SVG if gl2ps is fixed.
                    std::string srchstr (str);
                    size_t pos = srchstr.find ("px");
                    if (pos != std::string::npos)
                      {
                        header_found = true;
                        srchstr[pos+1] = 't';  // "px" -> "pt"
                        // Assume the second occurrence is at the same line
                        pos = srchstr.find ("px", pos);
                        srchstr[pos+1] = 't';  // "px" -> "pt"
                        std::strcpy (str, srchstr.c_str ());
                      }
                  }

                nwrite = std::fwrite (str, 1, nread, fp);
                if (nwrite != nread)
                  {
                    // FIXME: is this the best thing to do here?
                    respond_to_pending_signals ();   // Clear SIGPIPE signal
                    error ("gl2ps_renderer::draw: internal pipe error");
                  }
              }
          }
      }
    else
      opengl_renderer::draw (go);
  }

  int
  gl2ps_renderer::alignment_to_mode (int ha, int va) const
  {
    int gl2psa = GL2PS_TEXT_BL;

    if (ha == 0)
      {
        if (va == 0 || va == 3)
          gl2psa=GL2PS_TEXT_BL;
        else if (va == 2)
          gl2psa=GL2PS_TEXT_TL;
        else if (va == 1)
          gl2psa=GL2PS_TEXT_CL;
      }
    else if (ha == 2)
      {
        if (va == 0 || va == 3)
          gl2psa=GL2PS_TEXT_BR;
        else if (va == 2)
          gl2psa=GL2PS_TEXT_TR;
        else if (va == 1)
          gl2psa=GL2PS_TEXT_CR;
      }
    else if (ha == 1)
      {
        if (va == 0 || va == 3)
          gl2psa=GL2PS_TEXT_B;
        else if (va == 2)
          gl2psa=GL2PS_TEXT_T;
        else if (va == 1)
          gl2psa=GL2PS_TEXT_C;
      }

    return gl2psa;
  }

  void
  gl2ps_renderer::fix_strlist_position (double x, double y, double z,
                                        Matrix box, double rotation,
                                        std::list<text_renderer::string>& lst)
  {
    for (auto& txtobj : lst)
      {
        // Get pixel coordinates
        ColumnVector coord_pix = get_transform ().transform (x, y, z, false);

        // Translate and rotate
        double rot = rotation * 4.0 * atan (1.0) / 180;
        coord_pix(0) += (txtobj.get_x () + box(0))*cos (rot)
                        - (txtobj.get_y () + box(1))*sin (rot);
        coord_pix(1) -= (txtobj.get_y () + box(1))*cos (rot)
                        + (txtobj.get_x () + box(0))*sin (rot);

        GLint vp[4];
        m_glfcns.glGetIntegerv (GL_VIEWPORT, vp);

        txtobj.set_x (coord_pix(0));
        txtobj.set_y (vp[3] - coord_pix(1));
        txtobj.set_z (coord_pix(2));
      }
  }

  static std::string
  code_to_symbol (uint32_t code)
  {
    std::string retval;

    uint32_t idx = code - 945;
    if (idx < 25)
      {
        std::string characters ("abgdezhqiklmnxoprVstufcyw");
        retval = characters[idx];
        return retval;
      }

    idx = code - 913;
    if (idx < 25)
      {
        std::string characters ("ABGDEZHQIKLMNXOPRVSTUFCYW");
        retval = characters[idx];
      }
    else if (code == 978)
      retval = "U";
    else if (code == 215)
      retval = "\xb4";
    else if (code == 177)
      retval = "\xb1";
    else if (code == 8501)
      retval = "\xc0";
    else if (code == 8465)
      retval = "\xc1";
    else if (code == 8242)
      retval = "\xa2";
    else if (code == 8736)
      retval = "\xd0";
    else if (code == 172)
      retval = "\xd8";
    else if (code == 9829)
      retval = "\xa9";
    else if (code == 8472)
      retval = "\xc3";
    else if (code == 8706)
      retval = "\xb6";
    else if (code == 8704)
      retval = "\x22";
    else if (code == 9827)
      retval = "\xa7";
    else if (code == 9824)
      retval = "\xaa";
    else if (code == 8476)
      retval = "\xc2";
    else if (code == 8734)
      retval = "\xa5";
    else if (code == 8730)
      retval = "\xd6";
    else if (code == 8707)
      retval = "\x24";
    else if (code == 9830)
      retval = "\xa8";
    else if (code == 8747)
      retval = "\xf2";
    else if (code == 8727)
      retval = "\x2a";
    else if (code == 8744)
      retval = "\xda";
    else if (code == 8855)
      retval = "\xc4";
    else if (code == 8901)
      retval = "\xd7";
    else if (code == 8728)
      retval = "\xb0";
    else if (code == 8745)
      retval = "\xc7";
    else if (code == 8743)
      retval = "\xd9";
    else if (code == 8856)
      retval = "\xc6";
    else if (code == 8729)
      retval = "\xb7";
    else if (code == 8746)
      retval = "\xc8";
    else if (code == 8853)
      retval = "\xc5";
    else if (code == 8804)
      retval = "\xa3";
    else if (code == 8712)
      retval = "\xce";
    else if (code == 8839)
      retval = "\xca";
    else if (code == 8801)
      retval = "\xba";
    else if (code == 8773)
      retval = "\x40";
    else if (code == 8834)
      retval = "\xcc";
    else if (code == 8805)
      retval = "\xb3";
    else if (code == 8715)
      retval = "\x27";
    else if (code == 8764)
      retval = "\x7e";
    else if (code == 8733)
      retval = "\xb5";
    else if (code == 8838)
      retval = "\xcd";
    else if (code == 8835)
      retval = "\xc9";
    else if (code == 8739)
      retval = "\xbd";
    else if (code == 8776)
      retval = "\xbb";
    else if (code == 8869)
      retval = "\x5e";
    else if (code == 8656)
      retval = "\xdc";
    else if (code == 8592)
      retval = "\xac";
    else if (code == 8658)
      retval = "\xde";
    else if (code == 8594)
      retval = "\xae";
    else if (code == 8596)
      retval = "\xab";
    else if (code == 8593)
      retval = "\xad";
    else if (code == 8595)
      retval = "\xaf";
    else if (code == 8970)
      retval = "\xeb";
    else if (code == 8971)
      retval = "\xfb";
    else if (code == 10216)
      retval = "\xe1";
    else if (code == 10217)
      retval = "\xf1";
    else if (code == 8968)
      retval = "\xe9";
    else if (code == 8969)
      retval = "\xf9";
    else if (code == 8800)
      retval = "\xb9";
    else if (code == 8230)
      retval = "\xbc";
    else if (code == 176)
      retval = "\xb0";
    else if (code == 8709)
      retval = "\xc6";
    else if (code == 169)
      retval = "\xd3";

    if (retval.empty ())
      warning ("print: unhandled symbol %d", code);

    return retval;
  }

  static std::string
  select_font (caseless_str fn, bool isbold, bool isitalic)
  {
    std::transform (fn.begin (), fn.end (), fn.begin (), ::tolower);
    std::string fontname;
    if (fn == "times" || fn == "times-roman")
      {
        if (isitalic && isbold)
          fontname = "Times-BoldItalic";
        else if (isitalic)
          fontname = "Times-Italic";
        else if (isbold)
          fontname = "Times-Bold";
        else
          fontname = "Times-Roman";
      }
    else if (fn == "courier")
      {
        if (isitalic && isbold)
          fontname = "Courier-BoldOblique";
        else if (isitalic)
          fontname = "Courier-Oblique";
        else if (isbold)
          fontname = "Courier-Bold";
        else
          fontname = "Courier";
      }
    else if (fn == "symbol")
      fontname = "Symbol";
    else if (fn == "zapfdingbats")
      fontname = "ZapfDingbats";
    else
      {
        if (isitalic && isbold)
          fontname = "Helvetica-BoldOblique";
        else if (isitalic)
          fontname = "Helvetica-Oblique";
        else if (isbold)
          fontname = "Helvetica-Bold";
        else
          fontname = "Helvetica";
      }
    return fontname;
  }

  static void
  escape_character (const std::string chr, std::string& str)
  {
    std::size_t idx = str.find (chr);
    while (idx != std::string::npos)
      {
        str.insert (idx, 1, '\\');
        idx = str.find (chr, idx + 2);
      }
  }

  std::string
  gl2ps_renderer::strlist_to_svg (double x, double y, double z,
                                  Matrix box, double rotation,
                                  std::list<text_renderer::string>& lst)
  {
    if (lst.empty ())
      return "";

    //Use pixel coordinates to conform to gl2ps
    ColumnVector coord_pix = get_transform ().transform (x, y, z, false);

    std::ostringstream os;
    os << R"(<text xml:space="preserve" )";

    // Rotation and translation are applied to the whole text element
    os << "transform=\""
       << "translate(" << coord_pix(0) + box(0) << "," << coord_pix(1) - box(1)
       << ") rotate(" << -rotation << "," << -box(0) << "," << box(1)
       << ")\" ";

    // Use the first entry for the base text font
    auto p = lst.begin ();
    std::string name = p->get_family ();
    std::string weight = p->get_weight ();
    std::string angle = p->get_angle ();
    double size = p->get_size ();

    os << "font-family=\"" << name << "\" "
       << "font-weight=\"" << weight << "\" "
       << "font-style=\"" << angle << "\" "
       << "font-size=\"" << size << "\">";


    // build a tspan for each element in the strlist
    for (p = lst.begin (); p != lst.end (); p++)
      {
        os << "<tspan ";

        if (name.compare (p->get_family ()))
          os << "font-family=\"" << p->get_family () << "\" ";

        if (weight.compare (p->get_weight ()))
          os << "font-weight=\"" << p->get_weight () << "\" ";

        if (angle.compare (p->get_angle ()))
          os << "font-style=\"" << p->get_angle () << "\" ";

        if (size != p->get_size ())
          os << "font-size=\"" << p->get_size () << "\" ";

        os << "y=\"" << - p->get_y () << "\" ";

        Matrix col = p->get_color ();
        os << "fill=\"rgb(" << col(0)*255 << ","
           << col(1)*255 << "," << col(2)*255 << ")\" ";

        // provide an x coordinate for each character in the string
        os << "x=\"";
        std::vector<double> xdata = p->get_xdata ();
        for (auto q = xdata.begin (); q != xdata.end (); q++)
          os << (*q) << " ";
        os << '"';

        os << '>';

        // translate unicode and special xml characters
        if (p->get_code ())
          os << "&#" << p->get_code () <<  ";";
        else
          {
            const std::string str = p->get_string ();
            for (auto q = str.begin (); q != str.end (); q++)
              {
                std::stringstream chr;
                chr << *q;
                if (chr.str () == "\"")
                  os << "&quot;";
                else if (chr.str () == "'")
                  os << "&apos;";
                else if (chr.str () == "&")
                  os << "&amp;";
                else if (chr.str () == "<")
                  os << "&lt;";
                else if (chr.str () == ">")
                  os << "&gt;";
                else
                  os << chr.str ();
              }
          }
        os << "</tspan>";
      }
    os << "</text>";

    return os.str ();
  }

  std::string
  gl2ps_renderer::strlist_to_ps (double x, double y, double z,
                                 Matrix box, double rotation,
                                 std::list<text_renderer::string>& lst)
  {
    // Translate and rotate coordinates in order to use bottom-left alignment
    fix_strlist_position (x, y, z, box, rotation, lst);
    Matrix prev_color (1, 3, -1);

    std::ostringstream ss;
    ss << "gsave\n";

    static bool warned = false;

    for (const auto& txtobj : lst)
      {
        // Color
        if (txtobj.get_color () != prev_color)
          {
            prev_color = txtobj.get_color ();
            for (int i = 0; i < 3; i++)
              ss << prev_color(i) << " ";

            ss << "C\n";
          }

        // String
        std::string str;
        if (txtobj.get_code ())
          {
            fontname = "Symbol";
            str = code_to_symbol (txtobj.get_code ());
          }
        else
          {
            fontname = select_font (txtobj.get_name (),
                                    txtobj.get_weight () == "bold",
                                    txtobj.get_angle () == "italic");

            // Check that the string is composed of single byte characters
            const std::string tmpstr = txtobj.get_string ();
            const uint8_t *c
              = reinterpret_cast<const uint8_t *> (tmpstr.c_str ());

            for (size_t i = 0; i < tmpstr.size ();)
              {
                int mblen = octave_u8_strmblen_wrapper (c + i);

                // Replace multibyte or non ascii characters by a question mark
                if (mblen > 1)
                  {
                    str += "?";
                    if (! warned)
                      {
                        warning_with_id ("Octave:print:unsupported-multibyte",
                                         "print: only ASCII characters are "
                                         "supported for EPS and derived "
                                         "formats.");
                        warned = true;
                      }
                  }
                else if (mblen < 1)
                  {
                    mblen = 1;
                    str += "?";
                    if (! warned)
                      {
                        warning_with_id ("Octave:print:unhandled-character",
                                         "print: only ASCII characters are "
                                         "supported for EPS and derived "
                                         "formats.");
                        warned = true;
                      }
                  }
                else
                  str += tmpstr.at (i);

                i += mblen;
              }
          }

        escape_character ("\\", str);
        escape_character ("(", str);
        escape_character (")", str);

        ss << "(" << str << ") [";

        std::vector<double> xdata = txtobj.get_xdata ();
        for (size_t i = 1; i < xdata.size (); i++)
          ss << xdata[i] - xdata[i-1] << " ";

        ss << "10] " << rotation << " " << txtobj.get_x ()
           << " " << txtobj.get_y () << " " << txtobj.get_size ()
           << " /" << fontname << " SRX\n";
      }

    ss << "grestore\n";

    return ss.str ();
  }

  Matrix
  gl2ps_renderer::render_text (const std::string& txt,
                               double x, double y, double z,
                               int ha, int va, double rotation)
  {
    std::string saved_font = fontname;

    if (txt.empty ())
      return Matrix (1, 4, 0.0);

    Matrix bbox;
    std::string str = txt;
    std::list<text_renderer::string> lst;

    text_to_strlist (str, lst, bbox, ha, va, rotation);
    m_glfcns.glRasterPos3d (x, y, z);

    // For svg/eps directly dump a preformated text element into gl2ps output
    if (term.find ("svg") != std::string::npos)
      {
        std::string elt = strlist_to_svg (x, y, z, bbox, rotation, lst);
        if (! elt.empty ())
          gl2psSpecial (GL2PS_SVG, elt.c_str ());
      }
    else if (term.find ("eps") != std::string::npos)
      {
        std::string elt = strlist_to_ps (x, y, z, bbox, rotation, lst);
        if (! elt.empty ())
          gl2psSpecial (GL2PS_EPS, elt.c_str ());

      }
    else
      gl2psTextOpt (str.c_str (), fontname.c_str (), fontsize,
                    alignment_to_mode (ha, va), rotation);

    fontname = saved_font;

    return bbox;
  }

  void
  gl2ps_renderer::set_font (const base_properties& props)
  {
    opengl_renderer::set_font (props);

    // Set the interpreter so that text_to_pixels can parse strings properly
    if (props.has_property ("interpreter"))
      set_interpreter (props.get ("interpreter").string_value ());

    fontsize = props.get ("__fontsize_points__").double_value ();

    caseless_str fn = props.get ("fontname").xtolower ().string_value ();
    bool isbold
      =(props.get ("fontweight").xtolower ().string_value () == "bold");
    bool isitalic
      = (props.get ("fontangle").xtolower ().string_value () == "italic");

    fontname = select_font (fn, isbold, isitalic);
  }

  void
  gl2ps_renderer::draw_image (const image::properties& props)
  {
    octave_value cdata = props.get_color_data ();
    dim_vector dv (cdata.dims ());
    int h = dv(0);
    int w = dv(1);

    Matrix x = props.get_xdata ().matrix_value ();
    Matrix y = props.get_ydata ().matrix_value ();

    // Someone wants us to draw an empty image?  No way.
    if (x.isempty () || y.isempty ())
      return;

    // Sort x/ydata and mark flipped dimensions
    bool xflip = false;
    if (x(0) > x(1))
      {
        std::swap (x(0), x(1));
        xflip = true;
      }
    else if (w > 1 && x(1) == x(0))
      x(1) = x(1) + (w-1);

    bool yflip = false;
    if (y(0) > y(1))
      {
        std::swap (y(0), y(1));
        yflip = true;
      }
    else if (h > 1 && y(1) == y(0))
      y(1) = y(1) + (h-1);


    const ColumnVector p0 = xform.transform (x(0), y(0), 0);
    const ColumnVector p1 = xform.transform (x(1), y(1), 0);

    if (math::isnan (p0(0)) || math::isnan (p0(1))
        || math::isnan (p1(0)) || math::isnan (p1(1)))
      {
        warning ("opengl_renderer: image X,Y data too large to draw");
        return;
      }

    // image pixel size in screen pixel units
    float pix_dx, pix_dy;
    // image pixel size in normalized units
    float nor_dx, nor_dy;

    if (w > 1)
      {
        pix_dx = (p1(0) - p0(0)) / (w-1);
        nor_dx = (x(1) - x(0)) / (w-1);
      }
    else
      {
        const ColumnVector p1w = xform.transform (x(1) + 1, y(1), 0);
        pix_dx = p1w(0) - p0(0);
        nor_dx = 1;
      }

    if (h > 1)
      {
        pix_dy = (p1(1) - p0(1)) / (h-1);
        nor_dy = (y(1) - y(0)) / (h-1);
      }
    else
      {
        const ColumnVector p1h = xform.transform (x(1), y(1) + 1, 0);
        pix_dy = p1h(1) - p0(1);
        nor_dy = 1;
      }

    // OpenGL won't draw any of the image if its origin is outside the
    // viewport/clipping plane so we must do the clipping ourselves.

    int j0, j1, jj, i0, i1, ii;
    j0 = 0, j1 = w;
    i0 = 0, i1 = h;

    float im_xmin = x(0) - nor_dx/2;
    float im_xmax = x(1) + nor_dx/2;
    float im_ymin = y(0) - nor_dy/2;
    float im_ymax = y(1) + nor_dy/2;

    // Clip to axes or viewport
    bool do_clip = props.is_clipping ();
    Matrix vp = get_viewport_scaled ();

    ColumnVector vp_lim_min
      = xform.untransform (std::numeric_limits <float>::epsilon (),
                           std::numeric_limits <float>::epsilon ());
    ColumnVector vp_lim_max = xform.untransform (vp(2), vp(3));

    if (vp_lim_min(0) > vp_lim_max(0))
      std::swap (vp_lim_min(0), vp_lim_max(0));

    if (vp_lim_min(1) > vp_lim_max(1))
      std::swap (vp_lim_min(1), vp_lim_max(1));

    float clip_xmin
      = do_clip ? (vp_lim_min(0) > xmin ? vp_lim_min(0) : xmin) : vp_lim_min(0);

    float clip_ymin
      = do_clip ? (vp_lim_min(1) > ymin ? vp_lim_min(1) : ymin) : vp_lim_min(1);

    float clip_xmax
      = do_clip ? (vp_lim_max(0) < xmax ? vp_lim_max(0) : xmax) : vp_lim_max(0);

    float clip_ymax
      = do_clip ? (vp_lim_max(1) < ymax ? vp_lim_max(1) : ymax) : vp_lim_max(1);

    if (im_xmin < clip_xmin)
      j0 += (clip_xmin - im_xmin)/nor_dx + 1;

    if (im_xmax > clip_xmax)
      j1 -= (im_xmax - clip_xmax)/nor_dx;

    if (im_ymin < clip_ymin)
      i0 += (clip_ymin - im_ymin)/nor_dy + 1;

    if (im_ymax > clip_ymax)
      i1 -= (im_ymax - clip_ymax)/nor_dy;

    if (i0 >= i1 || j0 >= j1)
      return;

    float zoom_x;
    m_glfcns.glGetFloatv (GL_ZOOM_X, &zoom_x);
    float zoom_y;
    m_glfcns.glGetFloatv (GL_ZOOM_Y, &zoom_y);

    m_glfcns.glPixelZoom (m_devpixratio * pix_dx, - m_devpixratio * pix_dy);
    m_glfcns.glRasterPos3d (im_xmin + nor_dx*j0, im_ymin + nor_dy*i0, 0);

    // Expect RGB data
    if (dv.ndims () == 3 && dv(2) == 3)
      {
        if (cdata.is_double_type ())
          {
            const NDArray xcdata = cdata.array_value ();

            OCTAVE_LOCAL_BUFFER (GLfloat, a, 3*(j1-j0)*(i1-i0));

            for (int i = i0; i < i1; i++)
              {
                for (int j = j0, idx = (i-i0)*(j1-j0)*3; j < j1; j++, idx += 3)
                  {
                    if (! yflip)
                      ii = i;
                    else
                      ii = h - i - 1;

                    if (! xflip)
                      jj = j;
                    else
                      jj = w - j - 1;

                    a[idx]   = xcdata(ii,jj,0);
                    a[idx+1] = xcdata(ii,jj,1);
                    a[idx+2] = xcdata(ii,jj,2);
                  }
              }

            draw_pixels (j1-j0, i1-i0, a);

          }
        else if (cdata.is_single_type ())
          {
            const FloatNDArray xcdata = cdata.float_array_value ();

            OCTAVE_LOCAL_BUFFER (GLfloat, a, 3*(j1-j0)*(i1-i0));

            for (int i = i0; i < i1; i++)
              {
                for (int j = j0, idx = (i-i0)*(j1-j0)*3; j < j1; j++, idx += 3)
                  {
                    if (! yflip)
                      ii = i;
                    else
                      ii = h - i - 1;

                    if (! xflip)
                      jj = j;
                    else
                      jj = w - j - 1;

                    a[idx]   = xcdata(ii,jj,0);
                    a[idx+1] = xcdata(ii,jj,1);
                    a[idx+2] = xcdata(ii,jj,2);
                  }
              }

            draw_pixels (j1-j0, i1-i0, a);

          }
        else if (cdata.is_uint8_type ())
          {
            const uint8NDArray xcdata = cdata.uint8_array_value ();

            OCTAVE_LOCAL_BUFFER (GLubyte, a, 3*(j1-j0)*(i1-i0));

            for (int i = i0; i < i1; i++)
              {
                for (int j = j0, idx = (i-i0)*(j1-j0)*3; j < j1; j++, idx += 3)
                  {
                    if (! yflip)
                      ii = i;
                    else
                      ii = h - i - 1;

                    if (! xflip)
                      jj = j;
                    else
                      jj = w - j - 1;

                    a[idx]   = xcdata(ii,jj,0);
                    a[idx+1] = xcdata(ii,jj,1);
                    a[idx+2] = xcdata(ii,jj,2);
                  }
              }

            draw_pixels (j1-j0, i1-i0, a);

          }
        else if (cdata.is_uint16_type ())
          {
            const uint16NDArray xcdata = cdata.uint16_array_value ();

            OCTAVE_LOCAL_BUFFER (GLushort, a, 3*(j1-j0)*(i1-i0));

            for (int i = i0; i < i1; i++)
              {
                for (int j = j0, idx = (i-i0)*(j1-j0)*3; j < j1; j++, idx += 3)
                  {
                    if (! yflip)
                      ii = i;
                    else
                      ii = h - i - 1;

                    if (! xflip)
                      jj = j;
                    else
                      jj = w - j - 1;

                    a[idx]   = xcdata(ii,jj,0);
                    a[idx+1] = xcdata(ii,jj,1);
                    a[idx+2] = xcdata(ii,jj,2);
                  }
              }

            draw_pixels (j1-j0, i1-i0, a);

          }
        else
          warning ("opengl_renderer: invalid image data type (expected double, single, uint8, or uint16)");

        m_glfcns.glPixelZoom (zoom_x, zoom_y);

      }
  }

  void
  gl2ps_renderer::draw_pixels (int w, int h, const float *data)
  {
    // Clip data between 0 and 1 for float values
    OCTAVE_LOCAL_BUFFER (float, tmp_data, 3*w*h);

    for (int i = 0; i < 3*h*w; i++)
      tmp_data[i] = (data[i] < 0.0f ? 0.0f : (data[i] > 1.0f ? 1.0f : data[i]));

    gl2psDrawPixels (w, h, 0, 0, GL_RGB, GL_FLOAT, tmp_data);
  }

  void
  gl2ps_renderer::draw_pixels (int w, int h, const uint8_t *data)
  {
    // gl2psDrawPixels only supports the GL_FLOAT type.

    OCTAVE_LOCAL_BUFFER (float, tmp_data, 3*w*h);

    static const float maxval = std::numeric_limits<uint8_t>::max ();

    for (int i = 0; i < 3*w*h; i++)
      tmp_data[i] = data[i] / maxval;

    draw_pixels (w, h, tmp_data);
  }

  void
  gl2ps_renderer::draw_pixels (int w, int h, const uint16_t *data)
  {
    // gl2psDrawPixels only supports the GL_FLOAT type.

    OCTAVE_LOCAL_BUFFER (float, tmp_data, 3*w*h);

    static const float maxval = std::numeric_limits<uint16_t>::max ();

    for (int i = 0; i < 3*w*h; i++)
      tmp_data[i] = data[i] / maxval;

    draw_pixels (w, h, tmp_data);
  }

  void
  gl2ps_renderer::draw_text (const text::properties& props)
  {
    if (props.get_string ().isempty ())
      return;

    draw_text_background (props, true);

    // First set font properties: freetype will use them to compute
    // coordinates and gl2ps will retrieve the color directly from the
    // feedback buffer
    set_font (props);
    set_color (props.get_color_rgb ());

    std::string saved_font = fontname;

    // Alignment
    int halign = 0;
    int valign = 0;

    if (props.horizontalalignment_is ("center"))
      halign = 1;
    else if (props.horizontalalignment_is ("right"))
      halign = 2;

    if (props.verticalalignment_is ("top"))
      valign = 2;
    else if (props.verticalalignment_is ("baseline"))
      valign = 3;
    else if (props.verticalalignment_is ("middle"))
      valign = 1;

    // FIXME: handle margin and surrounding box
    // Matrix bbox;

    const Matrix pos = get_transform ().scale (props.get_data_position ());
    std::string str = props.get_string ().string_vector_value ().join ("\n");

    render_text (str, pos(0), pos(1), pos.numel () > 2 ? pos(2) : 0.0,
                 halign, valign, props.get_rotation ());
  }
}

#endif

namespace octave
{
  // If the name of the stream begins with '|', open a pipe to the command
  // named by the rest of the string.  Otherwise, write to the named file.

  void
  gl2ps_print (opengl_functions& glfcns, const graphics_object& fig,
               const std::string& stream, const std::string& term)
  {
#if defined (HAVE_GL2PS_H) && defined (HAVE_OPENGL)

    // FIXME: should we have a way to create a file that begins with the
    // character '|'?

    bool have_cmd = stream.length () > 1 && stream[0] == '|';

    FILE *fp = nullptr;

    unwind_protect frame;

    if (have_cmd)
      {
        // Create process and pipe gl2ps output to it.

        std::string cmd = stream.substr (1);

        fp = popen (cmd.c_str (), "w");

        if (! fp)
          error (R"(print: failed to open pipe "%s")", stream.c_str ());

        frame.add_fcn (safe_pclose, fp);
      }
    else
      {
        // Write gl2ps output directly to file.

        fp = sys::fopen (stream.c_str (), "w");

        if (! fp)
          error (R"(gl2ps_print: failed to create file "%s")", stream.c_str ());

        frame.add_fcn (safe_fclose, fp);
      }

    gl2ps_renderer rend (glfcns, fp, term);

    Matrix pos = fig.get ("position").matrix_value ();
    rend.set_viewport (pos(2), pos(3));
    rend.draw (fig, stream);

    // Make sure buffered commands are finished!!!
    rend.finish ();

#else

    octave_unused_parameter (glfcns);
    octave_unused_parameter (fig);
    octave_unused_parameter (stream);
    octave_unused_parameter (term);

    err_disabled_feature ("gl2ps_print", "gl2ps");

#endif
  }
}
