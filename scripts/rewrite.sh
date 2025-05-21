#!/bin/bash

SRCDIR=$1
WORKINGDIR=$2
CCLINE=$3

$CCLINE $SRCDIR/client/client/patch_interception.c -o $WORKINGDIR/patch_test -ldl -DCONFIG_TEST
RETVAL=$?
if [[ "x$RETVAL" != "x0" ]]; then
    exit -1
fi
LDSO=`ldd $WORKINGDIR/patch_test |& grep ld-linux | awk '{print $1}'`
$WORKINGDIR/patch_test `nm $LDSO | grep __xstat | awk '{print $1}' | sort | uniq` `nm $LDSO | grep __lxstat | awk '{print $1}' | sort | uniq` `nm $LDSO | grep rtld_errno | awk '{print $1}' | sort | uniq`
RETVAL=$?
rm -f $WORKINDIR/patch_test
exit $RETVAL
