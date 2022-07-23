#!/bin/bash
rm -f result.png .#* src/.#* thresholded.png road*.png sea*.png dark_image.png water*.png woods*.png buildings*.png lake*.png sands*.png streets*.png load_test.png map.png railway_lines_stage*.png railway_tunnels_stage*.png railway.png railway.json
rm -f map.json
#./map2json -f folkestone/folkestone.png
./map2json -f ashford/ashford.png
./map2json -f images/5_2-51727_431-51686_328-3870_6025.png --coverage 16
./map2json -f images/9_0-51576_618-51535_530-564_10076.png
