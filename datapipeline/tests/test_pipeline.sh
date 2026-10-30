#!/bin/bash
# End-to-end pipeline testi

# NOT: set -e KALDIRILDI
# Testler hataları kendileri sayar

PIPELINE=./pipeline
TEST_DIR=$(mktemp -d)
trap "rm -rf $TEST_DIR" EXIT

GECTI=0
BASARISIZ=0

test_baslik() { echo ""; echo "=== $1 ==="; }
test_ok()     { GECTI=$((GECTI+1)); echo "PASS: $1"; }
test_fail()   { BASARISIZ=$((BASARISIZ+1)); echo "FAIL: $1"; }

# 1. Yardım
test_baslik "Yardim"
if $PIPELINE -h | grep -q "Kullanim:"; then
    test_ok "yardim mesaji"
else
    test_fail "yardim mesaji"
fi

# 2. Basit şifreleme + deşifre
test_baslik "Sifrele + Coz"
echo "merhaba dunya" > $TEST_DIR/girdi.txt

$PIPELINE -i $TEST_DIR/girdi.txt \
    -o $TEST_DIR/kabul.txt \
    -r $TEST_DIR/red.log \
    -k $TEST_DIR/sifre.key \
    -q 2>/dev/null

if grep -q "AES:" $TEST_DIR/kabul.txt; then
    test_ok "sifreleme"
else
    test_fail "sifreleme"
fi

COZULEN=$($PIPELINE -d -i $TEST_DIR/kabul.txt -k $TEST_DIR/sifre.key -q 2>/dev/null | grep -c "merhaba dunya" || true)
if [ "$COZULEN" -eq 1 ]; then
    test_ok "desifre"
else
    test_fail "desifre"
fi

# 3. RED testleri
test_baslik "Guvenlik Kurallari"

# SQL injection
echo "' OR '1'='1" > $TEST_DIR/sqlinj.txt
$PIPELINE -i $TEST_DIR/sqlinj.txt \
    -o $TEST_DIR/k.txt -r $TEST_DIR/r.log -k $TEST_DIR/sifre.key -q 2>/dev/null

if grep -q "RED" $TEST_DIR/r.log; then
    test_ok "SQL injection reddedildi"
else
    test_fail "SQL injection reddedildi"
fi

# XSS
echo "<script>alert(1)</script>" > $TEST_DIR/xss.txt
$PIPELINE -i $TEST_DIR/xss.txt \
    -o $TEST_DIR/k2.txt -r $TEST_DIR/r2.log -k $TEST_DIR/sifre.key -q 2>/dev/null

if grep -q "RED" $TEST_DIR/r2.log; then
    test_ok "XSS reddedildi"
else
    test_fail "XSS reddedildi"
fi

# Path traversal
echo "../../etc/passwd" > $TEST_DIR/path.txt
$PIPELINE -i $TEST_DIR/path.txt \
    -o $TEST_DIR/k3.txt -r $TEST_DIR/r3.log -k $TEST_DIR/sifre.key -q 2>/dev/null

if grep -q "RED" $TEST_DIR/r3.log; then
    test_ok "path traversal reddedildi"
else
    test_fail "path traversal reddedildi"
fi

# 4. Uzun satır
test_baslik "Uzun Satir"
python3 -c "print('a' * 5000)" > $TEST_DIR/uzun.txt
$PIPELINE -i $TEST_DIR/uzun.txt \
    -o $TEST_DIR/k4.txt -r $TEST_DIR/r4.log -k $TEST_DIR/sifre.key -q 2>/dev/null

if grep -q "Uzunluk: 5000" $TEST_DIR/k4.txt; then
    test_ok "5000 karakterlik satir"
else
    test_fail "5000 karakterlik satir"
fi

# 5. Çok kelime
test_baslik "1000 Kelime"
python3 -c "print(' '.join(['kelime'] * 1000))" > $TEST_DIR/k1000.txt
$PIPELINE -i $TEST_DIR/k1000.txt \
    -o $TEST_DIR/k5.txt -r $TEST_DIR/r5.log -k $TEST_DIR/sifre.key -q 2>/dev/null

if grep -q "Kelime: 1000" $TEST_DIR/k5.txt; then
    test_ok "1000 kelime"
else
    test_fail "1000 kelime"
fi

# 6. Yanlış anahtar
test_baslik "Yanlis Anahtar"
echo "gizli veri" > $TEST_DIR/gizli.txt

# Şifrele (eski anahtarla)
$PIPELINE -i $TEST_DIR/gizli.txt \
    -o $TEST_DIR/k6.txt -r $TEST_DIR/r6.log -k $TEST_DIR/sifre.key -q 2>/dev/null

# Anahtarı değiştir
cp $TEST_DIR/sifre.key $TEST_DIR/sifre_backup.key
rm $TEST_DIR/sifre.key
$PIPELINE -i $TEST_DIR/gizli.txt \
    -o $TEST_DIR/k7.txt -r $TEST_DIR/r7.log -k $TEST_DIR/sifre.key -q 2>/dev/null

# Eski şifreli veriyi yeni anahtarla çözmeye çalış
$PIPELINE -d -i $TEST_DIR/k6.txt -k $TEST_DIR/sifre.key -q > /dev/null 2>&1
CIKIS=$?

if [ "$CIKIS" -ne 0 ]; then
    test_ok "yanlis anahtar reddedildi"
else
    test_fail "yanlis anahtar reddedildi"
fi

# Özet
echo ""
echo "=============================="
echo "$GECTI gecti, $BASARISIZ basarisiz"
echo "=============================="

[ "$BASARISIZ" -eq 0 ] && exit 0 || exit 1