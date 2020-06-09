#!/bin/bash

kill -2 $1

sleep 1

for ((i=0; i<50; i++));
do
  kill -2 $1
done
