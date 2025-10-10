#!/usr/bin/env bash
set -euxo pipefail

export SLURM_SRC="/home/${SLURM_USER}/slurm"
git clone -b ${SLURM_VERSION} --single-branch --depth=1 https://github.com/SchedMD/slurm.git ${SLURM_SRC}
cd ${SLURM_SRC}
./configure --enable-debug --prefix=/usr --sysconfdir=/etc/slurm --with-mysql_config=/usr/bin  --libdir=/usr/lib
make -j$(nproc)
make install

