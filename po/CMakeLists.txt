# CMakeLists.txt for i18n files

# include all po files in the directory
file(GLOB PO_FILES *.po)
update_translations(exiv2 ${PO_FILES})
add_translations(exiv2 ${PO_FILES})

##
# execute xgettext on every file
add_custom_target( pot-update
    COMMAND xgettext -d ${PACKAGE_NAME}
    -s -D ${CMAKE_CURRENT_SOURCE_DIR}/..
    -o ${PACKAGE_NAME}.pot
    -p ${CMAKE_CURRENT_SOURCE_DIR}/
    --from-code=UTF-8  --files-from=POTFILES.in
    -j
    --keyword=_
    --package-name=${PACKAGE_NAME}
    --package-version=${PACKAGE_VERSION}
    --copyright-holder=${PACKAGE_COPYRIGHT}
    --msgid-bugs-address=${PACKAGE_BUGREPORT}
)

# That's all Folks!
##
