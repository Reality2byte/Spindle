#!/bin/tcsh

setenv REALCWD `realpath $PWD`
echo $0 $* | sed "s#${REALCWD}#ABS#g"

exit 0