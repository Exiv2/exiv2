# Security Policy

## Supported Versions

| Exiv2 Version  | Branch | _Dot_ or _Security_ Releases |
|:--             |:--               |:-- |
| v0.27          | 0.27-maintenance | v0.27.1<br>v0.27.2<br>v0.27.3 |
| v0.26          | Branch 0.26      | None |
| v0.25          | Branch 0.25      | None |

## Security Process

Security alerts are published here:  https://cve.mitre.org/cgi-bin/cvekey.cgi?keyword=Exiv2  We open an issue with the label "Security" on GitHub and fix it.  It doesn't get special treatment and will be included in the next release of the branch.

Team Exiv2 does not back-port security (or any other fix) to earlier releases of the code.  An engineer at SUSE has patched and fixed some security releases for Exiv2 v0.26 and Exiv2 v0.25 in branches 0.26 and 0.25.  Exiv2 has provided several _**Dot Release**_ for v0.27.  Exiv2 has never issued a _**Security Release**_.

The version numbering scheme is explained below.  The design includes provision for a security release.  A _**Dot Release**_ is an updated version of the library with security PRs and other changes.  A _**Dot Release**_ offers the same API as its parent.  A _**Security Release**_ is an existing release PLUS one or more security PRs.  Nothing else is changed from it parent.  

Exiv2 is not sufficiently resourced to provide notifications of security issues to users.  If the community decide that Exiv2 must strengthen its security process, the community will have to provide the necessary resources.

## Version Numbering Scheme

| Version    | Name             | Status       | Purpose |
|:--         |:--               |:--           |:--      |
| v0.27.7.3  | Exiv2 v0.27.3    | GM           | Golden Master.  This is the final and official release. |
| v0.27.3.2  | Exiv2 v0.27.3.2  | RC2          | Release Candidate 2.                                    |
| v0.27.3.20 | Exiv2 v0.27.3.2  | RC2 Preview  | Dry-run for release candidate.  For team review.        |
| v0.27.3.81 | Exiv2 v0.27.3    | Security Fix | Security Release                          | 
| v0.27.3.29 | Exiv2 v0.27.3.29 | Development  | Should never be installed for production. |
| v0.27.4.9  | Exiv2 v0.27.4.9  | Development  | Should never be installed for production. |
| v0.27.99   | Exiv2 v0.28      | Development  | Should never be installed for production. |


## Reported CVEs

| CVE               | Description                     | Solution  | PR |
|:--                |:--                              |:--        |:--    |
| [CVE-2019-9144](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2019-9144)     | Crash in BigTiffImage::printIFD | Remove src/bigtiffimage.cpp | [#1331](https://github.com/Exiv2/exiv2/pull/1331) |
| to be continued   | | | |
