#!/bin/bash

d2=`awk '/wlp1s0:/ {print $2}' /proc/net/dev`
array=(0 0 0 0 0 0 0 0 0 0 0)
maxNet=100
counter=0
while true
do
  tput clear

  # TIME
  timeBoot=`awk '{print int($1)}' /proc/uptime`
  sec=`echo "$timeBoot%60" | bc`
  min=`echo "($timeBoot/60)%60" | bc`
  hr=`echo "($timeBoot/3600)%24" | bc`
  days=`echo "($timeBoot/(3600*24))" | bc`
  printf "DAYS  | %d\n" $days
  printf "HOURS | %d\n" $hr
  printf "MINS  | %d\n" $min
  printf "SEC   | %d\n\n" $sec

  # BATTERY
  batteryCap=`echo | cat /sys/class/power_supply/BAT0/uevent | grep "POWER_SUPPLY_CAPACITY=" | tr -d "POWER_SUPPLY_CAPACITY="`
  batteryDraw=`echo "$batteryCap/10" | bc`
  batteryRest=`echo "$batteryCap%10" | bc`
  printf "BATTERY\n"
  tput setaf 2
  for ((i=1; i<=$batteryDraw; i++)); do
    printf "\u2588"
  done
  if [ $batteryRest -gt 0 -a $batteryRest -le 3 ]
  then
    printf "\u258E"
  elif [ $batteryRest -gt 3 -a $batteryRest -le 6 ]
  then
    printf "\u258C"
  elif [ $batteryRest -gt 6 -a $batteryRest -le 9 ]
  then
    printf "\u258A"
  fi
  tput setaf 7
  for ((i=$batteryDraw+1; i<=10; i++)); do
    printf " "
  done
  printf "\u258C%d%%\n\n" $batteryCap
  # CHART
  d1=$d2
  d2=`awk '/wlp1s0:/ {print $2}' /proc/net/dev`
  currentNet=`echo "$d2-$d1" | bc`
  printf "Current transmission: %d B/s\n" $currentNet
  # taking maxNet
  if [ $currentNet -gt $maxNet ]
  then
    maxNet=$currentNet
  fi
  array[0]=$currentNet
  #array ma 11 elementow, zerowy odczytuje wartosc Current i przekazuje dalej do wykresu
  for ((i=10; i>0; i--)); do
    array[$i]=${array[$i-1]}
  done
  # taking new maxNet from full interval
  if [ $counter -eq 0 ]
  then
    maxNet=100
    for ((i=1; i<11; i++)); do
      if [ ${array[$i]} -gt $maxNet ]
      then
        maxNet=${array[$i]}
      fi
    done
  fi
  # counter+1
  counter=`echo "($counter+1)%10" | bc`
  # calc avgNet
  avgNet=0
  for ((i=1; i<11; i++)) do
    avgNet=`echo "($avgNet+${array[i]})" | bc`
  done
  avgNet=`echo "$avgNet/10" | bc`
  # printing values
  printf "Average transmission: %d B/s\n" $avgNet
  # DRAWING CHART
  tput setaf 2
  for ((i=10; i>0; i--)); do
    for ((k=1; k<11; k++)); do
      calculatedValue=`echo "(${array[$k]}*10)/$maxNet" | bc`
      if [ $calculatedValue -ge $i ]
      then
        printf "\u2588"
      else
        printf " "
      fi
    done
    if [ $i -eq 10 ]
    then
      printf " %d B/s" $maxNet
    elif [ $i -eq 1 ]
    then
      printf " 0 B/s"
    fi
    printf "\n"
  done
  tput setaf 7

  # SYS LOAD
  printf "\n"
  sysLoad=`awk '{print $4}' /proc/loadavg`
  printf "System load: %s" $sysLoad

  sleep 1
done
