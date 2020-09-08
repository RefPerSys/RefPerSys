#!/bin/bash -x
# script doc/iccait-2021/build.sh in http://refpersys.org/
# see https://gitlab.com/bstarynk/refpersys/
# for submission to https://panel.waset.org/conference/2021/05/paris/ICCAIT
# 

echo $0 Starting in $(pwd)
rm -rfv iccait2021-refpersys*.{aux,bcf,blg,log,pdf} generated-iccait2021-gitid.tex

printf "\n\n generating generated-iccait2021-gitid.tex\n"

rawgittag="$(git log --format=oneline -1 --abbrev=16 --abbrev-commit -q|cut -d' ' -f1)"

if git status -s | grep -q '^.M' > /dev/null ; then
    gittag=$(printf "%s++" "$rawgittag")
else
    gittag=$(printf "%s..." "$rawgittag")
fi
printf "\\\\newcommand{\\\\rpsgitcommit}[0]{%s}\n" "$gittag" > generated-iccait2021-gitid.tex

printf "\n\n ================ SVG processing by inkscape of "; echo *.svg "files."
for svgfile in *.svg ; do
    svgbase=$(basename $svgfile.svg)
    inkscape --without-gui --export-pdf=$svgbase.pdf $svgfile
    inkscape --without-gui --export-eps=$svgbase.eps $svgfile
done

printf "\n\n generating generated-iccait2021-timestamp\n"
date +"\\newcommand{\\rpstimestamp}[0]{%c}%n" > generated-iccait2021-timestamp.tex

## see http://graphviz.org/
printf "\n\n ================ dot processing by graphviz of "; echo  dot*.dot "files."
if ls -ls dot*.dot ; then
    for dotfile in dot*.dot ; do
	dotbase=$(basename $dotfile .dot)
	dot -v -Teps -o $dotbase.eps  $dotfile
	dot -v -Tpdf -o $dotbase.pdf  $dotfile
	dot -v -Tsvg -o $dotbase.svg  $dotfile
    done
fi

## the call for papers require two versions, a blind one and a visible one


printf "\n\n ===============================================\n"
printf "LaTeXing and BibTeXing (biber) files...\n"

pdflatex --shell-escape --halt-on-error iccait2021-refpersys
bibtex iccait2021-refpersys
pdflatex --shell-escape --halt-on-error iccait2021-refpersys
pdflatex --shell-escape --halt-on-error iccait2021-refpersys


