#!/bin/bash -e

if [ -z "${MY_DIR}" ]; then
	echo common.sh must be included after defining the MY_DIR variable
	exit 1
fi

BUILD_DIR="${MY_DIR}/xa-build"
ARTIFACTS_DIR="${MY_DIR}/artifacts"
BINARIES="lld llvm-mc llvm-objcopy"

function die()
{
	echo "$*"
	exit 1
}

function create_dir()
{
	local dir="${1}"

	if [ -d "${dir}" ]; then
		rm -rf "${dir}"
	fi
	install -d -m 755 "${dir}"
}