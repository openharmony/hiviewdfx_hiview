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
#include <unistd.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
class MockHiviewPlatform : public HiviewContext {
public:
    MockHiviewPlatform() = default;
    ~MockHiviewPlatform() = default;

    std::string GetHiViewDirectory(DirectoryType type __UNUSED)
    {
        return "/data/log/hiview";
    }

    std::shared_ptr<EventLoop> GetSharedWorkLoop()
    {
        return nullptr;
    }
};

constexpr int8_t STATE_COUNT = 2;
constexpr bool DYNAMIC_TRACE_FSM[STATE_COUNT][STATE_COUNT][STATE_COUNT] = {
    {{true,  false}, {false, false}},
    {{false, false}, {false, false}},
};
constexpr bool CHECK_DYNAMIC_TRACE_FSM[STATE_COUNT][STATE_COUNT] = {
    {true, true}, {false, true}
};
constexpr useconds_t SET_PROPERTY_MICRO_SECONDS_DELAY = 200 * 1000;
constexpr useconds_t STATE_CHANGE_CALLBACK_MICRO_SECONDS_DELAY = 1500 * 1000;
std::shared_ptr g_unifiedCollector = std::make_shared<UnifiedCollector>();
bool g_originalTestAppTraceOn, g_originalUCollectionSwitchOn, g_originalTraceCollectionSwitchOn;
} // namespace

inline const std::string ConvertBoolToString(bool value)
{
    return value ? "true" : "false";
}

void TraceStateChangeTest::SetUpTestCase()
{
}

void TraceStateChangeTest::TearDownTestCase()
{
}

void TraceStateChangeTest::SetUp()
{
    ASSERT_NE(g_unifiedCollector, nullptr);
    g_originalTestAppTraceOn = Parameter::IsTestAppTraceOn();
    g_originalUCollectionSwitchOn = Parameter::IsUCollectionSwitchOn();
    g_originalTraceCollectionSwitchOn = Parameter::IsTraceCollectionSwitchOn();
}

void TraceStateChangeTest::TearDown()
{
    Parameter::SetProperty(HIVIEW_UCOLLECTION_TEST_APP_TRACE_STATE, ConvertBoolToString(g_originalTestAppTraceOn));
    usleep(SET_PROPERTY_NANO_SECONDS_DELAY);
    Parameter::SetProperty(HIVIEW_UCOLLECTION_STATE, ConvertBoolToString(g_originalUCollectionSwitchOn));
    usleep(SET_PROPERTY_NANO_SECONDS_DELAY);
    Parameter::SetProperty(DEVELOP_HIVIEW_TRACE_RECORDER, ConvertBoolToString(g_originalTraceCollectionSwitchOn));
    usleep(SET_PROPERTY_NANO_SECONDS_DELAY);
}

/**
 * @tc.name: TraceStateChangeTest001
 * @tc.desc: Test UnifiedCollector state initialization
 * @tc.type: FUNC
 * @tc.require: issue#I9S4U8
 */
HWTEST_F(TraceStateChangeTest, TraceStateChangeTest001, TestSize.Level3)
{
    constexpr std::size_t stateConstantCount = 5;
    constexpr uint64_t testCaseNumber = 1<<stateConstantCount;
    MockHiviewPlatform hiviewContext;
    for (uint64_t binaryExpression = 0; binaryExpression < testCaseNumber; binaryExpression++)
    {
        bool isBetaVersion = (binaryExpression & 1<<0) != 0;
        bool isDeveloperMode = (binaryExpression & 1<<1) != 0;
        bool isTestAppTraceOn = (binaryExpression & 1<<2) != 0;
        bool isUCollectionSwitchOn = (binaryExpression & 1<<3) != 0;
        bool isTraceCollectionSwitchOn = (binaryExpression & 1<<4) != 0;

        Parameter::SetDeveloperMode(isDeveloperMode);
        Parameter::SetBetaVersion(isBetaVersion);
        Parameter::SetProperty(HIVIEW_UCOLLECTION_TEST_APP_TRACE_STATE, ConvertBoolToString(isTestAppTraceOn));
        Parameter::SetProperty(HIVIEW_UCOLLECTION_STATE, ConvertBoolToString(isUCollectionSwitchOn));
        Parameter::SetProperty(DEVELOP_HIVIEW_TRACE_RECORDER, ConvertBoolToString(isTraceCollectionSwitchOn));
       
        hiviewContext = MockHiviewPlatform();
        g_unifiedCollector->SetHiviewContext(&hiviewContext);
        g_unifiedCollector->OnLoad();
        bool targetTraceState = CHECK_DYNAMIC_TRACE_FSM[isDeveloperMode][isTestAppTraceOn] &&
            DYNAMIC_TRACE_FSM[isBetaVersion][isUCollectionSwitchOn][isTraceCollectionSwitchOn];
        EXPECT_EQ(AppCallerEvent::enableDynamicTrace_, targetTraceState);
        g_unifiedCollector->OnUnload();
    }
}

/**
 * @tc.name: TraceStateChangeTest002
 * @tc.desc: Test UnifiedCollector state change
 * @tc.type: FUNC
 * @tc.require: issue#I9S4U8
 */
HWTEST_F(TraceStateChangeTest, TraceStateChangeTest002, TestSize.Level3)
{
    constexpr std::size_t stateConstantCount = 2;
    constexpr uint64_t constantTestCaseNumber = 1<<stateConstantCount;
    constexpr std::size_t stateVariableCount = 3;
    constexpr uint64_t variableTestCaseNumber = 1<<stateVariableCount;
    MockHiviewPlatform hiviewContext;
    for (uint64_t constantBinary = 0; constantBinary < constantTestCaseNumber; constantBinary++)
    {
        bool isBetaVersion = (constantBinary & 1<<0) != 0;
        bool isDeveloperMode = (constantBinary & 1<<1) != 0;

        Parameter::SetBetaVersion(isBetaVersion);
        Parameter::SetDeveloperMode(isDeveloperMode);

        hiviewContext = MockHiviewPlatform();
        g_unifiedCollector->SetHiviewContext(&hiviewContext);
        g_unifiedCollector->OnLoad();
        for (uint64_t variableBinary = 0; variableBinary < variableTestCaseNumber; variableBinary++)
        {
            bool isTestAppTraceOn = (variableBinary & 1<<0) != 0;
            bool isUCollectionSwitchOn = (variableBinary & 1<<1) != 0;
            bool isTraceCollectionSwitchOn = (variableBinary & 1<<2) != 0;
            
            Parameter::SetProperty(HIVIEW_UCOLLECTION_TEST_APP_TRACE_STATE, ConvertBoolToString(isTestAppTraceOn));
            usleep(SET_PROPERTY_NANO_SECONDS_DELAY);
            Parameter::SetProperty(HIVIEW_UCOLLECTION_STATE, ConvertBoolToString(isUCollectionSwitchOn));
            usleep(SET_PROPERTY_NANO_SECONDS_DELAY);
            Parameter::SetProperty(DEVELOP_HIVIEW_TRACE_RECORDER, ConvertBoolToString(isTraceCollectionSwitchOn));
                        
            usleep(STATE_CHANGE_CALLBACK_NANO_SECONDS_DELAY);
            bool targetTraceState = CHECK_DYNAMIC_TRACE_FSM[isDeveloperMode][isTestAppTraceOn] &&
                DYNAMIC_TRACE_FSM[isBetaVersion][isUCollectionSwitchOn][isTraceCollectionSwitchOn];
            EXPECT_EQ(AppCallerEvent::enableDynamicTrace_, targetTraceState);
        }
        g_unifiedCollector->OnUnload();
    }
}