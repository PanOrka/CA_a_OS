#!/bin/bash

printf "PID : TIME : CMD : NUMBER OF OPENED FILES\n"
for FILE in `ls /proc | sort -n`
do
  name=`echo "$FILE"`
  if [[ "$name" < "a" ]]
  then
    time=`[ -f /proc/"$FILE"/status ] && awk '{print $14+$15}' /proc/"$FILE"/stat`
    cmd=`[ -f /proc/"$FILE"/status ] && cat /proc/"$FILE"/status | grep "Name:"`
    cmd=`echo ${cmd#*Name:}`
    nmbr=0
    for FILE in `[ -f /proc/"$FILE"/status ] && ls /proc/$FILE/fd`
    do
      nmbr=`echo "$nmbr+1" | bc`
    done
    printf "%s : " $name
    printf "%s : " $time
    printf "%s : " $cmd
    printf "%d" $nmbr
    printf "\n"
  fi
done
