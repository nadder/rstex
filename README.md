[![Build Status](https://travis-ci.com/nadder/rstex.svg?branch=master)](https://travis-ci.com/nadder/rstex)
# rsTeX and related programs

## rsTeX
This is a manual translation of [tex.web](https://ctan.org/pkg/tex) version 3.14159265 into standard C++.
Mostly it's a direct translation of the original pascal source with very minimal changes.
The amount of C++ (that is not also C) used is very minimal. Notably I added an array
class to keep the same indexing as in the original and to also get boundschecking.

What is the purpose of this program? Well, I've long been fascinated by
Donald Knuth and his work and especially TeX. TeX is one of the most well
documented programs out there and the source code is of course readily available
but, one lacking feature of current implementations is that because the source code is
machine converted into C it's not easy to understand or step through the code
in a debugger. So that's why I decided to translate the program to C++. By doing so
one also hopefully gains some understanding of what the code does.

Another aspect was that I was interested in knowing how long it would take
to write the program. I didn't keep a stop watch at hand but a rough estimate is
that it took about 30 hours to type the whole program in and more than twice that to
fix all typos and other bugs introduced before it finally passed the trip test.
Which was kind of what I expected except I thought the debugging time would be on 
par with the time it took to type it in. Had I been more systematic in my approach
and introduced more asserts and checks from the beginning the debugging time would
have been considerably shorter.

The program consists of two files rstex.h.pre and rstex.cpp.pre.
These two files must be preprocessed by another program which converts all
special strings in the program to numbers (as the original tangle program does),
and makes a string pool file.
After this preprocessing we are left with three files rstex.h, rstex.cpp, and tex.pool.
rstex.cpp can then be compiled by any non-ancient C++ compiler, and the executable can be run.

#### Notable omissions
* Currently no path searching is done so all files the program needs are supposed
  to exist in the current directory. (See the Unix and Windows specific versions to avoid this).
* No other files needed for a complete system is included, i.e. fmt files, fonts, tfm files
  macro packages etc.

#### Disclaimer
This is NOT the official version of TeX, it is only based upon the official version of TeX.
It may contain bugs not present in the official version. It does pass the trip test,
but the trip test does not test everything. In no way shall the author of this program
be responsible for any damages, direct or indirect, that may result out of its use.
It is not meant to be a working typesetting system, it is meant for educational purpose,
for those who want to understand how this complex software works.

## rsMetaFont
I have also translated [mf.web](https://ctan.org/pkg/metafont), currently only for windows, but the plan is to make a Qt version which
can run on both Linux and Windows.

## TeXFontViewerQt
A small program to view the original TeX font bitmaps (PXL, GF, PK, or MF).
This program works on both Linux and Windows, and Mac but requires [Qt](https://www.qt.io) to be installed.

## DVIViewer
Far from complete program to read DVI files, only for Windows at the moment but perhaps a Qt version
will exist some day.

# Copyright
These programs are copyright (C) 2018 by Richard Sandberg (mylodon at gmail dot com).
The original tex.web is copyright (C) 1982 by Donald Knuth.

# License
These programs are for educational purpose only, all commercial use
is strictly forbidden.

# Donate
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/nadder1)
