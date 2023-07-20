if not expat_dep.found()
  xmp_lib = files()
  subdir_done()
endif

libinc = [libinc, include_directories('include')]
xmp_lib = files(
  'src/ExpatAdapter.cpp',
  'src/MD5.cpp',
  'src/ParseRDF.cpp',
  'src/UnicodeConversions.cpp',
  'src/WXMPIterator.cpp',
  'src/WXMPMeta.cpp',
  'src/WXMPUtils.cpp',
  'src/XML_Node.cpp',
  'src/XMPCore_Impl.cpp',
  'src/XMPIterator.cpp',
  'src/XMPMeta-GetSet.cpp',
  'src/XMPMeta-Parse.cpp',
  'src/XMPMeta-Serialize.cpp',
  'src/XMPMeta.cpp',
  'src/XMPUtils-FileInfo.cpp',
  'src/XMPUtils.cpp',
)
