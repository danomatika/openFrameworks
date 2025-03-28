#!/bin/bash
set -ev
ROOT=${TRAVIS_BUILD_DIR:-"$( cd "$(dirname "$0")/../../.." ; pwd -P )"}
# Add compiler flag to reduce memory usage to enable builds to complete
# see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56746#c7
# the "proper" way does not work currently:
#export CXXFLAGS="$(CXXFLAGS) --param ftrack-macro-expansion=0"
CUSTOMFLAGS="-ftrack-macro-expansion=0 -fpic"

if [ "$OPT" == "qbs" ]; then
    echo "building with qbs"
    export PATH="$TRAVIS_BUILD_DIR/linuxbrew/.linuxbrew/bin:$HOME/linuxbrew/.linuxbrew/sbin:$PATH"

    echo "##[group]**** Building emptyExample ****"
    cd $ROOT/scripts/templates/linux64
    sed -i "s/property bool makeOF: true/property bool makeOF: false/g" qtcreator.qbs
    qbs
    echo "##[endgroup]"
else
    echo "building with makefiles"
    cd $ROOT

    echo "##[group]**** Building OF core ****"
    # this carries over to subsequent compilations of examples
    echo "PLATFORM_CFLAGS += $CUSTOMFLAGS" >> libs/openFrameworksCompiled/project/linux64/config.linux64.default.mk
    sed -i "s/PLATFORM_OPTIMIZATION_CFLAGS_DEBUG = .*/PLATFORM_OPTIMIZATION_CFLAGS_DEBUG = -g0/" libs/openFrameworksCompiled/project/makefileCommon/config.linux.common.mk
    cd libs/openFrameworksCompiled/project
    make -j2
    echo "##[endgroup]"

    echo "##[group]**** Building emptyExample ****"
    cd $ROOT/scripts/templates/linux64
    make -j2
    echo "##[endgroup]"

    echo "##[group]**** Building allAddonsExample ****"
    cd $ROOT
    cp scripts/templates/linux64/Makefile examples/templates/allAddonsExample/
    cp scripts/templates/linux64/config.make examples/templates/allAddonsExample/
    cd examples/templates/allAddonsExample/
    make -j2
    echo "##[endgroup]"
fi
