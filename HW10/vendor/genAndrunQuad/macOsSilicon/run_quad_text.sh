#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $0 <path-to-quad-text-file>"
  echo "Example: $0 test/textsmoke.4.quad"
}

if [[ $# -ne 1 ]]; then
  usage
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

QUAD_FILE="$1"
if [[ ! -f "${QUAD_FILE}" ]]; then
  if [[ -f "${REPO_ROOT}/${QUAD_FILE}" ]]; then
    QUAD_FILE="${REPO_ROOT}/${QUAD_FILE}"
  else
    echo "Error: quad text file not found: ${QUAD_FILE}" >&2
    exit 1
  fi
fi

MAIN="./quadToLLVM"

LIBSYSY="libsysy64.ll"

"${MAIN}" --quad-text "${QUAD_FILE}"

LLVM_FILE="${QUAD_FILE%.quad}.llvm"
if [[ "${LLVM_FILE}" == "${QUAD_FILE}" ]]; then
  LLVM_FILE="${QUAD_FILE}.llvm"
fi

if [[ ! -f "${LLVM_FILE}" ]]; then
  echo "Error: expected LLVM output not found: ${LLVM_FILE}" >&2
  exit 1
fi

LINKED_FILE="${LLVM_FILE}.linked"
trap 'rm -f "${LINKED_FILE}" "${LLVM_FILE}"' EXIT

echo "Linking ${LLVM_FILE} with ${LIBSYSY}"
llvm-link "${LLVM_FILE}" "${LIBSYSY}" -o "${LINKED_FILE}"

echo "Running ${LINKED_FILE}"
lli "${LINKED_FILE}"
