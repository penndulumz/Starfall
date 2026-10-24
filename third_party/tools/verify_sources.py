"""Verify mirrored source bytes against each pinned GitHub Git-tree manifest."""

import hashlib
import json
import sys
from pathlib import Path


third_party = Path(__file__).resolve().parents[1]
failures = []
checked = 0
for library in ("SDL_image", "SDL_mixer", "SDL_ttf", "freetype"):
    lock = third_party / library / "source-lock.json"
    if not lock.is_file():
        failures.append(f"Missing source manifest: {lock}")
        continue
    data = json.loads(lock.read_text(encoding="utf8"))
    for entry in data["files"]:
        source = third_party / library / "upstream" / entry["path"]
        if not source.is_file():
            failures.append(f"Missing source: {library}/{entry['path']}")
            continue
        content = source.read_bytes()
        digest = hashlib.sha1(
            b"blob " + str(len(content)).encode("ascii") + b"\0" + content
        ).hexdigest()
        if digest != entry["sha"] or len(content) != entry["size"]:
            failures.append(f"Source bytes differ: {library}/{entry['path']}")
        checked += 1
    print(f"{library}: {data['revision']}, {len(data['files'])} declared source files")

if failures:
    print("\n".join(failures), file=sys.stderr)
    raise SystemExit(1)
print(f"PASS: {checked} source files match their pinned upstream Git blobs")
