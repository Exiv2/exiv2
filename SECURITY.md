# Security Policy

## Supported Versions

| Exiv2 Version  | Branch | _Dot_ or _Security_ Releases |
|:--             |:--               |:-- |
| v0.27          | 0.27-maintenance | v0.27.1<br>v0.27.2<br>v0.27.3 |
| v0.26          | Branch 0.26      | None |
| v0.25          | Branch 0.25      | None |

## Security Process

If you have found a security vulnerability in Exiv2, please follow these steps:

* Send an email to our security contact person: [Kevin Backhouse](https://github.com/kevinbackhouse) (kevinbackhouse@github.com).
* We will create a draft [security advisory](https://github.com/Exiv2/exiv2/security/advisories) and invite you as a collaborator.
* Add a comment to the draft security advisory with a detailed description of the vulnerability. (Please don't use the "Description" field, because we'll use the "Description" field to write a short summary later.)
* Include all of the following details in your description of the vulnerability:
  * Exact version of Exiv2 that you tested. _For example: commit [194bb65ac568a5435874c9d9d73b1c8a68e4edec](https://github.com/Exiv2/exiv2/commit/194bb65ac568a5435874c9d9d73b1c8a68e4edec)_
  * Platform used. _For example: Ubuntu 20.04.2 LTS (x86\_64)_
  * Exact command used to build Exiv2. _For example: `mkdir build; cd build; cmake ..; make`_
  * Attach a copy of the image file that triggers the bug. _For example: `poc.jpg`_
  * Exact command line arguments that trigger the bug. _For example: `./bin/exiv2 poc.jpg`_
  * Crash output (stdout + stderr).
  * The source location of the bug and/or any other information that you are able to provide about what the cause of the bug is.

The draft security advisory is private until we publish it, so it is a good place to discuss the details of the vulnerability privately. For the initial email, just a summary of the issue is sufficient.

Team Exiv2 does not back-port security (or any other fix) to earlier releases of the code.  An engineer at SUSE has patched and fixed some security releases for Exiv2 v0.26 and Exiv2 v0.25 in branches 0.26 and 0.25.  Exiv2 has provided several _**Dot Release**_ for v0.27.  Exiv2 has never issued a _**Security Release**_.

The version numbering scheme is explained below.  The design includes provision for a security release.  A _**Dot Release**_ is an updated version of the library with security PRs and other changes.  A _**Dot Release**_ offers the same API as its parent.  A _**Security Release**_ is an existing release PLUS one or more security PRs.  Nothing else is changed from it parent.  

Users can register on GitHub.com to receive release notices for RC and GM Releases.  Additionally, we inform users when we begin a project to create a new release on FaceBook (https://facebook.com/exiv2) and Discuss Pixls (https://discuss.pixls.us).  The announcement of a new release project has a preliminay specification and schedule.

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
