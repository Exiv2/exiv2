# CMakeLists.txt for i18n files

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

set( PACKAGE_BUGREPORT      "http://github.com/exiv2/exiv2" )
set( PACKAGE_URL            "http://www.exiv2.org" )

##
# execute xgettext on every file
add_custom_target( pot-update
    COMMAND xgettext -d ${PROJECT_NAME}
    -s -D ${CMAKE_CURRENT_SOURCE_DIR}/..
    -o ${PROJECT_NAME}.pot
    -p ${CMAKE_CURRENT_SOURCE_DIR}/
    --from-code=UTF-8  --files-from=POTFILES.in
    -j
    --keyword=_
    --package-name=${PROJECT_NAME}
    --package-version=${PROJECT_VERSION}
    --msgid-bugs-address=${PACKAGE_BUGREPORT}
)

# That's all Folks!
##
