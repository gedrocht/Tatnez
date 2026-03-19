"""Copy generated Doxygen HTML into the MkDocs output directory."""

from __future__ import annotations

import shutil
import sys
from pathlib import Path


def main() -> int:
  if len(sys.argv) != 3:
    raise SystemExit("Usage: publish_api_docs.py <source_html_directory> <destination_directory>")

  source_html_directory = Path(sys.argv[1])
  destination_directory = Path(sys.argv[2])

  if not source_html_directory.exists():
    raise SystemExit(f"Source HTML directory does not exist: {source_html_directory}")

  if destination_directory.exists():
    shutil.rmtree(destination_directory)

  shutil.copytree(source_html_directory, destination_directory)
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
