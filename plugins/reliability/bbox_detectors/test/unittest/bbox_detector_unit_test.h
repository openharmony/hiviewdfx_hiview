/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: BBoxDetector unit test
 * Author     : wangyanteng
 * Create     : 2022-10-21
 * TestType   : FUNC
 * History    : NA
 */
#ifndef BBOX_DETECTOR_UNIT_TEST
#define BBOX_DETECTOR_UNIT_TEST

#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace HiviewDFX {
static const std::string TEST_CONFIG = "/data/test/test_data/BBoxDetector/common/";

class BBoxDetectorUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // BBOX_DETECTOR_UNIT_TEST
