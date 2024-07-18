#!/bin/sh
mkdir -p Fonts

FONTS=/usr/share/fonts/Lilyglyphs/
EXP=$(echo ${FONTS}*.otf)

for file in ${EXP}; do
    # echo $file
    name=$(basename $file);
    ttx -o ./Fonts/${name}.xml -t cmap $file;
done
