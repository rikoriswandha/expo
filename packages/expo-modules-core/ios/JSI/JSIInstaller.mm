// Copyright 2018-present 650 Industries. All rights reserved.

#import <jsi/jsi.h>

#import <ReactCommon/RCTTurboModule.h>

using namespace facebook;
using namespace facebook::react;

using PromiseInvocationBlock = void (^)(RCTPromiseResolveBlock resolveWrapper, RCTPromiseRejectBlock rejectWrapper);

void installRuntimeObjects(jsi::Runtime &runtime)
{
  jsi::Object expoModulesObject(runtime);

  auto callMethodAsyncName = jsi::PropNameID::forUtf8(runtime, "callMethodAsync");

  expoModulesObject.setProperty(
                                runtime,
                                callMethodAsyncName,
                                jsi::Function::createFromHostFunction(runtime, callMethodAsyncName, 2, [](jsi::Runtime &runtime, const jsi::Value &jsThis, const jsi::Value *jsArgv, size_t argc) {

                                  auto arg0 = jsArgv[0].asString(runtime);
                                  auto arg1 = jsArgv[1].asObject(runtime).asFunction(runtime);
//                                  jsi::Array args = jsArgv->asObject(runtime).asArray(runtime);
//                                  jsi::String arg1 = args.getValueAtIndex(runtime, 0).asString(runtime);
//                                  jsi::Function callback = args.getValueAtIndex(runtime, 1).asObject(runtime).asFunction(runtime);
                                  arg1.call(runtime, arg0, jsi::Value::null());
                                  return jsi::Value::undefined();
                                }));

  runtime.global().setProperty(runtime, "ExpoModules", expoModulesObject);
}

jsi::Value createPromise(jsi::Runtime &runtime, std::shared_ptr<CallInvoker> jsInvoker, PromiseInvocationBlock invoke)
{
  if (!invoke) {
    return jsi::Value::undefined();
  }

  jsi::Function Promise = runtime.global().getPropertyAsFunction(runtime, "Promise");

  PromiseInvocationBlock invokeCopy = [invoke copy];

  jsi::Function fn = jsi::Function::createFromHostFunction(
    runtime,
    jsi::PropNameID::forAscii(runtime, "fn"),
    2,
    [invokeCopy, jsInvoker](jsi::Runtime &rt, const jsi::Value &thisVal, const jsi::Value *args, size_t count) {
      if (count != 2) {
        throw std::invalid_argument(
            "Promise must pass constructor function two args. Passed " + std::to_string(count) + " args.");
      }
      if (!invokeCopy) {
        return jsi::Value::undefined();
      }

      auto weakResolveWrapper = CallbackWrapper::createWeak(args[0].getObject(rt).getFunction(rt), rt, jsInvoker);
      auto weakRejectWrapper = CallbackWrapper::createWeak(args[1].getObject(rt).getFunction(rt), rt, jsInvoker);

      __block BOOL resolveWasCalled = NO;
      __block BOOL rejectWasCalled = NO;

      RCTPromiseResolveBlock resolveBlock = ^(id result) {
        if (rejectWasCalled) {
          throw std::runtime_error("Tried to resolve a promise after it's already been rejected.");
        }

        if (resolveWasCalled) {
          throw std::runtime_error("Tried to resolve a promise more than once.");
        }

        auto strongResolveWrapper = weakResolveWrapper.lock();
        auto strongRejectWrapper = weakRejectWrapper.lock();
        if (!strongResolveWrapper || !strongRejectWrapper) {
          return;
        }

        strongResolveWrapper->jsInvoker().invokeAsync([weakResolveWrapper, weakRejectWrapper, result]() {
          auto strongResolveWrapper2 = weakResolveWrapper.lock();
          auto strongRejectWrapper2 = weakRejectWrapper.lock();
          if (!strongResolveWrapper2 || !strongRejectWrapper2) {
            return;
          }

          jsi::Runtime &rt = strongResolveWrapper2->runtime();
          jsi::Value arg = convertObjCObjectToJSIValue(rt, result);
          strongResolveWrapper2->callback().call(rt, arg);

          strongResolveWrapper2->destroy();
          strongRejectWrapper2->destroy();
        });

        resolveWasCalled = YES;
      };

      RCTPromiseRejectBlock rejectBlock = ^(NSString *code, NSString *message, NSError *error) {
        if (resolveWasCalled) {
          throw std::runtime_error("Tried to reject a promise after it's already been resolved.");
        }

        if (rejectWasCalled) {
          throw std::runtime_error("Tried to reject a promise more than once.");
        }

        auto strongResolveWrapper = weakResolveWrapper.lock();
        auto strongRejectWrapper = weakRejectWrapper.lock();
        if (!strongResolveWrapper || !strongRejectWrapper) {
          return;
        }

        NSDictionary *jsError = RCTJSErrorFromCodeMessageAndNSError(code, message, error);
        strongRejectWrapper->jsInvoker().invokeAsync([weakResolveWrapper, weakRejectWrapper, jsError]() {
          auto strongResolveWrapper2 = weakResolveWrapper.lock();
          auto strongRejectWrapper2 = weakRejectWrapper.lock();
          if (!strongResolveWrapper2 || !strongRejectWrapper2) {
            return;
          }

          jsi::Runtime &rt = strongRejectWrapper2->runtime();
          jsi::Value arg = convertNSDictionaryToJSIObject(rt, jsError);
          strongRejectWrapper2->callback().call(rt, arg);

          strongResolveWrapper2->destroy();
          strongRejectWrapper2->destroy();
        });

        rejectWasCalled = YES;
      };

      invokeCopy(resolveBlock, rejectBlock);
      return jsi::Value::undefined();
    }
  );

  return Promise.callAsConstructor(runtime, fn);
}
