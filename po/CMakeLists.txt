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

# .mo files creation and .po files update

FOREACH (PO_CURRENT ${PO_FILES})
    GET_FILENAME_COMPONENT(langue
                           ${PO_CURRENT}
                           NAME_WE
                          )

    SET(MO_FILE ${langue}.mo)
    ADD_CUSTOM_COMMAND(TARGET update_pot
                       POST_BUILD
                       COMMAND msgmerge --update ./${PO_CURRENT} ./${PACKAGE_NAME}.pot
                       DEPENDS ${PO_CURRENT} ${pot_file}
    )

#     MESSAGE ("DEPENDANCE : ${PO_CURRENT}")
#     MESSAGE ("DEPENDANCE : ${PACKAGE_NAME}.POT")
#     MESSAGE ("DEPENDANCE : ${LANGUE}")
#     MESSAGE ("DEPENDANCE : ${MO_FILE}")
#     MESSAGE ("DEPENDANCE : ${PO_FILES}")

    ADD_CUSTOM_COMMAND(
        OUTPUT ${MO_FILE}
        COMMAND msgfmt -c ${CMAKE_CURRENT_SOURCE_DIR}/${PO_CURRENT} -o ${CMAKE_CURRENT_SOURCE_DIR}/${langue}.mo
    )

    INSTALL(FILES ${langue}.gmo
            DESTINATION ${LOCALEDIR}/po/locale/${langue}/LC_MESSAGES
            RENAME ${langue}.mo
           )

    SET( POS_CURRENT ${POS_CURRENT} ${PO_CURRENT})
ENDFOREACH (PO_CURRENT)

# MESSAGE ("dependance : ${POS_CURRENT}")
ADD_CUSTOM_TARGET(build_mo ALL DEPENDS ${POS_CURRENT})
