/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fbjni/fbjni.h>
#include <ABI43_0_0React/ABI43_0_0renderer/core/ConcreteComponentDescriptor.h>
#include "ABI43_0_0AndroidTextInputShadowNode.h"

#include <ABI43_0_0yoga/ABI43_0_0CompactValue.h>
#include <ABI43_0_0yoga/ABI43_0_0YGEnums.h>
#include <ABI43_0_0yoga/ABI43_0_0YGValue.h>

namespace ABI43_0_0facebook {
namespace ABI43_0_0React {

/*
 * Descriptor for <AndroidTextInput> component.
 */
class AndroidTextInputComponentDescriptor final
    : public ConcreteComponentDescriptor<AndroidTextInputShadowNode> {
 public:
  AndroidTextInputComponentDescriptor(
      ComponentDescriptorParameters const &parameters)
      : ConcreteComponentDescriptor<AndroidTextInputShadowNode>(parameters) {
    // Every single `AndroidTextInputShadowNode` will have a reference to
    // a shared `TextLayoutManager`.
    textLayoutManager_ = std::make_shared<TextLayoutManager>(contextContainer_);
  }

  virtual State::Shared createInitialState(
      ShadowNodeFragment const &fragment,
      ShadowNodeFamily::Shared const &family) const override {
    int surfaceId = family->getSurfaceId();

    ABI43_0_0YGStyle::Edges theme;
    // TODO: figure out RTL/start/end/left/right stuff here
    if (surfaceIdToThemePaddingMap_.find(surfaceId) !=
        surfaceIdToThemePaddingMap_.end()) {
      theme = surfaceIdToThemePaddingMap_[surfaceId];
    } else {
      const jni::global_ref<jobject> &fabricUIManager =
          contextContainer_->at<jni::global_ref<jobject>>("FabricUIManager");

      auto env = jni::Environment::current();
      auto defaultTextInputPaddingArray = env->NewFloatArray(4);
      static auto getThemeData =
          jni::findClassStatic(UIManagerJavaDescriptor)
              ->getMethod<jboolean(jint, jfloatArray)>("getThemeData");

      if (getThemeData(
              fabricUIManager, surfaceId, defaultTextInputPaddingArray)) {
        jfloat *defaultTextInputPadding =
            env->GetFloatArrayElements(defaultTextInputPaddingArray, 0);
        theme[ABI43_0_0YGEdgeStart] = (ABI43_0_0YGValue){defaultTextInputPadding[0], ABI43_0_0YGUnitPoint};
        theme[ABI43_0_0YGEdgeEnd] = (ABI43_0_0YGValue){defaultTextInputPadding[1], ABI43_0_0YGUnitPoint};
        theme[ABI43_0_0YGEdgeTop] = (ABI43_0_0YGValue){defaultTextInputPadding[2], ABI43_0_0YGUnitPoint};
        theme[ABI43_0_0YGEdgeBottom] =
            (ABI43_0_0YGValue){defaultTextInputPadding[3], ABI43_0_0YGUnitPoint};
        surfaceIdToThemePaddingMap_.emplace(std::make_pair(surfaceId, theme));
        env->ReleaseFloatArrayElements(
            defaultTextInputPaddingArray, defaultTextInputPadding, JNI_ABORT);
      }
      env->DeleteLocalRef(defaultTextInputPaddingArray);
    }

    return std::make_shared<AndroidTextInputShadowNode::ConcreteState>(
        std::make_shared<AndroidTextInputState const>(AndroidTextInputState(
            0,
            {},
            {},
            {},
            {},
            {},
            textLayoutManager_,
            ((ABI43_0_0YGValue)theme[ABI43_0_0YGEdgeStart]).value,
            ((ABI43_0_0YGValue)theme[ABI43_0_0YGEdgeEnd]).value,
            ((ABI43_0_0YGValue)theme[ABI43_0_0YGEdgeTop]).value,
            ((ABI43_0_0YGValue)theme[ABI43_0_0YGEdgeBottom]).value)),
        family);
  }

 protected:
  void adopt(UnsharedShadowNode shadowNode) const override {
    assert(std::dynamic_pointer_cast<AndroidTextInputShadowNode>(shadowNode));
    auto textInputShadowNode =
        std::static_pointer_cast<AndroidTextInputShadowNode>(shadowNode);

    // `ParagraphShadowNode` uses `TextLayoutManager` to measure text content
    // and communicate text rendering metrics to mounting layer.
    textInputShadowNode->setTextLayoutManager(textLayoutManager_);

    textInputShadowNode->setContextContainer(
        const_cast<ContextContainer *>(getContextContainer().get()));

    int surfaceId = textInputShadowNode->getSurfaceId();
    if (surfaceIdToThemePaddingMap_.find(surfaceId) !=
        surfaceIdToThemePaddingMap_.end()) {
      ABI43_0_0YGStyle::Edges theme = surfaceIdToThemePaddingMap_[surfaceId];

      // Override padding
      // Node is still unsealed during adoption, before layout is complete
      // TODO: T62959168 account for RTL and paddingLeft when setting default
      // paddingStart, and vice-versa with paddingRight/paddingEnd.
      // For now this assumes no RTL.
      ABI43_0_0YGStyle::Edges result =
          textInputShadowNode->getConcreteProps().yogaStyle.padding();
      bool changedPadding = false;
      if (!textInputShadowNode->getConcreteProps().hasPadding &&
          !textInputShadowNode->getConcreteProps().hasPaddingStart &&
          !textInputShadowNode->getConcreteProps().hasPaddingLeft &&
          !textInputShadowNode->getConcreteProps().hasPaddingHorizontal) {
        changedPadding = true;
        result[ABI43_0_0YGEdgeStart] = theme[ABI43_0_0YGEdgeStart];
      }
      if (!textInputShadowNode->getConcreteProps().hasPadding &&
          !textInputShadowNode->getConcreteProps().hasPaddingEnd &&
          !textInputShadowNode->getConcreteProps().hasPaddingRight &&
          !textInputShadowNode->getConcreteProps().hasPaddingHorizontal) {
        changedPadding = true;
        result[ABI43_0_0YGEdgeEnd] = theme[ABI43_0_0YGEdgeEnd];
      }
      if (!textInputShadowNode->getConcreteProps().hasPadding &&
          !textInputShadowNode->getConcreteProps().hasPaddingTop &&
          !textInputShadowNode->getConcreteProps().hasPaddingVertical) {
        changedPadding = true;
        result[ABI43_0_0YGEdgeTop] = theme[ABI43_0_0YGEdgeTop];
      }
      if (!textInputShadowNode->getConcreteProps().hasPadding &&
          !textInputShadowNode->getConcreteProps().hasPaddingBottom &&
          !textInputShadowNode->getConcreteProps().hasPaddingVertical) {
        changedPadding = true;
        result[ABI43_0_0YGEdgeBottom] = theme[ABI43_0_0YGEdgeBottom];
      }

      // If the TextInput initially does not have paddingLeft or paddingStart, a
      // paddingStart may be set from the theme. If that happens, when there's a
      // paddingLeft update, we must explicitly unset paddingStart... (same with
      // paddingEnd)
      // TODO: support RTL
      if ((textInputShadowNode->getConcreteProps().hasPadding ||
           textInputShadowNode->getConcreteProps().hasPaddingLeft ||
           textInputShadowNode->getConcreteProps().hasPaddingHorizontal) &&
          !textInputShadowNode->getConcreteProps().hasPaddingStart) {
        result[ABI43_0_0YGEdgeStart] = ABI43_0_0YGValueUndefined;
      }
      if ((textInputShadowNode->getConcreteProps().hasPadding ||
           textInputShadowNode->getConcreteProps().hasPaddingRight ||
           textInputShadowNode->getConcreteProps().hasPaddingHorizontal) &&
          !textInputShadowNode->getConcreteProps().hasPaddingEnd) {
        result[ABI43_0_0YGEdgeEnd] = ABI43_0_0YGValueUndefined;
      }

      // Note that this is expensive: on every adopt, we need to set the Yoga
      // props again, which normally only happens during prop parsing. Every
      // commit, state update, etc, will incur this cost.
      if (changedPadding) {
        // Set new props on node
        const_cast<AndroidTextInputProps &>(
            textInputShadowNode->getConcreteProps())
            .yogaStyle.padding() = result;
        // Communicate new props to Yoga part of the node
        textInputShadowNode->updateYogaProps();
      }
    }

    textInputShadowNode->dirtyLayout();
    textInputShadowNode->enableMeasurement();

    ConcreteComponentDescriptor::adopt(shadowNode);
  }

 private:
  // TODO T68526882: Unify with Binding::UIManagerJavaDescriptor
  constexpr static auto UIManagerJavaDescriptor =
      "com/facebook/ABI43_0_0React/fabric/FabricUIManager";

  SharedTextLayoutManager textLayoutManager_;
  mutable better::map<int, ABI43_0_0YGStyle::Edges> surfaceIdToThemePaddingMap_;
};

} // namespace ABI43_0_0React
} // namespace ABI43_0_0facebook
