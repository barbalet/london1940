#!/bin/bash

make

mv map2json ~/london1940/utility/map2json
 
cd ..
cd examples

cp new_ashford.png ~/london1940/utility/new_ashford.png
cp new_maidstone.png ~/london1940/utility/new_maidstone.png
cp new_canterbury.png ~/london1940/utility/new_canterbury.png

cd ..
cd mapedit
./drawout.sh

mv drawout ~/london1940/output/


cd ~/london1940/utility/

# Ashford

./map2json -f new_ashford.png --apesdk

mv map.json ~/london1940/output/new_ashford.json
mv map2.json new_ashford_out2.json

# Maidstone

./map2json -f new_maidstone.png --apesdk

mv map.json ~/london1940/output/new_maidstone.json
mv map2.json new_maidstone_out2.json

# Canterbury

./map2json -f new_canterbury.png --apesdk

mv map.json ~/london1940/output/new_canterbury.json
mv map2.json new_canterbury_out2.json


mv new_ashford.png ~/london1940/output/new_ashford.png
mv new_maidstone.png ~/london1940/output/new_maidstone.png
mv new_canterbury.png ~/london1940/output/new_canterbury.png

rm *.png
rm map2json

cd ..
cd output

./drawout new_ashford
./drawout new_maidstone
./drawout new_canterbury

rm drawout

