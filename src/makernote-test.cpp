#include "makernote.hpp"

#include <iostream>
#include <string>
#include <utility>

void testMatch(const std::string& reg, const std::string& key);

int main()
{
    testMatch("Canon", "Canon");
    testMatch("Canon*", "Canon");
    testMatch("Canon*", "Canon Corp.");
    testMatch("*foo*bar*", "foobar");
    testMatch("*foo*bar*", "barfoofoobarbar");
    testMatch("foo*bar", "foo");
    testMatch("foo*bar", "bar");
    testMatch("foo*bar", "foobar");
    testMatch("foo*bar", "fooYAHOObar");
    testMatch("foo*bar", "Thefoobar");
    testMatch("foo*bar", "foobarTrick");
    testMatch("foo*bar", "ThefoobarTrick");
    testMatch("foo*bar", "ThefooYAHOObarTrick");

    testMatch("*", "anything");
    testMatch("**", "anything times two");

    testMatch("*bar", "bar");
    testMatch("b*bar", "bar");
    testMatch("b*bar", "bbar");
    testMatch("*foobar", "bar");
    testMatch("*bar", "foobar");

    return 0;
}

void testMatch(const std::string& reg, const std::string& key)
{
    int rc = Exiv2::MakerNoteFactory::match(reg, key);

    if (rc) {
        std::cout << "Key '" << key << "' matches '" << reg << "' "
                  << "with a score of " << rc << ".\n";
    }
    else {
        std::cout << "Key '" << key << "' does not match '" << reg << "'.\n";
    }
}
