#!/bin/bash -x

echo $0 Starting in $(pwd)
rm -rfv ecai2020-highlight-refpersys.{aux,bcf,blg,log,pdf} generated-ecai2020-gitid.tex

printf "\n\n generating ecai2020-gitid\n"

rawgittag="$(git log --format=oneline -1 --abbrev=16 --abbrev-commit -q|cut -d' ' -f1)"

if git status -s | grep -q '^.M' > /dev/null ; then
    gittag=$(printf "%s++" "$rawgittag")
else
    gittag=$(printf "%s..." "$rawgittag")
fi
printf "\\\\newcommand{\\\\rpsgitcommit}[0]{%s}\n" "$gittag" > generated-ecai2020-gitid.tex

printf "\n\n ================ SVG processing by inkscape of "; echo *.svg "files."
for svgfile in *.svg ; do
    svgbase=$(basename $svgfile .svg)
    inkscape --without-gui --export-pdf=$svgbase.pdf $svgfile
    inkscape --without-gui --export-eps=$svgbase.eps $svgfile
done


## see http://graphviz.org/
printf "\n\n ================ dot processing by graphviz of "; echo  dot*.dot "files."
ls -ls dot*.dot
for dotfile in dot*.dot ; do
    dotbase=$(basename $dotfile .dot)
    dot -v -Teps -o $dotbase.eps  $dotfile
    dot -v -Tpdf -o $dotbase.pdf  $dotfile
    dot -v -Tsvg -o $dotbase.svg  $dotfile
done


printf "\n\n ===============================================\n"
printf "LaTeXing and BibTeXing files...\n"

pdflatex --shell-escape --halt-on-error ecai2020-highlight-refpersys
bibtex ecai2020-highlight-refpersys
pdflatex --shell-escape --halt-on-error ecai2020-highlight-refpersys
bibtex ecai2020-highlight-refpersys
pdflatex --shell-escape --halt-on-error ecai2020-highlight-refpersys
bibtex ecai2020-highlight-refpersys
pdflatex --shell-escape --halt-on-error ecai2020-highlight-refpersys

