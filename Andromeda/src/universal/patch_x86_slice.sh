#!/bin/sh
set -eu
if [ "$#" -eq 0 ]; then exit 0; fi
exec "$@"