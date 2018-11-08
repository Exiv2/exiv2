# CMakeLists.txt for i18n files
find_package(MSGFMT REQUIRED)
if(MSGFMT_FOUND)
    message(STATUS "Program msgfmt found (${MSGFMT_EXECUTABLE})")
else()
    message(FATAL_ERROR "msgfmt not found")
endif()

# List of files PO files
set(PO_FILES bs.po
             de.po
             es.po
             fi.po
             fr.po
             gl.po
             ms.po
             pl.po
             pt.po
             ru.po
             sk.po
             sv.po
             ug.po
             uk.po
             vi.po
)

update_translations(exiv2 ${PO_FILES})
add_translations(exiv2 ${PO_FILES})

##
# execute xgettext on every file
add_custom_target( pot-update
    COMMAND xgettext --default-domain ${PROJECT_NAME}
    --sort-output --directory ${CMAKE_CURRENT_SOURCE_DIR}/..
    --output ${PROJECT_NAME}.pot
    --output-dir ${CMAKE_CURRENT_SOURCE_DIR}/
    --from-code=UTF-8  --files-from=POTFILES.in
    -j
    --keyword=_
    --package-name=${PROJECT_NAME}
    --package-version=${PROJECT_VERSION}
    --msgid-bugs-address=${PACKAGE_BUGREPORT}
)

# That's all Folks!
##
