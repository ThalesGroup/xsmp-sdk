# Copyright 2025 THALES ALENIA SPACE FRANCE. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Post-process the stubs produced by pybind11-stubgen.

pybind11-stubgen annotates fixed size sequences with its own
``pybind11_stubgen.typing_ext.FixedSize`` marker. Shipping that would force
every consumer of the stubs to install pybind11-stubgen, so the annotation is
reduced to the plain sequence type.

Each stub is also parsed to make sure the generator did not emit a signature
that is not valid python.
"""

# the script runs with the interpreter the bindings are built for, down to 3.7
from __future__ import annotations

import ast
import pathlib
import re
import sys

# below python 3.9 pybind11-stubgen annotates with typing_extensions.Annotated,
# as typing.Annotated does not exist yet
FIXED_SIZE = re.compile(
    r"(?:typing|typing_extensions)\.Annotated\["
    r"\s*([^\[\]]+(?:\[[^\[\]]*\])?)\s*,"
    r"\s*pybind11_stubgen\.typing_ext\.FixedSize\([^()]*\)\s*\]"
)


def postprocess(path: pathlib.Path) -> None:
    content = path.read_text(encoding="utf-8")
    updated = FIXED_SIZE.sub(r"\1", content)
    updated = re.sub(r"^import pybind11_stubgen[^\n]*\n", "", updated, flags=re.M)

    if "pybind11_stubgen" in updated:
        raise SystemExit(f"{path}: unhandled reference to pybind11_stubgen")

    try:
        ast.parse(updated, filename=str(path))
    except SyntaxError as err:
        raise SystemExit(f"{path}: generated stub is not valid python: {err}")

    if updated != content:
        path.write_text(updated, encoding="utf-8")


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        raise SystemExit(f"usage: {argv[0]} <stubs-directory>")

    root = pathlib.Path(argv[1])
    stubs = sorted(root.rglob("*.pyi"))
    if not stubs:
        raise SystemExit(f"{root}: no stub was generated")

    for stub in stubs:
        postprocess(stub)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
