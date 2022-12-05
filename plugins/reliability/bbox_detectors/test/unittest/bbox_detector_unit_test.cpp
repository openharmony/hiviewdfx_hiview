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
    EXPECT_EQ(testPlugin->CanProcessEvent(event), true);
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
     * @tc.steps: step2. Analysis panic event
     * @tc.steps: step3. check result
     */
    string stack = R"("dump_backtrace+0x0/0x184"
                     "show_stack+0x2c/0x3c",
                     "dump_stack+0xc0/0x11c",
                     "panic+0x1cc/0x3dc",
                     "sysrq_handle_term+0x0/0x94",
                     "__handle_sysrq+0x15c/0x184",
                     "write_sysrq_trigger+0xb0/0xf4",
                     "proc_reg_write+0x94/0x120",
                     "vfs_write+0x184/0x380",
                     "ksys_write+0x74/0xc8",
                     "__arm64_sys_write+0x24/0x34",
                     "el0_svc_common+0x104/0x180",
                     "do_el0_svc+0x2c/0x3c",
                     "el0_svc+0x10/0x1c",
                     "vks_write+0x123/0xa6",
                     "el0_sync+0x180/0x1c0"
                    )";

    std::map<std::string, std::string> eventInfos;
    eventInfos.insert(std::pair("END_STACK", stack));
    eventInfos.insert(std::pair("PNAME", "PANIC"));
    eventInfos.insert(std::pair("Eventid", "901000002"));
    Tbox::FilterTrace(eventInfos);

    EXPECT_STREQ(eventInfos["FIRST_FRAME"].c_str(), "vfs_write+0x184/0x380");
    EXPECT_STREQ(eventInfos["SECOND_FRAME"].c_str(), "ksys_write+0x74/0xc8");
}
}  // namespace HiviewDFX
}  // namespace OHOS
