# GPXSee
GPXSee is a Qt-based GPS log file viewer and analyzer that supports all common
GPS log file formats.

## Features
* Opens GPX, TCX, FIT, KML, NMEA, IGC, CUP, SIGMA SLF, Suunto SML, LOC, GeoJSON,
  OziExplorer (PLT, RTE, WPT), Garmin GPI&CSV, TomTom OV2&ITN, ONmove OMD/GHP,
  TwoNav (TRK, RTE, WPT) and geotagged JPEG files.
* User-definable online maps (OpenStreetMap/Google tiles, WMTS, WMS, TMS,
  QuadTiles).
* Offline maps (MBTiles, OziExplorer maps, TrekBuddy maps/atlases,
  Garmin IMG/GMAP & JNX maps, TwoNav RMaps, GeoTIFF images, BSB charts,
  ENC charts, KMZ maps, AlpineQuest maps, Locus/OsmAnd/RMaps SQLite maps,
  Mapsforge vector maps, QCT maps, GEMF maps, Osmdroid SQLite maps, Orux maps,
  ESRI World-File georeferenced images).
* Elevation, speed, heart rate, cadence, power, temperature and gear ratio/shifts
  graphs.
* Support for DEM files (SRTM HGT).
* Support for multiple tracks in one view.
* Support for POI files.
* Print/export to PDF/PNG.
* Full-screen mode.
* HiDPI/Retina displays & maps support.
* Real-time GPS position.
* Windows, macOS, Linux and Android builds.

![GPXSee - Linux](https://a.fsdn.com/con/app/proj/gpxsee/screenshots/linux2.png)

## Build
Build requirements:
* Qt5 >= 5.11 or Qt6 >= 6.2 (Android builds require Qt6)
* C++11 or newer compiler (tested: msvc2019, gcc 7.5.0, clang/Apple LLVM version
  10.0.0)

Build steps:
```shell
lrelease gpxsee.pro
qmake gpxsee.pro
make # nmake on windows
```



## Download
* [Windows & OS X builds](https://sourceforge.net/projects/gpxsee)
* [Linux packages](https://software.opensuse.org/download.html?project=home%3Atumic%3AGPXSee&package=gpxsee)
* [Android APKs](https://play.google.com/store/apps/details?id=org.gpxsee.gpxsee)

## Changelog
[Changelog](https://build.opensuse.org/package/view_file/home:tumic:GPXSee/gpxsee/gpxsee.changes)

## Homepage
[https://www.gpxsee.org](https://www.gpxsee.org)

## Maps
[GPXSee maps repository](https://github.com/tumic0/GPXSee-maps)

## Translations
GPXSee uses [Weblate](https://hosted.weblate.org/projects/gpxsee/translations/)
for translations.

## License
GPXSee is licensed under GPL-3.0 (only). However, some 3rd party parts are using
different, GPL compatible, licenses:

* [Oxygen icons](icons/GUI) - LGPLv3
* [Mapbox Maki icons](icons/map/IMG/POI) - CC0
* [Map Icons Collection](icons/symbols) - CC BY SA 3.0
* [RTree implementation](src/common/rtree.h) - Public domain
* [Albers](src/map/proj/albersequal.cpp), [Geocentric](src/map/geocentric.cpp),
  [LCC](src/map/proj/lambertconic.cpp), [Mercator](src/map/proj/mercator.cpp),
  [Polar Stereographic](src/map/proj/polarstereographic.cpp),
  [Polyconic](src/map/proj/polyconic.cpp) and
  [Transverse Mercator](src/map/proj/transversemercator.cpp) projections - NIMA
  Source Code Disclaimer
* [Projection parameters CSV files](pkg/csv) - BSD/EPSG/Public domain
* [Mapsforge render theme](data/default.xml) and its [icons](icons/map/mapsforge) - LGPLv3
