#!/bin/bash -x
# https://devtutorial.io/how-to-get-a-list-of-the-changed-files-in-git-p1201.html
if [ $# -gt 1 ]; then
    UNDO_GIT_ID=$1
else
    UNDO_GIT_ID=HEAD
fi
CHANGED_FILES=$(git diff --name-only $UNDO_GIT_ID)
ADDED_FILES=$(git status -s |fgrep '??' | cut -d\  -f2)
for f in $CHANGED_FILES ; do
    cp -v $f $f\~
    git checkout $f
done
