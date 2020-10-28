# Security Policy

## Supported Versions

| Version             | Supported          |
| -------             | ------------------ |
| 0.28                | :white_check_mark: |
| 0.27.4              | :white_check_mark: |
| 0.27.3 and earlier  | :x:                |

## The security release process

Security issues are reported as issues on https://github.com/exiv2/exiv2 with the label Security.

1.) Someone should feel responsible and make sure that it's really a security
    defect. If yes, we need one person who is responsible for this specific
    issue beyond this point. This person coordinates the further actions and
    might delegate tasks, of course.

2.) Create a bug report with limited access to "libssh core developers" only!
    Change the summary so that it starts with "[EMBARGOED][SECURITY]".
    Add all information including reproducer etc. to the bug report.

3.) Write an initial advisory and attach it to the bug report. A template is
    available below.

4.) Do a CVSSv3 calculation ([2]) and add it to the bug report and the advisory.

5.) Ask for a CVE (usually Red Hat Product Security  secalert@redhat.com (see
    [1] for GPG key etc.) does help out on this one) using the initial
    advisory as evidence.
    After receiving the CVE number, please add it as an alias to the bug report
    advisory and to the summary.

To:      Red Hat Product Security  secalert@redhat.com
Subject: Another libssh CVE (Name of issue)
See the attached initial advisory for what will be bug XXXX.
Please issue us a CVE. No release date is set yet, you will get the
normal mails when that is done.
Thanks!
(attach the advisory from step 3)

6.) Write patches and tests and add them to the bug report or push intermediate
    steps the the teams security git repository.

6a.) ALL commits in the patch should have a CVE prefix and a BUG tag eg:

CVE-2038-12345 packet: Introduced a packet filter

Check that incoming packets are received in the required state.

BUG: https://bugs.libssh.org/T9999

Signed-off-by: Lord Auth  lord@libssh.org

6b.) Ask for review of your WIP and final master patches ASAP

6c.) Prepare backports for all affected and supported versions
     (1 file per version, even if identical).
     Files should be named $CVE-$MAJOR-$VERSION.patch
     E.g.: CVE-2038-12345-0.9.1-01.patch

6d.) Run private Gitlab CI for each CVE and each maintained branch.
     (This allows the release manager to defer some patches if problems
      appear later in the process).

6e.) Once each CVE patch and branch has passed, ask for review ASAP.

7.) Once all patches and the advisory are available and reviewed, ask the
    release manager for a release date.

7a.) Between now and the release, the release manager or their
     delegate will run a private private Gitlab CI of the exact
     combination of patches the be released.

7b.) If multiple issues are being released in a batch, create a
     tracking bug that is blocked by the CVEs to be addressed in this
     bundle.

8.) Finish the advisory and attach it to the bug report as
    advisory-$CVE.txt.  Ask for review.  (This must be after getting
    the release date so the correct version numbers can be included).

9.) ~10 days before the release date, confirm every patch and the CVE
    text still has the correct reviews (due to additional feedback for
    example).

9a.) Add vendors to the CC list in the bug report.

9b.) Add the planned release date to the bug or the tracking bug if
     that has been created.  This will be the first useful e-mail seen
     by our vendors.
10.) 3 days before the release date, send an e-mail to
      libssh@libssh.org indicating there will be a security
     release and the broad component it impacts (client or server).
     This is to allow large installations time to prepare for security
     patching.
     For example:

To:  libssh@libssh.org
Subject: Heads-up: Security releases ahead!
Hi,
this is a heads-up that there will be libssh security updates on
Tuesday, XXXXX YYth. Please make sure that your libssh
installations will be updated soon after the release!
Impacted components:
client (CVSS 5.9, Medium)
server (CVSS 6.8 Medium) 

10a.) Note that this represents the final GO/NO-GO point.  In order to
      facilitate creation of tarballs etc, after this date the patches
      and CVE text must not change or else the release will need to
      be publicly rescheduled, patches dropped or other appropriate
      action taken at the absolute discretion of the release manager.

11.) The release manager or their delegate will prepare the tarballs
     in the stable branches:
     - git fetch
     - git rebase
     - git merge -ff-only the VERSION bump commit from the corresponding test
       branch
     - apply the security patchset
     - ChangeLog: add the release notes
     - Bump version
     DO NOT PUBLISH ANYTHING BEFORE THE END OF EMBARGO!
     On the release day:
     Upload the files and publish the announcements as described in steps 3.) and 4.)

12.) Additional steps for the release manager:
     - upload the signed security patch(es) to the ftp server:
       public_html/security/

13.) After shipping the releases, the responsible person must make
     sure that the patches find their way into the master branch and
     remove the [EMBARGOED] tag from the bug report.

14.) Mark as 'private' any sensitive comments or attachements.

15.) Address any minor improvements that were suggested after the
     patches were frozen and incorporate those into commits in master.

16.) Close out bug report when patches have been pushed to all relevant
     branches.

[1] https://access.redhat.com/security/team/contact/
[2] https://nvd.nist.gov/vuln-metrics/cvss/v3-calculator

## Security Advisory

===========================================================
== Subject:     
==
== CVE ID#:     
==
== Versions:    
==
== Summary:     
==
===========================================================

===========
Description
===========

==================
Patch Availability
==================

Patches addressing the issues have been posted to:
https://www.libssh.org/security/
Additionally, libssh $VERSIONS have been issued
as security releases to correct the defect.  SSH administrators are
advised to upgrade to these releases or apply the patch as soon
as possible.

==================
CVSSv3 calculation
==================
CVSS:3.0/AV:/AC:/PR:/UI:/S:/C:/I:/A: (X.X)

==========
Workaround
==========

=======
Credits
=======

Originally reported by $REPORTER.
Patches provided by $DEVELOPER of the libssh team.

==========================================================
== Team Exiv2
==========================================================
