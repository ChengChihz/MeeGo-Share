#!/bin/sh
# Application-private libraries only; no system libraries are replaced.
appdir=/opt/localsend-meego
LD_LIBRARY_PATH="$appdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH
exec "$appdir/n9-lanshare" "$@"
