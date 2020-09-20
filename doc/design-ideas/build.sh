#!/bin/bash -x
echo $0 starting in $(pwd)
rm -vf  refpersys-design.{aux,bcf,blg,log,run.xml,toc} dot*.{eps,pdf,svg}

################
## see http://inkscape.org/
## we need Inkscape 1.0
INKSCAPE_VERSION=$(inkscape --version | head -1)
if inkscape --version | fgrep 'Inkscape 1.0' ; then
    printf "Using %s\n" $INKSCAPE_VERSION
else
    printf "Bad inkscape version %s; %s needs 1.0 from http://inkscape.org/\n" $INKSCAPE_VERSION $0 > /dev/stderr
    exit 1
fi

printf "\n\n ================ SVG processing by inkscape of "; echo *.svg "files."

for svgfile in ../CC-BY-SA-icon.svg \
		   spiral-model-softdevel.svg spiral-stairs.svg \
	       heap-refpersys.svg ; do
    svgbase=$(basename $svgfile .svg)
    inkscape --batch --export-filename=$svgbase.pdf --export-type=pdf $svgfile
    inkscape --batch --export-filename=$svgbase.eps --export-type=eps $svgfile
done

################
## see http://graphviz.org/
printf "\n\n ================ dot processing by graphviz of "; echo  dot*.dot "files."
ls -ls dot*.dot
for dotfile in dot*.dot ; do
    dotbase=$(basename $dotfile .dot)
    dot -v -Teps -o $dotbase.eps  $dotfile
    dot -v -Tpdf -o $dotbase.pdf  $dotfile
    dot -v -Tsvg -o $dotbase.svg  $dotfile
done
ls -ls --sort=time dot*



################
printf "\n\n ================ latexing and bibering refpersys-design\n"

lualatex --shell-escape  --halt-on-error refpersys-design
biber refpersys-design
lualatex --shell-escape  --halt-on-error refpersys-design
[ -f refpersys-design.idx ] && texindy -v -C utf8 -I latex refpersys-design.idx
lualatex --shell-escape  --halt-on-error refpersys-design
[ -f refpersys-design.idx ] && texindy -v -C utf8 -I latex refpersys-design.idx
if lualatex --shell-escape  --halt-on-error refpersys-design ; then
    if [ "$1" != noinstall ]; then 
	[ -x $HOME/bin/install-refpersys-design-report.sh ] && $HOME/bin/install-refpersys-design-report.sh
    fi
fi
