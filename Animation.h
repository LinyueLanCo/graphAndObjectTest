#pragma once

#include "AnimationTypes.h"
#include "AnimationClip.h"
#include "AnimationClipManager.h"
#include "AnimatedSprite.h"

AnimationId getPlayerAnimationId(AnimationState state);
AnimationId getAnimationId(AnimationSetId setId, AnimationState state);
