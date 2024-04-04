find_package(Gettext REQUIRED)

set(PO_SOURCES
    bs.po
    ca.po
    da.po
    de.po
    es.po
    fi.po
    fr.po
    gl.po
    ms.po
    nl.po
    pl.po
    pt.po
    ru.po
    sk.po
    sv.po
    ug.po
    uk.po
    vi.po
)

gettext_create_translations(exiv2.pot ALL ${PO_SOURCES})

find_program(XGETTEXT xgettext)

add_custom_target(
  update-pot
  COMMAND
    ${XGETTEXT} -o ${CMAKE_CURRENT_SOURCE_DIR}/exiv2.pot -D ${PROJECT_SOURCE_DIR} --keyword=_ --keyword=N_
    --files-from=${CMAKE_CURRENT_SOURCE_DIR}/POTFILES.in --from-code=UTF-8 --copyright-holder='Exiv2 authors'
    --msgid-bugs-address='https://github.com/Exiv2/exiv2/issues' --package-name=${CMAKE_PROJECT_NAME} --package-version=${CMAKE_PROJECT_VERSION}
  COMMENT "Extract translatable messages to exiv2.pot"
)
