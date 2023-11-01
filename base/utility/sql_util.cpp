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
#include "sql_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace SqlUtil {
std::string GenerateCreateSql(const std::string& table,
    const std::vector<std::pair<std::string, std::string>>& fields)
{
    std::string sql;
    sql += "CREATE TABLE IF NOT EXISTS ";
    sql += table;
    sql += "(";
    sql += "id INTEGER PRIMARY KEY AUTOINCREMENT";  // default field: id
    for (auto field : fields) {
        sql += ", ";
        sql += field.first;
        sql += " ";
        sql += field.second;
    }
    sql += ")";
    return sql;
}

std::string GenerateExistSql(const std::string& table)
{
    std::string sql = "SELECT name FROM sqlite_sequence WHERE name = '" + table + "'";
    return sql;
}

std::string GenerateDropSql(const std::string& table)
{
    std::string sql;
    sql += "DROP TABLE IF EXISTS ";
    sql += table;
    return sql;
}
} // namespace SqlUtil
} // namespace HiviewDFX
} // namespace OHOS
