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

#include "trace_state_change_test.h"
#include "unified_collector.h"
#include "parameter_ex.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr int8_t STATE_COUNT = 2;
constexpr bool DYNAMIC_TRACE_FSM[STATE_COUNT][STATE_COUNT][STATE_COUNT] = {
    {{true,  false}, {false, false}},
    {{false, false}, {false, false}},
};
const bool CHECK_DYNAMIC_TRACE_FSM[STATE_COUNT][STATE_COUNT] = {
    {true, true}, {false, true}
};
}

void TraceStateChangeTest::SetUpTestCase()
{
}

void TraceStateChangeTest::TearDownTestCase()
{
}

void TraceStateChangeTest::SetUp()
{    
}

void TraceStateChangeTest::TearDown()
{
}

inline const std::string ConvertBoolToString(bool value)
{
    return value ? "true" : "false";
}

/**
 * @tc.name: TraceStateChangeTest001
 * @tc.desc: Test UnifiedCollector state change
 * @tc.type: FUNC
 * @tc.require: issueI5NULM
 */
HWTEST_F(TraceStateChangeTest, TraceStateChangeTest001, TestSize.Level3)
{
    std::shared_ptr unifiedCollector = std::make_shared<UnifiedCollector>();
    ASSERT_NE(unifiedCollector, nullptr);

    unifiedCollector->OnLoad();

    // bool isDeveloperMode = false;
    // bool isTestAppTraceOn = false;
    // bool isBetaVersion = false;
    // bool isUCollectionSwitchOn = false;
    // bool isTraceCollectionSwitchOn = false;
    bool isDeveloperMode, isTestAppTraceOn, isBetaVersion, isUCollectionSwitchOn, isTraceCollectionSwitchOn;
    constexpr std::size_t stateVariableCount = 5;    
    constexpr uint64_t testCaseNumber = 1<<stateVariableCount;
    for(uint64_t binaryExpression = 0; binaryExpression < testCaseNumber; binaryExpression++)
    {
        isDeveloperMode = (binaryExpression & 1<<0) != 0;
        isTestAppTraceOn = (binaryExpression & 1<<1) != 0;
        isBetaVersion = (binaryExpression & 1<<2) != 0;
        isUCollectionSwitchOn = (binaryExpression & 1<<3) != 0;
        isTraceCollectionSwitchOn = (binaryExpression & 1<<4) != 0;
        
        Parameter::SetDeveloperMode(isDeveloperMode);
        Parameter::SetBetaVersion(isBetaVersion);

        Parameter::SetProperty(HIVIEW_UCOLLECTION_TEST_APP_TRACE_STATE, ConvertBoolToString(isTestAppTraceOn));
        Parameter::SetProperty(HIVIEW_UCOLLECTION_STATE, ConvertBoolToString(isUCollectionSwitchOn));
        Parameter::SetProperty(DEVELOP_HIVIEW_TRACE_RECORDER, ConvertBoolToString(isTraceCollectionSwitchOn));

        bool targetTraceState = CHECK_DYNAMIC_TRACE_FSM[isDeveloperMode][isTestAppTraceOn] && 
            DYNAMIC_TRACE_FSM[isBetaVersion][isUCollectionSwitchOn][isTraceCollectionSwitchOn];
        ASSERT_EQ(AppCallerEvent::enableDynamicTrace_, targetTraceState);
    }    

    unifiedCollector->OnUnload();
}

