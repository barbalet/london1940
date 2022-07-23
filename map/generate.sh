#!/bin/bash

make
cp map2json ashford/
cp map2json maidstone/
cp map2json southend/
cp map2json folkestone/
cp map2json canterbury/

cd ashford
mv new_ashford.json new_ashford2.json
./map2json -f new_ashford.png --apesdk
mv map.json new_ashford.json
cd ..

cd folkestone
mv new_folkestone.json new_folkestone2.json
./map2json -f new_folkestone.png --apesdk
mv map.json new_folkestone.json
cd ..

cd canterbury
mv new_canterbury.json new_canterbury2.json
./map2json -f new_canterbury.png --apesdk
mv map.json new_canterbury.json
cd ..

cd folkestone
mv new_folkestone.json new_folkestone2.json
./map2json -f new_folkestone.png --apesdk
mv map.json new_folkestone.json
cd ..

cd southend
mv new_southend.json new_southend2.json
./map2json -f new_southend.png --apesdk
mv map.json new_southend.json
cd ..

cd maidstone
mv new_maidstone.json new_maidstone2.json
./map2json -f new_maidstone.png --apesdk
mv map.json new_maidstone.json
cd ..
