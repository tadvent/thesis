编译环境：
MSYS + MinGW 4.7 + CMake 2.8

编译：
在 build 目录下
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make
make install

在 bin 目录下会生成 isdf.exe 程序
