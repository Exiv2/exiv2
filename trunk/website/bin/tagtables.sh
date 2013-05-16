#! /bin/sh

# Copy the tag tables from the documentation. 
# The documentation must be built.
tabs="__Canon__ \
      __CanonCs__ \
      __CanonSi__ \
      __CanonPa__ \
      __CanonCf__ \
      __CanonPi__ \
      __Exif__ \
      __Fujifilm__ \
      __Iptc__ \
      __Minolta__ \
      __MinoltaCsNew__ \
      __MinoltaCs5D__ \
      __MinoltaCs7D__ \
      __Nikon1__ \
      __Nikon2__ \
      __Nikon3__ \
      __Olympus__ \
      __Panasonic__ \
      __Sigma__ \
      __Sony__"

exv_base=./exiv2

for i in $tabs; do cp -f $exv_base/doc/templates/$i var/; done
