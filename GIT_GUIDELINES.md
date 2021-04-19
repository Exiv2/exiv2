# Guidelines for using git

## Commit messages

The first line of a commit message should be < 80 characters long and briefly
describe the whole commit. Optionally, you can prefix the summary with a
tag/module in square brackets (e.g. travis if your commit changed something for
Travis, or testsuite for the testsuite, or tiff-parser, etc.).  If the commit
requires additional explanation, a blank line can be put below the summary
followed by a more thorough explanation.

A commit message can look like this:
```
[travis] Fix mac osx jobs

- Specify concrete ubuntu and mac versions
- Use latest conan version
- Fix the profiles for linux and mac
- Use new version of expat (avilable in conan-center)
- Install urllib3 as suggested in python guidelines
- Use virtualenv with python3
```

The advantage of this approach is that we always see the brief summary via `git
log --oneline` and on GitHub. The 80 characters limit ensures that the message
does not wrap.

Please avoid overly generic commit messages like "fixed a bug", instead write
e.g. "fixed an overflow in the TIFF parser". If your commit fixes a specific
issue on GitHub then provide its number in the commit message. A message of the
form "fixes #aNumber" result in GitHub automatically closing issue #aNumber once
the issue got merged (please write that in the detailed description below the
summary). If the commit fixes an issue that got a CVE assigned, then you must
mention the CVE number in the commit message. Please also mention it in commit
messages for accompanying commits (like adding regression tests), so that
downstream package maintainers can cherry-pick the respective commits easily.

If you have trouble finding a brief summary that fits into 80 characters, then
you should probably split your commit.


## When to commit

Commits should be atomic, i.e. they should make one self-contained
change. Consider the following example: you want to fix an issue, which requires
you to perform two changes in two separate files to fix the issue. Then you also
want to reformat both files using clang-format and add a regression test or a
unit test.

This would result in the following commits:
1. the fix for the issue in the two source files
2. addition of a unit test or regression test (provided that it does not require
   additional changes to other logical units)
3. Application of clang format to the first source file
4. Application of clang format to the second source file

We can summarize this in the following guidelines:
- Large formatting changes should be separate commits for each source file (that
  way changes can be reviewed easily, as formatting changes result in very noisy
  diffs)

- Changes made in different source which do not make sense without each other
  should be committed together

- Changes made in the same file which do not belong together should not be
  committed together

- If changes are requested during the code review, then they should be either
  included in the previous already created commits, if that is applicable.
  For example if a variable's name should be changed, then that should be
  included into the already created commit. A bigger change, like a new function
  or class will probably require a separate commit.

- Please keep in mind that your commits might be cherry-picked into an older
  branch. Therefore split your commits accordingly, so that changes into
  separate modules go into separate commits.

- Every commit should keep the code base in a buildable state. The test suite
  needn't pass on every commit, but must pass before being merged into
  `master`.

These are however not strict rules and it always depends on the case. If in
doubt: ask.


## Keeping the history linear

We prefer to keep the git log nearly linear with the individual pull requests
still visible, since they usually form one logical unit. It should look roughly
like this:
```
*   9f74f247 Merge pull request #227 from frli8848/master
|\
| * 73ac02d7 Added test for Sigma lenses
| * fc8b45dd Added the Sigma 120-300mm F2.8 DG OS HSM | S for Nikon mount.
| * 34a3be02 Added Sigma 50mm F1.4 DG HSM | A mount/UPC code (for Nikon mount).
| * 21522702 Added Sigma 20mm F1.4 DG HSM | A mount/UPC code (for Nikon mount).
|/
*   f9d421b1 Merge pull request #109 from D4N/error_codes_enum
|\
| * 3965a44d Replace error variable names in test suite with enum error codes
| * a15f090f Modified test suite so that case sensitive keys are possible
| * efe2ccdc Replaced all hardcoded error codes with ker... constants
| * d897997b Force error code usage to construct a Exiv2::BasicError
| * d3c3c036 Incorporated error codes into errList
| * b80fa1b4 Added error codes from src/error.cpp into an enumeration
|/
*   efee9a2b Merge pull request #205 from D4N/CVE-2017-1000127_reproducer
```
As can be seen, the two pull requests are still distinguishable but the history
is still nearly linear. This ensures that cherry-picking and bisecting works
without issues.

To ensure such a linear history, do **not** use GitHub's `Update Branch` button!
This creates a merge commit in your pull request's branch and can results in
rather complicated logs, like this:
```
* |
|\ \
| * |
* | |
|\ \ \
| |/ /
|/| |
| * |
| * |
| * |
| * |
| * |
|/ /
* |
|\ \
| |/
|/|
| *
| *
| *
|/
*
```

Instead of using the `Update Branch` button use `git pull --rebase`. For the
following example, we'll assume that we are working in a branch called
`feature_xyz` that should be merged into the branch `master`. Furthermore the
remote `origin` is a fork of exiv2 and the remote `upstream` is the "official"
exiv2 repository.

Before we start working, the `master` branch looks like this:
```
$ git log master --oneline --graph
*   efee9a2b (master) Merge pull request #something
|\
| * ead7f309 A commit on master
|/
*   55001c8d Merge pull request #something else
```

We create a new branch `feature_xyz` based on `master`, create two new commits
`My commit 1` and `My commit 2` and submit a pull request into master. The log
of the branch `feature_xyz` now looks like this:
```
$ git log feature_xyz --oneline --graph
* 893fffa5 (HEAD -> feature_xyz) My commit 2
* a2a22fb9 My commit 1
*   efee9a2b (master) Merge pull request #something
|\
| * ead7f309 A commit on master
|/
*   55001c8d Merge pull request #something else
```

If now new commits are pushed to `master`, resulting in this log:
```
$ git log master --oneline --graph
* 0d636cc9 (HEAD -> master) Hotfix for issue #something completely different
*   efee9a2b Merge pull request #something
|\
| * ead7f309 A commit on master
|/
*   55001c8d Merge pull request #something else
```
then the branch `feature_xyz` is out of date with `master`, because it lacks the
commit `0d636cc9`. We could now merge both branches (via the cli or GitHub's
`Update Branch` button), but that will result in a messy history. Thus **don't**
do it! If you do it, you'll have to remove the merge commits manually.

Instead run: `git pull --rebase upstream master` in the `feature_xyz`
branch. Git will pull the new commit `0d636cc9` from master into your branch
`feature_xyz` and apply the two commits `My commit 1` and `My commit 2` on top
of it:
```
$ git log feature_xyz --oneline --graph
* 22a7a8c2 (HEAD -> feature_xyz) My commit 2
* efe2ccdc My commit 1
* 0d636cc9 (master) Hotfix for issue #something completely different
*   efee9a2b Merge pull request #something
|\
| * ead7f309 A commit on master
|/
*   55001c8d Merge pull request #something else
```
Please note, that the hash of `My commit 1` and `My commit 2` changed! That
happened because their parent changed. Therefore you have to force push your
changes via `git push --force` next time you push your changes upstream.


## Merging pull requests

Most pull requests should be merged by creating a merge commit (the default on
GitHub). Small pull requests (= only one can commit) can be rebased on top of
master.


## Branches and tags

- The `master` branch is the current "main" development branch. It is protected
  so that changes can be only included via reviewed pull requests. New releases
  are made by tagging a specific commit on `master`.

- Releases are tagged with a tag of the form `v$major.$minor`. The tag is not
  changed when changes are backported.

- For each release a branch of the form `$major.$minor` should be created to
  store backported changes. It should be branched of from `master` at the commit
  which was tagged with `v$major.$minor`.

- All other branches are development branches for pull requests, experiments,
  etc. They should be deleted once the pull request got merged or the branch is
  no longer useful.

- Exiv2 team members can create branches for pull requests in the main
  repository if they want to collaborate with others (e.g. for big changes that
  require a lot of work). No one should not `push --force` in these branches
  without coordinating with others and should only `push --force-with-lease`.

  When only one person will work on a pull request, then the branch can be
  created in their personal fork or in the main repository (note that branches
  in the main repository provide an automatic continuous integration).


## Backporting changes

We try to backport critical bugfixes to the latest released version on a best
effort basis. We lack the man power to support older releases, but accept
patches for these.

Bugfixes for crashes, memory corruptions, overflows and other potentially
dangerous bugs **must** be backported. The same applies to bugfixes for issues
that got a CVE assigned.


## Final remarks

Since git is a fully distributed version control system, all changes stay on
your machine until you push them. Thus, if you are in doubt whether a trickier
step with git might screw up your repository, you can simply create a backup of
your whole exiv2 folder. In case the tricky step went downhill, you can restore
your working copy of exiv2 and no one will ever know (unless you did a `git
push`)!


## Additional material

- [The git book](https://git-scm.com/book/en/v2/)

- `man git` and `man git $command`

- [amending and interactive
  rebase](https://git-scm.com/book/en/v2/Git-Tools-Rewriting-History)

- [interactive
  staging](https://git-scm.com/book/en/v2/Git-Tools-Interactive-Staging) (for
  Emacs users: consider using [magit](https://magit.vc/) for interactive
  staging)
