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
#include <unordered_map>

#include <gtest/gtest.h>

#include "event_dispatcher.h"
#include "hiview_global.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
class TestPlugin : public Plugin {
public:
    void OnEventListeningCallback(const Event &msg) override
    {
        testMsg_ = msg.eventName_;
    }

    std::string GetTestMsg()
    {
        return testMsg_;
    }

    void ResetTestMsg()
    {
        testMsg_ = "";
    }

private:
    std::string testMsg_ = "";
};

class TestEventListener : public EventListener {
public:
    void OnUnorderedEvent(const Event &msg) override
    {
        testMsg_ = msg.eventName_;
    }

    std::string GetListenerName() override
    {
        return "Test_Listener";
    }

    std::string GetTestMsg()
    {
        return testMsg_;
    }

    void ResetTestMsg()
    {
        testMsg_ = "";
    }

private:
    std::string testMsg_ = "";
};

class TestHiviewContext : public HiviewContext {
public:
    void AddListenerInfo(uint32_t type, const std::string& name, const std::set<std::string>& eventNames,
        const std::set<EventListener::EventIdRange>& listenerInfo) override
    {
        listeners_[name] = listenerInfo;
        type_ = type;
    }

    void AddListenerInfo(uint32_t type, std::weak_ptr<Plugin> plugin, const std::set<std::string>& eventNames,
        const std::set<EventListener::EventIdRange>& listenerInfo) override
    {
        if (auto sp = plugin.lock(); sp != nullptr) {
            AddListenerInfo(type, sp->GetName(), eventNames, listenerInfo);
        }
    }

    bool GetListenerInfo(uint32_t type, const std::string& name,
        std::set<EventListener::EventIdRange> &listenerInfo) override
    {
        if (type == type_ && listeners_.find(name) != listeners_.end()) {
            listenerInfo = listeners_[name];
            return true;
        }
        return false;
    }

private:
    std::unordered_map<std::string, std::set<EventListener::EventIdRange>> listeners_;
    uint32_t type_ = 0;
};

class EventDispatcherTest : public testing::Test {
public:
    void SetUp()
    {
        static TestHiviewContext context;
        HiviewGlobal::CreateInstance(context);
        ASSERT_NE(HiviewGlobal::GetInstance(), nullptr);
    }

    void TearDown()
    {
        HiviewGlobal::ReleaseInstance();
    }
};

namespace {
constexpr uint32_t TEST_EVENT_TYPE = 0;
const std::string TEST_EVENT_NAME = "TEST_EVENT";
const std::string TEST_PLUGIN_NAME = "TEST_PLUGIN";
}
}  // namespace HiviewDFX
}  // namespace OHOS

void DispatchEventTest(EventDispatcher& dispatcher)
{
    Event event("TestSender");
    event.eventName_ = TEST_EVENT_NAME;
    dispatcher.DispatchEvent(event);
}

void DispatchInvalidEventTest(EventDispatcher& dispatcher)
{
    Event event("TestSender");
    event.messageType_ = Event::MessageType::FAULT_EVENT;
    event.eventName_ = TEST_EVENT_NAME;
    dispatcher.DispatchEvent(event);
}

std::shared_ptr<TestPlugin> CreatePlugin()
{
    auto plugin = std::make_shared<TestPlugin>();
    static TestHiviewContext pluginContext;
    plugin->SetHiviewContext(&pluginContext);
    plugin->SetName(TEST_PLUGIN_NAME);
    std::set<EventListener::EventIdRange> listenerInfo = { EventListener::EventIdRange(0, 10) };
    plugin->AddEventListenerInfo(TEST_EVENT_TYPE, listenerInfo);
    return plugin;
}

std::shared_ptr<TestEventListener> CreateListener()
{
    auto listener = std::make_shared<TestEventListener>();
    std::set<EventListener::EventIdRange> listenerInfo = { EventListener::EventIdRange(0, 10) };
    listener->AddListenerInfo(TEST_EVENT_TYPE, listenerInfo);
    return listener;
}

/**
 * @tc.name: EventDispatcherTest001
 * @tc.desc: Test event is dispatched to the EventListener.
 * @tc.type: FUNC
 * @tc.require: issueI642OH
 */
HWTEST_F(EventDispatcherTest, EventDispatcherTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create a dispatcher object.
     * @tc.steps: step2. create an EventListener object.
     * @tc.steps: step3. register the listener to the dispatcher.
     * @tc.steps: step4. dispatch event to the listener.
     */
    EventDispatcher dispatcher;
    dispatcher.AddInterestType(TEST_EVENT_TYPE);

    auto listener = CreateListener();
    dispatcher.RegisterListener(listener);

    DispatchEventTest(dispatcher);
    ASSERT_EQ(listener->GetTestMsg(), TEST_EVENT_NAME);

    // invalid cleanup
    listener->ResetTestMsg();
    ASSERT_TRUE(listener->GetTestMsg().empty());
    dispatcher.ClearInvalidListeners();
    DispatchEventTest(dispatcher);
    ASSERT_EQ(listener->GetTestMsg(), TEST_EVENT_NAME);

    // valid cleanup
    listener.reset();
    dispatcher.ClearInvalidListeners();
    DispatchEventTest(dispatcher);
}

/**
 * @tc.name: EventDispatcherTest002
 * @tc.desc: Test event is dispatched to the Plugin.
 * @tc.type: FUNC
 * @tc.require: issueI642OH
 */
HWTEST_F(EventDispatcherTest, EventDispatcherTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create a dispatcher object.
     * @tc.steps: step2. create a Plugin object.
     * @tc.steps: step3. register the plugin to the dispatcher.
     * @tc.steps: step4. dispatch event to the plugin.
     */
    EventDispatcher dispatcher;
    dispatcher.AddInterestType(TEST_EVENT_TYPE);

    auto plugin = CreatePlugin();
    dispatcher.RegisterListener(plugin);

    DispatchEventTest(dispatcher);
    ASSERT_EQ(plugin->GetTestMsg(), TEST_EVENT_NAME);

    plugin.reset();
    dispatcher.ClearInvalidListeners();
}

/**
 * @tc.name: EventDispatcherTest003
 * @tc.desc: invalid dispatch.
 * @tc.type: FUNC
 * @tc.require: issueI642OH
 */
HWTEST_F(EventDispatcherTest, EventDispatcherTest003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create a dispatcher object.
     * @tc.steps: step2. create a Plugin object.
     * @tc.steps: step3. register the plugin to the dispatcher.
     * @tc.steps: step4. dispatch event to the plugin.
     */
    // invalid dispatch, because interest type is null
    EventDispatcher dispatcher;
    auto plugin = CreatePlugin();
    dispatcher.RegisterListener(plugin);
    auto listener = CreateListener();
    dispatcher.RegisterListener(listener);
    DispatchEventTest(dispatcher);
    ASSERT_TRUE(plugin->GetTestMsg().empty());
    ASSERT_TRUE(listener->GetTestMsg().empty());

    dispatcher.AddInterestType(TEST_EVENT_TYPE);
    dispatcher.RegisterListener(plugin);
    dispatcher.RegisterListener(listener);
    DispatchEventTest(dispatcher);
    ASSERT_EQ(plugin->GetTestMsg(), TEST_EVENT_NAME);
    ASSERT_EQ(listener->GetTestMsg(), TEST_EVENT_NAME);
    plugin->ResetTestMsg();
    listener->ResetTestMsg();

    // invalid dispatch, because message type is invalid
    DispatchInvalidEventTest(dispatcher);
    ASSERT_TRUE(plugin->GetTestMsg().empty());
    ASSERT_TRUE(listener->GetTestMsg().empty());

    // invalid dispatch, because listener is invalid
    plugin.reset();
    listener.reset();
    DispatchEventTest(dispatcher);

    dispatcher.ClearInvalidListeners();
}

/**
 * @tc.name: EventDispatcherTest004
 * @tc.desc: null dispatch.
 * @tc.type: FUNC
 * @tc.require: issueI642OH
 */
HWTEST_F(EventDispatcherTest, EventDispatcherTest004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create a dispatcher object.
     * @tc.steps: step2. create a Plugin object.
     * @tc.steps: step3. register the plugin to the dispatcher.
     * @tc.steps: step4. dispatch event to the plugin.
     */
    // null registration
    EventDispatcher dispatcher;
    dispatcher.AddInterestType(TEST_EVENT_TYPE);
    std::shared_ptr<TestPlugin> plugin = nullptr;
    dispatcher.RegisterListener(plugin);
    std::shared_ptr<TestEventListener> listener = nullptr;
    dispatcher.RegisterListener(listener);

    plugin = CreatePlugin();
    dispatcher.RegisterListener(plugin);

    // duplicate registration
    dispatcher.RegisterListener(plugin);
    DispatchEventTest(dispatcher);
    ASSERT_EQ(plugin->GetTestMsg(), TEST_EVENT_NAME);

    // reset the first plugin and register another plugin
    plugin.reset();
    auto plugin2 = CreatePlugin();
    dispatcher.RegisterListener(plugin2);
    DispatchEventTest(dispatcher);
    ASSERT_EQ(plugin2->GetTestMsg(), TEST_EVENT_NAME);

    plugin2.reset();
    dispatcher.ClearInvalidListeners();
}
