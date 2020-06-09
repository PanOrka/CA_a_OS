#!/bin/bash

wget --quiet -O ./cat.jpg https://cdn2.thecatapi.com/images/itfFA4NWS.jpg
wget --quiet -O ./random http://api.icndb.com/jokes/random
catimg ./cat.jpg
cat ./random | jq '.value' | jq '.joke'
