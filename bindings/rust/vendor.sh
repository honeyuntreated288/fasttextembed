#!/usr/bin/env bash
# Copy the C engine into ./csrc so the crate is self-contained (builds from crates.io).
# csrc/ is gitignored + regenerated; run before `cargo package`/`cargo publish`.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"; root="$here/../.."
rm -rf "$here/csrc"; mkdir -p "$here/csrc"
cp -R "$root/src" "$here/csrc/src"
cp -R "$root/include" "$here/csrc/include"
echo "vendored engine sources -> bindings/rust/csrc/"
