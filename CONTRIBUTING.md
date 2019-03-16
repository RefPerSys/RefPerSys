# Contribution Guidelines for `refpersys`

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
  * assign issue to developer
  * assigned developer branches off master with branch name reflected issue number
  * developer makes regular, small commits on issue branch through `git -asm "<one-line message">`
  * developer runs `git rebase master` when done with feature development on issue branch
  * developer opens a merge request to a `staging` branch
  * team lead runs `git rebase master` on `staging` branch and merges developer's issue branch
  * team lead runs `git rebase -i` to squash issue branch commits to a single well-documented commit
  * team lead opens merge request to master branch
  * project manager reviews and merges `staging` branch to `master`

