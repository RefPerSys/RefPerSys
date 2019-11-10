#!/bin/bash -x

echo $0 Starting in $(pwd)
rm -rfv ecai.{aux,bcf,blg,log,pdf}

printf "\n\n ===============================================\n"
printf "Generating LaTeX file...\n"

lualatex --shell-escape --halt-on-error ecai
biber ecai
lualatex --shell-escape --halt-on-error ecai

