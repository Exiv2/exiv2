# Security Policy

## Supported Versions

| Exiv2 Version    | Date       | Tag     |  Branch          | _Dot/Security_ Release | Date       | Tag |
|:--               |:--         |:-       |:--               |:--                     |:-          |:-   |
| v0.28            | 2023-05-08 | v0.28.0 | 0.28.x           | v0.28.7                | 2025-08-31 | v0.28.7 |

## Security Process

If you have found a security vulnerability in Exiv2, please follow these steps:

* Click [this link](https://github.com/Exiv2/exiv2/security/advisories/new) to create a draft security advisory.
* Write a detailed description of the vulnerability in the draft advisory.
* Include all of the following details in your description of the vulnerability:
  * Exact version of Exiv2 that you tested. _For example: commit [194bb65ac568a5435874c9d9d73b1c8a68e4edec](https://github.com/Exiv2/exiv2/commit/194bb65ac568a5435874c9d9d73b1c8a68e4edec)_
  * Platform used. _For example: Ubuntu 22.04.3 LTS (x86\_64)_
  * Exact command used to build Exiv2. _For example: `mkdir build; cd build; cmake ..; make`_
  * Attach a copy of the image file that triggers the bug. _For example: `poc.jpg`_
  * Exact command line arguments that trigger the bug. _For example: `./bin/exiv2 poc.jpg`_
  * Crash output (stdout + stderr).
  * The source location of the bug and/or any other information that you are able to provide about what the cause of the bug is.

The draft security advisory is private until we publish it, so it is a good place to discuss the details of the vulnerability privately.

To qualify as a security issue, the bug **must** be reproducible on an official release of Exiv2, via a realistic attack vector. As a general rule, that means it should be possible to trigger the bug by running the `exiv2` command-line application on a malicious input file. Please note that the applications in the `samples` sub-directory are demo applications that are not intended for production use, so we usually do not consider bugs in those applications to be security vulnerabilities. However, if one of the sample applications reveals a legitimate bug in the exiv2 library then we will still consider it as a potential security issue.

Official releases are listed [here](https://github.com/Exiv2/exiv2/releases) (not including those labeled "pre-release"). Bugs that are only reproducible on the [main branch](https://github.com/Exiv2/exiv2/tree/main) or on a pre-release are not security issues and can be reported as regular [issues](https://github.com/Exiv2/exiv2/issues).

Team Exiv2 does not back-port security (or any other fix) to earlier releases of the code.  An engineer at SUSE has patched and fixed some security releases for Exiv2 v0.26 and Exiv2 v0.25 in branches 0.26 and 0.25.  Exiv2 has provided several _**Dot Release**_ for v0.27.  Exiv2 has never issued a _**Security Release**_.

The version numbering scheme is explained below.  The design includes provision for a security release.  A _**Dot Release**_ is an updated version of the library with security PRs and other changes.  A _**Dot Release**_ offers the same API as its parent.  A _**Security Release**_ is an existing release PLUS one or more security PRs.  Nothing else is changed from it parent.

Users can register on GitHub.com to receive release notices for RC and GM Releases.  Additionally, we inform users when we begin a project to create a new release on FaceBook (https://facebook.com/exiv2) and Discuss Pixls (https://discuss.pixls.us).  The announcement of a new release project has a preliminary specification and schedule.

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

## Fixed CVEs

The list of Exiv2 vulnerabilities that were previously found and fixed can be found at https://github.com/Exiv2/exiv2/security/advisories.
