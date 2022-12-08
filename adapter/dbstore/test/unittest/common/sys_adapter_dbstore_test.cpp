/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sys_adapter_dbstore_test.h"
#include <vector>

namespace OHOS {
namespace HiviewDFX {
void SysAdapterDbstoreTest::SetUpTestCase() {}

void SysAdapterDbstoreTest::TearDownTestCase() {}

void SysAdapterDbstoreTest::SetUp()
{
    storeMgr_ = std::make_unique<StoreManager>();
}

void SysAdapterDbstoreTest::TearDown() {}

/**
 * @tc.name: StoreManagerTest001
 * @tc.desc: test GetDocStore
 * @tc.type: FUNC
 * @tc.require: issues/I658Z7
 */
HWTEST_F(SysAdapterDbstoreTest, StoreManagerTest001, testing::ext::TestSize.Level3)
{
    Option option;
    option.db = "/data/test/test.db";
    option.flag = Option::NO_TRIM_ON_CLOSE;
    auto dbPtr = storeMgr_->GetDocStore(option);
    ASSERT_TRUE(dbPtr != nullptr);
    Entry entry;
    entry.id = 0;
    entry.value = "test1";
    ASSERT_TRUE(dbPtr->Put(entry)!= 0);
    Entry entry1;
    entry1.id = 0;
    entry1.value = R"~({"domain_":"demo","name_":"StoreManagerTest001","type_":1,"tz_":8,
        "time_":1620271291200,"pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20",
        "trace_flag_":4,"keyBool":1,"keyChar":97})~";
    ASSERT_TRUE(dbPtr->Put(entry1) == 0);
    std::vector<Entry> entrys;
    Entry entry2;
    entry2.id = 0;
    entry2.value = R"~({"domain_":"demo","name_":"StoreManagerTest001","type_":1,"tz_":8,
        "time_":1620271291200,"pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20",
        "trace_flag_":4,"keyBool":1,"keyChar":97})~";
    entrys.push_back(entry2);
    ASSERT_TRUE(dbPtr->PutBatch(entrys) == 0);
    std::cout << "num:" << dbPtr->GetNum() << std::endl;
    ASSERT_TRUE(dbPtr->GetNum() == 2);
    Option option2;
    option2.db = "/data/test/test.db";
    option2.flag = Option::RDONLY;
    auto dbPtr2 = storeMgr_->GetDocStore(option2);
    ASSERT_TRUE(dbPtr2 != nullptr);
    Option option3;
    option3.db = "/data/test/test.db";
    option3.flag = Option::TRUNC;
    auto dbPtr3 = storeMgr_->GetDocStore(option3);
    ASSERT_TRUE(dbPtr3 != nullptr);
    Option option4;
    option4.db = "/data/test/test1.db";
    Option option5;
    option5.db = "/data/test/test.db";
    std::string bakupfile = "/data/test/back_up_test.db";
    std::string bakupfile1 = "/data/test/back_up_test.db1";
    ASSERT_TRUE(storeMgr_->CloseDocStore(option4) != 0);
    ASSERT_TRUE(storeMgr_->OnlineBackupDocStore(option4, bakupfile1) != 0);
    storeMgr_->OnlineBackupDocStore(option5, bakupfile);
    ASSERT_TRUE(storeMgr_->DeleteDocStore(option4) != 0);
    ASSERT_TRUE(storeMgr_->DeleteDocStore(option5) == 0);
}
} // namespace HiviewDFX
} // namespace OHOS