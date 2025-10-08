#!/usr/bin/env bash
set -euxo pipefail

mkdir mpich
pushd mpich
wget -O - https://www.mpich.org/static/downloads/${MPICH_VERSION}/mpich-${MPICH_VERSION}.tar.gz | tar xvz --strip-components 1
mkdir -p build
pushd build
../configure --prefix=/usr --disable-fortran --with-slurm=/usr/include/slurm 
make -j$(nproc) install
popd
popd
rm -rf mpich

