
echo "*******************************************************************************"
echo "Setting up environment..."
echo "*******************************************************************************"

set -o errexit

BASE_PWD=$PWD
PATH=".:./asm/bin/:$PATH"
INROM="ripple.nes"
OUTROM="ripple_en.nes"
WLADX="./wla-dx/binaries/wla-6502"
WLALINK="./wla-dx/binaries/wlalink"

cp "$INROM" "$OUTROM"

mkdir -p out

echo "*******************************************************************************"
echo "Building tools..."
echo "*******************************************************************************"

make blackt
make libnes
make

if [ ! -f $WLADX ]; then
  
  echo "********************************************************************************"
  echo "Building WLA-DX..."
  echo "********************************************************************************"
  
  cd wla-dx
    cmake -G "Unix Makefiles" .
    make
  cd $BASE_PWD
  
fi

echo "*******************************************************************************"
echo "Doing initial ROM prep..."
echo "*******************************************************************************"

romprep "$OUTROM" "$OUTROM"

#echo "*******************************************************************************"
#echo "Building hacks..."
#echo "*******************************************************************************"
#mkdir -p out/asm
#ca65 asm/text.asm -o out/asm/text.out
#ld65 -C asm/text.cfg out/asm/text.out

echo "*******************************************************************************"
echo "Building font..."
echo "*******************************************************************************"

mkdir -p out/font
fontbuild font/trans/ripple_font.png 0x4E out/font/font_8x8.bin short prepatterns

mkdir -p out/font_credits
fontbuild rsrc/font_credits/ripple_font.png 0x2B out/font_credits/font.bin tall noprepatterns

echo "*******************************************************************************"
echo "Generating raw strings..."
echo "*******************************************************************************"

mkdir -p out/rawstr

scriptconv_raw script/password.txt table/ripple_en.tbl out/rawstr/password.bin
scriptconv_raw script/password_bad.txt table/ripple_en.tbl out/rawstr/password_bad.bin
scriptconv_raw script/password_cheat.txt table/ripple_en.tbl out/rawstr/password_cheat.bin

scriptconv_raw script/items_0.txt table/ripple_en.tbl out/rawstr/items_0.bin
scriptconv_raw script/items_1.txt table/ripple_en.tbl out/rawstr/items_1.bin
scriptconv_raw script/items_2.txt table/ripple_en.tbl out/rawstr/items_2.bin
scriptconv_raw script/items_3.txt table/ripple_en.tbl out/rawstr/items_3.bin
scriptconv_raw script/items_4.txt table/ripple_en.tbl out/rawstr/items_4.bin
#> out/rawstr/items_1.bin
#> out/rawstr/items_2.bin
#> out/rawstr/items_3.bin
#> out/rawstr/items_4.bin

echo "*******************************************************************************"
echo "Generating main script..."
echo "*******************************************************************************"

mkdir -p out/script

scriptconv script/ table/ripple_en.tbl out/script/

echo "*******************************************************************************"
echo "Generating images..."
echo "*******************************************************************************"

mkdir -p out/precmp
mkdir -p out/maps
tilemapper_nes tilemappers/title.txt

cp "out/font/font_8x8.bin" "out/precmp/font_8x8.bin"
cp "rsrc/title/raw/interface_grp.bin" "out/precmp/interface_grp.bin"

mkdir -p "out/cmp"
for file in out/precmp/*; do
  grp_cmp "$file" "out/cmp/$(basename $file)"
done

echo "*******************************************************************************"
echo "Generating new graphics banks..."
echo "*******************************************************************************"

cp rsrc/grp/bank1_grp_08.png rsrc/grp/bank1/grp_08.png
cp rsrc/grp/bank1_grp_10.png rsrc/grp/bank1/grp_10.png
cp rsrc/grp/bank2_grp_01.png rsrc/grp/bank2/grp_01.png
cp rsrc/grp/bank2_grp_02.png rsrc/grp/bank2/grp_02.png
cp rsrc/grp/bank2_grp_08.png rsrc/grp/bank2/grp_08.png

mkdir -p out/grp/raw/bank0
mkdir -p out/grp/raw/bank1
mkdir -p out/grp/raw/bank2
#mkdir -p out/grp/raw/bank3
mkdir -p out/grp/cmp/bank0
mkdir -p out/grp/cmp/bank1
mkdir -p out/grp/cmp/bank2
#mkdir -p out/grp/cmp/bank3
undumpbankgrp.sh

grpbuild rsrc/grp/bank0/index.txt out/grp/bank0.bin
grpbuild rsrc/grp/bank1/index.txt out/grp/bank1.bin
grpbuild rsrc/grp/bank2/index.txt out/grp/bank2.bin

echo "********************************************************************************"
echo "Applying ASM patches..."
echo "********************************************************************************"

mkdir -p "out/asm"
cp "$OUTROM" "asm/ripple.nes"

cd asm
  # apply hacks
  ../$WLADX -I ".." -o "boot.o" "boot.s"
  ../$WLALINK -v linkfile ripple_build.nes
cd $BASE_PWD

mv -f "asm/ripple_build.nes" "$OUTROM"
rm "asm/ripple.nes"
rm asm/*.o

#echo "*******************************************************************************"
#echo "Patching hacks to ROM..."
#echo "*******************************************************************************"
#filepatch "$WORKROM" 0x1D6AD build/asm/text_1D6AD.bin "$WORKROM"
#filepatch "$WORKROM" 0x1FE98 build/asm/text_1FE98.bin "$WORKROM"
#filepatch "$WORKROM" 0x1D56E build/asm/text_1D56E.bin "$WORKROM"
#filepatch "$WORKROM" 0x1C47B build/asm/text_1C47B.bin "$WORKROM"
#filepatch "$WORKROM" 0x17050 build/asm/text_17050.bin "$WORKROM"
#filepatch "$WORKROM" 0x17071 build/asm/text_17071.bin "$WORKROM"
#filepatch "$WORKROM" 0x33E2 build/asm/text_33E2.bin "$WORKROM"
#filepatch "$WORKROM" 0x29C5 build/asm/text_29C5.bin "$WORKROM"
#filepatch "$WORKROM" 0x3F96 build/asm/text_3F96.bin "$WORKROM"

echo "*******************************************************************************"
echo "Finalizing ROM..."
echo "*******************************************************************************"

romfinalize "$OUTROM" "$OUTROM"

echo "*******************************************************************************"
echo "Success!"
echo "Output file:" $OUTROM
echo "*******************************************************************************"
