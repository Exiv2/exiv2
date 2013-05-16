// g++ rssdate.cpp -o rssdate

#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include <ctime>

int main(int argc, char* const argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " dd-Mon-yyyy\n";
        return 1;
    }
    struct tm tm;
    strptime(argv[1], "%d-%b-%Y", &tm);
    char s[128];
    strftime(s, 128, "%a, %d %b %Y 00:00:00 +0800", &tm);
    std::cout << s;
    return 0;
}
