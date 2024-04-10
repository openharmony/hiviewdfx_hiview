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

#include "sys_event_store_utility_test.h"

#include <gmock/gmock.h>

#include "base_def.h"
#include "content_reader.h"
#include "content_reader_factory.h"
#include "content_reader_version_1.h"
#include "content_reader_version_2.h"
#include "content_reader_version_3.h"
#include "logger.h"
#include "sys_event_doc_reader.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("ContentReaderVersionTest");
namespace {
constexpr int8_t TEST_READER_VERSION = 100;
constexpr uint32_t TEST_EVENT_SIZE = 200;
class ContentReaderVersionTest : public ContentReader {
public:
    int ReadDocDetails(std::ifstream& docStream, EventStore::DocHeader& header,
        uint64_t& docHeaderSize, std::string& sysVersion) override
    {
        if (!docStream.is_open()) {
            return DOC_STORE_ERROR_IO;
        }
        docHeaderSize = 0;
        sysVersion = "";
        return DOC_STORE_SUCCESS;
    }

    bool IsValidMagicNum(const uint64_t magicNum) override
    {
        HIVIEW_LOGD("magicNum is %{public}" PRIu64 ".", magicNum);
        return true;
    }

protected:
    int GetContentHeader(uint8_t* content, EventStore::ContentHeader& header) override
    {
        if (content == nullptr) {
            return DOC_STORE_ERROR_INVALID;
        }
        HIVIEW_LOGD("seq is %{public}" PRId64 ".", header.seq);
        return DOC_STORE_SUCCESS;
    }

    size_t GetContentHeaderSize()
    {
        return sizeof(EventStore::ContentHeader);
    }
};
}

void SysEventStoreUtilityTest::SetUpTestCase()
{
}

void SysEventStoreUtilityTest::TearDownTestCase()
{
}

void SysEventStoreUtilityTest::SetUp()
{
}

void SysEventStoreUtilityTest::TearDown()
{
}

/**
 * @tc.name: SysEventStoreUtilityTest001
 * @tc.desc: ContentReaderFactory test
 * @tc.type: FUNC
 * @tc.require: issueI9DJP3
 */
HWTEST_F(SysEventStoreUtilityTest, SysEventStoreUtilityTest001, testing::ext::TestSize.Level3)
{
    std::shared_ptr<ContentReader> reader = std::make_shared<ContentReaderVersionTest>();
    ContentReaderFactory::GetInstance().Register(TEST_READER_VERSION, reader);
    auto regReader = ContentReaderFactory::GetInstance().Get(TEST_READER_VERSION);
    ASSERT_EQ(reader, regReader);
    auto invalidReader = ContentReaderFactory::GetInstance().Get(EventStore::EVENT_DATA_FORMATE_VERSION::INVALID);
    ASSERT_EQ(invalidReader, nullptr);
}

/**
 * @tc.name: SysEventStoreUtilityTest002
 * @tc.desc: ContentReader test
 * @tc.type: FUNC
 * @tc.require: issueI9DJP3
 */
HWTEST_F(SysEventStoreUtilityTest, SysEventStoreUtilityTest002, testing::ext::TestSize.Level3)
{
    std::ifstream dbFileStream;
    dbFileStream.open("/data/test/test_data/version1_db_file", std::ios::binary);
    ASSERT_TRUE(dbFileStream.is_open());
    auto version = ContentReader::ReadFmtVersion(dbFileStream);
    ASSERT_EQ(version, EventStore::EVENT_DATA_FORMATE_VERSION::VERSION1);
    std::shared_ptr<ContentReader> reader = ContentReaderFactory::GetInstance().Get(version);
    ASSERT_NE(reader, nullptr);
    ASSERT_TRUE(reader->IsValidMagicNum(MAGIC_NUM_VERSION1));
    EventStore::DocHeader header;
    uint64_t docHeaderSize;
    std::string sysVersion;
    reader->ReadDocDetails(dbFileStream, header, docHeaderSize, sysVersion);
    ASSERT_EQ(header.version, EventStore::EVENT_DATA_FORMATE_VERSION::VERSION1);
    ASSERT_EQ(sysVersion.size(), 0);
    dbFileStream.close();
    dbFileStream.open("/data/test/test_data/version2_db_file", std::ios::binary);
    ASSERT_TRUE(dbFileStream.is_open());
    version = ContentReader::ReadFmtVersion(dbFileStream);
    ASSERT_EQ(version, EventStore::EVENT_DATA_FORMATE_VERSION::VERSION2);
    reader = ContentReaderFactory::GetInstance().Get(version);
    ASSERT_NE(reader, nullptr);
    ASSERT_TRUE(reader->IsValidMagicNum(MAGIC_NUM_VERSION2));
    reader->ReadDocDetails(dbFileStream, header, docHeaderSize, sysVersion);
    ASSERT_EQ(header.version, EventStore::EVENT_DATA_FORMATE_VERSION::VERSION2);
    ASSERT_EQ(sysVersion.size(), 0);
    dbFileStream.close();
}

/**
 * @tc.name: SysEventStoreUtilityTest003
 * @tc.desc: ContentReader test
 * @tc.type: FUNC
 * @tc.require: issueI9DJP3
 */
HWTEST_F(SysEventStoreUtilityTest, SysEventStoreUtilityTest003, testing::ext::TestSize.Level3)
{
    std::ifstream dbFileStream;
    dbFileStream.open("/data/test/test_data/version3_db_file", std::ios::binary);
    auto version = ContentReader::ReadFmtVersion(dbFileStream);
    ASSERT_EQ(version, EventStore::EVENT_DATA_FORMATE_VERSION::VERSION3);
    auto reader = ContentReaderFactory::GetInstance().Get(version);
    ASSERT_NE(reader, nullptr);
    ASSERT_TRUE(reader->IsValidMagicNum(MAGIC_NUM_VERSION3));
    EventStore::DocHeader header;
    uint64_t docHeaderSize = 0;
    std::string sysVersion;
    reader->ReadDocDetails(dbFileStream, header, docHeaderSize, sysVersion);
    ASSERT_EQ(header.version, EventStore::EVENT_DATA_FORMATE_VERSION::VERSION3);
    ASSERT_GT(sysVersion.size(), 0);
    uint8_t content[TEST_EVENT_SIZE] = {0};
    dbFileStream.seekg(docHeaderSize, std::ios::beg);
    dbFileStream.read(reinterpret_cast<char*>(&content), TEST_EVENT_SIZE);
    uint32_t contentSize = TEST_EVENT_SIZE;
    EventInfo info;
    auto ret = reader->ReadRawData(info, content, contentSize);
    ASSERT_NE(ret, nullptr);
    dbFileStream.close();
}
} // namespace HiviewDFX
} // namespace OHOS