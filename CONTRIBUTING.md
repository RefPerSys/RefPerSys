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

  * each contributor commits to the `master` branch
  * the issue tracker on GitLab is used to track the progress of tasks

