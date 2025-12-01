#!/bin/sh

TEST_SRCDIR=$1
TEST_BUILDDIR=$2
COMPILE=$3

mkdir -p $TEST_BUILDDIR
$COMPILE -o $TEST_BUILDDIR/localeapp $TEST_SRCDIR/app.c -ldl -lpthread
$COMPILE -shared -fPIC -o $TEST_BUILDDIR/liblocaleaudit.so $TEST_SRCDIR/auditor.c
LD_AUDIT=$TEST_BUILDDIR/liblocaleaudit.so $TEST_BUILDDIR/localeapp

