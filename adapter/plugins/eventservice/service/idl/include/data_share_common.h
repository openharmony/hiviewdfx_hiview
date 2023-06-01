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

#ifndef OHOS_HIVIEWDFX_DATA_SHARE_COMMON_H
#define OHOS_HIVIEWDFX_DATA_SHARE_COMMON_H

namespace OHOS {
namespace HiviewDFX {

constexpr const char* TIME_STAMP_FORMAT = "%Y%m%d%H%M%S";
constexpr const char* BUNDLE_UNINSTALL = "BUNDLE_UNINSTALL";
constexpr const char* BUNDLE_NAME = "BUNDLE_NAME";
constexpr const char* DOMAIN = "Domain";
constexpr const char* FILE_SUFFIX = ".evt";
constexpr const char* DOMAIN_PATH = "/Domain-";
constexpr const char* PATH_DIR = "/data/log/hiview/system_event_db/events/";
constexpr const char* TEMP_SRC_DIR = "/data/log/hiview/system_event_db/events/eventTemp.evt";
constexpr const char* TEMP_EXPORT_SRC_DIR = "/data/log/hiview/system_event_db/events/eventExportTemp.evt";
constexpr const char* DATABASE_DIR = "/data/log/hiview/system_event_db/subscribers/";

constexpr const char* SUCCESS_CODE = "000";
constexpr const char* QUERY_FAILED_CODE = "001";

constexpr const char* DATABASE_NAME = "subscribers.db";
constexpr const char* SQL_TEXT_TYPE = "TEXT NOT NULL";
constexpr const char* SQL_INT_TYPE = "INTEGER NOT NULL";
constexpr const char* SQL_BIGINT_TYPE = "BIGINT NOT NULL";

constexpr int MAXIMUM_FILE_SIZE = 1024 * 1024 * 5;  // Maximum file size 5M at one time
constexpr int MAXIMUM_EVENT_SIZE = 1024 * 768;      // Maximum event 768K
constexpr int DELAY_TIME = 2; // delay 2s

namespace SubscribeStore {
constexpr int DB_SUCC = 0;
constexpr int DB_FAILED = -1;

namespace EventTable {
constexpr const char* TABLE = "subscribe_events";
constexpr const char* FIELD_ID = "id";
constexpr const char* FIELD_UID = "uid";
constexpr const char* FIELD_BUNDLE_NAME = "bundle_name";
constexpr const char* FIELD_SUBSCRIBETIME = "subscribe_time";
constexpr const char* FIELD_EVENTLIST = "event_list";
}  // namespace EventTable

}  // namespace SubscribeStore

}  // namespace HiviewDFX
}  // namespace OHOS

#endif  // OHOS_HIVIEWDFX_DATA_SHARE_COMMON_H
