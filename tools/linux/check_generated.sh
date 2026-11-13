#!/bin/bash
# Checks that the committed generated resources are what their generator
# writes now: gl/res/gl.rc, resource.h, bitmaps/ and icons/ from
# gl/res/tools/prune_rc.py (UI refresh spec 10). Run from the repository root:
#     tools/linux/check_generated.sh
# icons/fluent/ (UI refresh U2) is not prune_rc.py's and is skipped.
# Exit code 0 when they match; otherwise the differences are printed.
cd "$(dirname "$0")/../.." || exit 100
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
python3 gl/res/tools/prune_rc.py . "$out" >/dev/null || { echo "prune_rc.py failed"; exit 1; }
fails=0
for f in gl.rc resource.h; do
    if ! cmp -s "$out/$f" "gl/res/$f"; then
        echo "DIFFERS gl/res/$f"
        [ "$f" = gl.rc ] && diff <(iconv -f UTF-16LE -t UTF-8 "gl/res/$f") <(iconv -f UTF-16LE -t UTF-8 "$out/$f") | head -20
        fails=$((fails + 1))
    fi
done
for d in bitmaps icons; do
    if ! diff -rq -x fluent "$out/$d" "gl/res/$d" >/dev/null; then
        echo "DIFFERS gl/res/$d/"
        diff -rq -x fluent "$out/$d" "gl/res/$d" | head -10
        fails=$((fails + 1))
    fi
done
echo "check_generated: $fails difference(s)"
exit $fails
