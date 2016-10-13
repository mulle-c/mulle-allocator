#!/usr/bin/env bash
#  run-test.sh
#  MulleObjC
#
#  Created by Nat! on 01.11.13.
#  Copyright (c) 2013 Mulle kybernetiK. All rights reserved.
#  (was run-mulle-scion-test)

PROJECTDIR="`dirname "$PWD"`"
PROJECTNAME="`basename "${PROJECTDIR}"`"
LIBRARY_SHORTNAME="mulle_test_allocator"
ADDITIONAL_LIBS=mulle_allocator

. "mulle-tests/test-c-common.sh"
. "mulle-tests/test-tools-common.sh"
. "mulle-tests/test-staticlib-common.sh"
. "mulle-tests/run-test-common.sh"
