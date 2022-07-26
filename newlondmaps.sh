#!/bin/bash

curl -o newlondmaps.zip https://barbalet.com/newlondmaps.zip
unzip newlondmaps.zip -d ~/london1940
rm -f newlondmaps.zip
