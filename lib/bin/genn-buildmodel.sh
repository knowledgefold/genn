#!/bin/bash

# display genn-buildmodel.sh help
genn_help () {
    echo "=== genn-buildmodel.sh script usage ==="
    echo "genn-buildmodel.sh [cdho] model"
    echo "-c            only generate simulation code for the CPU"
    echo "-d            enables the debugging mode"
    echo "-h            shows this help message"
    echo "-o outpath    changes the output directory"
}

# handle script errors
genn_error () { # $1=line, $2=code, $3=message
    echo "genn-buildmodel.sh:$1: error $2: $3"
    exit "$2"
}
trap 'genn_error $LINENO 50 "command failure"' ERR

# parse command options
OUTPUT_PATH="$PWD";
while [[ -n "${!OPTIND}" ]]; do
    while getopts "cdo:h" option; do
	case $option in
	    c) CPU_ONLY=1;;
	    d) DEBUG=1;;
	    h) genn_help; exit;;
	    o) OUTPUT_PATH="$OPTARG";;
	    ?) genn_help; exit;;
	esac
    done
    if [[ $OPTIND > $# ]]; then break; fi
    MODEL="${!OPTIND}"
    let OPTIND++
done
if [[ -z "$MODEL" ]]; then
    genn_error $LINENO 2 "no model file given"
fi

# convert relative paths to absolute paths
pushd $(dirname $MODEL) > /dev/null
MODEL="$PWD/$(basename $MODEL)"
popd > /dev/null
pushd $OUTPUT_PATH > /dev/null
OUTPUT_PATH="$PWD"
popd > /dev/null

# checking GENN_PATH is defined
if [[ -z "$GENN_PATH" ]]; then
    if [[ $(uname -s) == "Linux" ]]; then
	echo "GENN_PATH is not defined - trying to auto-detect"
	export GENN_PATH="$(readlink -f $(dirname $0)/../..)"
    else
	genn_error $LINENO 3 "GENN_PATH is not defined"
    fi
fi

# generate model code
cd "$OUTPUT_PATH"
make clean -f "$GENN_PATH/lib/src/GNUmakefile"
if [[ -n "$DEBUG" ]]; then
    echo "debugging mode ON"
    make debug -f "$GENN_PATH/lib/src/GNUmakefile" MODEL="$MODEL" CPU_ONLY=$CPU_ONLY
    gdb -tui --args ./generateALL "$OUTPUT_PATH"
else
    make -f "$GENN_PATH/lib/src/GNUmakefile" MODEL="$MODEL" CPU_ONLY=$CPU_ONLY
    ./generateALL "$OUTPUT_PATH"
fi

echo "model build complete"