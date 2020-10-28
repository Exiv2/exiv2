# Security Policy

## Supported Versions

| Exiv2 Version       | Security Support        | Security Releases |
|:--                  |:--                      |:-- |
| 0.27 and "dots"     | Process Defined         | v0.27.1, v0.27.2, v0.27.3 |
| 0.26                | Branch 0.26             | None |
| 0.25                | Branch 0.25             | None |

## Security Process

Security alerts are published here:  https://cve.mitre.org/cgi-bin/cvekey.cgi?keyword=Exiv2  We open an issue with the label "Security" on GitHub and fix it.  It doesn't get special treatment and will be included in the next release of the branch.

Team Exiv2 does not back-port security (or any other fix) to earlier releases of the code.  An engineer at SUSE has patched and fixed some security releases for Exiv2 v0.26 and Exiv2 v0.25 in branches 0.26 and 0.25.  Exiv2 has never issued a "security release" which would be an existing release PLUS one _or more_ security PRs.

The version numbering scheme is explained below.  The design includes provision for a security release.

Exiv2 is not sufficiently resourced to provide notifications of security issues to users.  If the community decide that Exiv2 must strengthen its security process, the community will have to provide the necessary resources.

## Version Numbering Scheme

| Version    | Name             | Status       | Purpose |
|:--         |:--               |:--           |:--      |
| 0.27.7.3   | Exiv2 v0.27.3    | GM           | Golden Master.  This is the final and official release. |
| 0.27.3.2   | Exiv2 v0.27.3.2  | RC2          | Release Candidate 2.                                    |
| v0.27.3.20 | Exiv2 v0.27.3.2  | RC2 Preview  | Dry-run for release candidate.  For team review.        |
| v0.27.3.81 | Exiv2 v0.27.3    | Security Fix | Security Release                          | 
| v0.27.3.29 | Exiv2 v0.27.3.29 | Development  | Should never be installed for production. |
| v0.27.4.9  | Exiv2 v0.27.4.9  | Development  | Should never be installed for production. |
| v0.27.99   | Exiv2 v0.28      | Development  | Should never be installed for production. |


## Reported CVEs

| CVE               | Description                     | Solution  | PR | URL |
|:--                |:--                              |:--        |:--    |:--  |
| CVE-2019-9144     | Crash in BigTiffImage::printIFD | Remove src/bigtiffimage.cpp | https://github.com/Exiv2/exiv2/pull/1331 | https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2019-9144 |
| to be continued   | | | | |
