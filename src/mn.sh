#!/bin/sh
# Create a module that depends on all MakerNote subclasses to
# force initialisation of static data in the corresponding 
# components when using the static library.
cat > mn.cpp <<EOF
#include "canonmn.hpp"
namespace {
    Exif::CanonMakerNote canonMakerNote;
} 
EOF
