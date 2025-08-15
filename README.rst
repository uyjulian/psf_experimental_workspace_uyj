PSF/PSF2 experimental workspace
===============================

Testing area for PSF and PSF2 format files.

How to play PSF or PSF2 files
-----------------------------

Use the PSF component for foobar2000. See
https://www.foobar2000.org/components/author/kode54

Description of different libraries
----------------------------------

Audio Overload
~~~~~~~~~~~~~~

| HLE playback with no ROM file needed.
| However, it does not implement ROM module behavior entirely correctly.

PSX MIPS CPU module appears to be based off of MAME 0.97 or below.

https://rbelmont.mameworld.info/?page_id=221

Highly Experimental
~~~~~~~~~~~~~~~~~~~

| LLE playback with ROM file required.
| Can pass in original or trimmed ROM file.

https://web.archive.org/web/20120701000530/http://www.neillcorlett.com/he

Sexypsf
~~~~~~~

HLE playback with no ROM file needed.

Based on PCSX 1.4 source code.

http://projects.raphnet.net/#sexypsf

Description of programs
-----------------------

PSFLab
~~~~~~

GUI utility for debugging PSF

https://web.archive.org/web/20120626042118/http://www.neillcorlett.com/psflab/

PSF-o-Cycle
~~~~~~~~~~~

Additional PSF modification functions

https://web.archive.org/web/20120705060713/http://www.neillcorlett.com/psfocycle/

PSFPoint
~~~~~~~~

Command line PSF tagger

https://web.archive.org/web/20120718162806/http://www.neillcorlett.com/psfpoint/

Simple PSF utilities
~~~~~~~~~~~~~~~~~~~~

| Includes three command-line utilities:
| bin2psf - Convert any binary file into a PSF file with arbitrary
  version number. Useful for QSF files and other future sub-formats.
| exe2psf - Convert a PS-X EXE file to a PSF1 file.
| psf2exe - Convert a PSF1 file to a PS-X EXE file.
| Source code is included, but not commented very well, for all three
  utilities.

https://web.archive.org/web/20120716233303/http://www.neillcorlett.com/downloads/simple_psf_utils.zip

mkpsf2
~~~~~~

Scans a given directory and creates a PSF2 file with all its contents.
Source code is included.

https://web.archive.org/web/20110702212147/http://www.neillcorlett.com/downloads/mkpsf2.zip

PSF2 Starter Kit
~~~~~~~~~~~~~~~~

Starter modules for PSF2

- cdvdnul.irx
- fakesif.irx
- myhost.irx
- psf2.irx
- sq.irx

https://web.archive.org/web/20110807193409/http://www.neillcorlett.com/downloads/PSF2Kit.zip

VGMToolbox
~~~~~~~~~~

Various utilities

https://sourceforge.net/projects/vgmtoolbox/

Playback of PSF through SDL2
----------------------------

.. code:: bash

   ./psf_tester infile.psf

Playback of PSF through PCM
---------------------------

.. code:: bash

   ./psf_tester infile.psf2 | ffplay -f s16le -ar 48000 -ac 2 -

.. code:: bash

   ./psf_tester infile.psf | ffplay -f s16le -ar 44100 -ac 2 -

License
-------

| License of own written code is MIT license.
| For license of other code, see top of code.
