#!/bin/bash
set -e

PIPELINE=./pipeline
TEST_DIR=/tmp/bench_test
rm -rf $TEST_DIR
mkdir -p $TEST_DIR

echo "Test dosyasi olusturuluyor..."
python3 -c "
import random
for i in range(100000):
    uzunluk = random.randint(10, 200)
    print(''.join(random.choices('abcdefghijklmnopqrstuvwxyz0123456789 ', k=uzunluk)))
" > $TEST_DIR/buyuk.txt

SATIR_SAYISI=$(wc -l < $TEST_DIR/buyuk.txt)
BOYUT=$(du -h $TEST_DIR/buyuk.txt | cut -f1)

echo "Test dosyasi: $SATIR_SAYISI satir, $BOYUT"
echo ""

# ===== SIFRELEME =====
echo "=== Sifreleme ==="
/usr/bin/time -v -o $TEST_DIR/time_sifre.log \
    $PIPELINE -i $TEST_DIR/buyuk.txt \
    -o $TEST_DIR/kabul.txt -r $TEST_DIR/red.log \
    -k $TEST_DIR/sifre.key -q 2> $TEST_DIR/pipeline_sifre.log > /dev/null

grep -E "Elapsed|Maximum resident" $TEST_DIR/time_sifre.log

# Kontrol
if [ ! -f $TEST_DIR/kabul.txt ]; then
    echo "HATA: kabul.txt olusmadi!"
    echo "--- pipeline_sifre.log ---"
    cat $TEST_DIR/pipeline_sifre.log
    exit 1
fi

KABUL_SATIR=$(wc -l < $TEST_DIR/kabul.txt)
RED_SATIR=$(wc -l < $TEST_DIR/red.log 2>/dev/null || echo 0)

echo ""
echo "=== Kontrol ==="
echo "kabul.txt: $KABUL_SATIR satir"
echo "red.log:   $RED_SATIR satir"

# ===== DESIFRE (TUM ARSIVLERI BIRLESTIR) =====
echo ""
echo "=== Desifre (Birlesik) ==="

# 1. Tüm kabul.txt* dosyalarını geçici bir dosyada birleştir
#    Sıralama: kabul.txt, kabul.txt.1, kabul.txt.2, ...
#    (glob sırası: kabul.txt önce, sonra .1, .2 ...)
BIRLESIK=$TEST_DIR/gecici_kabul.txt
cat $TEST_DIR/kabul.txt* > $BIRLESIK

# 2. Birleşik dosya boyutu
BIRLESIK_SATIR=$(wc -l < $BIRLESIK)
BIRLESIK_BOYUT=$(du -h $BIRLESIK | cut -f1)
echo "Birlesik dosya: $BIRLESIK_SATIR satir, $BIRLESIK_BOYUT"
echo "Arsivler: $(ls $TEST_DIR/kabul.txt* | wc -l) adet"
echo ""

# 3. Birleşik dosyayı desifre et
/usr/bin/time -v -o $TEST_DIR/time_desifre.log \
    $PIPELINE -d -i $BIRLESIK \
    -k $TEST_DIR/sifre.key -q 2> $TEST_DIR/pipeline_desifre.log > /dev/null

grep -E "Elapsed|Maximum resident" $TEST_DIR/time_desifre.log

# 4. Geçici birleşik dosyayı temizle (orijinal arşivlere dokunma!)
rm -f $BIRLESIK

# ===== REFERANS =====
echo ""
echo "=== Referans: cat ==="
/usr/bin/time -v -o $TEST_DIR/time_cat.log \
    cat $TEST_DIR/buyuk.txt 2> /dev/null > /dev/null

grep -E "Elapsed|Maximum resident" $TEST_DIR/time_cat.log

# ===== OZET =====
echo ""
echo "=== OZET ==="
echo "Girdi:  $SATIR_SAYISI satir, $BOYUT"
echo "Kabul:  $KABUL_SATIR satir (kabul.txt)"
echo "Red:    $RED_SATIR satir"
echo ""
echo "Arsivler ($TEST_DIR/):"
ls -la $TEST_DIR/kabul.txt* 2>/dev/null | awk '{print "  " $9 " (" $5 " byte)"}'
echo ""
echo "Not: Orijinal arsivler korundu. Sadece gecici birlestirme silindi."