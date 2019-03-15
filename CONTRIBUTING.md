# Contribution Guidelines for `refpersys`

These are the development guidelines for the `refpersys` project. At this stage,
these conventions are largely a work in progress, and should *not* be considered
as canonical as they would evolve as required.

## Coding Conventions

  * use K&R C style
  * typedef'd opaque types are preferred wherever possible
  * prefix types with namespace `rps_`
  * lowercase snake case for all types (except enumerations)
  * lowercase snake case for all callable units
  * uppercase snake case for all symbolic constants and enumerations
  * prefix callable units for `type` with `rps_type_`
  * prefix constants for `type` with `RPS_TYPE_`
  * interface code goes to header files in `inc` directory
  * implementation code goes to source files in `src` directory

## Workflow

  * create issue on tracker
  * assign issue to contributor
  * `git checkout -b` off master with branch name reflecting issue number
  * contributor makes regular, small commits on issue branch through `git -asm`
  * `git rebase master` when done with feature development
  * open a merge request to a `staging` branch
  * `git rebase master` on `staging` branch and `git merge --no-ff` issue branch
  * `git rebase -i` to squash issue branch commits to a single well-documented commit
  * open merge request to master branch
  * review and `git merge --no-ff` `staging` branch to `master`

The rationale for this approach is as follows:
  * merge conflicts are minimised
  * development history is preserved
  * `master` branch is kept clean
  * `git log --graph` output is nice and tidy

