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

#include "fold_app_usage_test.h"

#include "file_util.h"
#include "fold_app_usage_db_helper.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_store.h"
#include "sql_util.h"
#include "time_util.h"
#include "usage_event_common.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string LOG_DB_TABLE_NAME = "app_events";

int64_t g_today0Time = 0;
int64_t g_dayGapTime = 0;
int64_t g_hourGapTime = 0;
int64_t g_startTime = 0;
int64_t g_endTime = 0;
int g_screenStat = 0;
}

void FoldAppUsageTest::SetUpTestCase(void) {}

void FoldAppUsageTest::TearDownTestCase(void) {}

void FoldAppUsageTest::SetUp(void)
{
    if (!FileUtil::FileExists("/data/test/sys_event_logger/") &&
        !FileUtil::ForceCreateDirectory("/data/test/sys_event_logger/")) {
        return;
    }

    g_today0Time = TimeUtil::Get0ClockStampMs();
    g_dayGapTime = static_cast<int64_t>(TimeUtil::MILLISECS_PER_DAY);
    g_hourGapTime = static_cast<int64_t>(TimeUtil::SECONDS_PER_HOUR * TimeUtil::SEC_TO_MILLISEC);
    g_startTime = g_today0Time - g_dayGapTime;
    g_endTime = g_today0Time - 1;
}

void FoldAppUsageTest::TearDown(void) {}

/**
 * @tc.name: FoldAppUsageTest001
 * @tc.desc: check fold app usage func get 1104 data from db.
 * @tc.type: FUNC
 */
HWTEST_F(FoldAppUsageTest, FoldAppUsageTest001, TestSize.Level1)
{
    FoldAppUsageDbHelper dbHelper("/data/test/");
    AppEventRecord record1{1104, 1000, "app1", 11, 11, "55", g_today0Time - 5 * g_dayGapTime, 1, 2, 3, 4};

    AppEventRecord record2{1104, 1000, "app1", 11, 11, "55", g_startTime - 5 * g_hourGapTime, 1, 2, 3, 4};

    AppEventRecord record3{1104, 1000, "app1", 11, 11, "55", g_startTime + 7 * g_hourGapTime, 1, 2, 3, 4};

    AppEventRecord record4{1104, 1000, "app2", 11, 11, "55", g_startTime + 8 * g_hourGapTime, 2, 2, 2, 2};

    AppEventRecord record5{1104, 1000, "app2", 11, 11, "55", g_startTime + 9 * g_hourGapTime, 3, 3, 3, 3};

    EXPECT_EQ(dbHelper.AddAppEvent(record1), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record2), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record3), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record4), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record5), 0);

    std::unordered_map<std::string, FoldAppUsageInfo> all1104Infos;
    dbHelper.QueryStatisticEventsInPeriod(g_startTime, g_endTime, all1104Infos);
    EXPECT_EQ(all1104Infos.size(), 2);
    EXPECT_EQ(all1104Infos["app155"].package, "app1");
    EXPECT_EQ(all1104Infos["app155"].version, "55");
    EXPECT_EQ(all1104Infos["app155"].foldVer, 1);
    EXPECT_EQ(all1104Infos["app155"].foldHor, 2);
    EXPECT_EQ(all1104Infos["app155"].expdVer, 3);
    EXPECT_EQ(all1104Infos["app155"].expdHor, 4);
    EXPECT_EQ(all1104Infos["app255"].package, "app2");
    EXPECT_EQ(all1104Infos["app255"].version, "55");
    EXPECT_EQ(all1104Infos["app255"].foldVer, 5);
    EXPECT_EQ(all1104Infos["app255"].foldHor, 5);
    EXPECT_EQ(all1104Infos["app255"].expdVer, 5);
    EXPECT_EQ(all1104Infos["app255"].expdHor, 5);
}

/**
 * @tc.name: FoldAppUsageTest002
 * @tc.desc: check fold app usage func get 1103, 1101 data from db.
 * @tc.type: FUNC
 */
HWTEST_F(FoldAppUsageTest, FoldAppUsageTest002, TestSize.Level1)
{
    FoldAppUsageDbHelper dbHelper("/data/test/");
    AppEventRecord record6{1101, 4000, "app3", 11, 12, "55", g_startTime + 10 * g_hourGapTime, 0, 0, 0, 0};

    AppEventRecord record7{1103, 5000, "app3", 12, 22, "55", g_startTime + 11 * g_hourGapTime, 1, 2, 3, 4};

    AppEventRecord record8{1103, 6000, "app3", 22, 21, "55", g_startTime + 12 * g_hourGapTime, 0, 0, 0, 0};

    AppEventRecord record9{1103, 7000, "app4", 11, 21, "55", g_startTime + 15 * g_hourGapTime, 0, 0, 0, 0};

    AppEventRecord record10{1103, 8000, "app4", 21, 22, "55", g_startTime + 16 * g_hourGapTime, 0, 0, 0, 0};

    AppEventRecord record11{1102, 9000, "app4", 22, 12, "55", g_today0Time + 2 * g_hourGapTime, 0, 0, 0, 0};

    EXPECT_EQ(dbHelper.AddAppEvent(record6), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record7), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record8), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record9), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record10), 0);
    EXPECT_EQ(dbHelper.AddAppEvent(record11), 0);

    FoldAppUsageInfo app3Info;
    app3Info.package = "app3";
    dbHelper.QueryForegroundAppsInfo(g_startTime, g_endTime, g_screenStat, app3Info);
    EXPECT_EQ(app3Info.version, "");
    EXPECT_EQ(app3Info.foldVer, 43199999); // from 12:00:00 to 23:59:59
    EXPECT_EQ(app3Info.foldHor, 1000);     // from ts, 5000 to 6000
    EXPECT_EQ(app3Info.expdVer, 0);
    EXPECT_EQ(app3Info.expdHor, 1000);     // from ts, 4000 to 5000
    FoldAppUsageInfo app4Info;
    app4Info.package = "app4";
    dbHelper.QueryForegroundAppsInfo(g_startTime, g_endTime, g_screenStat, app4Info);
    EXPECT_EQ(app4Info.version, "");
    EXPECT_EQ(app4Info.foldVer, 1000);     // from ts, 7000 to 8000
    EXPECT_EQ(app4Info.foldHor, 28799999); // from 16:00:00 to 23:59:59
    EXPECT_EQ(app4Info.expdVer, 54000000); // from 00:00:00 to 15:00:00
    EXPECT_EQ(app4Info.expdHor, 0);
}

/**
 * @tc.name: FoldAppUsageTest003
 * @tc.desc: check fold app usage func remove data and get creenState from db.
 * @tc.type: FUNC
 */
HWTEST_F(FoldAppUsageTest, FoldAppUsageTest003, TestSize.Level1)
{
    FoldAppUsageDbHelper dbHelper("/data/test/");
    g_screenStat = dbHelper.QueryFinalScreenStatus(g_endTime);
    EXPECT_EQ(g_screenStat, 22);

    int deleteEventNum = dbHelper.DeleteEventsByTime(g_startTime - 2 * g_dayGapTime);
    EXPECT_EQ(deleteEventNum, 1);

    FileUtil::ForceRemoveDirectory("/data/test/sys_event_logger/", true);
}
} // namespace HiviewDFX
} // namespace OHOS
