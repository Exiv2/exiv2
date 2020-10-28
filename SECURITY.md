# Security Policy

## Supported Versions

| Exiv2 Version       | Security Support        | Security Releases |
|:--                  |:--                      |:-- |
| 0.27 and "dots"     | Process Defined         | v0.27.1, v0.27.2, v0.27.3 |
| 0.26                | Branch 0.26             | None |
| 0.25                | Branch 0.25             | None |

## The security release process

Security alerts are published here:  https://cve.mitre.org/cgi-bin/cvekey.cgi?keyword=Exiv2  We open an issue with the label "Security" on GitHub and fix it.  It doesn't get special treatment and will be included in the next release of the branch.

Team Exiv2 does not back-port security (or any other fix) to earlier releases of the code.  An engineer at SUSE has patched and fixed some security releases for Exiv2 v0.26 and Exiv2 v0.25 in branches 0.26 and 0.25.  Exiv2 has never issued a "security release" which would be an existing release PLUS one _or more_ security PRs.

The version numbering scheme is explained here: [https://clanmills.com/exiv2/book/#13-9](https://clanmills.com/exiv2/book/#13-9).  The design includes provision for a security release.

Exiv2 is not sufficiently resourced to provide notifications of security issues to users.  If the community decide that Exiv2 must strengthen its security process, the community will have to provide the necessary resources.

## Reported CVEs

| CVE               | Description                     | Solution  | PR | URL |
|:--                |:--                              |:--        |:--    |:--  |
| CVE-2019-9144     | Crash in BigTiffImage::printIFD | Remove src/bigtiffimage.cpp | #1331 | https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2019-9144 |
| to be continued   | | | | |
