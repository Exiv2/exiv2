# CMake build system for exiv2 library and executables
# Copyright 2008 by Patrick Spendrin <ps_ml@gmx.de>
# Copyright 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#    3. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# automatically include all po files in the directory
FILE(GLOB PO_FILES *.po)

UPDATE_TRANSLATIONS(exiv2 ${PO_FILES})

ADD_TRANSLATIONS(exiv2 ${PO_FILES})

ADD_CUSTOM_TARGET( pot-update
                   COMMAND xgettext -d ${PACKAGE_NAME}
                                    -s
                                    -D ${CMAKE_CURRENT_SOURCE_DIR}/..
                                    -o ${PACKAGE_NAME}.pot
                                    -p ${CMAKE_CURRENT_SOURCE_DIR}/
                                    --from-code=UTF-8 
                                    --files-from=POTFILES.in
                                    -j
                                    --keyword=_
                                    --package-name=${PACKAGE_NAME}
                                    --package-version=${PACKAGE_VERSION}
                                    --copyright-holder=${PACKAGE_COPYRIGHT}
                                    --msgid-bugs-address=${PACKAGE_BUGREPORT}
                 )
