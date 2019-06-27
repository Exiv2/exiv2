# Guidelines for using git

## Commit messages

- The first line of a commit message SHOULD be < 80 characters long and briefly
  describe the whole commit.

- You CAN prefix the summary with a tag/module in square brackets (e.g. travis
  if your commit changed something for Travis, or testsuite for the testsuite, or
  tiff-parser, etc.).

- If the commit requires additional explanation, a blank line SHOULD be put
  below the summary followed by a more thorough explanation.

- If an issue on GitHub is fixed, then this issue SHOULD be mentioned in the
  commit message.

  A message of the form "fixes #aNumber" result in GitHub automatically closing
  issue #aNumber once the issue got merged (this line SHOULD be placed in the
  detailed description below the summary).

- If the commit fixes an issue that got a CVE assigned, then you MUST mention
  the CVE number in the commit message. Please also mention it in commit
  messages for accompanying commits (like adding regression tests), so that
  downstream package maintainers can cherry-pick the respective commits easily.

A good commit message would look like this:
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

Commit messages SHOULD not be overly generic like "fixed a bug", instead write
e.g. "fixed an overflow in the TIFF parser".

If you have trouble finding a brief summary that fits into 80 characters, then
you SHOULD split your commit.


## When to commit

- Commits SHOULD be atomic, i.e. they should contain **one** self-contained
  change.

- Large formatting changes SHOULD be separate commits for each source file (that
  way changes can be reviewed easily, as formatting changes result in very noisy
  diffs)

- Changes made in different source which do not make sense without each other
  SHOULD be committed together

- Changes made in the same file which do not belong together SHOULD not be
  committed together

- If changes are requested during the code review, then they SHOULD be included
  in the previous already created commits or added as a fixup commit. A bigger
  change (like a new function or class) CAN require a separate commit.

- Every commit should keep the code base in a buildable state. The test suite
  needn't pass on every commit, but must pass before being merged into
  `master` or a maintenance branch.


### Example:

You want to fix an issue, which requires you to perform two changes in two
separate files to fix the issue. Then you also want to reformat both files using
clang-format and add a regression test or a unit test.

This would result in the following commits:
1. the fix for the issue in the two source files
2. addition of a unit test or regression test (provided that it does not require
   additional changes to other logical units)
3. Application of clang format to the first source file
4. Application of clang format to the second source file


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

- GitHub's `Update Branch` button SHOULD not be used, `git pull --rebase` SHOULD
  be used instead.


## Merging pull requests

- Most pull requests SHOULD be merged by creating a merge commit (the default on
  GitHub).

- Small pull requests (= only one commit) CAN be rebased.

- Pull requests with GPG signed commits MUST NOT be rebased by anyone except the
  commit author, as the signature will otherwise be dropped. This does only
  apply if the signature belongs to the commit author, signatures of the
  committer can be removed.

- If a pull request contains merge commits to the target branch, then these MUST
  be removed prior to merging.

- fixup-commits and commits containing modifications after a review MUST NOT be
  merged and MUST be removed before merging, either by squashing the whole pull
  request or via `git rebase --autosquash` (for fixup commits).


## Branches and tags

- The `master` branch is the current "main" development branch. It is protected
  so that changes can be only included via reviewed pull requests. New releases
  are made by tagging a specific commit on `master`.

- Releases are tagged with a tag of the form `$major.$minor`. The tag MUST NOT
  be not changed when changes are backported.

- For each release a branch of the form `$major.$minor-maintenance` SHOULD be
  created to store backported changes. It should be branched of from `master` at
  the commit which was tagged with `$major.$minor`.

- All other branches are development branches for pull requests, experiments,
  etc. They SHOULD be deleted once the pull request got merged or the branch is
  no longer useful.

- Exiv2 team members SHOULD create branches for pull requests in the main
  repository (so that GitLab CI will run on these branches). No one SHOULD not
  `push --force` in these branches without coordinating with others and should
  only `push --force-with-lease`.


## Backporting changes

We try to backport critical bugfixes to the latest released version on a best
effort basis. We lack the man power to support older releases, but accept
patches for these.

Bugfixes for crashes, memory corruptions, overflows and other potentially
dangerous bugs **MUST** be backported. The same applies to bugfixes for issues
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
