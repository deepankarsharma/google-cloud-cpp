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

#ifndef GOOGLE_CLOUD_CPP_BIGTABLE_ADMIN_COLUMN_FAMILY_H_
#define GOOGLE_CLOUD_CPP_BIGTABLE_ADMIN_COLUMN_FAMILY_H_

#include "bigtable/client/version.h"

#include <chrono>
#include <memory>

#include <google/bigtable/admin/v2/bigtable_table_admin.pb.h>
#include <google/bigtable/admin/v2/table.pb.h>

#include "bigtable/client/internal/conjunction.h"

namespace bigtable {
inline namespace BIGTABLE_CLIENT_NS {
/**
 * Implement a thin wrapper around google::bigtable::admin::v2::GcRule.
 *
 * Provides functions to create GcRules in a convenient form.
 */
class GcRule {
 public:
  /// Create a garbage collection rule that keeps the last @p n versions.
  static GcRule MaxNumVersions(std::int32_t n) {
    GcRule tmp;
    tmp.gc_rule_.set_max_num_versions(n);
    return tmp;
  }

  /**
   * Return a garbage collection rule that deletes cells in a column older than
   * the given duration.
   *
   * The function accepts any instantiation of `std::chrono::duration<>` for the
   * @p duration parameter.  For example:
   *
   * @code
   * auto rule1 = bigtable::GcRule::MaxAge(std::chrono::hours(48));
   * auto rule2 = bigtable::GcRule::MaxAge(std::chrono::seconds(48 * 3600));
   * @endcode
   *
   * @tparam Rep a placeholder to match the Rep tparam for @p duration type, the
   *     semantics of this template parameter are documented in
   *     `std::chrono::duration<>` (in brief, the underlying arithmetic type
   *     used to store the number of ticks), for our purposes it is simply a
   *     formal parameter.
   * @tparam Period a placeholder to match the Period tparam for @p duration
   *     type, the semantics of this template parameter are documented in
   *     `std::chrono::duration<>` (in brief, the length of the tick in seconds,
   *     expressed as a `std::ratio<>`), for our purposes it is simply a formal
   *     parameter.
   *
   * @see
   * [std::chrono::duration<>](http://en.cppreference.com/w/cpp/chrono/duration)
   *     for more details.
   */
  template <typename Rep, typename Period>
  static GcRule MaxAge(std::chrono::duration<Rep, Period> duration) {
    GcRule tmp;
    auto& max_age = *tmp.gc_rule_.mutable_max_age();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    max_age.set_seconds(seconds.count());
    std::chrono::nanoseconds nanos =
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration) -
        seconds;
    max_age.set_nanos(static_cast<std::int32_t>(nanos.count()));
    return tmp;
  }

  /**
   * Return a GcRule that deletes cells if all the rules passed in would delete
   * the cells.
   *
   * @tparam GcRuleTypes the type of the GC rule arguments.  They must all be
   *     convertible to GcRule.
   * @param gc_rules the set of GC rules.
   */
  template <typename... GcRuleTypes>
  static GcRule Intersection(GcRuleTypes&&... gc_rules) {
    // This ugly thing provides a better compile-time error message than just
    // letting the compiler figure things out N levels deep as it recurses on
    // add_intersection().
    static_assert(
        internal::conjunction<
            std::is_convertible<GcRuleTypes, GcRule>...>::value,
        "The arguments to Intersection must be convertible to GcRule");
    GcRule tmp;
    auto& intersection = *tmp.gc_rule_.mutable_intersection();
    std::initializer_list<GcRule> list{std::forward<GcRuleTypes>(gc_rules)...};
    for (GcRule const& rule : list) {
      *intersection.add_rules() = rule.as_proto();
    }
    return tmp;
  }

  /**
   * Return a GcRule that deletes cells if any the rules passed in would delete
   * the cells.
   *
   * @tparam GcRuleTypes the type of the GC rule arguments.  They must all be
   *     convertible to GcRule.
   * @param gc_rules the set of GC rules.
   */
  template <typename... GcRuleTypes>
  static GcRule Union(GcRuleTypes&&... gc_rules) {
    // This ugly thing provides a better compile-time error message than just
    // letting the compiler figure things out N levels deep as it recurses on
    // add_intersection().
    static_assert(internal::conjunction<
                      std::is_convertible<GcRuleTypes, GcRule>...>::value,
                  "The arguments to Union must be convertible to GcRule");
    GcRule tmp;
    auto& union_ = *tmp.gc_rule_.mutable_union_();
    std::initializer_list<GcRule> list{std::forward<GcRuleTypes>(gc_rules)...};
    for (GcRule const& rule : list) {
      *union_.add_rules() = rule.as_proto();
    }
    return tmp;
  }

  /// Convert to the proto form.
  google::bigtable::admin::v2::GcRule as_proto() const { return gc_rule_; }

  /// Move the internal proto out.
  google::bigtable::admin::v2::GcRule as_proto_move() {
    return std::move(gc_rule_);
  }

  //@{
  /// @name Use default constructors and assignments.
  GcRule(GcRule&& rhs) noexcept = default;
  GcRule& operator=(GcRule&& rhs) noexcept = default;
  GcRule(GcRule const& rhs) = default;
  GcRule& operator=(GcRule const& rhs) = default;
  //@}

 private:
  GcRule() {}

 private:
  google::bigtable::admin::v2::GcRule gc_rule_;
};

class ColumnFamilyModification {
 public:
  /// Return a modification that creates a new column family.
  static ColumnFamilyModification Create(std::string id, GcRule gc) {
    ColumnFamilyModification tmp;
    tmp.mod_.set_id(std::move(id));
    *tmp.mod_.mutable_create()->mutable_gc_rule() = gc.as_proto_move();
    return tmp;
  }

  /// Return a modification that creates a new column family.
  static ColumnFamilyModification Update(std::string id, GcRule gc) {
    ColumnFamilyModification tmp;
    tmp.mod_.set_id(std::move(id));
    *tmp.mod_.mutable_update()->mutable_gc_rule() = gc.as_proto_move();
    return tmp;
  }

  /// Return a modification that drops the @p id column family.
  static ColumnFamilyModification Drop(std::string id) {
    ColumnFamilyModification tmp;
    tmp.mod_.set_id(std::move(id));
    tmp.mod_.set_drop(true);
    return tmp;
  }

  /// Convert to the proto form.
  ::google::bigtable::admin::v2::ModifyColumnFamiliesRequest::Modification
  as_proto() const {
    return mod_;
  }

  /// Move out the underlying proto contents.
  ::google::bigtable::admin::v2::ModifyColumnFamiliesRequest::Modification
  as_proto_move() {
    return std::move(mod_);
  }

  //@{
  /// @name Use default constructors and assignments.
  ColumnFamilyModification(ColumnFamilyModification&& rhs) noexcept = default;
  ColumnFamilyModification& operator=(ColumnFamilyModification&& rhs) noexcept =
      default;
  ColumnFamilyModification(ColumnFamilyModification const& rhs) = default;
  ColumnFamilyModification& operator=(ColumnFamilyModification const& rhs) =
      default;
  //@}

 private:
  ColumnFamilyModification() {}

 private:
  ::google::bigtable::admin::v2::ModifyColumnFamiliesRequest::Modification mod_;
};

}  // namespace BIGTABLE_CLIENT_NS
}  // namespace bigtable

#endif  // GOOGLE_CLOUD_CPP_BIGTABLE_ADMIN_COLUMN_FAMILY_H_
