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
#include <gtest/gtest.h>
#include <iostream>
#include <unistd.h>
#include "xcollecter.h"
#include "hitrace_dump.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
class XcollecterTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {}
    static void TearDownTestCase(void)
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

class TestCollectCallback : public CollectCallback {
public:
    virtual void Handle(std::shared_ptr<CollectItemResult> data, bool isFinish)
    {}
};

/**
 * @tc.name: XcollecterTest001
 * @tc.desc: Test Xcollecter, Service module HiTrace sequence diagram (obaining trace data in a fixed period)
 * @tc.type: FUNC
 */
HWTEST_F(XcollecterTest, XcollecterTest001, TestSize.Level3)
{
    std::shared_ptr<CollectParameter> collectParameter = std::make_shared<CollectParameter>("xperf");
    collectParameter->SetCollectItems({"/hitrace/client/dump"});
    Xcollecter xcollecter;

    sleep(10);

    std::shared_ptr<CollectTaskResult> result = xcollecter.SubmitTask(collectParameter);

    std::shared_ptr<CollectItemResult> item = nullptr;
    while ((item = result->Next()) != nullptr) {
        std::string traceFile;
        item->GetCollectItemValue("/hitrace/client/dump", traceFile);
        std::cout << "collect" << traceFile << std::endl;
    }
}

/**
 * @tc.name: XcollecterTest002
 * @tc.desc: Test Xcollecter
 * @tc.type: FUNC
 */
HWTEST_F(XcollecterTest, XcollecterTest002, TestSize.Level3)
{
    std::shared_ptr<CollectParameter> collectParameter = std::make_shared<CollectParameter>("xperf");
    collectParameter->SetCollectItems({"/hitrace/cmd/open"});
    Xcollecter xcollecter;
    sleep(10);
    std::shared_ptr<CollectTaskResult> result = xcollecter.SubmitTask(collectParameter);
    std::shared_ptr<CollectItemResult> item = nullptr;
    while ((item = result->Next()) != nullptr) {
        std::string traceFile;
        item->GetCollectItemValue("/hitrace/cmd/open", traceFile);
        std::cout << "collect" << traceFile << std::endl;
    }
}

/**
 * @tc.name: XcollecterTest003
 * @tc.desc: Test Xcollecter
 * @tc.type: FUNC
 */
HWTEST_F(XcollecterTest, XcollecterTest003, TestSize.Level3)
{
    std::shared_ptr<CollectParameter> collectParameter = std::make_shared<CollectParameter>("xperf");
    collectParameter->SetCollectItems({"/hitrace/cmd/traceon"});
    Xcollecter xcollecter;
    sleep(10);
    std::shared_ptr<CollectTaskResult> result = xcollecter.SubmitTask(collectParameter);
    std::shared_ptr<CollectItemResult> item = nullptr;
    while ((item = result->Next()) != nullptr) {
        std::string traceFile;
        item->GetCollectItemValue("/hitrace/cmd/traceon", traceFile);
        std::cout << "collect" << traceFile << std::endl;
    }
}

/**
 * @tc.name: XcollecterTest004
 * @tc.desc: Test Xcollecter
 * @tc.type: FUNC
 */
HWTEST_F(XcollecterTest, XcollecterTest004, TestSize.Level3)
{
    std::shared_ptr<CollectParameter> collectParameter = std::make_shared<CollectParameter>("xperf");
    collectParameter->SetCollectItems({"/hitrace/cmd/traceoff"});
    Xcollecter xcollecter;
    sleep(10);
    std::shared_ptr<CollectTaskResult> result = xcollecter.SubmitTask(collectParameter);
    std::shared_ptr<CollectItemResult> item = nullptr;
    while ((item = result->Next()) != nullptr) {
        std::string traceFile;
        item->GetCollectItemValue("/hitrace/cmd/traceoff", traceFile);
        std::cout << "collect" << traceFile << std::endl;
    }
}

/**
 * @tc.name: XcollecterTest005
 * @tc.desc: Test Xcollecter
 * @tc.type: FUNC
 */
HWTEST_F(XcollecterTest, XcollecterTest005, TestSize.Level3)
{
    std::shared_ptr<CollectParameter> collectParameter = std::make_shared<CollectParameter>("xperf");
    collectParameter->SetCollectItems({"/hitrace/cmd/close"});
    Xcollecter xcollecter;
    sleep(10);
    std::shared_ptr<CollectTaskResult> result = xcollecter.SubmitTask(collectParameter);
    std::shared_ptr<CollectItemResult> item = nullptr;
    while ((item = result->Next()) != nullptr) {
        std::string traceFile;
        item->GetCollectItemValue("/hitrace/cmd/close", traceFile);
        std::cout << "collect" << traceFile << std::endl;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
