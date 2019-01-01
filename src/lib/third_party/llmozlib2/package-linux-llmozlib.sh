#!/bin/bash
# This script generates a llmozlib source tarball for public consumption.

echo Exporting tree...
(rm -rf packagedllm && svn export . packagedllm) || exit

export PACKAGENAME=llmozlibsrc-sl-`date +%Y-%m-%d`.tar.bz2
echo Packaging to ${PACKAGENAME}
(cd packagedllm && tar --numeric-owner -X ../exclude.lst -cjf ../${PACKAGENAME} . ) || exit

echo Done.
ls -la ${PACKAGENAME}
