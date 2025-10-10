#!/usr/bin/env bash
set -euxo pipefail

# Sets up passwordless ssh between nodes as required for
# Spindle using --with-rsh-launch
ssh-keygen -t ed25519 -f /home/${USER}/.ssh/id_ed25519 -N "" -q
cp /home/${USER}/.ssh/id_ed25519.pub /home/${USER}/.ssh/authorized_keys
chmod 600 /home/${USER}/.ssh/authorized_keys
cp /home/${USER}/ssh_config /home/${USER}/.ssh/config
chmod 600 /home/${USER}/.ssh/config
rm -f /home/${USER}/ssh_config
