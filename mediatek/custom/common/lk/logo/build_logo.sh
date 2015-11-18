#!/bin/bash

function help()
{
    echo "eg:buildlogodire [orgin directory name] [target directory name]"
}
#$2 target file name
function buildlogodire()
{
    if [ $# -ne 2 ]; then 
	echo "need two args!"
	return
    fi

    if [ ! -d $1 ]; then
        echo "$1 can't find!"
        return
    fi

    if [ -d $2 ]; then
        echo "$2 exists already!"
	return
    fi

    cp -r $1 $2
    arg1=$1
    arg2=$2

    for originfile in `ls $2`
    do
#       echo $originfile
	target=${originfile/$arg1/$arg2}
#        echo ${target}
        mv $2/$originfile $2/$target
    done
}


echo "Build logo resource..."
buildlogodire $1 $2













