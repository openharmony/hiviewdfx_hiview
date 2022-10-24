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

#include "bbox_detector_unit_test.h"

#include "bbox_detector_plugin.h"
#include "sys_event.h"
#include "smart_parser.h"
#include "tbox.h"
using namespace std;

namespace OHOS {
namespace HiviewDFX {
using namespace testing::ext;
void BBoxDetectorUnitTest::SetUpTestCase(void) {}

void BBoxDetectorUnitTest::TearDownTestCase(void) {}

void BBoxDetectorUnitTest::SetUp(void) {}

void BBoxDetectorUnitTest::TearDown(void) {}

/**
 * @tc.name: BBoxDetectorUnitTest001
 * @tc.desc: check bbox config parser whether it is passed.
 *           1.parse bbox config;
 *           2.check result;
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(BBoxDetectorUnitTest, BBoxDetectorUnitTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init bboxDetector and parse hisysevent.
     */

    /**
     * @tc.steps: step2. check func.
     * @tc.expect: get right result for checking
     */
    
    SysEventCreator sysEventCreator("KERNEL_VENDOR", "PANIC", SysEventCreator::FAULT);
    auto sysEvent = make_shared<SysEvent>("test", nullptr, sysEventCreator);
    auto testPlugin = make_shared<BBoxDetectorPlugin>();
    shared_ptr<Event> event = dynamic_pointer_cast<Event>(sysEvent);
    ASSERT_EQ(testPlugin->CanProcessEvent(event), true);
}

/**
 * @tc.name: BBoxDetectorUnitTest002
 * @tc.desc: check whether fault is processed,and check whether fault file is pasered
 *           1. check whether fault file is pasered;
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(BBoxDetectorUnitTest, BBoxDetectorUnitTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct panic file path
     * @tc.steps: step2. Analysis panic file
     * @tc.steps: step3. check result
     */
    std::string TEST_CONFIG = "/data/test/test_data/BBoxDetector/common/";
    std::string dynamicPaths = "/data/test/test_data/BBoxDetector/common/19700106031950-00001111";
    auto eventInfos = SmartParser::Analysis(dynamicPaths, TEST_CONFIG, "PANIC");
    Tbox::FilterTrace(eventInfos);

    ASSERT_EQ(eventInfos["F1NAME"], "sysrq_handle_term+0x0/0x94");
    ASSERT_EQ((eventInfos["F2NAME"], "__handle_sysrq+0x15c/0x184");
    ASSERT_EQ((eventInfos["F3NAME"], "el0_sync+0x180/0x1c0");
}
}  // namespace HiviewDFX
}  // namespace OHOS
