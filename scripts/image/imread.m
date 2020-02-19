########################################################################
##
## Copyright (C) 2002-2020 The Octave Project Developers
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
## @deftypefn  {} {[@var{img}, @var{map}, @var{alpha}] =} imread (@var{filename})
## @deftypefnx {} {[@dots{}] =} imread (@var{url})
## @deftypefnx {} {[@dots{}] =} imread (@dots{}, @var{ext})
## @deftypefnx {} {[@dots{}] =} imread (@dots{}, @var{idx})
## @deftypefnx {} {[@dots{}] =} imread (@dots{}, @var{param1}, @var{value1}, @dots{})
## Read images from various file formats.
##
## Read an image as a matrix from the file @var{filename} or from the online
## resource @var{url}.  If neither is given, but @var{ext} was specified, look
## for a file with the extension @var{ext}.
##
## The size and class of the output depends on the format of the image.  A
## color image is returned as an @nospell{MxNx3} matrix.  Grayscale and
## black-and-white images are of size @nospell{MxN}@.  Multipage images will
## have an additional 4th dimension.
##
## The bit depth of the image determines the class of the output:
## @qcode{"uint8"}, @qcode{"uint16"}, or @qcode{"single"} for grayscale and
## color, and @qcode{"logical"} for black-and-white.  Note that indexed images
## always return the indexes for a colormap, independent of whether @var{map}
## is a requested output.  To obtain the actual RGB image, use @code{ind2rgb}.
## When more than one indexed image is being read, @var{map} is obtained from
## the first.  In some rare cases this may be incorrect and @code{imfinfo} can
## be used to obtain the colormap of each image.
##
## See the Octave manual for more information in representing images.
## (@pxref{Representing Images})
##
## Some file formats, such as TIFF and GIF, are able to store multiple images
## in a single file.  @var{idx} can be a scalar or vector specifying the
## index of the images to read.  By default, Octave will read only the first
## page.
##
## Depending on the file format, it is possible to configure the reading of
## images with @var{parameter}, @var{value} pairs.  The following options are
## supported:
##
## @table @asis
## @item @qcode{"Frames"} or @qcode{"Index"}
## This is an alternative method to specify @var{idx}.  When specifying it
## in this way, its value can also be the string @qcode{"all"}.
##
## @item @qcode{"Info"}
## This option exists for @sc{matlab} compatibility, but has no effect.  For
## maximum performance when reading multiple images from a single file, use
## the @qcode{"Index"} option.
##
## @item @qcode{"PixelRegion"}
## Controls the image region that is read.  The value must be a cell array with
## two arrays of 3 elements @code{@{[@var{rows}], [@var{cols}]@}}.  The
## elements in the array are the start, increment, and end pixel to be read.
## If the increment value is omitted it defaults to 1.  For example, the
## following are all equivalent:
##
## @example
## @group
## imread (filename, "PixelRegion", @{[200 600], [300 700]@});
## imread (filename, "PixelRegion", @{[200 1 600], [300 1 700]@});
## imread (filename)(200:600, 300:700);
## @end group
## @end example
##
## @end table
##
## @seealso{imwrite, imfinfo, imformats}
## @end deftypefn

function [img, varargout] = imread (filename, varargin)

  if (nargin < 1)
    print_usage ();
  elseif (! ischar (filename))
    error ("imread: FILENAME must be a string");
  endif

  [img, varargout{1:nargout-1}] = ...
    imageIO ("imread", @__imread__, "read", filename, varargin{:});

endfunction


%!testif HAVE_MAGICK
%! vpng = [ ...
%!  137,  80,  78,  71,  13,  10,  26,  10,   0,   0, ...
%!    0,  13,  73,  72,  68,  82,   0,   0,   0,   3, ...
%!    0,   0,   0,   3,   8,   2,   0,   0,   0, 217, ...
%!   74,  34, 232,   0,   0,   0,   1, 115,  82,  71, ...
%!   66,   0, 174, 206,  28, 233,   0,   0,   0,   4, ...
%!  103,  65,  77,  65,   0,   0, 177, 143,  11, 252, ...
%!   97,   5,   0,   0,   0,  32,  99,  72,  82,  77, ...
%!    0,   0, 122,  38,   0,   0, 128, 132,   0,   0, ...
%!  250,   0,   0,   0, 128, 232,   0,   0, 117,  48, ...
%!    0,   0, 234,  96,   0,   0,  58, 152,   0,   0, ...
%!   23, 112, 156, 186,  81,  60,   0,   0,   0,  25, ...
%!   73,  68,  65,  84,  24,  87,  99,  96,  96,  96, ...
%!  248, 255, 255,  63, 144,   4,  81, 111, 101,  84, ...
%!   16,  28, 160,  16,   0, 197, 214,  13,  34,  74, ...
%!  117, 213,  17,   0,   0,   0,   0,  73,  69,  78, ...
%!   68, 174,  66,  96, 130];
%! filename = [tempname() ".png"];
%! unwind_protect
%!   fid = fopen (filename, "wb");
%!   fwrite (fid, vpng);
%!   fclose (fid);
%!   A = imread (filename);
%! unwind_protect_cleanup
%!   unlink (filename);
%! end_unwind_protect
%! assert (A(:,:,1), uint8 ([0, 255, 0; 255, 237, 255; 0, 255, 0]));
%! assert (A(:,:,2), uint8 ([0, 255, 0; 255,  28, 255; 0, 255, 0]));
%! assert (A(:,:,3), uint8 ([0, 255, 0; 255,  36, 255; 0, 255, 0]));

%!function [r, cmap, a] = write_and_read (w, f_ext, varargin)
%!  filename = [tempname() "." f_ext];
%!  unwind_protect
%!    imwrite (w, filename);
%!    [r, cmap, a] = imread (filename, varargin{:});
%!  unwind_protect_cleanup
%!    unlink (filename);
%!  end_unwind_protect
%!endfunction

## test PixelRegion option
%!testif HAVE_MAGICK
%! w = randi (255, 100, 100, "uint8");
%! [r, cmap, a] = write_and_read (w, "tif", "PixelRegion", {[50 70] [20 40]});
%! assert (r, w(50:70, 20:40));
%! [r, cmap, a] = write_and_read (w, "tif", "PixelRegion", {[50 2 70] [20 3 40]});
%! assert (r, w(50:2:70, 20:3:40));

## If a file does not exist, it's the job of imread to check the file
## exists before sending it over to __imread__ or whatever function
## is defined in imformats to handle that specific format.  This is the
## same in imfinfo.  So in this test we replace one format in imformats
## with something that will not give an error if the file is missing
## and make sure we do get an error.
%!testif HAVE_MAGICK
%! fmt = fmt_ori = imformats ("jpg");
%! fmt.read = @true;
%! error_thrown = false;
%! imformats ("update", "jpg", fmt);
%! unwind_protect
%!   try
%!     imread ("I_sure_hope_this_file_does_not_exist.jpg");
%!   catch
%!     error_thrown = true;
%!   end_try_catch
%! unwind_protect_cleanup
%!   imformats ("update", "jpg", fmt_ori);
%! end_unwind_protect
%! assert (error_thrown, true);

## make one of the formats read, return what it received as input to
## confirm that the input parsing is working correctly
%!testif HAVE_MAGICK
%! fname = [tempname() ".jpg"];
%! def_fmt = imformats ();
%! fid = fopen (fname, "w");
%! unwind_protect
%!   fmt = imformats ("jpg");
%!   fmt.read = @(varargin) varargin;
%!   imformats ("update", "jpg", fmt);
%!   assert (imread (fname), {fname});
%!   assert (imread (fname, "jpg"), {fname});
%!   assert (imread (fname(1:end-4), "jpg"), {fname});
%!   extra_inputs = {"some", 89, i, {6 7 8}};
%!   assert (imread (fname, extra_inputs{:}), {fname, extra_inputs{:}});
%!   assert (imread (fname, "jpg", extra_inputs{:}), {fname, extra_inputs{:}});
%!   assert (imread (fname(1:end-4), "jpg", extra_inputs{:}), {fname, extra_inputs{:}});
%! unwind_protect_cleanup
%!   fclose (fid);
%!   unlink (fname);
%!   imformats (def_fmt);
%! end_unwind_protect

## Test for bug #41584 (some GM coders report saturated channels as binary)
%!testif HAVE_MAGICK <41584>
%! im = zeros ([16 16 3], "uint8");
%! im(:,:,1) = 255;
%! im(:,:,3) = repmat (0:16:255, [16 1]);
%! [r, cmap, a] = write_and_read (im, "png");
%! assert (class (r), "uint8");
%! assert (isempty (cmap));
%! assert (isempty (a));
