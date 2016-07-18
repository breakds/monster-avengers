#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ $# -eq 0 ]; then
    $DIR/../regression $HOME/dataset/mhx/mhx.db $(ls $DIR/*.lisp)
else
    $DIR/../regression $HOME/dataset/mhx/mhx.db $DIR/$1.lisp
fi
