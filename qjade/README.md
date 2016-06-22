qjade
-------

iSoftLinux AppStore frontend.


## Dependencies (In General)

### Libraries

Qt5 (of course) with qml, quick, network, svg, xml Modules.
There are more to come.


## Build && Install

Under distributions that uses qtchooser (like iSoftLinux, ArchLinux, Ubuntu, 
etc.)

```
$ export QT_SELECT=5
$ qmake PREFIX=/usr BRAND=(BRANDING THAT FITS)
$ make
# make install
```

Under distributions that just doesn't use qtchooser (AOSC OS2, maybe there's 
another one)

```
$ /usr/lib/qt5/bin/qmake PREFIX=/usr BRAND=(BRANDING THAT FITS)
$ make
# make install
```


## Localization

```
lupdate qml/*.qml -ts translations/qjade_zh_CN.ts
```

```
lrelease translations/qjade_zh_CN.ts
```


## How To Play with Brandings

### What Are Brandings?

Branding files goes in branding/$(BRAND).xml and images/$(BRAND).png (Image 
file is declared in XML)
And build with paramenter BRAND=$(BRAND).

### What If There's No Branding Support?

Well then this piece of software just seemed to have no point to share and put 
into general usage.

### How to use brand?

* simply use brand/generic.xml as an example to start with, 
and fill in information by following the comments.

* also you need to design your Linux distribution logo, 
declare the image file and put it into place that fits.
