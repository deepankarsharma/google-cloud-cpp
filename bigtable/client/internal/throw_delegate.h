// Copyright 2018 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GOOGLE_CLOUD_CPP_BIGTABLE_CLIENT_INTERNAL_THROW_DELEGATE_H_
#define GOOGLE_CLOUD_CPP_BIGTABLE_CLIENT_INTERNAL_THROW_DELEGATE_H_

#include <grpc++/grpc++.h>
#include "bigtable/client/version.h"

namespace bigtable {
inline namespace BIGTABLE_CLIENT_NS {
namespace internal {

//@{
/**
 * @name Delete exception raising to hidden functions.
 *
 * The following functions raise the corresponding exception, unless the user
 * has disabled exception handling, in which case they print the error message
 * to std::cerr and call std::abort().
 *
 * We copied this technique from Abseil.  Unfortunately we cannot use it
 * directly because it is not a public interface for Abseil.
 */
[[noreturn]] void RaiseInvalidArgument(char const* msg);
[[noreturn]] void RaiseInvalidArgument(std::string const& msg);

[[noreturn]] void RaiseRangeError(char const* msg);
[[noreturn]] void RaiseRangeError(std::string const& msg);

[[noreturn]] void RaiseRuntimeError(char const* msg);
[[noreturn]] void RaiseRuntimeError(std::string const& msg);

[[noreturn]] void RaiseLogicError(char const* msg);
[[noreturn]] void RaiseLogicError(std::string const& msg);

[[noreturn]] void RaiseRpcError(grpc::Status const& status, char const* msg);
[[noreturn]] void RaiseRpcError(grpc::Status const& status,
                                std::string const& msg);
//@}

}  // namespace internal
}  // namespace BIGTABLE_CLIENT_NS
}  // namespace bigtable

#endif  // GOOGLE_CLOUD_CPP_BIGTABLE_CLIENT_INTERNAL_THROW_DELEGATE_H_
