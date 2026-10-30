#!/bin/bash
# Tüm testleri çalıştır

set -e

echo "===== TEST PAKETI ====="
echo ""

# 1. Birim testleri
echo "--- Birim Testleri ---"

if [ -f test_aes ]; then
    ./test_aes || exit 1
else
    echo "test_aes bulunamadi (make test_aes yap)"
fi

# 2. End-to-end
echo ""
echo "--- End-to-End Testleri ---"
bash tests/test_pipeline.sh

echo ""
echo "===== TAMAM ====="