#!/bin/sh -e
#
# This code has been taken, with modifications, from the public domain libabc
# example project.
# URL: http://0pointer.de/blog/projects/libabc.html

if [ -f .git/hooks/pre-commit.sample -a ! -f .git/hooks/pre-commit ] ; then
        cp -p .git/hooks/pre-commit.sample .git/hooks/pre-commit && \
        chmod +x .git/hooks/pre-commit && \
        echo "Activated pre-commit hook."
fi

autoreconf --install --symlink

libdir() {
        echo $(cd $1/$(gcc -print-multi-os-directory); pwd)
}

args=

echo
echo "----------------------------------------------------------------"
echo "Initialized build system. For a common configuration please run:"
echo "----------------------------------------------------------------"
echo
echo "./configure CFLAGS='-g -Wall -Wextra -O' $args"
echo
