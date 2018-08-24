
mkdir -p rsrc/title
mkdir -p rsrc/title/raw
mkdir -p rsrc/title/rawgrp
mkdir -p rsrc/title/full

#./grp_decmp ripple_noheader.nes 0x142A4 0xA4 rsrc/title/raw/grp.bin

# the game decompresses 0xA4 patterns of data, but the pattern count seems to
# be wrong -- the last patterns are just garbage
# the correct figure appears to be 0x89
./grp_decmp ripple_noheader.nes 0x142A4 0x89 rsrc/title/raw/grp.bin
./nes_tiledmp rsrc/title/raw/grp.bin 0 rsrc/title/rawgrp/grp.png
./titlerip ripple_noheader.nes rsrc/title/full/title.png

./grp_decmp ripple_noheader.nes 0x14000 0x40 rsrc/title/raw/interface_grp.bin
./nes_tiledmp rsrc/title/raw/interface_grp.bin 0 rsrc/title/rawgrp/interface_grp.png
