#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ $# -eq 0 ]; then
    $DIR/regression $HOME/Downloads/mh4g.db $(ls $DIR/queries/*)
else
    $DIR/regression $HOME/Downloads/mh4g.db $DIR/queries/$1.lisp
fi

