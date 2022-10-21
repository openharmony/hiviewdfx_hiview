/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: bbox detector module test
 * Author     : liuwei
 * Create     : 2022-09-26
 * TestType   : FUNC
 * History    : NA
 */

#include "bbox_detector_module_test.h"

#include "bbox_detector_plugin.h"
#include "common_defines.h"
#include "event.h"
#include "log_util.h"
#include "sys_event.h"
#include "string_util.h"
#include "time_util.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
using namespace testing::ext;
void BBoxDetectorModuleTest::SetUpTestCase(void) {}

void BBoxDetectorModuleTest::TearDownTestCase(void) {}

void BBoxDetectorModuleTest::SetUp(void) {}

void BBoxDetectorModuleTest::TearDown(void) {}

/**
 * @tc.name: BBoxDetectorModuleTest001
 * @tc.desc: check whether fault is processed.
 *           1. check whether event is valid;
 *           2. check whether category and reason is ignored;
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(BBoxDetectorModuleTest, BBoxDetectorModuleTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct PANIC SysEvent
     * @tc.steps: step2. construct BBOXDetectorPlugin
     * @tc.steps: step3. OnEvent
     * @tc.steps: step4. check result
     */
    SysEventCreator sysEventCreator("KERNEL_VENDOR", "PANIC", SysEventCreator::FAULT);
    sysEventCreator.SetKeyValue("SUMMARY", "bootup_keypoint:97");
    sysEventCreator.SetKeyValue("name_", "PANIC");
    sysEventCreator.SetKeyValue("HAPPEN_TIME", "443990995");
    sysEventCreator.SetKeyValue("LOG_PATH", "/data/hisi_logs/");
    sysEventCreator.SetKeyValue("SUB_LOG_PATH", "19700106031950-00001111");
    sysEventCreator.SetKeyValue("MODULE", "AP");
    sysEventCreator.SetKeyValue("REASON", "AP_S_PANIC");
    auto sysEvent = make_shared<SysEvent>("test", nullptr, sysEventCreator);
    auto testPlugin = make_shared<BBoxDetectorPlugin>();
    shared_ptr<Event> event = dynamic_pointer_cast<Event>(sysEvent);
    testPlugin->OnLoad();
    testPlugin->OnEvent(event);
    ASSERT_EQ(sysEvent->GetEventValue("MODULE"), "AP");
    ASSERT_EQ(sysEvent->GetEventValue("REASON"), "AP_S_PANIC");
    ASSERT_EQ(sysEvent->GetEventValue("LOG_PATH"), "/data/hisi_logs/19700106031950-00001111");
}
}  // namespace HiviewDFX
}  // namespace OHOS
