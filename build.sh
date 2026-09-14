#!/data/data/com.termux/files/usr/bin/bash
set -e
cd "$(dirname "$0")"

AAPT2=$HOME/android-tools/build-tools/aapt2
ZIPALIGN=$HOME/android-tools/build-tools/zipalign
ANDROID_JAR=$HOME/android-tools/android-34/android.jar
R8LIB=$HOME/android-tools/r8lib.jar
KS=debug.keystore

rm -rf build
mkdir -p build/gen build/classes build/dex build/lib/arm64-v8a

echo "[1/6] compile res..."
$AAPT2 compile --dir res -o build/res.zip

echo "[2/6] link..."
$AAPT2 link build/res.zip \
  -o build/base.apk \
  -I "$ANDROID_JAR" \
  --manifest AndroidManifest.xml \
  --java build/gen \
  --min-sdk-version 24 \
  --target-sdk-version 34 \
  --auto-add-overlay

echo "[3/6] javac..."
javac -source 17 -target 17 \
  -cp "$ANDROID_JAR" \
  -d build/classes \
  -sourcepath "src:build/gen" \
  $(find src -name "*.java")

echo "[3.5/6] native (arm64, static c++)..."
CLANG=$HOME/android-ndk-r29/toolchains/llvm/prebuilt/linux-aarch64/bin/clang++
$CLANG --target=aarch64-linux-android24 -O2 -std=c++17 -shared -fPIC -static-libstdc++ \
  -o build/lib/arm64-v8a/libmemtop.so \
  jni/meminfo.cpp
$HOME/android-ndk-r29/toolchains/llvm/prebuilt/linux-aarch64/bin/llvm-strip --strip-unneeded build/lib/arm64-v8a/libmemtop.so
ls -lh build/lib/arm64-v8a/

echo "[4/6] d8..."
java -cp "$R8LIB" com.android.tools.r8.D8 \
  --lib "$ANDROID_JAR" \
  --min-api 24 \
  --output build/dex \
  $(find build/classes -name "*.class")

cp build/base.apk build/unsigned.apk
python3 -c "import zipfile; z=zipfile.ZipFile('build/unsigned.apk','a',zipfile.ZIP_DEFLATED); z.write('build/dex/classes.dex','classes.dex'); z.close()"
python3 -c "import zipfile; z=zipfile.ZipFile('build/unsigned.apk','a',zipfile.ZIP_STORED); z.write('build/lib/arm64-v8a/libmemtop.so','lib/arm64-v8a/libmemtop.so'); z.close()"

echo "[5/6] zipalign..."
$ZIPALIGN -f 4 build/unsigned.apk build/aligned.apk

echo "[6/6] sign..."
if [ ! -f "$KS" ]; then
  keytool -genkeypair -keystore "$KS" -alias androiddebugkey \
    -keypass android -storepass android \
    -keyalg RSA -keysize 2048 -validity 10000 \
    -dname "CN=Android Debug,O=Android,C=US" >/dev/null 2>&1
fi
apksigner sign --ks "$KS" --ks-pass pass:android --key-pass pass:android \
  --out md3-empty.apk build/aligned.apk

echo "OK: $(pwd)/md3-empty.apk"
apksigner verify --print-certs md3-empty.apk | head -n 5
ls -lh md3-empty.apk
