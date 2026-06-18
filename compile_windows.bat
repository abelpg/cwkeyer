rd /s /q build-release

cmake -B build-release  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\mingw_64\bin  -G Ninja
cmake --build build-release --config Release

cd build-release
cpack -G ZIP
