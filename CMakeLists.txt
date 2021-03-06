# Copyright 2017 Google Inc.
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

cmake_minimum_required(VERSION 3.5)

# Define the project name, version, and main programming language.
set(PACKAGE_NAME      "google-cloud-cpp")
set(PACKAGE_VERSION   "0.1")
set(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/GoogleCloudPlatform/google-cloud-cpp/issues")
project(${PACKAGE_NAME} CXX C)

set(PROJECT_THIRD_PARTY_DIR "${PROJECT_SOURCE_DIR}/third_party")

# WARNING: do not change any compiler flags or variables in this
# CMakeList file.  The grpc subdirectory (and its children), have
# carefully crafted flags, and you may break them by using your own
# settings.  In particular -Werror -Wall breaks protobuf or grpc in many
# platforms, for an example of these problems (but not the only one) see:
#    https://github.com/grpc/grpc/pull/11376

include (cmake/IncludeGrpc.cmake)

# Enable testing in this directory so we can do a top-level `make test`.
enable_testing()

# Add subprojects here.
add_subdirectory(bigtable)

add_subdirectory(tests/grpc-crash)
