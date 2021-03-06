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

#include "bigtable/client/testing/table_integration_test.h"
#include <google/protobuf/text_format.h>
#include "bigtable/client/internal/make_unique.h"

namespace bigtable {
namespace testing {

std::string TableTestEnvironment::project_id_;
std::string TableTestEnvironment::instance_id_;

void TableIntegrationTest::SetUp() {
  admin_client_ = bigtable::CreateDefaultAdminClient(
      ::bigtable::testing::TableTestEnvironment::project_id(),
      bigtable::ClientOptions());
  table_admin_ = bigtable::internal::make_unique<bigtable::TableAdmin>(
      admin_client_, ::bigtable::testing::TableTestEnvironment::instance_id());
  data_client_ = bigtable::CreateDefaultDataClient(
      ::bigtable::testing::TableTestEnvironment::project_id(),
      ::bigtable::testing::TableTestEnvironment::instance_id(),
      bigtable::ClientOptions());
}

std::unique_ptr<bigtable::Table> TableIntegrationTest::CreateTable(
    std::string const& table_name, bigtable::TableConfig& table_config) {
  table_admin_->CreateTable(table_name, table_config);
  return bigtable::internal::make_unique<bigtable::Table>(data_client_,
                                                          table_name);
}

void TableIntegrationTest::DeleteTable(std::string const& table_name) {
  table_admin_->DeleteTable(table_name);
}

std::vector<bigtable::Cell> TableIntegrationTest::ReadRows(
    bigtable::Table& table, bigtable::Filter filter) {
  auto reader = table.ReadRows(
      bigtable::RowSet(bigtable::RowRange::InfiniteRange()), std::move(filter));
  std::vector<bigtable::Cell> result;
  for (auto const& row : reader) {
    std::copy(row.cells().begin(), row.cells().end(),
              std::back_inserter(result));
  }
  return result;
}

/// A helper function to create a list of cells.
void TableIntegrationTest::CreateCells(
    bigtable::Table& table, std::vector<bigtable::Cell> const& cells) {
  std::map<std::string, bigtable::SingleRowMutation> mutations;
  for (auto const& cell : cells) {
    std::string key = cell.row_key();
    auto inserted = mutations.emplace(key, bigtable::SingleRowMutation(key));
    inserted.first->second.emplace_back(
        bigtable::SetCell(cell.family_name(), cell.column_qualifier(),
                          cell.timestamp(), cell.value()));
  }
  bigtable::BulkMutation bulk;
  for (auto& kv : mutations) {
    bulk.emplace_back(std::move(kv.second));
  }
  table.BulkApply(std::move(bulk));
}

void TableIntegrationTest::CheckEqualUnordered(
    std::vector<bigtable::Cell> expected, std::vector<bigtable::Cell> actual) {
  std::sort(expected.begin(), expected.end());
  std::sort(actual.begin(), actual.end());
  EXPECT_THAT(actual, ::testing::ContainerEq(expected));
}

}  // namespace testing

int CellCompare(bigtable::Cell const& lhs, bigtable::Cell const& rhs) {
  auto compare_row_key = lhs.row_key().compare(rhs.row_key());
  if (compare_row_key != 0) {
    return compare_row_key;
  }
  auto compare_family_name = lhs.family_name().compare(rhs.family_name());
  if (compare_family_name != 0) {
    return compare_family_name;
  }
  auto compare_column_qualifier =
      lhs.column_qualifier().compare(rhs.column_qualifier());
  if (compare_column_qualifier != 0) {
    return compare_column_qualifier;
  }
  if (lhs.timestamp() < rhs.timestamp()) {
    return -1;
  }
  if (lhs.timestamp() > rhs.timestamp()) {
    return 1;
  }
  auto compare_value = lhs.value().compare(rhs.value());
  if (compare_value != 0) {
    return compare_value;
  }
  if (lhs.labels() < rhs.labels()) {
    return -1;
  }
  if (lhs.labels() == rhs.labels()) {
    return 0;
  }
  return 1;
}

//@{
/// @name Helpers for GTest.
bool operator==(Cell const& lhs, Cell const& rhs) {
  return CellCompare(lhs, rhs) == 0;
}

bool operator<(Cell const& lhs, Cell const& rhs) {
  return CellCompare(lhs, rhs) < 0;
}

/**
 * This function is not used in this file, but it is used by GoogleTest; without
 * it, failing tests will output binary blobs instead of human-readable text.
 */
void PrintTo(bigtable::Cell const& cell, std::ostream* os) {
  *os << "  row_key=" << cell.row_key() << ", family=" << cell.family_name()
      << ", column=" << cell.column_qualifier()
      << ", timestamp=" << cell.timestamp() << ", value=" << cell.value()
      << ", labels={";
  char const* del = "";
  for (auto const& label : cell.labels()) {
    *os << del << label;
    del = ",";
  }
  *os << "}";
}
//@}

}  // namespace bigtable
