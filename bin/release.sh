#! /bin/sh
#
# (c) 2016 Nat! for Mulle kybernetiK
#
# BSD3-License
#

PROJECT="MulleAllocator"    # requires camel-case
DESC="Flexible C memory allocation scheme with leak checking"
DEPENDENCIES='${REMOTEROOTDIR}/software/mulle-c11
${REMOTEROOTDIR}/software/mulle-thread'  # no camel case, will be evaled later!
LANGUAGE=c                  # c,cpp, objc

#
# Ideally you don't hafta change anything below this line
#
# source mulle-homebrew.sh (clumsily)

. ./bin/repository-info.sh || exit 1
. ./bin/mulle-homebrew/mulle-homebrew.sh || exit 1

# parse options
homebrew_parse_options "$@"

# dial past options
while [ $# -ne 0 ]
do
   case "$1" in
      -*)
         shift
      ;;
      *)
         break;
      ;;
   esac
done

#
# these can usually be deduced, if you follow the conventions
#
NAME="`get_name_from_project "${PROJECT}" "${LANGUAGE}"`"
HEADER="`get_header_from_name "${NAME}"`"
VERSIONNAME="`get_versionname_from_project "${PROJECT}"`"
VERSION="`get_header_version "${HEADER}" "${VERSIONNAME}"`"


# --- HOMEBREW FORMULA ---
# Information needed to construct a proper brew formula
#
HOMEPAGE="${REMOTEURL}/${NAME}"


# --- HOMEBREW TAP ---
# Specify to where and under what name to publish via your brew tap
#
RBFILE="${NAME}.rb"                    # ruby file for brew
HOMEBREWTAP="../homebrew-software"     # your tap repository path


# --- GIT ---
# tag to tag your release
# and the origin where
TAG="${1:-${TAGPREFIX}${VERSION}}"


main()
{
   git_main "${ORIGIN}" "${TAG}" || exit 1
   homebrew_main
}

main "$@"
