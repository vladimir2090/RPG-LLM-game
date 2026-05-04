#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PYTHON="$ROOT_DIR/.venv/bin/python"

if [[ ! -x "$PYTHON" ]]; then
    echo ".venv is missing. Run: uv venv .venv && uv pip install --python .venv/bin/python torch"
    exit 1
fi

"$PYTHON" "$ROOT_DIR/ai/slime_ai.py"
