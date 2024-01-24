#!/bin/sh
# Author: Octavio Chiman

set -e
set -u

if [ $# -lt 2 ]
then
  echo "Wrong number of arguments"
  exit 1
else
   
   writefile=$1
   writestr=$2
fi


namefile=${writefile##*/}
filepath=${writefile%/*}

echo "Dir path is: $filepath"
echo "Name file is: $namefile"

if [ ! -d $filepath ]; then
  mkdir -p $filepath
  echo "Sucessfully created directory: $filepath"
fi

echo "$writestr" > $filepath/$namefile
if [ $? -eq 0 ] 
then 
  echo "Successfully created file $filepath/$namefile"
  exit 0 
else 
  echo "Could not create file" >&2 
  exit 1 
fi
