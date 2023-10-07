/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <iostream>

#include "trace_collector.h"

#include <gtest/gtest.h>
#include <unistd.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectClient;
using namespace OHOS::HiviewDFX::UCollect;

namespace {
constexpr int SLEEP_DURATION = 10;
void Sleep()
{
    sleep(SLEEP_DURATION);
}
}

class TraceCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: TraceCollectorTest001
 * @tc.desc: use trace in snapshot mode.
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest001, TestSize.Level1)
{
    auto traceCollector = TraceCollector::Create();
    ASSERT_TRUE(traceCollector != nullptr);
    const std::vector<std::string> tagGroups = {"scene_performance"};
    auto openRet = traceCollector->OpenSnapshot(tagGroups);
    if (openRet.retCode == UcError::SUCCESS) {
        Sleep();
        auto dumpRes = traceCollector->DumpSnapshot();
        ASSERT_TRUE(dumpRes.data.size() >= 0);
        auto closeRet = traceCollector->Close();
        ASSERT_TRUE(closeRet.retCode == UcError::SUCCESS);
    }
}

/**
 * @tc.name: TraceCollectorTest002
 * @tc.desc: use trace in recording mode.
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest002, TestSize.Level1)
{
    auto traceCollector = TraceCollector::Create();
    ASSERT_TRUE(traceCollector != nullptr);
    std::string args = "tags:sched clockType:boot bufferSize:1024 overwrite:1";
    auto openRet = traceCollector->OpenRecording(args);
    if (openRet.retCode == UcError::SUCCESS) {
        auto recOnRet = traceCollector->RecordingOn();
        ASSERT_TRUE(recOnRet.retCode == UcError::SUCCESS);
        Sleep();
        auto recOffRet = traceCollector->RecordingOff();
        ASSERT_TRUE(recOffRet.data.size() >= 0);
    }
}
