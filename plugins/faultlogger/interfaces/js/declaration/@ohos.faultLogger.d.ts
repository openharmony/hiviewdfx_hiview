/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
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

import { AsyncCallback } from "./basic";


/**
 * This module provides the capability to query faultlog data.
 *
 * @since 8
 * @sysCap SystemCapability.HiviewDFX.Hiview.FaultLogger
 * @devices phone, tablet
 * @import import sensor from '@ohos.faultlogger'
 */

declare namespace FaultLogger {
  /**
   * The type of fault type.
   * @devices phone, tablet
   * @sysCap SystemCapability.Sensors.Sensor
   * @since 8
   */
  enum FaultType {
    /**
     * NO_SPECIFIC log type not distinguished.
     * @since 8 
     */
    NO_SPECIFIC = 0,
    /**
     * CPP_CRASH CPP crash log type
     * @since 8 
     */
    CPP_CRASH = 2,
    /**
     * JS_CRASH JS crash log type
     * @since 8 
     */
    JS_CRASH = 3,
    /**
     * APP_FREEZE app feeeze log type
     * @since 8 
     */
    APP_FREEZE = 4,
  }

  /**
   * Query the result of the current application FaultLog in callback Mode
   * @param faultType fault type to query
   * @param callback faultlog information data callback function
   * @since 8
   */
  function querySelfFaultLog(faultType: FaultType, callback: AsyncCallback<Array<FaultLogInfo>>) : void;

  /**
   * Query the result of the current application FaultLog in return promise mode.
   * @param faultType fault type to query
   * @return return faultlog information data by promise
   * @since 8
   */
  function querySelfFaultLog(faultType: FaultType) : Promise<Array<FaultLogInfo>>;

  /**
   * FaultLog information data structure
   * @devices phone, tablet
   * @sysCap SystemCapability.Sensors.Sensor
   * @since 8
   */
  interface FaultLogInfo {
    /**
     * pid Process id
     * @since 8
     */
    pid: number;

    /**
     * uid user id
     * @since 8
     */
    uid: number;

    /**
     * type fault type
     * @since 8
     */
    type: FaultType;

    /**
     * second level timestamp
     * @since 8
     */
    timestamp: number;

    /**
     * reason fault reason
     * @since 8
     */
    reason: string;

    /**
     * module fault module
     * @since 8
     */
    module: string;

    /**
     * summary fault summary
     * @since 8
     */
    summary: string;

    /**
     * fullLog fault log
     * @since 8
     */
    fullLog: string;
  }
}

export default FaultLogger;
