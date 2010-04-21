# - FindMSGFMT.cmake for Hugin 0.7
# file taken from rosegarden
#
# Find the gnu gettext msgfmt program and define
# a macro that calls it to add message translations
#
# Defined variables:
#  MSGFMT_FOUND
#  MSGFMT_EXECUTABLE
#
# Macro:
#  ADD_TRANSLATIONS
#
# mod for Windows build 07Nov2007 TKSharpless --
#   look in C:\MinGW after std. Unix places, that's where 
#   gettext automatically installed itself on my system.
# TODO find where else on Windows, Mac, etc gettext might be

IF(MSGFMT_EXECUTABLE)
    SET(MSGFMT_FOUND TRUE)
ELSE(MSGFMT_EXECUTABLE)
    FIND_PROGRAM(MSGFMT_EXECUTABLE NAMES msgfmt gmsgfmt msgfmt.exe
                                   PATHS /bin /usr/bin /usr/local/bin c:/MinGW/bin ${SOURCE_BASE_DIR}/gettext/bin)
    IF(MSGFMT_EXECUTABLE)
        SET(MSGFMT_FOUND TRUE)
    ELSE(MSGFMT_EXECUTABLE)
        IF(NOT MSGFMT_FIND_QUIETLY)
            IF(MSGFMT_FIND_REQUIRED)
                MESSAGE(FATAL_ERROR "msgfmt program couldn't be found")
            ENDIF(MSGFMT_FIND_REQUIRED)
        ENDIF(NOT MSGFMT_FIND_QUIETLY)
    ENDIF(MSGFMT_EXECUTABLE)
    MARK_AS_ADVANCED(MSGFMT_EXECUTABLE)
ENDIF (MSGFMT_EXECUTABLE)

MACRO(ADD_TRANSLATIONS _baseName)
    SET(_outputs)
    FOREACH(_file ${ARGN})
        GET_FILENAME_COMPONENT(_file_we ${_file} NAME_WE)
        SET(_out "${CMAKE_CURRENT_BINARY_DIR}/${_file_we}.gmo")
        SET(_in  "${CMAKE_CURRENT_SOURCE_DIR}/${_file_we}.po")
        ADD_CUSTOM_COMMAND(OUTPUT ${_out}
                           COMMAND ${MSGFMT_EXECUTABLE} -o ${_out} ${_in}
                           DEPENDS ${_in})
        SET(_dest "${LOCALEDIR}/${_file_we}/LC_MESSAGES/")
        INSTALL(FILES ${_out}
                DESTINATION dest
                RENAME ${_baseName}.mo)
        SET(_outputs ${_outputs} ${_out})
    ENDFOREACH(_file)
    SET(MSGFMT_TARGET translations${_baseName})
    ADD_CUSTOM_TARGET(${MSGFMT_TARGET} ALL DEPENDS ${_outputs})
ENDMACRO(ADD_TRANSLATIONS)
