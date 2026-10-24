"""Check a portable ZIP's integrity, complete manifest, and runtime-only policy."""

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import zipfile


def verify(archive: Path) -> dict:
    with zipfile.ZipFile(archive) as package:
        if package.testzip() is not None:
            raise ValueError("ZIP CRC check failed")
        files = {}
        folded = set()
        for entry in package.infolist():
            name = entry.filename
            path = PurePosixPath(name)
            if "\\" in name or path.is_absolute() or ".." in path.parts or ":" in name:
                raise ValueError(f"Unsafe archive path: {name}")
            normalized = name.rstrip("/").casefold()
            if normalized in folded:
                raise ValueError(f"Duplicate/case-colliding archive path: {name}")
            folded.add(normalized)
            if not entry.is_dir():
                files[name] = package.read(entry)
        manifest = json.loads(files.pop("PACKAGE-MANIFEST.json"))
        if manifest["format"] != "Starfall-package-v1" or manifest["platform"] != "Windows-x64":
            raise ValueError("Unexpected package manifest format/platform")
        declared = {item["path"]: item for item in manifest["files"]}
        if len(declared) != len(manifest["files"]) or files.keys() != declared.keys():
            raise ValueError("Manifest file list differs from the complete archive")
        for name, data in files.items():
            item = declared[name]
            if len(data) != item["bytes"] or hashlib.sha256(data).hexdigest() != item["sha256"]:
                raise ValueError(f"Hash/length mismatch: {name}")
            lower = name.casefold()
            parts = PurePosixPath(lower).parts
            if any(part in {".git", "build", "qa", "tests", "fixtures", "scores"} for part in parts):
                raise ValueError(f"Development/profile directory in portable package: {name}")
            if lower.endswith((".obj", ".o", ".pdb", ".tmp", ".bmp", ".lib")) or PurePosixPath(lower).name == "leaderboard.txt":
                raise ValueError(f"Development/profile file in portable package: {name}")
        binaries = {name for name in files if name.lower().endswith((".dll", ".exe"))}
        expected = {"Starfall.exe", "SDL2.dll", "SDL2_image.dll", "SDL2_ttf.dll", "SDL2_mixer.dll"}
        if binaries != expected:
            raise ValueError(f"Unexpected executable/DLL set: {sorted(binaries)}")
        required = {"PLAY.bat", "READ-ME-FIRST.txt", "LICENSE.txt", "SDL2-LICENSE.txt",
                    "resources/reference/NOTICE.md", "docs/USER.md", "docs/PLAYER.md",
                    "docs/CREDITS.md", "third_party/RESOURCE-MANIFEST.md"}
        if not required <= files.keys():
            raise ValueError(f"Missing package entry points/notices: {sorted(required - files.keys())}")
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    recorded = Path(str(archive) + ".sha256").read_text(encoding="utf-8").split()[0]
    if digest != recorded:
        raise ValueError("Archive checksum sidecar mismatch")
    return {"archive": str(archive), "sha256": digest, "files": len(files) + 1,
            "source_commit": manifest["source_commit"], "compiler": manifest["compiler"],
            "result": "ZIP integrity, exact file hashes/list, and portable contents passed"}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    arguments = parser.parse_args()
    print(json.dumps(verify(arguments.archive), indent=2))
