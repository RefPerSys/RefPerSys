#!/bin/bash -x
# see project refpersys.org -  git@github.com:RefPerSys/RefPerSys.git
# file doc/IntelliSys2021/make-paper.sh
# submission to  the https://saiconference.com/IntelliSys conference
## Important Dates
##
##  Submission Deadline: 15 February 2021 (FIRM DEADLINE)
##  Acceptance Notification : 01 March 2021
##  Author Registration : 15 March 2021
##  Camera Ready Submission : 01 April 2021
##  Conference Dates : 2-3 September 2021


echo $0 Starting in $(pwd)
rm -rfv refpersys-IntelliSys2021.{aux,bcf,blg,log,pdf} generated-ecai2020-gitid.tex

printf "\n\n generating IntelliSys2021-gitid\n"

rawgittag="$(git log --format=oneline -1 --abbrev=16 --abbrev-commit -q|cut -d' ' -f1)"

if git status -s | grep -q '^.M' > /dev/null ; then
    gittag=$(printf "%s++" "$rawgittag")
else
    gittag=$(printf "%s..." "$rawgittag")
fi
printf "\\\\newcommand{\\\\rpsgitcommit}[0]{%s}\n" "$gittag" > generated-intellisys-commands.tex-$$
date +"\\newcommand{\\rpsdate}[0]{%Y, %b, %d %r}%n" >> generated-intellisys-commands.tex-$$
mv generated-intellisys-commands.tex-$$ generated-intellisys-commands.tex

printf "\n\n ================ SVG processing by inkscape of "; echo *.svg "files."
for svgfile in *.svg ; do
    svgbase=$(basename $svgfile .svg)
    inkscape --without-gui --export-pdf=$svgbase.pdf $svgfile
    inkscape --without-gui --export-eps=$svgbase.eps $svgfile
done


## see http://graphviz.org/
if ls -ls dot*.dot ; then
    printf "\n\n ================ dot processing by graphviz of "; echo  dot*.dot "files."
    for dotfile in dot*.dot ; do
	dotbase=$(basename $dotfile .dot)
	dot -v -Teps -o $dotbase.eps  $dotfile
	dot -v -Tpdf -o $dotbase.pdf  $dotfile
	dot -v -Tsvg -o $dotbase.svg  $dotfile
    done
else
    printf "\n\n ================ No dot processing by graphviz."
fi


printf "\n\n ===============================================\n"
printf "LaTeXing and BibTeXing files...\n"

pdflatex --shell-escape --halt-on-error refpersys-IntelliSys2021
bibtex refpersys-IntelliSys2021
pdflatex --shell-escape --halt-on-error refpersys-IntelliSys2021
bibtex refpersys-IntelliSys2021
pdflatex --shell-escape --halt-on-error refpersys-IntelliSys2021
bibtex refpersys-IntelliSys2021
pdflatex --shell-escape --halt-on-error refpersys-IntelliSys2021

## end of script  doc/IntelliSys2021/make-paper.sh in RefPerSys


