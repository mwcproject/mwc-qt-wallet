This is a stub page. More detailed instructions will be coming soon. This should be the bare minimum to be able to build
the project for now.

# Prereqs

1.) You will need cmake, make, and clang installed.
2.) You will need qt5 libraries installed. If they are not currently installed, you can run the following command to
install them:

```# brew install qt5```

# IDE setup

TODO: include IDE setup information

# Build process

1.) Modify the CMakeLists.txt file to point to your installation of qt5. By default qt5 will be installed on mac at
"/usr/local/opt/qt/lib/cmake/Qt5"
2.) Run cmake command:

```# cmake CMakeLists.txt```

3.) Run make command:

```# make```

This will result in a binary called mwc-gui being created in the root directory of the project.

# Running the binary

1.) Before you can run the code, you will need a mwc713 binary. The latest binaries can be found on the releases page:
http://www.github.com/mwcproject/mwc713/releases. Place this binary in the root directory of the mwc-qt-wallet project.
2.) Execute mwc-gui binary

```# ./mwc-gui```
