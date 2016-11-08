#! /bin/sh

# ARCHIVEURL will be evaled later! keep it in single quotes
# where homebrew grabs the archive off
ARCHIVEURL='https://github.com/mulle-nat/${NAME}/archive/${VERSION}.tar.gz'

# written into formula for homebrew, will be evaled
HOMEPAGE='https://github.com/mulle-nat/${NAME}'

# git remote to push to, usually origin
ORIGIN='public'

# base tap for dependencies
DEPENDENCY_TAP='mulle-kybernetik/software'


# keep this
:
