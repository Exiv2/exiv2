Contributing to the Exiv2 Project
======================

# Contents #

* [1. Introduction](#1-introduction)
* [2. Contributing code via GitHub](#2-contributing-code-via-github)
* [3. Contributing code via email](#3-contributing-code-via-email)
* [4. Contributing Lens Data](#4-contributing-lens-data)
* [5. Reporting Bugs](#5-reporting-bugs)

# 1. Introduction #

We welcome any help, for example contributing lens data (images), code contributions and bug reports.

# 2. Contributing code via GitHub #

Code contributions can be performed via *pull requests* (PR) on GitHub (if you cannot or do not want to use GitHub, see [3. Contributing code via email](#3-contributing-code-via-email)).
For this to work you first need to [create a user account on GitHub](https://help.github.com/articles/signing-up-for-a-new-github-account/) if don't already have one.
A pull request should preferable contain only one new feature or bug fix etc. Since it is not uncommon to work on several PRs at the same time
it is recommended to create a new _branch_ for each PR. In this way PRs can easily be separated and the review and merge process becomes cleaner.
As a rule-of-thumb:

- PRs should be kept at a manageable size. Try to focus in just one goal per PR. If you find yourself doing several things in a PR that were not expected,
  then try to split the different tasks into different PRs.
- Commits should always change a *single* logical unit so that cherry-picking & reverting is simple.
- Commit messages should be as informative and concise as possible. The first line of the commit message should be < 80 characters and
  describe the commit briefly. If the 80 characters are too short for a summary, then consider splitting the commit. Optionally, add one blank line
  below the short summary and write a more detailed explanation if necessary.

See the [GIT_GUIDELINES.md](git_guidelines.md) file for a more detailed description of the git workflow.

Below we outline the recommended steps in the code contribution workflow. We use `your-username` to refer to your username on GitHub, `exiv2_upstream` is used when we
set the upstream remote repository for Exiv2 (we could have picked any name by try to avoid already used names like, in particular, `origin` and `master`), and
we use the name `my-new-feature` for the branch that we create (e.g., the branch name should reflect the code change being made).

**Important**: If your PR lives for a long time, then don't press the button _Update branch_ in the Pull Request view, instead follow the steps below, as
that avoids additional merge commits.

Once you have a GitHub login:

1. Fork the Exiv2 git repository to your GitHub account by pressing the _Fork_ button at: [https://github.com/Exiv2/exiv2](https://github.com/Exiv2/exiv2)
(more details [here](https://guides.github.com/activities/forking/)).

2. Then start a terminal (or use your favorite git GUI app) and clone your fork of the Exiv2 repo:

        $ git clone https://github.com:your-username/exiv2.git
        $ cd exiv2

3.  Add a new remote pointing to upstream exiv2 repository:

        $ git remote add exiv2_upstream https://github.com/Exiv2/exiv2.git

    and verify that you have the two remotes configured correctly:

        $ git remote -v
        exiv2_upstream  https://github.com/Exiv2/exiv2.git (fetch)
        exiv2_upstream  https://github.com/Exiv2/exiv2.git (push)
        origin  https://github.com/your-username/exiv2.git (fetch)
        origin  https://github.com/your-username/exiv2.git (push)

4. Next, create a branch for your PR from `exiv2_upstream/master` (which we also need to fetch first):

        $ git fetch exiv2_upstream master
        $ git checkout -b my-new-feature exiv2_upstream/master --no-track

    NB: This is an important step to avoid draging in old commits!

5. Configure the project and check that it builds (if not, please report a bug):

        $ rm -rf build
        $ mkdir build && cd build
        $ cmake -DCMAKE_BUILD_TYPE=Release ..
        $ make

6. Now, make your change(s), add tests for your changes, and commit each change:

        ...

        $ git commit -m "Commit message 1"

        ...

        $ git commit -m "Commit message 2"

7. Make sure the tests pass:

        $ make tests         # Integration tests
        $./bin/unit_tests    # Unit tests

    Exiv2's (new) test system is described in more detail in the [doc.md](tests/doc.md) and [writing_tests.md](tests/writing_tests.md) files, and a description of the old
    test system can be found in the Redmine wiki: [How do I run the test suite for Exiv2](http://dev.exiv2.org/projects/exiv2/wiki/How_do_I_run_the_test_suite_for_Exiv2)

8. Push the changes to your fork on GitHub:

        $ git push origin my-new-feature

9. Create the PR by pressing the _New pull request_ button on: `https://github.com/your-username/exiv2`. Please select the option "Allow edits from maintainers" as outlined
   [here](https://help.github.com/en/articles/allowing-changes-to-a-pull-request-branch-created-from-a-fork).

10. Now wait for an Exiv2 project member(s) to respond to your PR. Follow the discussion on your PR at [https://github.com/Exiv2/exiv2/pulls](GitHub).
    You may have to do some updates to your PR until it gets accepted.

11. After the PR has been reviewed you must _rebase_ your repo copy since there may have been several changes to the upstream repository.

    Switch to your branch again

        $ git checkout my-new-feature

    And rebase it on top of master:

        $ git pull --rebase exiv2_upstream master

    When you perform a rebase the commit history is rewritten and, therefore, the next time you try to push your branch to your fork repository you will need to use
    the `--force-with-lease` option:

        $ git push --force-with-lease

Also, follow the coding guidelines outlined in [CODING_GUIDELINES.md](CODING_GUIDELINES.md).

# 3. Contributing Code via email #

If you cannot or do not want to use GitHub, you can still submit patches via email by using our [sourcehut mirror](https://git.sr.ht/~d4n/exiv2).

Prepare your changes in your local clone of the [GitHub](https://github.com/Exiv2/exiv2.git) or [sourcehut](https://git.sr.ht/~d4n/exiv2) repository following our
[CODING_GUIDELINES.md](CODING_GUIDELINES.md) and [GIT_GUIDELINES.md](git_guidelines.md). Send your patches to the
[~d4n/exiv2-patches@lists.sr.ht](mailto:~d4n/exiv2-patches@lists.sr.ht) mailing list. Please use `git send-email` as outlined in https://git-send-email.io/ to
simplify the integration of your patches.

# 4. Contributing Lens Data #

In order for the Exiv2 project to support a new lens we need an example image containing the Exif metadata of that lens. This is a good way for
non-programmers to contribute to the project and example images can be submitted using the following procedure:

1. Create a new Issue by pressing the _New issue_ button here: [https://github.com/Exiv2/exiv2/issues](https://github.com/Exiv2/exiv2/issues),
2. In the new Issue, enter/add the lens mount and full lens name for each lens,
3. Take a (small) .jpg image (with the lens cap on) with each lens and transfer the .jpg file(s) to disk __without processing it__ in a desktop or server software (this is important to preserve the exif metadata in the file),
4. Attach the .jpg image(s) to the Issue (one can just drag-and-drop the image(s) or paste it/them from the clipboard).

Note that we are not only interested in non-supported lenses since we also look for example images to expand and improve the Exiv2 code tests.

# 5. Reporting Bugs #

Bugs should be reported by creating Issues on GitHub. However, before reporting a bug first check the Issue list if the bug is already known, and only if you cannot find any previous bug report
then create a new Issue. When reporting a bug try to describe the problem in as much detail as possible and if the bug is triggered by an input file then attach that file to the GitHub Issue, too.
