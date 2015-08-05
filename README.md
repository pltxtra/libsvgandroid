# libsvgandroid
SVG render library for Android by Anton Persson. Based on libsvg by Carl D. Worth.

# license
libsvgandroid is released under the GNU Lesser General Public License, version 3 or later.

Please note that libsvgandroid depends on additional third party libraries that has
their own licenses. If you distribute software including or using libsvgandroid make
sure you follow lgpl v3+, and also the additional licensing requirements specified in
the third party libraries.

# requirements

 * A development host running some form of GNU/Linux (tested on Ubuntu 15.04)
 * the Android SDK
 * the Android NDK

# configure

Quick:

```
./configure --ndk-path ~/Source/Android/android-ndk --sdk-path ~/Source/Android/android-sdk-linux --target-platform android-10 --bootstrap
```

--bootstrap will download required third party libraries and cross compile them for Android.

# compiling

either:

```
make debug
```

or:

```
make release
```

# usage

Include libsvgandroid.so as a prebuilt shared library in your Android.mk file. Please refer to the
Android NDK documentation for information about specifics.

Include libsvgandroid.jar in your libs directory. Please refer to the Android SDK documentation for
information about specifics.
