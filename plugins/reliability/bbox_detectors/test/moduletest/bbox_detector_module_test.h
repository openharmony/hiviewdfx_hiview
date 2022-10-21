/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: BBoxDetector module test
 * Author     : liuwei
 * Create     : 2022-09-26
 * TestType   : FUNC
 * History    : NA
 */
#ifndef BBOX_DETECTOR_MODULE_TEST
#define BBOX_DETECTOR_MODULE_TEST

#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace HiviewDFX {

class BBoxDetectorModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // BBOX_DETECTOR_MODULE_TEST
