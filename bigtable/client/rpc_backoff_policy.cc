// Copyright 2017 Google Inc.
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

#include "bigtable/client/rpc_backoff_policy.h"

namespace {
// Define the defaults using a pre-processor macro, this allows the application
// developers to change the defaults for their application by compiling with
// different values.
#ifndef BIGTABLE_CLIENT_DEFAULT_INITIAL_DELAY
#define BIGTABLE_CLIENT_DEFAULT_INITIAL_DELAY std::chrono::milliseconds(10)
#endif  // BIGTABLE_CLIENT_DEFAULT_INITIAL_DELAY

#ifndef BIGTABLE_CLIENT_DEFAULT_MAXIMUM_DELAY
#define BIGTABLE_CLIENT_DEFAULT_MAXIMUM_DELAY std::chrono::minutes(5)
#endif  // BIGTABLE_CLIENT_DEFAULT_MAXIMUM_DELAY

const auto default_initial_delay = BIGTABLE_CLIENT_DEFAULT_INITIAL_DELAY;
const auto default_maximum_delay = BIGTABLE_CLIENT_DEFAULT_MAXIMUM_DELAY;
}  // anonymous namespace

namespace bigtable {
inline namespace BIGTABLE_CLIENT_NS {
std::unique_ptr<RPCBackoffPolicy> DefaultRPCBackoffPolicy() {
  return std::unique_ptr<RPCBackoffPolicy>(new ExponentialBackoffPolicy(
      default_initial_delay, default_maximum_delay));
}

std::unique_ptr<RPCBackoffPolicy> ExponentialBackoffPolicy::clone() const {
  return std::unique_ptr<RPCBackoffPolicy>(new ExponentialBackoffPolicy(*this));
}

void ExponentialBackoffPolicy::setup(grpc::ClientContext& /*unused*/) const {}

std::chrono::milliseconds ExponentialBackoffPolicy::on_completion(
    grpc::Status const& status) {
  using namespace std::chrono;
  std::uniform_int_distribution<int> rng_distribution(
      current_delay_range_.count() / 2, current_delay_range_.count());
  // Randomized sleep period because it is possible that after some time all
  // client have same sleep period if we use only exponential backoff policy.
  auto delay = microseconds(rng_distribution(generator_));
  current_delay_range_ *= 2;
  if (current_delay_range_ >= maximum_delay_) {
    current_delay_range_ = maximum_delay_;
  }
  return duration_cast<milliseconds>(delay);
}

}  // namespace BIGTABLE_CLIENT_NS
}  // namespace bigtable
