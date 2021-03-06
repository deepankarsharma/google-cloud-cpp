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

#include "bigtable/client/internal/make_unique.h"
#include "bigtable/client/table.h"
#include "bigtable/client/testing/table_test_fixture.h"

/// Define helper types and functions for this test.
namespace {
class TableReadRowTest : public bigtable::testing::TableTestFixture {};
}  // anonymous namespace

TEST_F(TableReadRowTest, ReadRowSimple) {
  using namespace ::testing;
  namespace btproto = ::google::bigtable::v2;

  auto response = bigtable::testing::ReadRowsResponseFromString(R"(
      chunks {
        row_key: "r1"
        family_name { value: "fam" }
        qualifier { value: "col" }
        timestamp_micros: 42000
        value: "value"
        commit_row: true
      }
)");

  auto stream =
      bigtable::internal::make_unique<bigtable::testing::MockResponseStream>();
  EXPECT_CALL(*stream, Read(_))
      .WillOnce(Invoke([&response](btproto::ReadRowsResponse *r) {
        *r = response;
        return true;
      }))
      .WillOnce(Return(false));
  EXPECT_CALL(*stream, Finish()).WillOnce(Return(grpc::Status::OK));

  EXPECT_CALL(*bigtable_stub_, ReadRowsRaw(_, _))
      .WillOnce(Invoke([&stream, this](grpc::ClientContext *,
                                       btproto::ReadRowsRequest const &req) {
        EXPECT_EQ(1, req.rows().row_keys_size());
        EXPECT_EQ("r1", req.rows().row_keys(0));
        EXPECT_EQ(1, req.rows_limit());
        EXPECT_EQ(table_.table_name(), req.table_name());
        return stream.release();
      }));

  auto result = table_.ReadRow("r1", bigtable::Filter::PassAllFilter());
  EXPECT_TRUE(std::get<0>(result));
  auto row = std::get<1>(result);
  EXPECT_EQ("r1", row.row_key());
}

TEST_F(TableReadRowTest, ReadRowMissing) {
  using namespace ::testing;
  namespace btproto = ::google::bigtable::v2;

  auto stream =
      bigtable::internal::make_unique<bigtable::testing::MockResponseStream>();
  EXPECT_CALL(*stream, Read(_)).WillOnce(Return(false));
  EXPECT_CALL(*stream, Finish()).WillOnce(Return(grpc::Status::OK));

  EXPECT_CALL(*bigtable_stub_, ReadRowsRaw(_, _))
      .WillOnce(Invoke([&stream, this](grpc::ClientContext *,
                                       btproto::ReadRowsRequest const &req) {
        EXPECT_EQ(1, req.rows().row_keys_size());
        EXPECT_EQ("r1", req.rows().row_keys(0));
        EXPECT_EQ(1, req.rows_limit());
        EXPECT_EQ(table_.table_name(), req.table_name());
        return stream.release();
      }));

  auto result = table_.ReadRow("r1", bigtable::Filter::PassAllFilter());
  EXPECT_FALSE(std::get<0>(result));
}
