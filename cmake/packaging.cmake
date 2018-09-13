set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_CONTACT "Luis Díaz Más <piponazo@gmail.com>")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_IGNORE_FILES "build.*;\.git/;test;third-party;")

include (CPack)
