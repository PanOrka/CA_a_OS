#!/bin/bash

lynx -dump $1 > data1.txt
while true
do
  sleep $2
  lynx -dump $1 > data2.txt
  answer=`diff -q data1.txt data2.txt`
  if [[ $answer ]]
  then
    echo "Nastapila zmiana strony"
    lynx -dump $1 > data1.txt
  fi
done
