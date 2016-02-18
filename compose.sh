#!/usr/bin/env bash
# . <(sed '/^export/!s/^/export /' "env.properties")
set -a
. env.properties
docker-compose "$@"