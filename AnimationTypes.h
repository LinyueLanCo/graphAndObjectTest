#pragma once

// AnimationState：
 // 动画表现状态枚举。当前主要由 Animator 保存和切换。
 // 它描述“现在应该播放什么动画”，不代表实体真实物理状态。
enum AnimationState
{
	ANIM_IDLE_L,
	ANIM_IDLE_R,
	ANIM_WALK_LEFT,
	ANIM_WALK_RIGHT,
	ANIM_RUN_LEFT,
	ANIM_RUN_RIGHT,

	ANIM_JUMP_START_L,
	ANIM_JUMP_START_R,
	ANIM_JUMP_LOOP_L,
	ANIM_JUMP_LOOP_R,
	ANIM_JUMP_END_L,
	ANIM_JUMP_END_R,

	ANIM_COUNT
};

// AnimationSetId：
 // 动画资源组 ID，用来描述“这个实体使用哪一套动画图片资源”。
 // 它不代表实体实例编号，也不代表动画状态；AnimationState + AnimationSetId 才能定位到具体 AnimationClip。
enum AnimationSetId
{
	ANIM_SET_NONE,
	ANIM_SET_PLAYER1,
};


// AnimationId：
 // 资源层动画 ID。Animator 选择 AnimationState 后，
 // 会通过 getPlayerAnimationId 转换成 AnimationId，再向 AnimationClipManager 请求 AnimationClip。
enum AnimationId
{
	ANIM_ID_PLAYER_IDLE_L,
	ANIM_ID_PLAYER_IDLE_R,
	ANIM_ID_PLAYER_WALK_L,
	ANIM_ID_PLAYER_WALK_R,
	ANIM_ID_PLAYER_RUN_L,
	ANIM_ID_PLAYER_RUN_R,
	ANIM_ID_PLAYER_JUMP_START_L,
	ANIM_ID_PLAYER_JUMP_START_R,
	ANIM_ID_PLAYER_JUMP_LOOP_L,
	ANIM_ID_PLAYER_JUMP_LOOP_R,
	ANIM_ID_PLAYER_JUMP_END_L,
	ANIM_ID_PLAYER_JUMP_END_R,
	ANIM_ID_COUNT
};

