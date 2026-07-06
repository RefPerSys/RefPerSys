#!/usr/bin/env bash
set -euo pipefail

export HOME="${HOME:-/tmp/refpersys-home}"
export REFPERSYS_TOPDIR="${REFPERSYS_TOPDIR:-$PWD}"
export CC="${CC:-/usr/bin/gcc-${GCC_VERSION:-15}}"
export CXX="${CXX:-/usr/bin/g++-${GCC_VERSION:-15}}"
export CXXFLAGS="${CXXFLAGS:--O2 -g -fPIC}"
export CARBURETTA="${CARBURETTA:-/usr/local/bin/carburetta}"
export RPS_BUILDER_PERSON="${RPS_BUILDER_PERSON:-RefPerSys Docker Builder}"
export RPS_BUILDER_EMAIL="${RPS_BUILDER_EMAIL:-refpersys-docker@example.invalid}"
export RPS_LIBBACKTRACE="${RPS_LIBBACKTRACE:-1}"
export PATH="/usr/lib/ccache:/usr/local/bin:$PATH"

mkdir -p "$HOME" "$HOME/tmp"

if [[ "$(id -u)" -eq 0 && -n "${REFPERSYS_HOST_UID:-}" && "${REFPERSYS_HOST_UID}" != "0" ]]; then
  target_uid="${REFPERSYS_HOST_UID}"
  target_gid="${REFPERSYS_HOST_GID:-$target_uid}"
  target_user="$(getent passwd "$target_uid" | cut -d: -f1 || true)"

  if [[ -z "$target_user" ]]; then
    target_user="refpersys"
    if getent passwd "$target_user" >/dev/null; then
      target_user="refpersys${target_uid}"
    fi
    if ! getent group "$target_gid" >/dev/null; then
      printf '%s:x:%s:\n' "$target_user" "$target_gid" >> /etc/group
    fi
    printf '%s:x:%s:%s::%s:/bin/bash\n' \
      "$target_user" "$target_uid" "$target_gid" "$HOME" >> /etc/passwd
  fi

  chown "$target_uid:$target_gid" "$HOME" "$HOME/tmp"
  exec setpriv --reuid "$target_uid" --regid "$target_gid" --clear-groups "$0" "$@"
fi

if [[ $# -eq 0 ]]; then
  set -- bash -lc 'gmake config && gmake -j"$(nproc)" all && gmake test00'
fi

exec "$@"
