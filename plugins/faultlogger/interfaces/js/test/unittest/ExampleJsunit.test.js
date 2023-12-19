/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
import faultlogger from '@ohos.faultLogger'
import hiSysEvent from '@ohos.hiSysEvent'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import faultloggerTestNapi from "libfaultlogger_test_napi.so"

describe("FaultlogJsTest", function () {
    beforeAll(function() {
        /*
         * @tc.setup: setup invoked before all testcases
         */
         console.info('FaultlogJsTest beforeAll called')
    })

    afterAll(function() {
        /*
         * @tc.teardown: teardown invoked after all testcases
         */
         console.info('FaultlogJsTest afterAll called')
    })

    beforeEach(function() {
        /*
         * @tc.setup: setup invoked before each testcases
         */
         console.info('FaultlogJsTest beforeEach called')
    })

    afterEach(function() {
        /*
         * @tc.teardown: teardown invoked after each testcases
         */
         console.info('FaultlogJsTest afterEach called')
    })

    async function msleep(time) {
        let promise = new Promise((resolve, reject) => {
            setTimeout(() => resolve("done!"), time)
        });
        let result = await promise;
    }

    /**
     * test
     *
     * @tc.number: FaultlogJsException_001
     * @tc.name: FaultlogJsException_001
     * @tc.desc: API8 检验函数参数输入错误时程序是否会崩溃
     * @tc.require: AR000GICT2
     * @tc.author:
     * @tc.type: Function
     * @tc.size: MediumTest
     * @tc.level: Level 0
     */
    it('FaultlogJsException_001', 0, async function (done) {
        console.info("---------------------------FaultlogJsException_001----------------------------------");
        try {
            let ret1 = faultlogger.querySelfFaultLog("faultloggertestsummary01");
            console.info("FaultlogJsException_001 ret1 == " + ret1);
            let a = expect(ret1).assertEqual(undefined);
            console.info('ret1 assertEqual(undefined) ' + a);

            let ret2 = faultlogger.querySelfFaultLog(faultlogger.FaultType.JS_CRASH, "faultloggertestsummary01");
            console.info("FaultlogJsException_001 ret2 == " + ret2);
            expect(ret2).assertEqual(undefined);

            let ret3 = faultlogger.querySelfFaultLog();
            console.info("FaultlogJsException_001 ret3 == " + ret3);
            expect(ret3).assertEqual(undefined);
            done();
            return;
        } catch(err) {
            console.info(err);
        }
        expect(false).assertTrue();
        done();
    })

    /**
     * test
     *
     * @tc.number: FaultlogJsException_002
     * @tc.name: FaultlogJsException_002
     * @tc.desc: API9 检验函数参数输入错误时程序是否会崩溃并校验错误码
     * @tc.require: issueI5VRCC
     * @tc.author:
     * @tc.type: Function
     * @tc.size: MediumTest
     * @tc.level: Level 0
     */
     it('FaultlogJsException_002', 0, function () {
        console.info("---------------------------FaultlogJsException_002----------------------------------");
        try {
            let ret = faultlogger.query("faultloggertestsummary02");
            console.info("FaultlogJsException_002 ret == " + ret);
            return;
        } catch(err) {
            console.info(err.code);
            console.info(err.message);
            expect(err.code == 401).assertTrue();
        }
    })

    /**
     * test
     *
     * @tc.number: FaultlogJsException_003
     * @tc.name: FaultlogJsException_003
     * @tc.desc: API9 检验函数参数输入错误时程序是否会崩溃并校验错误码
     * @tc.require: issueI5VRCC
     * @tc.author:
     * @tc.type: Function
     * @tc.size: MediumTest
     * @tc.level: Level 0
     */
     it('FaultlogJsException_003', 0, function () {
        console.info("---------------------------FaultlogJsException_003----------------------------------");
        try {
            let ret = faultlogger.query(faultlogger.FaultType.JS_CRASH, "faultloggertestsummary03");
            console.info("FaultlogJsException_003 ret == " + ret);
            return;
        } catch(err) {
            console.info(err.code);
            console.info(err.message);
            expect(err.code == 401).assertTrue();
        }
    })

    /**
     * test
     *
     * @tc.number: FaultlogJsException_004
     * @tc.name: FaultlogJsException_004
     * @tc.desc: API9 检验函数参数输入错误时程序是否会崩溃并校验错误码
     * @tc.require: issueI5VRCC
     * @tc.author:
     * @tc.type: Function
     * @tc.size: MediumTest
     * @tc.level: Level 0
     */
     it('FaultlogJsException_004', 0, function () {
        console.info("---------------------------FaultlogJsException_004----------------------------------");
        try {
            let ret = faultlogger.query();
            console.info("FaultlogJsException_004 ret == " + ret);
            return;
        } catch(err) {
            console.info(err.code);
            console.info(err.message);
            expect(err.code == 401).assertTrue();
        }
    })

    /**
     * test
     *
     * @tc.number: FaultlogJsException_005
     * @tc.name: FaultlogJsException_005
     * @tc.desc: API9 检验函数参数输入错误时程序是否会崩溃并校验错误码
     * @tc.require: issueI5VRCC
     * @tc.author:
     * @tc.type: Function
     * @tc.size: MediumTest
     * @tc.level: Level 0
     */
    it('FaultlogJsException_005', 0, function () {
        console.info("---------------------------FaultlogJsException_005----------------------------------");
        try {
            let ret = faultlogger.query("aaa", "bbb", "ccc");
            console.info("FaultlogJsException_005 ret == " + ret);
            return;
        } catch(err) {
            console.info(err.code);
            console.info(err.message);
            expect(err.code == 401).assertTrue();
        }
    })

    /**
     * test
     *
     * @tc.number: FaultlogJsTest_005
     * @tc.name: FaultlogJsTest_005
     * @tc.desc: API9 检验promise同步方式获取faultlog日志
     * @tc.require: issueI5VRCC
     * @tc.author:
     * @tc.type: Function
     * @tc.size: MediumTest
     * @tc.level: Level 0
     */
     it('FaultlogJsTest_005', 0, async function (done) {
        console.info("---------------------------FaultlogJsTest_005----------------------------------");
        try {
            const res = faultloggerTestNapi.triggerCppCrash();
            console.info("FaultlogJsTest_005 res:" + res);
            let now = Date.now();
            console.info("FaultlogJsTest_005 now:" + now);
            await msleep(3000); // 3000: sleep 3000ms

            console.info("--------FaultlogJsTest_005 start query ----------");
            let ret = await faultlogger.query(faultlogger.FaultType.CPP_CRASH);
            console.info("FaultlogJsTest_005 query ret length:" + ret.length);
            expect(ret.length).assertLarger(0);
            console.info("FaultlogJsTest_005 check reason, index:" + (ret[0].reason.indexOf("Signal:SIGABRT")));
            expect(ret[0].reason.indexOf("Signal:SIGABRT") != -1).assertTrue();
            console.info("FaultlogJsTest_005 check fullLog, index:" + ret[0].fullLog.indexOf("Fault thread Info"));
            expect(ret[0].fullLog.indexOf("Fault thread Info") != -1).assertTrue();
            done();
            return;
        } catch (err) {
            console.info("catch (err) == " + err);
        }
        console.info("FaultlogJsTest_005 error");
        expect(false).assertTrue();
        done();
    })

    /**
     * test
     *
     * @tc.number: FaultlogJsTest_006
     * @tc.name: FaultlogJsTest_006
     * @tc.desc: API9 检验通过回调方式获取faultlog日志
     * @tc.require: issueI5VRCC
     * @tc.author:
     * @tc.type: Function
     * @tc.size: MediumTest
     * @tc.level: Level 0
     */
    it('FaultlogJsTest_006', 0, async function (done) {
        console.info("---------------------------FaultlogJsTest_006----------------------------------");
        try {
            let now = Date.now();
            console.info("FaultlogJsTest_006 start + " + now);
            let module = "com.ohos.hiviewtest.faultlogjs";
            const loopTimes = 10;
            for (let i = 0; i < loopTimes; i++) {
                console.info("--------FaultlogJsTest_006 + " + i + "----------");
                faultlogger.addFaultLog(i - 100,
                    faultlogger.FaultType.APP_FREEZE, module, "faultloggertestsummary06 " + i);
                await msleep(300);
            }
            await msleep(1000);

            console.info("--------FaultlogJsTest_006 4----------");
            function queryFaultLogCallback(error, ret) {
                if (error) {
                    console.info('FaultlogJsTest_006  once error is ' + error);
                } else {
                    console.info("FaultlogJsTest_006 ret == " + ret.length);
                    expect(ret.length).assertEqual(loopTimes);
                    for (let i = 0; i < loopTimes; i++) {
                        console.info("faultloggertestsummary06 " + i + " fullLog.length " + ret[i].fullLog.length);
                        console.info(ret[i].fullLog);
                        if (ret[i].fullLog.indexOf("faultloggertestsummary06 " + (loopTimes - 1 - i)) != -1) {
                            console.info("FaultlogJsTest_006 " + ret[i].fullLog.length);
                            expect(true).assertTrue();
                        } else {
                            expect(false).assertTrue();
                        }
                    }
                }
                done();
            }
            faultlogger.query(faultlogger.FaultType.APP_FREEZE, queryFaultLogCallback);
            return;
        } catch (err) {
            console.info(err);
        }
        console.info("FaultlogJsTest_006 error");
        expect(false).assertTrue();
        done();
    })

    /**
     * test
     *
     * @tc.number: FaultlogJsTest_007
     * @tc.name: FaultlogJsTest_007
     * @tc.desc: API9 检验通过回调方式获取faultlog日志的顺序
     * @tc.require: issueI5VRCC
     * @tc.author:
     * @tc.type: Function
     * @tc.size: MediumTest
     * @tc.level: Level 0
     */
    it('FaultlogJsTest_007', 0, async function (done) {
        console.info("---------------------------FaultlogJsTest_007----------------------------------");
        try {
            let now = Date.now();
            console.info("FaultlogJsTest_007 start + " + now);
            let module = "com.ohos.hiviewtest.faultlogjs";
            faultlogger.addFaultLog(0,
                faultlogger.FaultType.APP_FREEZE, module, "faultloggertestsummary07");
            await msleep(1000);
            hiSysEvent.write({
                domain: "ACE",
                name: "JS_ERROR",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PACKAGE_NAME: "com.ohos.faultlogger.test",
                    PROCESS_NAME: "com.ohos.faultlogger.test",
                    MSG: "faultlogger testcase test.",
                    REASON: "faultlogger testcase test."
                }
            }).then(
                (value) => {
                    console.log(`HiSysEvent json-callback-success value=${value}`);
                })
            await msleep(1000);
            faultlogger.addFaultLog(0,
                    faultlogger.FaultType.CPP_CRASH, module, "faultloggertestsummary07");
            await msleep(1000);
            console.info("--------FaultlogJsTest_007");
            function queryFaultLogCallback(error, ret) {
                if (error) {
                    console.info('FaultlogJsTest_007  once error is ' + error);
                } else {
                    console.info("FaultlogJsTest_007 ret == " + ret.length);
                    expect(ret[0].type).assertEqual(faultlogger.FaultType.CPP_CRASH);
                    expect(ret[1].type).assertEqual(faultlogger.FaultType.JS_CRASH);
                    expect(ret[1].timestamp).assertLess(ret[0].timestamp);
                    expect(ret[2].type).assertEqual(faultlogger.FaultType.APP_FREEZE);
                    expect(ret[2].timestamp).assertLess(ret[1].timestamp);
                }
                done();
            }
            faultlogger.query(faultlogger.FaultType.NO_SPECIFIC, queryFaultLogCallback);
            return;
        } catch (err) {
            console.info(err);
        }
        console.info("FaultlogJsTest_007 error");
        expect(false).assertTrue();
        done();
    })
})