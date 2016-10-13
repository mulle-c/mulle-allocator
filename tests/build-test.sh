#! /bin/sh

PROJECTDIR="`dirname "$PWD"`"
PROJECTNAME="`basename "${PROJECTDIR}"`"
LIBRARY_SHORTNAME="mulle_test_allocator"


. "mulle-tests/test-c-common.sh"
. "mulle-tests/test-tools-common.sh"
. "mulle-tests/test-staticlib-common.sh"
. "mulle-tests/build-test-common.sh"
