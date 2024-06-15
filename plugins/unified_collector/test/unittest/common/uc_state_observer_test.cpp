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

#include "uc_state_observer_test.h"

#include "uc_app_state_observer.h"
#include "uc_render_state_observer.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::HiviewDFX;

void UCStateObserverTest::SetUpTestCase() {}

void UCStateObserverTest::TearDownTestCase() {}

void UCStateObserverTest::SetUp() {}

void UCStateObserverTest::TearDown() {}

/**
 * @tc.name: UCStateObserverTest001
 * @tc.desc: test UcAppStateObserver callback
 * @tc.type: FUNC
 */
HWTEST_F(UCStateObserverTest, UCStateObserverTest001, TestSize.Level3)
{
    UcAppStateObserver appStateObserver;
    AppStateData appStateData;
    appStateData.pid = 100; // 100 : test pid
    appStateData.state = static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND);
    appStateObserver.OnForegroundApplicationChanged(appStateData);
    ASSERT_TRUE(true);
    appStateData.state = static_cast<int32_t>(ApplicationState::APP_STATE_BACKGROUND);
    appStateObserver.OnForegroundApplicationChanged(appStateData);
    ASSERT_TRUE(true);
    ProcessData processData;
    processData.pid = 100; // 100 : test pid
    appStateObserver.OnProcessCreated(processData);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: UCStateObserverTest002
 * @tc.desc: test UcRenderStateObserver callback
 * @tc.type: FUNC
 */
HWTEST_F(UCStateObserverTest, UCStateObserverTest002, TestSize.Level3)
{
    UcRenderStateObserver renderStateObserver;
    RenderStateData renderStateData;
    renderStateData.pid = 100; // 100 : test pid
    renderStateObserver.OnRenderStateChanged(renderStateData);
    ASSERT_TRUE(true);
}
