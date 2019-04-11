1) Verify Boost on Ubuntu 18.04
1.1 Install OpenCL lib for ubuntu: 
    sudo apt update sudo apt-get install ocl-icd-opencl-dev
1.2 Install Boost lib:
    sudo apt-get install libboost-all-dev
1.3 Get the old version of boost compute source code which contains examples
    git clone https://github.com/boostorg/compute.git
1.4 Compile and run the example  --->>>> example... 
    g++ -I/usr2/yijiangy/code_base/hexagon/compute/build/include/compute sort_vector.cpp -lOpenCL

TIPS: Ubuntu Nvidia card install and seletc.
$ ubuntu-drivers devices
sudo ubuntu-drivers autoinstall
sudo prime-select

=======================boost + OpenCL on Android ====== 
1) Download android NDK to get the llvm crosscompile toolchain
https://developer.android.com/ndk/downloads?hl=es-419

2) Download boost source code from:
https://www.boost.org/users/download/

3) Cross Compile boost source for arm64 with llvm clang:
Refer to: https://developer.arm.com/tools-and-software/server-and-hpc/arm-architecture-tools/resources/porting-and-tuning/building-boost-with-arm-compiler/procedure 

3.1) Run bootstrap.sh in boost source code folder. 
    ./bootstrap.sh --with-toolset=gcc --prefix=$BOOST_ROOT/install --with-libraries=all

3.2) Generate a user-config.jam to select llvm cross compile toolchain
     touch user-config.jam echo "using clang : arm64 : aarch64-linux-android24-clang++ ;" > user-config.jam 
     echo "using mpi ;" >> user-config.jam

3.3) Cross compile boost library for arm64 android
      ./b2 install --prefix=../boost toolset=clang 
       --toolsetroot=${NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/bin 
       --user-config=./user-config.jam architecture=arm address-model=64

3.4) Cross compile the test cpp from example( mentioned above in ubuntu OpenCL verification)
      arch64-linux-android24-clang++ -I/usr2/yijiangy/code_base/boost/include
      -I/usr2/yijiangy/code_base/opencl 
      -L/usr2/yijiangy/code_base/boost/lib 
      -L/usr2/yijiangy/code_base/opencl sort_vector.cpp 
      -lOpenCL -lCB -lgsl -lutils -lcutils -lui -llog -lz -lbacktrace -lbinder -lhardware -lsync -lunwind -lbase 
      -llzma -lc++ -static-libstdc++
TIPS: 
    Compile may require android libraries, try to pull the missing libraries from /system/lib64 and /vendor/lib64. 
