#!/usr/bin/env bash
#
# Copyright (c) 2019-present The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

export CONTAINER_NAME=ci_native_previous_releases_minimal
export CI_IMAGE_NAME_TAG="mirror.gcr.io/ubuntu:24.04"
export PACKAGES="gcc g++ python3-zmq libsqlite3-dev libevent-dev libboost-dev"
export DEP_OPTS=""
export RUN_UNIT_TESTS=false
export RUN_FUNCTIONAL_TESTS=true
export TEST_RUNNER_EXTRA="--previous-releases feature_coinstatsindex_compatibility.py"
export GOAL="install"
export CI_LIMIT_STACK_SIZE=1
export DOWNLOAD_PREVIOUS_RELEASES="true"

export NO_DEPENDS=1

export BITCOIN_CONFIG="\
 --preset=dev-mode \
 -DBUILD_GUI=OFF \
 -DWITH_ZMQ=OFF \
 -DWITH_USDT=OFF \
 -DENABLE_IPC=OFF \
 -DREDUCE_EXPORTS=ON \
 -DCMAKE_BUILD_TYPE=Release \
"
