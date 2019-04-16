#include "Analytics.hpp"
#include "Firebase.hpp"
#include "scripting/js-bindings/manual/cocos2d_specifics.hpp"
#include "scripting/js-bindings/manual/js_manual_conversions.h"
#include <sstream>
#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include "utils/PluginUtils.h"
#include "firebase/analytics.h"

static void printLog(const char* str) {
    CCLOG("%s", str);
}

///////////////////////////////////////
//
//  Plugin Init
//
///////////////////////////////////////

static bool jsb_analytics_init(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_analytics_init");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 0) {

        printLog("[FirebaseAnalytics] Init plugin");
        firebase::App* app = firebase_app();
        if(app == NULL) {
            printLog("Firebase not initialized!");
            rec.rval().set(JSVAL_FALSE);
            return false;
        }
        firebase::analytics::Initialize(*app);
        rec.rval().set(JSVAL_TRUE);
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

///////////////////////////////////////
//
//  Analytics
//
///////////////////////////////////////

static bool jsb_analytics_log_event(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_analytics_log_event");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 1) {
        // event name
        bool ok = true;
        std::string event;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &event);
        firebase::analytics::LogEvent(event.c_str());
        rec.rval().set(JSVAL_TRUE);
        return true;
    } else if(argc == 2) {
        // event name, params dictionary
        bool ok = true;
        std::string event;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &event);

        JS::RootedValue arg1Val(cx, args.get(1));

        JS::RootedObject tmp(cx, arg1Val.toObjectOrNull());
        if (!tmp) {
            CCASSERT(false, "invalid parameters object");
        }
        std::vector<firebase::analytics::Parameter> result;
    
        JS::RootedObject it(cx, JS_NewPropertyIterator(cx, tmp));
    
        while (true) {
            JS::RootedId idp(cx);
            JS::RootedValue key(cx);
            if (! JS_NextProperty(cx, it, idp.address()) || ! JS_IdToValue(cx, idp, &key)) {
                CCASSERT(false, "invalid parameters property");
            }
        
            if (key.isNullOrUndefined()) {
                break; // end of iteration
            }
        
            if (!key.isString()) {
                continue; // only take account of string key
            }
        
            JSStringWrapper keyWrapper(key.toString(), cx);
        
            JS::RootedValue value(cx);
            JS_GetPropertyById(cx, tmp, idp, &value);
            if (value.isString()) {
                JSStringWrapper valueWrapper(value.toString(), cx);
                std::string tmp(valueWrapper.get());
                result.push_back(firebase::analytics::Parameter(keyWrapper.get(), firebase::Variant(tmp)));
            } else if(value.isBoolean()) {
                result.push_back(firebase::analytics::Parameter(keyWrapper.get(), firebase::Variant(value.get().toBoolean())));
            } else if(value.isDouble()) {
                result.push_back(firebase::analytics::Parameter(keyWrapper.get(), firebase::Variant(value.get().toDouble())));
            } else if(value.isInt32()) {
                result.push_back(firebase::analytics::Parameter(keyWrapper.get(), firebase::Variant(value.get().toInt32())));
            } else {
                CCASSERT(false, "event parameter has not supported type");
            }
        }
        firebase::analytics::LogEvent(event.c_str(), &result[0], result.size());
        return true;
    } else if(argc == 3) {
        // event name, parameter, value
        bool ok = true;
        std::string event;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &event);

        std::string parameter;
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &parameter);

        JS::RootedValue arg2Val(cx, args.get(2));
        if (arg2Val.isString()) {
            JSStringWrapper valueWrapper(arg2Val.toString(), cx);
            firebase::analytics::LogEvent(event.c_str(), parameter.c_str(), valueWrapper.get());
        } else if(arg2Val.isBoolean()) {
            firebase::analytics::LogEvent(event.c_str(), parameter.c_str(), arg2Val.get().toBoolean());
        } else if(arg2Val.isDouble()) {
            firebase::analytics::LogEvent(event.c_str(), parameter.c_str(), arg2Val.get().toDouble());
        } else if(arg2Val.isInt32()) {
            firebase::analytics::LogEvent(event.c_str(), parameter.c_str(), arg2Val.get().toInt32());
        } else {
            CCASSERT(false, "jsval_to_dictionary: not supported map type");
        }
        rec.rval().set(JSVAL_TRUE);
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_analytics_set_current_screen(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_analytics_set_current_screen");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 2) {
        // screen name, screen class
        bool ok = true;
        std::string screen_name, screen_class;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &screen_name);
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &screen_class);
        
        firebase::analytics::SetCurrentScreen(screen_name.c_str(), screen_class.c_str());
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_analytics_set_user_id(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_analytics_set_user_id");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 1) {
        // user id
        bool ok = true;
        std::string user_id;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &user_id);
        
        firebase::analytics::SetUserId(user_id.c_str());
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

static bool jsb_analytics_set_user_property(JSContext *cx, uint32_t argc, jsval *vp)
{
    printLog("jsb_analytics_set_user_property");
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    JS::CallReceiver rec = JS::CallReceiverFromVp(vp);
    if(argc == 2) {
        // property name, property value
        bool ok = true;
        std::string property_name, property_value;
        JS::RootedValue arg0Val(cx, args.get(0));
        ok &= jsval_to_std_string(cx, arg0Val, &property_name);
        JS::RootedValue arg1Val(cx, args.get(1));
        ok &= jsval_to_std_string(cx, arg1Val, &property_value);
        
        firebase::analytics::SetUserProperty(property_name.c_str(), property_value.c_str());
        return true;
    } else {
        JS_ReportError(cx, "Invalid number of arguments");
        return false;
    }
}

///////////////////////////////////////
//
//  Register JS API
//
///////////////////////////////////////

void register_all_analytics_framework(JSContext* cx, JS::HandleObject obj) {
    printLog("[FirebaseAnalytics] register js interface");
    JS::RootedObject ns(cx);
    get_or_create_js_obj(cx, obj, "analytics", &ns);

    JS_DefineFunction(cx, ns, "init", jsb_analytics_init, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "logEvent", jsb_analytics_log_event, 1, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "setScreenName", jsb_analytics_set_current_screen, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "setUserID", jsb_analytics_set_user_id, 1, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, ns, "setUserProperty", jsb_analytics_set_user_property, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT);
}
