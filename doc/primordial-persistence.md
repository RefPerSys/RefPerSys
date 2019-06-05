# Primordial Persistence Milestone

## Overview

The ability of RefPerSys to achieve primordial persistence is the first
important milestone in its development. In a nutshell, once RefPerSys achieves
this milestone, it will be able to load and dump the first set of primordial
objects that are hand-written in the persistore/initdata.rps file. In other
words, RefPerSys will have achieved a nascent ability to bootstrap.

## Persistent Store Files

The objects of RefPerSys are persisted on to disk through human-readable text
files ending with the `*.rps` identifier. These files are stored in the
persistore/ directory, and are limited in size to that supported by GitLab.
Although the Git version control system does not inherently limit the size or
number of files, GitLab does so.

## Manifest File

In addition to the `*.rps` files in the `persistore/` directory, there is a
special `MANIFEST.rps` file that lists out the persistent files in the directory
along with additional metadata details such as the machine hostname and
modification timestamps (to be detailed later). The syntax of the manifest file
has not been determined yet, but we know that it would need to be human
readable, and should also be designed to be easily read by the standard Linux
utilities such as `grep` and `awk`.

An important purpose of the `MANIFEST.rps` is file is to point to the persistent
file containing the primordial objects. By convention, this file is named as
`initdata.rps`. It is important to note that the initial handwritten
`initdata.rps` file itself will be loaded and parsed by RefPerSys, and generated
verbatim as part of the bootstrapping process. The mechanism is outlined below.

## Mechanism

TODO

