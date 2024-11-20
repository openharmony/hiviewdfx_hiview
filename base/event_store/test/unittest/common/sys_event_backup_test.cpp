/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <iostream>

#include "file_util.h"
#include "hiview_platform.h"
#include "sys_event_backup.h"
#include "sys_event_dao.h"
#include "sys_event_sequence_mgr.h"

namespace OHOS::HiviewDFX {
namespace EventStore {
using namespace testing::ext;
namespace {
const std::string BACKUP_DIR = "/data/test/backup/";
const std::string RESTORE_DIR = "/data/test/restore/";
}

class SysEventBackupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SysEventBackupTest::SetUpTestCase()
{
    OHOS::HiviewDFX::HiviewPlatform &platform = HiviewPlatform::GetInstance();
    std::string configDir("/data/test/test_data/hiview_platform_config");
    if (!platform.InitEnvironment(configDir)) {
        std::cout << "fail to init environment" << std::endl;
    } else {
        std::cout << "init environment successful" << std::endl;
    }
}

void SysEventBackupTest::TearDownTestCase()
{
}

void SysEventBackupTest::SetUp()
{
}

void SysEventBackupTest::TearDown()
{
}

/**
 * @tc.name: SysEventBackupTest001
 * @tc.desc: test for backup
 * @tc.type: FUNC
 * @tc.require: issueI9S20W
 */
HWTEST_F(SysEventBackupTest, SysEventBackupTest001, TestSize.Level1)
{
    SysEventBackup backup("");
    ASSERT_FALSE(backup.Backup());
}

/**
 * @tc.name: SysEventBackupTest002
 * @tc.desc: test for backup
 * @tc.type: FUNC
 * @tc.require: issueI9S20W
 */
HWTEST_F(SysEventBackupTest, SysEventBackupTest002, TestSize.Level1)
{
    SysEventBackup backup(BACKUP_DIR);

    // seq id file not exists
    ASSERT_FALSE(backup.Backup());

    // seq id file exists, but no event db files
    std::string databaseDir(SysEventDao::GetDatabaseDir());
    ASSERT_TRUE(!databaseDir.empty() && FileUtil::FileExists(databaseDir));
    FileUtil::CreateFile(databaseDir + SEQ_PERSISTS_FILE_NAME);
    ASSERT_FALSE(backup.Backup());
    FileUtil::CreateFile(databaseDir + SEQ_PERSISTS_BACKUP_FILE_NAME);
    ASSERT_FALSE(backup.Backup());

    FileUtil::ForceCreateDirectory(databaseDir + "RELIABILITY");
    FileUtil::CreateFile(databaseDir + "RELIABILITY/APP_FREEZE-1-CRITICAL-59179.db");
    FileUtil::SaveStringToFile(databaseDir + "RELIABILITY/CPP_CRASH-1-CRITICAL-39179.db", "test_content");
    ASSERT_TRUE(backup.Backup());
    ASSERT_TRUE(FileUtil::FileExists(BACKUP_DIR + "sysevent.zip"));
    ASSERT_FALSE(FileUtil::FileExists(BACKUP_DIR + "sysevent_tmp.zip"));
    ASSERT_FALSE(FileUtil::FileExists(BACKUP_DIR + "sysevent.zip.bak"));

    // seq id file exists, no need restore
    ASSERT_FALSE(backup.Restore(RESTORE_DIR));

    // backup file exists, but restore dir not exists
    FileUtil::RemoveFile(databaseDir + SEQ_PERSISTS_FILE_NAME);
    FileUtil::RemoveFile(databaseDir + SEQ_PERSISTS_BACKUP_FILE_NAME);
    ASSERT_FALSE(backup.Restore(RESTORE_DIR));

    // backup file and restore dir both exist
    FileUtil::ForceCreateDirectory(RESTORE_DIR);
    ASSERT_TRUE(backup.Restore(RESTORE_DIR));
    ASSERT_TRUE(FileUtil::FileExists(RESTORE_DIR + SEQ_PERSISTS_FILE_NAME));
    ASSERT_TRUE(FileUtil::FileExists(RESTORE_DIR + SEQ_PERSISTS_BACKUP_FILE_NAME));
}
}
}