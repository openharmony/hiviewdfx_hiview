/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class HiRetrievalConfigAni implements hiRetrieval.HiRetrievalConfig {
    userType: string = "";
    deviceType: string = "";
    deviceModel: string = "";
}

export default namespace hiRetrieval {
    loadLibraryWithPermissionCheck("hiretrieval_ani", "@ohos.hiviewdfx.hiRetrieval");

    export interface HiRetrievalConfig {
        userType: string;
        deviceType: string;
        deviceModel: string;
    }

    export native function init(): void;
    export native function participate(config: HiRetrievalConfig): void;
    export native function quit(): void;
    export native function isParticipant(): boolean;
    export native function getLastParticipationTimestamp(): long;
    export native function run(): void;
    export native function getCurrentConfig(): HiRetrievalConfig;
}