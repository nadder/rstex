This is a modified version of rstex for Unix-like systems. Similar to Knuth's changes for SPARC.
The following extra features are added
* Increased memory, hash size, string pool etc.
* -format=<format file> command line option for ``preloading'' format.
* Path searching using default locations and environment variables:
  TEXINPUTS, TEXFONTS, TEXFORMATS, and TEXPOOL. e.g. TEXFONTS=.:/home/fonts
* When e is pressed in response to an error message, brings up editor using default or
  environment variable TEXEDIT, e.g. TEXEDIT="emacs +%d %s"

To create a plain format file you must:
1. Download the files needed by plain.tex.
2. Set the environment paths correctly. On a TeXLive install it may look like this e.g.:
  TEXINPUTS=.:/usr/local/texlive/2018/texmf-dist/tex/plain/base:/usr/local/texlive/2018/texmf-dist/tex/generic/hyphen
  TEXFONTS=/usr/local/texlive/2018/texmf-dist/fonts/tfm/public/cm:/usr/local/texlive/2018/texmf-dist/fonts/tfm/public/knuth-lib

Now you can create a plain format by running:

"rstex plain '\dump'"

From now on, you can run a texjob with:

"rstex -format=plain mytexfile.tex"

to start rsTeX with plain preloaded.
