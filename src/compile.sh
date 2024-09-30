#!/bin/bash
#########################################################
# Fast Compile.....                                     #
# Made For compiling the 50 someodd attacks needed      #
# for msdos    .. but can be used to mass compile       #
# any c sources....                                     #
#                                                       #
# Usage ./compile.sh in the dir of the c sources ez eh? #
#                                                       #
# Spikeman                          spikeman@myself.com #
# Thanks spikeman! great code! it works for msdos too.  #
#########################################################
#
# Trap all kills (except kill -9) cause we don't
# want to abort while compiling
#
trap "" 0 1 2 3 15

for file in `find . -name "*.c"`
do
        echo -n "Compiling $file..."
        exe=`echo $file |cut -f2 -d. |cut -f2 -d/`
        if cc $file -o $exe 1>/dev/null 2>/dev/null
        then
                echo "compiled cleanly."
        else
                echo "had some sort of compile error/warning."
        fi
done
