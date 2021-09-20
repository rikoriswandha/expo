// Copyright 2018-present 650 Industries. All rights reserved.

#import <jsi/jsi.h>

#import <ReactCommon/CallInvoker.h>

using namespace facebook;

namespace expo {

static NSString *convertToNSString(jsi::Runtime &runtime, const jsi::String &value) {
  return [NSString stringWithUTF8String:value.utf8(runtime).c_str()];
}

static id convertToNSObject(jsi::Runtime &runtime, const jsi::Value &value, std::shared_ptr<react::CallInvoker> jsInvoker)
{
  
}

} // namespace expo
