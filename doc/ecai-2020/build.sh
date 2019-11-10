#!/bin/bash -x

echo $0 Starting in $(pwd)
rm -rfv ecai.{aux,bcf,blg,log,pdf}

printf "\n\n ===============================================\n"
printf "Generating LaTeX file...\n"

#TDOD: why does lualatex not produce the correct formatting? need to find out

pdflatex --shell-escape --halt-on-error ecai2020-highlight-refpersys
bibtex ecai2020-highlight-refpersys
pdflatex --shell-escape --halt-on-error ecai2020-highlight-refpersys

