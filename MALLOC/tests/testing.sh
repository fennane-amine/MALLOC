#!/bin/bash
GREEN="\033[32m"
WHITE="\033[0m"
RED="\033[31m"

make
testingmalloc()
{
    while read -r line;do
        echo $line
        echo $line > t
        line=$(sed -e "s|~|$HOME|g" < t)
        rm t
        $line > testing__malloc
        LD_PRELOAD=./libmalloc.so $line > testing_my_malloc
        comp=$(diff testing__malloc testing_my_malloc)
        if [ "$comp" == "" ]; then
            echo -e "$GREEN[ ok ]$WHITE"
        else
            echo -e "$RED[ error ]$WHITE"
        fi
    done <tests/testing_commands
    rm testing__malloc
    rm testing_my_malloc
}
testingmalloc
