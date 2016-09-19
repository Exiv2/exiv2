#include <QCoreApplication>
#include <exiv2/exiv2.hpp>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    exv_grep_keys_t keys;
    Exiv2::dumpLibraryInfo(std::cout,keys);

    return 0;
    // return a.exec();
}

