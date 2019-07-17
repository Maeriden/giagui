## Prerequisites
```
g++   >= 8.3
make
cmake >= 3.13
```
Other C++17-capable compilers will probably work too.

## Dependencies
```
Qt      >= 5.11
Qt-svg  >= 5.11
cpptoml >= 0.1.1 (included in this repo as a submodule)
h3      >= 3.4.0 (included in this repo as a submodule)
```

## Compilation

```
git clone https://gitlab.com/alecel/gia-gui.git
cd gia-gui
git checkout v2
git submodule init
git submodule update

# Compile and install cpptoml. Skip if already installed
pushd 3rdparty/cpptoml
mkdir build && cd build
cmake ..
make
sudo make install
popd

# Compile and install h3. Skip if already installed
pushd 3rdparty/h3
mkdir build && cd build
cmake ..
make
sudo make install
popd

mkdir build && cd build
cmake ..
make
```

## Installing

The software does not have an installation script.
You have to manually move it to the appropriate system directory and make a desktop file.
