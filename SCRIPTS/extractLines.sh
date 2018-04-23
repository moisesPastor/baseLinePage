#!/bin/bash

if [ ! $# -eq 2 ]; then
  echo "Use: ${0##*/} <testFileList> <Path_Dir_Output>"
  exit 1
fi

#set -x : Display commands and their arguments as they are executed.
#set -v : Display shell input lines as they are read.
#set -n : Read commands but do not execute them. This may be used to check a shell script for syntax errors.
#set -e stop the script if an error occurs

PATH=$PATH:../../BIN:.
DOUT=${2/\//}


for file in `cat $1`; do
     Nom=`basename $file .$EXT`
     NOM=`echo -e ${DOUT}"/"${Nom}`
     #echo -e $NOM" "`lineExtractor -x ${NOM}.xml -i ${NOM}.jpg -t -v 1`
     echo -e $NOM" "`~/baseLinePage/SRC/lineExtractor/lineExtractor -x ${NOM}.xml -i ${NOM}.jpg -t -v 1`
done
