#! /bin/sh

## From README.md:
## To binarize trees use:
##
##        ./norm-trees.sh --originalparsetrees [original-treebank-filename] --normalisedparsetrees [output-in-binarized-form-filename]


# export PATH
# source ~/.bash_rc

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $@ > /dev/stderr

java -Xmx10g -cp $DIR/../train/src/jar/commons-cli-1.2.jar:$DIR/../train/src/jar/trove.jar:$DIR/../train/src/jar/berkeleyParser.jar:$DIR/../train/src/jar/colt.jar:$DIR/../train/src/jar/trees.jar:$DIR/../train/src/jar/lpcfg-smoothclusterestimation-universal.jar lpcfgsmoothclusterestimationuniversal.ScriptNormalizeParseTrees $@
exit
