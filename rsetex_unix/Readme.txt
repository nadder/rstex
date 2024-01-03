This is a version of rstex_unix with e-tex extensions.

To use LaTeX the following procedure may be followed:
* Download the LaTeX files from a TeX distribution, e.g. TeXLive.
* Set the environment variables pointing to the LaTeX files on your system e.g.
 - TEXINPUTS=.:/usr/local/texlive/2018/texmf-dist/tex/latex/base:
             /usr/local/texlive/2018/texmf-dist/tex/generic/hyphen
 - TEXFONTS=/usr/local/texlive/2018/texmf-dist/fonts/tfm/public/cm:
            /usr/local/texlive/2018/texmf-dist/fonts/tfm/public/knuth-lib:
            /usr/local/texlive/2018/texmf-dist/fonts/tfm/public/latex-fonts
            
* Run "./rsetex *latex.ltx"
  (you may need to escape the asterisk depending on shell)
* The previous step should create the file "latex.fmt" in your current directory.
* Run "./rsetex -format=latex mysimplelatex.tex" with any simple latex document.
* If everything works, no errors should be printed, and the files "mysimplelatex.dvi" and
  "mysimplelatex.log" should be created.
* Celebrate!

Tips:
* If rsetex complains about not finding a file, locate the file on your disc
  and add the path to the appropriate environment variable.
* If rsetex complains about capacity exceeded, locate the relevant parameter in rsetex.h and increase it.
