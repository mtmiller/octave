Summary of important user-visible changes for version 3.8 (2013-12-27):
----------------------------------------------------------------------

  ** One of the biggest new features for Octave 3.8 is a graphical user
     interface.  It is the one thing that users have requested most
     often over the last few years and now it is almost ready.  But
     because it is not quite as polished as we would like, we have
     decided to wait until the 4.0.x release series before making the
     GUI the default interface (until then, you can use the --force-gui
     option to start the GUI).

     Given the length of time and the number of bug fixes and
     improvements since the last major release Octave, we also decided
     against delaying the release of all these new improvements any
     longer just to perfect the GUI.  So please enjoy the 3.8 release of
     Octave and the preview of the new GUI.  We believe it is working
     reasonably well, but we also know that there are some obvious rough
     spots and many things that could be improved.

     WE NEED YOUR HELP.  There are many ways that you can help us fix
     the remaining problems, complete the GUI, and improve the overall
     user experience for both novices and experts alike:

       * If you are a skilled software developer, you can help by
         contributing your time to help with Octave's development.  See
         http://octave.org/get-involved.html for more information.

       * If Octave does not work properly, you are encouraged
         report the problems you find.  See http://octave.org/bugs.html
         for more information about how to report problems.

       * Whether you are a user or developer, you can help to fund the
         project.  Octave development takes a lot of time and expertise.
         Your contributions help to ensure that Octave will continue to
         improve.  See http://octave.org/donate.html for more details.

    We hope you find Octave to be useful.  Please help us make it even
    better for the future!

 ** Octave now uses OpenGL graphics by default with FLTK widgets.  If
    OpenGL libraries or FLTK widgets are not available when Octave is
    built, gnuplot is used.  You may also choose to use gnuplot for
    graphics by executing the command

      graphics_toolkit ("gnuplot")

    Adding this command to your ~/.octaverc file will set the default
    for each session.

 ** Printing or saving figures with OpenGL graphics requires the
    gl2ps library which is no longer distributed with Octave.  The
    software is widely available in most package managers.  If a
    pre-compiled package does not exist for your system, you can find
    the current sources at http://www.geuz.org/gl2ps/.

 ** Octave now supports nested functions with scoping rules that are
    compatible with Matlab.  A nested function is one declared and defined
    within the body of another function.  The nested function is only
    accessible from within the enclosing function which makes it one
    method for making private functions whose names do not conflict with those
    in the global namespace (See also subfunctions and private functions).
    In addition, variables in the enclosing function are visible within the
    nested function.  This makes it possible to have a pseudo-global variable
    which can be seen by a group of functions, but which is not visible in
    the global namespace.

    Example:
    function outerfunc (...)
      ...
      function nested1 (...)
        ...
        function nested2 (...)
           ...
        endfunction
      endfunction

      function nested3 (...)
        ...
      endfunction
    endfunction

 ** Line continuations inside character strings have changed.

    The sequence '...' is no longer recognized as a line continuation
    inside a character string.  A backslash '\' followed by a newline
    character is no longer recognized as a line continuation inside
    single-quoted character strings.  Inside double-quoted character
    strings, a backslash followed by a newline character is still
    recognized as a line continuation, but the backslash character must
    be followed *immediately* by the newline character.  No whitespace or
    end-of-line comment may appear between them.

 ** Backslash as a continuation marker outside of double-quoted strings
    is now deprecated.

    Using '\' as a continuation marker outside of double quoted strings
    is now deprecated and will be removed from a future version of
    Octave.  When that is done, the behavior of

      (a \
       b)

    will be consistent with other binary operators.

 ** Redundant terminal comma accepted by parser

    A redundant terminal comma is now accepted in matrix
    definitions which allows writing code such as

    [a,...
     b,...
     c,...
    ] = deal (1,2,3)

 ** Octave now has limited support for named exceptions

    The following syntax is now accepted:

      try
        statements
      catch exception-id
        statements
      end

    The exception-id is a structure with the fields "message" and
    "identifier".  For example

      try
        error ("Octave:error-id", "error message");
      catch myerr
        printf ("identifier: %s\n", myerr.identifier);
        printf ("message:    %s\n", myerr.message);
      end_try_catch

    When classdef-style classes are added to Octave, the exception-id
    will become an MException object.

 ** Warning states may now be set temporarily, until the end of the
    current function, using the syntax

      warning STATE ID "local"

    in which STATE may be "on", "off", or "error".  Changes to warning
    states that are set locally affect the current function and all
    functions called from the current scope.  The previous warning state
    is restored on return from the current function.  The "local"
    option is ignored if used in the top-level workspace.

 ** Warning IDs renamed:

    Octave:array-as-scalar => Octave:array-to-scalar
    Octave:array-as-vector => Octave:array-to-vector

 ** 'emptymatch', 'noemptymatch' options added to regular expressions.

    With this addition Octave now accepts the entire set of Matlab options
    for regular expressions.  'noemptymatch' is the default, but 'emptymatch'
    has certain uses where you need to match an assertion rather than actual
    characters.  For example,

    regexprep ('World', '^', 'Hello ', 'emptymatch')
      => Hello World

    where the pattern is actually the assertion '^' or start-of-line.

 ** For compatibility with Matlab, the regexp, regexpi, and regexprep
    functions now process backslash escape sequences in single-quoted pattern
    strings.  In addition, the regexprep function now processes backslash
    escapes in single-quoted replacement strings.  For example,

    regexprep (str, '\t', '\n')

    would search the variable str for a TAB character (escape sequence \t)
    and replace it with a NEWLINE (escape sequence \n).  Previously the
    expression would have searched for a literal '\' followed by 't' and
    replaced the two characters with the sequence '\', 'n'.

 ** A TeX parser has been implemented for the FLTK toolkit and is the default
    for any text object including titles and axis labels.  The TeX parser is
    supported only for display on a monitor, not for printing.

    A quick summary of features:

    Code         Feature     Example             Comment
    -----------------------------------------------------------------
    _            subscript   H_2O                formula for water
    ^            exponent    y=x^2               formula for parabola
    \char        symbol      \beta               Greek symbol beta
    \fontname    font        \fontname{Arial}    set Arial font
    \fontsize    fontsize    \fontsize{16}       set fontsize 16
    \color[rgb]  fontcolor   \color[rgb]{1 0 1}  set magenta color
    \bf          bold        \bfBold Text        bold font
    \it          italic      \itItalic Text      italic font
    \sl          slanted     \slOblique Text     slanted font
    \rm          normal      \bfBold\rmNormal    normal font
    {}           group       {\bf Bold}Normal    group objects
                             e^{i*\pi} = -1      group objects

 ** The m-files in the plot directory have been overhauled.

    The plot functions now produce output that is nearly visually compatible
    with Matlab.  Plot performance has also increased, dramatically for some
    functions such as comet and waitbar.  Finally, the documentation for most
    functions has been updated so it should be clearer both how to use a
    function and when a function is appropriate.

 ** The m-files in the image directory have been overhauled.

    The principal benefit is that Octave will now no longer automatically
    convert images stored with integers to doubles.  Storing images as uint8
    or uint16 requires only 1/8 or 1/4 the memory of an image stored using
    doubles.  For certain operations, such as fft2, the image must still be
    converted to double in order to work.

    Other changes include fixes to the way indexed images are read from a
    colormap depending on the image class (integer images have a -1 offset to
    the colormap row number).

 ** The imread and imwrite functions have been completely rewritten.

    The main changes relate to the alpha channel, support for reading and
    writing of floating point images, implemented writing of indexed images,
    and appending images to multipage image files.

    The issues that may arise due to backwards incompatibility are:

      * imwrite no longer interprets a length of 2 or 4 in the third dimension
        as grayscale or RGB with alpha channel (a length of 4 will be saved
        as a CMYK image).  Alpha channel must be passed as separate argument.

      * imread will always return the colormap indexes when reading an indexed
        image, even if the colormap is not requested as output.

      * transparency values are now inverted from previous Octave versions
        (0 is for completely transparent instead of completely opaque).

    In addition, the function imformats has been implemented to expand
    reading and writing of images of different formats through imread
    and imwrite.

 ** The colormap function now provides new options--"list", "register",
    and "unregister"--to list all available colormap functions, and to
    add or remove a function name from the list of known colormap
    functions.  Packages that implement extra colormaps should use these
    commands with PKG_ADD and PKG_DEL statements.

 ** strsplit has been modified to be compatible with Matlab.  There
    are two instances where backward compatibility is broken.

    (1) Delimiters are now string vectors, not scalars.

    Octave's legacy behavior

      strsplit ("1 2, 3", ", ")
      ans =
      {
       [1,1] = 1
       [1,2] = 2
       [1,3] =
       [1,4] = 3
      }

    Matlab compatible behavior

      strsplit ("1 2, 3", ", ")
      ans =
      {
       [1,1] = 1 2
       [1,2] = 3
      }

    (2) By default, Matlab treats consecutive delimiters as a single
    delimiter.  By default, Octave's legacy behavior was to return an
    empty string for the part between the delmiters.

    Where legacy behavior is desired, the call to strsplit() may be
    replaced by ostrsplit(), which is Octave's original implementation of
    strsplit().

 ** The datevec function has been extended for better Matlab compatibility.
    It now accepts string inputs in the following numerical formats: 12, 21,
    22, 26, 29, 31.  This is undocumented, but verifiable, Matlab behavior.
    In addition, the default for formats which do not specify a date is
    January 1st of the current year.  The previous default was the current day,
    month, and year.  This may produce changes in existing scripts.

 ** The error function and its derivatives has been extended to accept complex
    arguments.  The following functions now accept complex inputs:

    erf  erfc  erfcx

    In addition two new error functions erfi (imaginary error function) and
    dawson (scaled imaginary error function) have been added.

 ** The glpk function has been modified to reflect changes in the GLPK
    library.  The "round" and "itcnt" options have been removed.  The
    "relax" option has been replaced by the "rtest" option.  The numeric
    values of error codes and of some options have also changed.

 ** The kurtosis function has changed definition to be compatible with
    Matlab.  It now returns the base kurtosis instead of the "excess kurtosis".
    The old behavior can be had by changing scripts to normalize with -3.

               "excess kurtosis" = kurtosis (x) - 3

 ** The moment function has changed definition to be compatible with
    Matlab.  It now returns the central moment instead of the raw moment.
    The old behavior can be had by passing the type argument "r" for raw.

 ** The default name of the Octave crash dump file is now
    "octave-workspace" instead of "octave-core".  The exact name can
    always be customized with the octave_core_file_name function.

 ** A citation command has been added to display information on how to
    cite Octave and packages in publications.  The package system will
    look for and install CITATION files from packages.

 ** The java package from Octave Forge is now part of core Octave.  The
    following new functions are available for interacting with Java
    directly from Octave:

      debug_java     java_matrix_autoconversion
      isjava         java_unsigned_autoconversion
      java2mat       javaaddpath
      javaArray      javaclasspath
      javaMethod     javamem
      javaObject     javarmpath
                     usejava

    In addition, the following functions that use the Java interface
    are now available (provided that Octave is compiled with support for
    Java enabled):

      helpdlg    listdlg   questdlg
      inputdlg   msgbox    warndlg

 ** Other new functions added in 3.8.0:

      atan2d                     erfi             lines
      base64_decode              expint           linsolve
      base64_encode              findfigs         missing_component_hook
      betaincinv                 flintmax         polyeig
      built_in_docstrings_file   fminsearch       prefdir
      cmpermute                  gallery          preferences
      cmunique                   gco              readline_re_read_init_file
      colorcube                  hdl2struct       readline_read_init_file
      copyobj                    history_save     rgbplot
      dawson                     imformats        save_default_options
      dblist                     importdata       shrinkfaces
      desktop                    isaxes           splinefit
      doc_cache_create           iscolormap       stemleaf
      ellipj                     isequaln         strjoin
      ellipke                    jit_debug        struct2hdl
      erfcinv                    jit_enable       tetramesh
                                 jit_startcnt     waterfall

 ** Deprecated functions.

    The following functions were deprecated in Octave 3.4 and have been
    removed from Octave 3.8.

      autocor    dispatch              is_global    setstr
      autocov    fstat                 krylovb      strerror
      betai      gammai                perror       values
      cellidx    glpkmex               replot
      cquad      is_duplicate_entry    saveimage

    The following functions have been deprecated in Octave 3.8 and will
    be removed from Octave 3.12 (or whatever version is the second major
    release after 3.8):

      default_save_options    java_new
      gen_doc_cache           java_set
      interp1q                java_unsigned_conversion
      isequalwithequalnans    javafields
      java_convert_matrix     javamethods
      java_debug              re_read_readline_init_file
      java_get                read_readline_init_file
      java_invoke             saving_history


    The following keywords have been deprecated in Octave 3.8 and will
    be removed from Octave 3.12 (or whatever version is the second major
    release after 3.8):

      static

    The following configuration variables have been deprecated in Octave
    3.8 and will be removed from Octave 3.12 (or whatever version is the
    second major release after 3.8):

      CC_VERSION  (now GCC_VERSION)
      CXX_VERSION (now GXX_VERSION)

    The internal class <Octave_map> has been deprecated in Octave 3.8 and will
    be removed from Octave 3.12 (or whatever version is the second major
    release after 3.8).  Replacement classes are <octave_map> (struct array)
    or <octave_scalar_map> for a single structure.

Summary of important user-visible changes for version 3.6 (2012-01-15):
----------------------------------------------------------------------

 ** The PCRE library is now required to build Octave.  If a pre-compiled
    package does not exist for your system, you can find PCRE sources
    at http://www.pcre.org

 ** The ARPACK library is no longer distributed with Octave.
    If you need the eigs or svds functions you must provide an
    external ARPACK through a package manager or by compiling it
    yourself.  If a pre-compiled package does not exist for your system,
    you can find the current ARPACK sources at
    http://forge.scilab.org/index.php/p/arpack-ng

 ** Many of Octave's binary operators (.*, .^, +, -, ...) now perform
    automatic broadcasting for array operations which allows you to use
    operator notation instead of calling bsxfun or expanding arrays (and
    unnecessarily wasting memory) with repmat or similar idioms.  For
    example, to scale the columns of a matrix by the elements of a row
    vector, you may now write

      rv .* M

    In this expression, the number of elements of rv must match the
    number of columns of M.  The following operators are affected:

      plus      +  .+
      minus     -  .-
      times     .*
      rdivide   ./
      ldivide   .\
      power     .^  .**
      lt        <
      le        <=
      eq        ==
      gt        >
      ge        >=
      ne        !=  ~=
      and       &
      or        |
      atan2
      hypot
      max
      min
      mod
      rem
      xor

    additionally, since the A op= B assignment operators are equivalent
    to A = A op B, the following operators are also affected:

      +=  -=  .+=  .-=  .*=  ./=  .\=  .^=  .**=  &=  |=

    See the "Broadcasting" section in the new "Vectorization and Faster
    Code Execution" chapter of the manual for more details.

 ** Octave now features a profiler, thanks to the work of Daniel Kraft
    under the Google Summer of Code mentorship program.  The manual has
    been updated to reflect this addition.  The new user-visible
    functions are profile, profshow, and profexplore.

 ** Overhaul of statistical distribution functions

    Functions now return "single" outputs for inputs of class "single".

    75% reduction in memory usage through use of logical indexing.

    Random sample functions now use the same syntax as rand and accept
    a comma separated list of dimensions or a dimension vector.

    Functions have been made Matlab-compatible with regard to special
    cases (probability on boundaries, probabilities for values outside
    distribution, etc.).  This may cause subtle changes to existing
    scripts.

    negative binomial function has been extended to real, non-integer
    inputs.  The discrete_inv function now returns v(1) for 0 instead of
    NaN.  The nbincdf function has been recoded to use a closed form
    solution with betainc.

 ** strread, textscan, and textread have been completely revamped.

    They now support nearly all Matlab functionality including:

      * Matlab-compatible whitespace and delimiter defaults

      * Matlab-compatible options: 'whitespace', treatasempty', format
        string repeat count, user-specified comment style, uneven-length
        output arrays, %n and %u conversion specifiers (provisionally)

 ** All .m string functions have been modified for better performance or
    greater Matlab compatibility.  Performance gains of 15X-30X have
    been demonstrated.  Operations on cell array of strings no longer pay
    quite as high a penalty as those on 2-D character arrays.

      deblank:  Now requires character or cellstr input.

      strtrim:  Now requires character or cellstr input.
                No longer trims nulls ("\0") from string for Matlab
                compatibility.

      strmatch: Follows documentation precisely and ignores trailing spaces
                in pattern and in string.  Note that this is documented
                Matlab behavior but the implementation apparently does
                not always follow it.

      substr:   Now possible to specify a negative LEN option which
                extracts to within LEN of the end of the string.

      strtok:   Now accepts cellstr input.

      base2dec, bin2dec, hex2dec:
                Now accept cellstr inputs.

      dec2base, dec2bin, dec2hex:
                Now accept cellstr inputs.

      index, rindex:
                Now accept 2-D character array input.

      strsplit: Now accepts 2-D character array input.

 ** Geometry functions derived from Qhull (convhull, delaunay, voronoi)
    have been revamped.  The options passed to the underlying qhull
    command have been changed for better results or for Matlab
    compatibility.

      convhull: Default options are "Qt" for 2D, 3D, 4D inputs
                Default options are "Qt Qx" for 5D and higher

      delaunay: Default options are "Qt Qbb Qc Qz" for 2D and 3D inputs
                Default options are "Qt Qbb Qc Qx" for 4D and higher

      voronoi:  No default arguments

 ** Date/Time functions updated.  Millisecond support with FFF format
    string now supported.

    datestr: Numerical formats 21, 22, 29 changed to match Matlab.
             Now accepts cellstr input.

 ** The following warning IDs have been removed:

      Octave:associativity-change
      Octave:complex-cmp-ops
      Octave:empty-list-elements
      Octave:fortran-indexing
      Octave:precedence-change

 ** The warning ID Octave:string-concat has been renamed to
    Octave:mixed-string-concat.

 ** Octave now includes the following Matlab-compatible preference
    functions:

      addpref  getpref  ispref  rmpref  setpref

 ** The following Matlab-compatible handle graphics functions have been
    added:

      guidata         uipanel        uitoolbar
      guihandles      uipushtool     uiwait
      uicontextmenu   uiresume       waitfor
      uicontrol       uitoggletool

    The uiXXX functions above are experimental.

    Except for uiwait and uiresume, the uiXXX functions are not
    supported with the FLTK+OpenGL graphics toolkit.

    The gnuplot graphics toolkit does not support any of the uiXXX
    functions nor the waitfor function.

 ** New keyword parfor (parallel for loop) is now recognized as a valid
    keyword.  Implementation, however, is still mapped to an ordinary
    for loop.

 ** Other new functions added in 3.6.0:

      bicg                       nthargout                   usejava
      is_dq_string               narginchk                   waitbar
      is_sq_string               python                      zscore
      is_function_handle         register_graphics_toolkit
      loaded_graphics_toolkits   recycle

 ** Deprecated functions.

    The following functions were deprecated in Octave 3.2 and have been
    removed from Octave 3.6.

      create_set          spcholinv    splu
      dmult               spcumprod    spmax
      iscommand           spcumsum     spmin
      israwcommand        spdet        spprod
      lchol               spdiag       spqr
      loadimage           spfind       spsum
      mark_as_command     sphcat       spsumsq
      mark_as_rawcommand  spinv        spvcat
      spatan2             spkron       str2mat
      spchol              splchol      unmark_command
      spchol2inv          split        unmark_rawcommand

    The following functions have been deprecated in Octave 3.6 and will
    be removed from Octave 3.10 (or whatever version is the second major
    release after 3.6):

      cut                polyderiv
      cor                shell_cmd
      corrcoef           studentize
      __error_text__     sylvester_matrix
      error_text

 ** The following functions have been modified for Matlab compatibility:

      randperm

Summary of important user-visible changes for version 3.4.3 (2011-10-10):
------------------------------------------------------------------------

 ** Octave 3.4.3 is a bug fixing release.

Summary of important user-visible changes for version 3.4.2 (2011-06-24):
------------------------------------------------------------------------

 ** Octave 3.4.2 fixes some minor installation problems that affected
    version 3.4.1.

Summary of important user-visible changes for version 3.4.1 (2011-06-15):
------------------------------------------------------------------------

 ** Octave 3.4.1 is primarily a bug fixing release.

 ** IMPORTANT note about binary incompatibility in this release:

    Binary compatibility for all 3.4.x releases was originally planned,
    but this is impossible for the 3.4.1 release due to a bug in the way
    shared libraries were built in Octave 3.4.0.  Because of this bug,
    .oct files built for Octave 3.4.0 must be recompiled before they
    will work with Octave 3.4.1.

    Given that there would be binary incompatibilities with shared
    libraries going from Octave 3.4.0 to 3.4.1, the following
    incompatible changes were also made in this release:

      * The Perl Compatible Regular Expression (PCRE) library is now
        required to build Octave.

      * Octave's libraries and .oct files are now installed in
        subdirectories of $libdir instead of $libexecdir.

    Any future Octave 3.4.x release versions should remain binary
    compatible with Octave 3.4.1 as proper library versioning is now
    being used as recommended by the libtool manual.

 ** The following functions have been deprecated in Octave 3.4.1 and will
    be removed from Octave 3.8 (or whatever version is the second major
    release after 3.4):

      cquad  is_duplicate_entry  perror  strerror

 ** The following functions are new in 3.4.1:

      colstyle  gmres  iscolumn  isrow  mgorth  nproc  rectangle

 ** The get_forge_pkg function is now private.

 ** The rectangle_lw, rectangle_sw, triangle_lw, and triangle_sw
    functions are now private.

 ** The logistic_regression_derivatives and logistic_regression_likelihood
    functions are now private.

 ** ChangeLog files in the Octave sources are no longer maintained
    by hand.  Instead, there is a single ChangeLog file generated from
    the Mercurial version control commit messages.  Older ChangeLog
    information can be found in the etc/OLD-ChangeLogs directory in the
    source distribution.

Summary of important user-visible changes for version 3.4 (2011-02-08):
----------------------------------------------------------------------

 ** BLAS and LAPACK libraries are now required to build Octave.  The
    subset of the reference BLAS and LAPACK libraries has been removed
    from the Octave sources.

 ** The ARPACK library is now distributed with Octave so it no longer
    needs to be available as an external dependency when building
    Octave.

 ** The `lookup' function was extended to be more useful for
    general-purpose binary searching.  Using this improvement, the
    ismember function was rewritten for significantly better
    performance.

 ** Real, integer and logical matrices, when used in indexing, will now
    cache the internal index_vector value (zero-based indices) when
    successfully used as indices, eliminating the conversion penalty for
    subsequent indexing by the same matrix.  In particular, this means it
    is no longer needed to avoid repeated indexing by logical arrays
    using find for performance reasons.

 ** Logical matrices are now treated more efficiently when used as
    indices.  Octave will keep the index as a logical mask unless the
    ratio of true elements is small enough, using a specialized
    code.  Previously, all logical matrices were always first converted
    to index vectors.  This results in savings in both memory and
    computing time.

 ** The `sub2ind' and `ind2sub' functions were reimplemented as compiled
    functions for better performance.  These functions are now faster,
    can deliver more economized results for ranges, and can reuse the
    index cache mechanism described in previous paragraph.

 ** The built-in function equivalents to associative operators (`plus',
    `times', `mtimes', `and', and `or') have been extended to accept
    multiple arguments.  This is especially useful for summing
    (multiplying, etc.) lists of objects (of possibly distinct types):

      matrix_sum = plus (matrix_list{:});

 ** An FTP object type based on libcurl has been implemented.  These
    objects allow ftp connections, downloads and uploads to be
    managed.  For example,

      fp = ftp ("ftp.octave.org);
      cd (fp, "gnu/octave");
      mget (fp, "octave-3.2.3.tar.bz2");
      close (fp);

 ** The default behavior of `assert (observed, expected)' has been
    relaxed to employ less strict checking that does not require the
    internals of the values to match.  This avoids previously valid
    tests from breaking due to new internal classes introduced in future
    Octave versions.

    For instance, all of these assertions were true in Octave 3.0.x
    but false in 3.2.x due to new optimizations and improvements:

      assert (2*linspace (1, 5, 5), 2*(1:5))
      assert (zeros (0, 0), [])
      assert (2*ones (1, 5), (2) (ones (1,5)))

 ** The behavior of library functions `ismatrix', `issquare', and
    `issymmetric' has been changed for better consistency.

    * The `ismatrix' function now returns true for all numeric,
      logical and character 2-D or N-D matrices.  Previously, `ismatrix'
      returned false if the first or second dimension was zero.
      Hence, `ismatrix ([])' was false,
      while `ismatrix (zeros (1,2,0))' was true.

    * The `issquare' function now returns a logical scalar, and is
      equivalent to the expression

        ismatrix (x) && ndims (x) == 2 && rows (x) == columns (x)

      The dimension is no longer returned.  As a result, `issquare ([])'
      now yields true.

    * The `issymmetric' function now checks for symmetry instead of
      Hermitianness.  For the latter, ishermitian was created.  Also,
      logical scalar is returned rather than the dimension, so
      `issymmetric ([])' is now true.

 ** Function handles are now aware of overloaded functions.  If a
    function is overloaded, the handle determines at the time of its
    reference which function to call.  A non-overloaded version does not
    need to exist.

 ** Overloading functions for built-in classes (double, int8, cell,
    etc.) is now compatible with Matlab.

 ** Function handles can now be compared with the == and != operators,
    as well as the `isequal' function.

 ** Performance of concatenation (using []) and the functions `cat',
    `horzcat', and `vertcat' has been improved for multidimensional
    arrays.

 ** The operation-assignment operators +=, -=, *= and /= now behave more
    efficiently in certain cases.  For instance, if M is a matrix and S a
    scalar, then the statement

      M += S;

    will operate on M's data in-place if it is not shared by another
    variable, usually increasing both time and memory efficiency.

    Only selected common combinations are affected, namely:

      matrix += matrix
      matrix -= matrix
      matrix .*= matrix
      matrix ./= matrix

      matrix += scalar
      matrix -= scalar
      matrix *= scalar
      matrix /= scalar

      logical matrix |= logical matrix
      logical matrix &= logical matrix

    where matrix and scalar belong to the same class.  The left-hand
    side must be a simple variable reference.

    Moreover, when unary operators occur in expressions, Octave will
    also try to do the operation in-place if it's argument is a
    temporary expression.

 ** The effect of comparison operators (<, >, <=, and >=) applied to
    complex numbers has changed to be consistent with the strict
    ordering defined by the `max', `min', and `sort' functions.  More
    specifically, complex numbers are compared by lexicographical
    comparison of the pairs `[abs(z), arg(z)]'.  Previously, only real
    parts were compared; this can be trivially achieved by converting
    the operands to real values with the `real' function.

 ** The automatic simplification of complex computation results has
    changed.  Octave will now simplify any complex number with a zero
    imaginary part or any complex matrix with all elements having zero
    imaginary part to a real value.  Previously, this was done only for
    positive zeros.  Note that the behavior of the complex function is
    unchanged and it still produces a complex value even if the
    imaginary part is zero.

 ** As a side effect of code refactoring in liboctave, the binary
    logical operations are now more easily amenable to compiler
    optimizations and are thus significantly faster.

 ** Octave now allows user-defined `subsasgn' methods to optimize out
    redundant copies.  For more information, see the manual.

 ** More efficient matrix division handling.  Octave is now able to
    handle the expressions

      M' \ V
      M.' \ V
      V / M

    (M is a matrix and V is a vector) more efficiently in certain cases.
    In particular, if M is triangular, all three expressions will be
    handled by a single call to xTRTRS (from LAPACK), with appropriate
    flags.  Previously, all three expressions required a physical
    transpose of M.

 ** More efficient handling of certain mixed real-complex matrix
    operations.  For instance, if RM is a real matrix and CM a complex
    matrix,

      RM * CM

    can now be evaluated either as

      complex (RM * real (CM), RM * imag (CM))

    or as

      complex (RM) * CM,

    depending on the dimensions.  The first form requires more
    temporaries and copying, but halves the FLOP count, which normally
    brings better performance if RM has enough rows.  Previously, the
    second form was always used.

    Matrix division is similarly affected.

 ** More efficient handling of triangular matrix factors returned from
    factorizations.  The functions for computing QR, LU and Cholesky
    factorizations will now automatically return the triangular matrix
    factors with proper internal matrix_type set, so that it won't need
    to be computed when the matrix is used for division.

 ** The built-in `sum' function now handles the non-native summation
    (i.e., double precision sum of single or integer inputs) more
    efficiently, avoiding a temporary conversion of the whole input
    array to doubles.  Further, `sum' can now accept an extra option
    argument, using a compensated summation algorithm rather than a
    straightforward sum, which significantly improves precision if lots
    of cancellation occurs in the summation.

 ** The built-in `bsxfun' function now uses optimized code for certain
    cases where built-in operator handles are passed in.  Namely, the
    optimizations concern the operators `plus', `minus', `times',
    `ldivide', `rdivide', `power', `and', `or' (for logical arrays),
    the relational operators `eq', `ne', `lt', `le', `gt', `ge', and the
    functions `min' and `max'.  Optimizations only apply when both
    operands are of the same built-in class.  Mixed real/complex and
    single/double operations will first convert both operands to a
    common type.

 ** The `strfind' and `strrep' functions now have compiled
    implementations, facilitating significantly more efficient searching
    and replacing in strings, especially with longer patterns.  The code
    of `strcat' has been vectorized and is now much more efficient when
    many strings are concatenated.  The `strcmpi' and `strncmpi'
    functions are now built-in functions, providing better performance.

 ** 'str2double' now has a compiled implementation and the API conforms
    to Matlab.  The additional Octave-specific features of returning a
    boolean matrix indicating which elements were successfully converted
    has been removed.

 ** Matlab-style ignoring input and output function arguments using
    tilde (~) is now supported.  Ignored output arguments may be
    detected from a function using the built-in function `isargout'.
    For more details, consult the manual.

 ** The list datatype, deprecated since the introduction of cells, has
    been removed.

 ** The accumarray function has been optimized and is now significantly
    faster in certain important cases.

 ** The behavior of isreal and isnumeric functions was changed to be more
    Matlab-compatible.

 ** The integer math & conversion warnings (Octave:int-convert-nan,
    Octave:int-convert-non-int-val, Octave:int-convert-overflow,
    Octave:int-math-overflow) have been removed.

 ** rem and mod are now built-in functions.  They also handle integer
    types efficiently using integer arithmetic.

 ** Sparse indexing and indexed assignment has been mostly rewritten.
    Since Octave uses compressed column storage for sparse matrices,
    major attention is devoted to operations manipulating whole columns.
    Such operations are now significantly faster, as well as some other
    important cases.

    Further, it is now possible to pre-allocate a sparse matrix and
    subsequently fill it by assignments, provided they meet certain
    conditions.  For more information, consult the `spalloc' function,
    which is no longer a mere dummy.  Consequently, nzmax and nnz are no
    longer always equal in Octave.  Octave may also produce a matrix
    with nnz < nzmax as a result of other operations, so you should
    consistently use nnz unless you really want to use nzmax (i.e., the
    space allocated for nonzero elements).

    Sparse concatenation is also affected, and concatenating sparse
    matrices, especially larger collections, is now significantly more
    efficient.  This applies to both the [] operator and the
    cat/vertcat/horzcat functions.

 ** It is now possible to optionally employ the xGESDD LAPACK drivers
    for computing the singular value decomposition using svd, instead
    of the default xGESVD, using the configuration pseudo-variable
    svd_driver.  The xGESDD driver can be up to 6x times faster when
    singular vectors are requested, but is reported to be somewhat less
    robust on highly ill-conditioned matrices.

 ** Configuration pseudo-variables, such as page_screen_output or
    confirm_recursive_rmdir (or the above mentioned svd_driver), now
    accept a "local" option as second argument, requesting the change
    to be undone when the current function returns:

    function [status, msg] = rm_rf (dir)
      confirm_recursive_rmdir (false, "local");
      [status, msg] = rmdir (dir, "s");
      ...
    endfunction

    Upon return, confirm_recursive_rmdir will be restored to the value
    it had on entry to the function, even if there were subsequent
    changes to the variable in function rm_rf or any of the functions
    it calls.

 ** pkg now accepts a -forge option for downloading and installing
    packages from Octave Forge automatically.  For example,

      pkg install -forge general

    will automatically download the latest release of the general
    package and attempt to install it.  No automatic resolving of
    dependencies is provided.  Further,

      pkg list -forge

    can be used to list all available packages.

 ** The internal data representation of structs has been completely
    rewritten to make certain optimizations feasible.  The field data
    can now be shared between structs with equal keys but different
    dimensions or values, making operations that preserve the fields
    faster.  Economized storage is now used for scalar structs (just
    like most other scalars), making their usage more
    memory-efficient.  Certain array-like operations on structs
    (concatenation, uniform cellfun, num2cell) have gained a
    significant speed-up.  Additionally, the octave_scalar_map class
    now provides a simpler interface to work with scalar structs within
    a C++ DLD function.

 ** Two new formats are available for displaying numbers:

      format short eng
      format long eng

    Both display numbers in engineering notation, i.e., mantissa +
    exponent where the exponent is a multiple of 3.

 ** The following functions are new in Octave 3.4:
      accumdim    erfcx        nfields      pqpnonneg  uigetdir
      bitpack     fileread     nth_element  quadcc     uigetfile
      bitunpack   fminbnd      onCleanup    randi      uiputfile
      blkmm       fskipl       pbaspect     repelems   uimenu
      cbrt        ifelse       pie3         reset      whitebg
      curl        ishermitian  powerset     rsf2csf
      chop        isindex      ppder        saveas
      daspect     luupdate     ppint        strread
      divergence  merge        ppjumps      textread

 ** Using the image function to view images with external programs such
    as display, xv, and xloadimage is no longer supported.  The
    image_viewer function has also been removed.

 ** The behavior of struct assignments to non-struct values has been
    changed.  Previously, it was possible to overwrite an arbitrary
    value:

      a = 1;
      a.x = 2;

    This is no longer possible unless a is an empty matrix or cell
    array.

 ** The dlmread function has been extended to allow specifying a custom
    value for empty fields.

 ** The dlmread and dlmwrite functions have been modified to accept
    file IDs (as returned by fopen) in addition to file names.

 ** Octave can now optimize away the interpreter overhead of an
    anonymous function handle, if the function simply calls another
    function or handle with some of its parameters bound to certain
    values.  Example:

      f = @(x) sum (x, 1);

    When f is called, the call is forwarded to @sum with the constant 1
    appended, and the anonymous function call does not occur on the
    call stack.

 ** For compatibility with Matlab, mu2lin (x) is now equivalent to
    mu2lin (x, 0).

 ** The new function `history_control' may be used to control the way
    command lines are added to the history list when Octave is using
    readline for command-line editing.  For example

      history_control ("ignoredups")

    tells Octave to avoid adding duplicate lines to the history list.

 ** Octave now uses the gnulib library for improved portability and to
    avoid bugs in operating system functions.

 ** Deprecated functions.

    The following functions were deprecated in Octave 3.0 and have been
    removed from Octave 3.4.

      beta_cdf         geometric_pdf        pascal_pdf
      beta_inv         geometric_rnd        pascal_rnd
      beta_pdf         hypergeometric_cdf   poisson_cdf
      beta_rnd         hypergeometric_inv   poisson_inv
      binomial_cdf     hypergeometric_pdf   poisson_pdf
      binomial_inv     hypergeometric_rnd   poisson_rnd
      binomial_pdf     intersection         polyinteg
      binomial_rnd     is_bool              setstr
      chisquare_cdf    is_complex           struct_contains
      chisquare_inv    is_list              struct_elements
      chisquare_pdf    is_matrix            t_cdf
      chisquare_rnd    is_scalar            t_inv
      clearplot        is_square            t_pdf
      clg              is_stream            t_rnd
      com2str          is_struct            uniform_cdf
      exponential_cdf  is_symmetric         uniform_inv
      exponential_inv  is_vector            uniform_pdf
      exponential_pdf  isstr                uniform_rnd
      exponential_rnd  lognormal_cdf        weibcdf
      f_cdf            lognormal_inv        weibinv
      f_inv            lognormal_pdf        weibpdf
      f_pdf            lognormal_rnd        weibrnd
      f_rnd            meshdom              weibull_cdf
      gamma_cdf        normal_cdf           weibull_inv
      gamma_inv        normal_inv           weibull_pdf
      gamma_pdf        normal_pdf           weibull_rnd
      gamma_rnd        normal_rnd           wiener_rnd
      geometric_cdf    pascal_cdf
      geometric_inv    pascal_inv

    The following functions were deprecated in Octave 3.2 and will
    be removed from Octave 3.6 (or whatever version is the second major
    release after 3.2):

      create_set          spcholinv    splu
      dmult               spcumprod    spmax
      iscommand           spcumsum     spmin
      israwcommand        spdet        spprod
      lchol               spdiag       spqr
      loadimage           spfind       spsum
      mark_as_command     sphcat       spsumsq
      mark_as_rawcommand  spinv        spvcat
      spatan2             spkron       str2mat
      spchol              splchol      unmark_command
      spchol2inv          split        unmark_rawcommand

    The following functions have been deprecated in Octave 3.4 and will
    be removed from Octave 3.8 (or whatever version is the second major
    release after 3.4):

      autocor  cellidx   gammai     is_global  replot     values
      autocov  dispatch  glpkmex    krylovb    saveimage
      betai    fstat     intwarning perror     strerror

Summary of important user-visible changes for version 3.2 (2009-06-05):
----------------------------------------------------------------------

 ** Compatibility with Matlab graphics has been improved.

    The hggroup object and associated listener callback functions have
    been added allowing the inclusion of group objects.  Data sources
    have been added to these group objects such that

           x = 0:0.1:10;
           y = sin (x);
           plot (x, y, "ydatasource", "y");
           for i = 1 : 100
             pause(0.1)
             y = sin (x + 0.1 * i);
             refreshdata ();
           endfor

    works as expected.  This capability has be used to introduce
    stem-series, bar-series, etc., objects for better Matlab
    compatibility.

 ** New graphics functions:

      addlistener                  ezcontour   gcbo         refresh
      addproperty                  ezcontourf  ginput       refreshdata
      allchild                     ezmesh      gtext        specular
      available_graphics_toolkits  ezmeshc     intwarning   surfl
      graphics_toolkit             ezplot      ishghandle   trisurf
      cla                          ezplot3     isocolors    waitforbuttonpress
      clabel                       ezpolar     isonormals
      comet                        ezsurf      isosurface
      dellistener                  findall     linkprop
      diffuse                      gcbf        plotmatrix

 ** New experimental OpenGL/FLTK based plotting system.

    An experimental plotting system based on OpenGL and the FLTK
    toolkit is now part of Octave.  This graphics toolkit is disabled by
    default.  You can switch to using it with the command

        graphics_toolkit ("fltk")

    for all future figures or for a particular figure with the command

        graphics_toolkit (h, "fltk")

    where "h" is a valid figure handle.

 ** Functions providing direct access to gnuplot have been removed.

    The functions __gnuplot_plot__, __gnuplot_set__, __gnuplot_raw__,
     __gnuplot_show__, __gnuplot_replot__, __gnuplot_splot__,
     __gnuplot_save_data__ and __gnuplot_send_inline_data__ have been
     removed from Octave.  These function were incompatible with the
     high level graphics handle code.

 ** The Control, Finance and Quaternion functions have been removed.

    These functions are now available as separate packages from

      http://octave.sourceforge.net/packages.html

    and can be reinstalled using the Octave package manager (see
    the pkg function).

 ** Specific sparse matrix functions removed.

    The following functions, which handled only sparse matrices have
    been removed.  Instead of calling these functions directly, you
    should use the corresponding function without the "sp" prefix.

      spatan2     spcumsum  spkron   spprod
      spchol      spdet     splchol  spqr
      spchol2inv  spdiag    splu     spsum
      spcholinv   spfind    spmax    spsumsqk
      spcumprod   spinv     spmin

 ** Improvements to the debugger.

    The interactive debugging features have been improved.  Stopping
    on statements with dbstop should work correctly now.  Stepping
    into and over functions, and stepping one statement at a time
    (with dbstep) now works.  Moving up and down the call stack with
    dbup and dbdown now works.  The dbstack function is now available
    to print the current function call stack.  The new dbquit function
    is available to exit the debugging mode.

 ** Improved traceback error messages.

    Traceback error messages are much more concise and easier to
    understand.  They now display information about the function call
    stack instead of the stack of all statements that were active at
    the point of the error.

 ** Object Oriented Programming.

    Octave now includes OOP features and the user can create their own
    class objects and overloaded functions and operators.  For
    example, all methods of a class called "myclass" will be found in
    a directory "@myclass" on the users path.  The class specific
    versions of functions and operators take precedence over the
    generic versions of these functions.

    New functions related to OOP include

      class  inferiorto  isobject  loadobj  methods  superiorto

    See the Octave manual for more details.

 ** Parsing of Command-style Functions.

    Octave now parses command-style functions without needing to first
    declare them with "mark_as_command".  The rules for recognizing a
    command-style function calls are

      * A command must appear as the first word in a statement,
        followed by a space.

      * The first character after the space must not be '=' or '('

      * The next token after the space must not look like a binary
        operator.

    These rules should be mostly compatible with the way Matlab parses
    command-style function calls and allow users to define commands in
    .m files without having to mark them as commands.

    Note that previous versions of Octave allowed expressions like

      x = load -text foo.dat

    but an expression like this will now generate a parse error.  In
    order to assign the value returned by a function to a variable,
    you must use the normal function call syntax:

      x = load ("-text", "foo.dat");

 ** Block comments.

    Commented code can be between matching "#{" and "#}" or "%{" and
    "%}" markers, even if the commented code spans several line.  This
    allows blocks code to be commented, without needing to comment
    each line.  For example,

    function [s, t] = func (x, y)
      s = 2 * x;
    #{
      s *= y;
      t = y + x;
    #}
    endfunction

    the lines "s *= y;" and "t = y + x" will not be executed.

 ** If any subfunction in a file ends with "end" or "endfunction", then
    they all must end that way.  Previously, Octave accepted

      function main ()
        ...
      # no endfunction here.
      function sub ()
        ...
      endfunction

    but this is no longer allowed.

 ** Special treatment in the parser of expressions like "a' * b".

    In these cases the transpose is no longer explicitly formed and
    BLAS libraries are called with the transpose flagged,
    significantly improving performance for these kinds of
    operations.

 ** Single Precision data type.

    Octave now includes a single precision data type.  Single
    precision variables can be created with the "single" command, or
    from functions like ones, eye, etc.  For example,

      single (1)
      ones (2, 2, "single")
      zeros (2, 2, "single")
      eye (2, 2, "single")
      Inf (2, 2, "single")
      NaN (2, 2, "single")
      NA (2, 2, "single")

    all create single precision variables.  For compatibility with
    Matlab, mixed double/single precision operators and functions
    return single precision types.

    As a consequence of this addition to Octave the internal
    representation of the double precision NA value has changed, and
    so users that make use of data generated by Octave with R or
    visa-versa are warned that compatibility might not be assured.

 ** Improved array indexing.

    The underlying code used for indexing of arrays has been
    completely rewritten and indexing is now significantly faster.

 ** Improved memory management.

    Octave will now attempt to share data in some cases where previously
    a copy would be made, such as certain array slicing operations or
    conversions between cells, structs and cs-lists.  This usually reduces
    both time and memory consumption.
    Also, Octave will now attempt to detect and optimize usage of a vector
    as a stack, when elements are being repeatedly inserted at/removed from
    the end of the vector.

 ** Improved performance for reduction operations.

    The performance of the sum, prod, sumsq, cumsum, cumprod, any, all,
    max and min functions has been significantly improved.

 ** Sorting and searching.

    The performance of sort has been improved, especially when sorting
    indices are requested.  An efficient built-in issorted
    implementation was added.  The sortrows function now uses a more
    efficient algorithm, especially in the homogeneous case.  The lookup
    function is now a built-in function performing a binary search,
    optimized for long runs of close elements.  Lookup also works with
    cell arrays of strings.

 ** Range arithmetics

    For some operations on ranges, Octave will attempt to keep the
    result as a range.  These include negation, adding a scalar,
    subtracting a scalar, and multiplying by a scalar.  Ranges with zero
    increment are allowed and can be constructed using the built-in
    function `ones'.

 ** Various performance improvements.

    Performance of a number of other built-in operations and functions
    was improved, including:

    * logical operations
    * comparison operators
    * element-wise power
    * accumarray
    * cellfun
    * isnan
    * isinf
    * isfinite
    * nchoosek
    * repmat
    * strcmp

 ** 64-bit integer arithmetic.

    Arithmetic with 64-bit integers (int64 and uint64 types) is fully
    supported, with saturation semantics like the other integer types.
    Performance of most integer arithmetic operations has been
    improved by using integer arithmetic directly.  Previously, Octave
    performed integer math with saturation semantics by converting the
    operands to double precision, performing the operation, and then
    converting the result back to an integer value, truncating if
    necessary.

 ** Diagonal and permutation matrices.

    The interpreter can now treat diagonal and permutation matrices as
    special objects that store only the non-zero elements, rather than
    general full matrices.  Therefore, it is now possible to construct
    and use these matrices in linear algebra without suffering a
    performance penalty due to storing large numbers of zero elements.

 ** Improvements to fsolve.

    The fsolve function now accepts an option structure argument (see
    also the optimset function).  The INFO values returned from fsolve
    have changed to be compatible with Matlab's fsolve function.
    Additionally, fsolve is now able to solve overdetermined systems,
    complex-differentiable complex systems, systems with a sparse
    jacobian and can work in single precision if given single precision
    inputs.  It can also be called recursively.

 ** Improvements to the norm function.

    The norm function is now able to compute row or column norms of a
    matrix in a single call, as well as general matrix p-norms.

 ** New functions for computing some eigenvalues or singular values.

    The eigs and svds functions have been included in Octave.  These
    functions require the ARPACK library (now distributed under a
    GPL-compatible license).

 ** New QR and Cholesky factorization updating functions.

      choldelete  cholshift   qrdelete  qrshift
      cholinsert  cholupdate  qrinsert  qrupdate

 ** New quadrature functions.

      dblquad  quadgk  quadv  triplequad

 ** New functions for reading and writing images.

    The imwrite and imread functions have been included in Octave.
    These functions require the GraphicsMagick library.  The new
    function imfinfo provides information about an image file (size,
    type, colors, etc.)

 ** The input_event_hook function has been replaced by the pair of
    functions add_input_event_hook and remove_input_event_hook so that
    more than one hook function may be installed at a time.

 ** Other miscellaneous new functions.

      addtodate          hypot                       reallog
      bicgstab           idivide                     realpow
      cellslices         info                        realsqrt
      cgs                interp1q                    rectint
      command_line_path  isdebugmode                 regexptranslate
      contrast           isfloat                     restoredefaultpath
      convn              isstrprop                   roundb
      cummin             log1p                       rundemos
      cummax             lsqnonneg                   runlength
      datetick           matlabroot                  saveobj
      display            namelengthmax               spaugment
      expm1              nargoutchk                  strchr
      filemarker         pathdef                     strvcat
      fstat              perl                        subspace
      full               prctile                     symvar
      fzero              quantile                    treelayout
      genvarname         re_read_readline_init_file  validatestring
      histc

 ** Changes to strcat.

    The strcat function is now compatible with Matlab's strcat
    function, which removes trailing whitespace when concatenating
    character strings.  For example

      strcat ('foo ', 'bar')
      ==> 'foobar'

    The new function cstrcat provides the previous behavior of
    Octave's strcat.

 ** Improvements to the help functions.

    The help system has been mostly re-implemented in .m files to make
    it easier to modify.  Performance of the lookfor function has been
    greatly improved by caching the help text from all functions that
    are distributed with Octave.  The pkg function has been modified
    to generate cache files for external packages when they are
    installed.

 ** Deprecated functions.

    The following functions were deprecated in Octave 3.0 and will be
    removed from Octave 3.4 (or whatever version is the second major
    release after 3.0):

      beta_cdf         geometric_pdf       pascal_pdf
      beta_inv         geometric_rnd       pascal_rnd
      beta_pdf         hypergeometric_cdf  poisson_cdf
      beta_rnd         hypergeometric_inv  poisson_inv
      binomial_cdf     hypergeometric_pdf  poisson_pdf
      binomial_inv     hypergeometric_rnd  poisson_rnd
      binomial_pdf     intersection        polyinteg
      binomial_rnd     is_bool             setstr
      chisquare_cdf    is_complex          struct_contains
      chisquare_inv    is_list             struct_elements
      chisquare_pdf    is_matrix           t_cdf
      chisquare_rnd    is_scalar           t_inv
      clearplot        is_square           t_pdf
      clg              is_stream           t_rnd
      com2str          is_struct           uniform_cdf
      exponential_cdf  is_symmetric        uniform_inv
      exponential_inv  is_vector           uniform_pdf
      exponential_pdf  isstr               uniform_rnd
      exponential_rnd  lognormal_cdf       weibcdf
      f_cdf            lognormal_inv       weibinv
      f_inv            lognormal_pdf       weibpdf
      f_pdf            lognormal_rnd       weibrnd
      f_rnd            meshdom             weibull_cdf
      gamma_cdf        normal_cdf          weibull_inv
      gamma_inv        normal_inv          weibull_pdf
      gamma_pdf        normal_pdf          weibull_rnd
      gamma_rnd        normal_rnd          wiener_rnd
      geometric_cdf    pascal_cdf
      geometric_inv    pascal_inv

    The following functions are now deprecated in Octave 3.2 and will
    be removed from Octave 3.6 (or whatever version is the second major
    release after 3.2):

      create_set          spcholinv  spmax
      dmult               spcumprod  spmin
      iscommand           spcumsum   spprod
      israwcommand        spdet      spqr
      lchol               spdiag     spsum
      loadimage           spfind     spsumsq
      mark_as_command     spinv      str2mat
      mark_as_rawcommand  spkron     unmark_command
      spatan2             splchol    unmark_rawcommand
      spchol              split
      spchol2inv          splu

Summary of important user-visible changes for version 3.0 (2007-12-21):
----------------------------------------------------------------------

 ** Compatibility with Matlab graphics is much better now.  We now
    have some graphics features that work like Matlab's Handle
    Graphics (tm):

    + You can make a subplot and then use the print function to
      generate a file with the plot.

    + RGB line colors are supported if you use gnuplot 4.2.  Octave
      can still use gnuplot 4.0, but there is no way to set arbitrary
      line colors with it when using the Matlab-style plot functions.
      There never was any way to do this reliably with older versions
      of gnuplot (whether run from Octave or not) since it only
      provided a limited set to choose from, and they were terminal
      dependent, so choosing color 1 with the X11 terminal would be
      different from color 1 with the PostScript terminal.  Valid RGB
      colors for gnuplot 4.0 are the eight possible combinations of 0
      and 1 for the R, G and B values. Invalid values are all mapped
      to the same color.

      This also affects patch objects used in the bar, contour, meshc
      and surfc functions, where the bars and contours will be
      monochrome. A workaround for this is to type "colormap gmap40"
      that loads a colormap that in many cases will be adequate for
      simple bar and contour plots.

    + You can control the width of lines using (for example):

        line (x, y, "linewidth", 4, "color", [1, 0, 0.5]);

      (this also shows the color feature).

    + With gnuplot 4.2, image data is plotted with gnuplot and may be
      combined with other 2-d plot data.

    + Lines for contour plots are generated with an Octave function, so
      contour plots are now 2-d plots instead of special 3-d plots, and
      this allows you to plot additional 2-d data on top of a contour
      plot.

    + With the gnuplot "extended" terminals the TeX interpreter is
    emulated. However, this means that the TeX interpreter is only
    supported on the postscript terminals with gnuplot 4.0. Under
    gnuplot 4.2 the terminals aqua, dumb, png, jpeg, gif, pm, windows,
    wxt, svg and x11 are supported as well.

    + The following plot commands are now considered obsolete and will
      be removed from a future version of Octave:

        __gnuplot_set__
        __gnuplot_show__
        __gnuplot_plot__
        __gnuplot_splot__
        __gnuplot_replot__

      Additionally, these functions no longer have any effect on plots
      created with the Matlab-style plot commands (plot, line, mesh,
      semilogx, etc.).

    + Plot property values are not extensively checked.  Specifying
      invalid property values may produce unpredictable results.

    + Octave now sends data over the same pipe that is used to send
      commands to gnuplot.  While this avoids the problem of
      cluttering /tmp with data files, it is no longer possible to use
      the mouse to zoom in on plots.  This is a limitation of gnuplot,
      which is unable to zoom when the data it plots is not stored in
      a file.  Some work has been done to fix this problem in newer
      versions of gnuplot (> 4.2.2).  See for example, this thread

        http://www.nabble.com/zooming-of-inline-data-tf4357017.html#a12416496

      on the gnuplot development list.


 ** The way Octave handles search paths has changed.  Instead of
    setting the built-in variable LOADPATH, you must use addpath,
    rmpath, or path to manipulate the function search path.  These
    functions will maintain "." at the head of the path, for
    compatibility with Matlab.

    Leading, trailing or doubled colons are no longer special.
    Now, all elements of the search path are explicitly included in
    the path when Octave starts.  To display the path, use the path
    function.

    Path elements that end in // are no longer searched recursively.
    Instead, you may use addpath and the genpath function to add an
    entire directory tree to the path.  For example,

      addpath (genpath ("~/octave"));

    will add ~/octave and all directories below it to the head of the
    path.


 ** Previous versions of Octave had a number of built-in variables to
    control warnings (for example, warn_divide_by_zero).  These
    variables have been replaced by warning identifiers that are used
    with the warning function to control the state of warnings.

    For example, instead of writing

      warn_divide_by_zero = false;

    to disable divide-by-zero warnings, you should write

      warning ("off", "Octave:divide-by-zero");

    You may use the same technique in your own code to control
    warnings.  For example, you can use

      warning ("My-package:phase-of-the-moon",
               "the phase of the moon could cause trouble today");

    to allow users to control this warning using the
    "My-package:phase-of-the-moon" warning identifier.

    You may also enable or disable all warnings, or turn them into
    errors:

      warning ("on", "all");
      warning ("off", "all");
      warning ("error", "Octave:divide-by-zero");
      warning ("error", "all");

    You can query the state of current warnings using

      warning ("query", ID)
      warning ("query")

    (only those warning IDs which have been explicitly set are
    returned).

    A partial list and description of warning identifiers is available
    using

      help warning_ids


 ** All built-in variables have been converted to functions.  This
    change simplifies the interpreter and allows a consistent
    interface to internal variables for user-defined packages and the
    core functions distributed with Octave.  In most cases, code that
    simply accesses internal variables does not need to change.  Code
    that sets internal variables will change.  For example, instead of
    writing

      PS1 = ">> ";

    you will need to write

      PS1 (">> ");

    If you need write code that will run in both old and new versions
    of Octave, you can use something like

      if (exist ("OCTAVE_VERSION") == 5)
        ## New:
        PS1 (">> ");
      else
        ## Old:
        PS1 = ">> ";
      endif


 ** For compatibility with Matlab, the output order of Octave's
    "system" function has changed from

      [output, status] = system (cmd);

    to

      [status, output] = system (cmd);


 ** For compatibility with Matlab, the output of Octave's fsolve
    function has been changed from

      [x, info, msg] = fsolve (...);

    to

      [x, fval, info] = fsolve (...);


 ** For compatibility with Matlab, normcdf, norminv, normpdf, and
    normrnd have been modified to compute distributions using the
    standard deviation instead of the variance.


 ** For compatibility with Matlab, gamcdf, gaminv, gampdf, gamrnd,
    expcdf, expinv, exppdf and exprnd have been modified to compute
    the distributions using the standard scale factor rather than
    one over the scale factor.

---------------------------------------------------------

See NEWS.2 for old news.
