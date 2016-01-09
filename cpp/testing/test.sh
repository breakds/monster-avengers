#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ $# -eq 0 ]; then
    $DIR/regression $HOME/dataset/mh4g/mh4g.db $(ls $DIR/queries/*)
else
    $DIR/regression $HOME/dataset/mh4g/mh4g.db $DIR/queries/$1.lisp
fi

