#!/bin/sh
# Author: Octavio Chiman

set -e
set -u

NUMFILES=0
NUMLINES=0

if [ $# -lt 2 ]
then
  echo "Wrong number of arguments"
  exit 1
else
   
   filedir=$1
   searchstr=$2
   if [ ! -d $1 ]
   then
       echo "The 1st argument is not a directory"
       exit 1
   fi
fi

echo $searchstr
echo $filedir

NUMLINES="$(grep -or $searchstr $filedir | wc -l)"
NUMFILES="$(find $filedir -type f -exec grep -l $searchstr {} \; | wc -l)"

echo "The number of files are ${NUMFILES} and the number of matching lines are ${NUMLINES}" 

