# CIE 3.0 PKCS11 MIDDLEWARE [![Copr build status](https://copr.fedorainfracloud.org/coprs/lucamagrone/CIE-Middleware/package/cie-middleware/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/lucamagrone/CIE-Middleware/package/cie-middleware)

CIE (Carta di Identità Elettronica) Linux middleware

## Disclaimer

This product is **beta software**. Use it in production at your own judgment.

## Requirements

Install with your package manger the following packages and the corresponding development variant:

- libcurl
- bzip2
- cryptopp
- freetype2
- libpng
- libxml-2.0
- openssl
- libpodofo
- zlib
- fontconfig
- libpcsclite

## Build

### libcie-pkcs11

If you have installed CMake on your system you can build the libcie-pkcs11 module with:

`./build.sh` or `bash build.sh`

### CIEID

If you have installed maven on your system you can build the project with:

`git submodule init`

`git submodule update`

`sed -i "s/1.5/1.6/g" Core/pom.xml`

`sed -i "s/1.5/1.6/g" Twinkle/pom.xml`

`cd Core && mvn package && mvn install && cd -`

`cd Twinkle && mvn package && mvn install && cd -`

`mvn package`

`mvn install`

#### Notes

To start CIEID:

`mvn dependency:build-classpath -Dmdep.outputFile=.cp`

`java -cp $(cat .cp):target/cieid-1.0.0.jar -Djna.library.path="./libcie-pkcs11" it.ipzs.cieid.MainApplication`

When directly calling the JVM be sure to make `libcie-pkcs11.so` available
to JNA either using the `jna.library.path` property or installing the library
in a path searched by default, e.g. `/usr/local/lib`.
