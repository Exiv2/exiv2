headers = files(
  'exiv2/basicio.hpp',
  'exiv2/bmffimage.hpp',
  'exiv2/bmpimage.hpp',
  'exiv2/config.h',
  'exiv2/convert.hpp',
  'exiv2/cr2image.hpp',
  'exiv2/crwimage.hpp',
  'exiv2/datasets.hpp',
  'exiv2/easyaccess.hpp',
  'exiv2/epsimage.hpp',
  'exiv2/error.hpp',
  'exiv2/exif.hpp',
  'exiv2/exiv2.hpp',
  'exiv2/futils.hpp',
  'exiv2/gifimage.hpp',
  'exiv2/http.hpp',
  'exiv2/image.hpp',
  'exiv2/image_types.hpp',
  'exiv2/iptc.hpp',
  'exiv2/jp2image.hpp',
  'exiv2/jpgimage.hpp',
  'exiv2/metadatum.hpp',
  'exiv2/mrwimage.hpp',
  'exiv2/orfimage.hpp',
  'exiv2/pgfimage.hpp',
  'exiv2/photoshop.hpp',
  'exiv2/pngimage.hpp',
  'exiv2/preview.hpp',
  'exiv2/properties.hpp',
  'exiv2/psdimage.hpp',
  'exiv2/rafimage.hpp',
  'exiv2/rw2image.hpp',
  'exiv2/slice.hpp',
  'exiv2/tags.hpp',
  'exiv2/tgaimage.hpp',
  'exiv2/tiffimage.hpp',
  'exiv2/types.hpp',
  'exiv2/value.hpp',
  'exiv2/version.hpp',
  'exiv2/webpimage.hpp',
  'exiv2/xmp_exiv2.hpp',
  'exiv2/xmpsidecar.hpp',
)

if get_option('video')
  headers += files('exiv2/asfvideo.hpp', 'exiv2/matroskavideo.hpp', 'exiv2/quicktimevideo.hpp', 'exiv2/riffvideo.hpp')
endif

if zlib_dep.found()
  headers += files('exiv2/pngimage.hpp')
endif

headers += exiv_conf
headers += cfile

install_headers(
  headers,
  subdir: 'exiv2',
)
