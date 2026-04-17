#!/bin/bash -x
# file RefPerSys/doc/pfia2026/build-pfia2026-refpersys.sh
# Build a submission in French as PDF file to https://pfia26.cril.fr/
# 
# https://pfia26.cril.fr/journ%C3%A9es/journee3/incertitude-ia/ 
echo $0 Starting in $(pwd)
rm -rfv RefPerSys-PFIA-2026-Starynkevitch.{aux,bcf,blg,log,pdf} generated-pfia2026-gitid.tex

printf "\n\n generating generated-pfia2026.tex\n"

rawgittag="$(git log --format=oneline -1 --abbrev=16 --abbrev-commit -q|cut -d' ' -f1)"

if git status -s | grep -q '^.M' > /dev/null ; then
    gittag=$(printf "%s++" "$rawgittag")
else
    gittag=$(printf "%s..." "$rawgittag")
fi

echo '%generated file generated-pfia2026.tex by build-pfia2026-refpersys.sh' > generated-pfia2026.tex
printf "\\\\newcommand{\\\\rpsgitcommit}[0]{%s}\n" "$gittag" >> generated-pfia2026.tex
env LANG=fr_FR.UTF-8 date +"\\newcommand{\\rpsgitdate}[0]{%d %b %Y}%n" >> generated-pfia2026.tex
echo '%end of generated file generated-pfia2026.tex' >> generated-pfia2026.tex

printf "\n\n ================ SVG processing by inkscape of "; echo *.svg "files."
for svgfile in *.svg ; do
    svgbase=$(basename $svgfile .svg)
    inkscape -o $svgbase.pdf $svgfile
    inkscape -o $svgbase.eps $svgfile
done

### running LuaLaTeX
lualatex --shell-escape  --halt-on-error RefPerSys-PFIA-2026-Starynkevitch

### running BibTeX
bibtex RefPerSys-PFIA-2026-Starynkevitch

if [[ $? == 0 ]]; then
   lualatex --shell-escape  --halt-on-error RefPerSys-PFIA-2026-Starynkevitch
fi
##% For emacs:
##%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
## Local Variables: ;;
## compile-command: "./build-pfia2026-refpersys.sh" ;;
## End: ;;
##%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
## eof build-pfia2026-refpersys.sh

