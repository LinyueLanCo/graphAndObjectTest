#include "Animation.h"


// 功能：把动画表现状态转换为玩家动画资源 ID。
AnimationId getPlayerAnimationId(AnimationState state)
{
	if (state == ANIM_IDLE_L)
	{
		return ANIM_ID_PLAYER_IDLE_L;
	}

	if (state == ANIM_IDLE_R)
	{
		return ANIM_ID_PLAYER_IDLE_R;
	}

	if (state == ANIM_WALK_LEFT)
	{
		return ANIM_ID_PLAYER_WALK_L;
	}

	if (state == ANIM_WALK_RIGHT)
	{
		return ANIM_ID_PLAYER_WALK_R;
	}

	if (state == ANIM_RUN_LEFT)
	{
		return ANIM_ID_PLAYER_RUN_L;
	}

	if (state == ANIM_RUN_RIGHT)
	{
		return ANIM_ID_PLAYER_RUN_R;
	}
	if (state == ANIM_JUMP_START_L)
	{
		return ANIM_ID_PLAYER_JUMP_START_L;
	}

	if (state == ANIM_JUMP_START_R)
	{
		return ANIM_ID_PLAYER_JUMP_START_R;
	}

	if (state == ANIM_JUMP_LOOP_L)
	{
		return ANIM_ID_PLAYER_JUMP_LOOP_L;
	}

	if (state == ANIM_JUMP_LOOP_R)
	{
		return ANIM_ID_PLAYER_JUMP_LOOP_R;
	}

	if (state == ANIM_JUMP_END_L)
	{
		return ANIM_ID_PLAYER_JUMP_END_L;
	}

	if (state == ANIM_JUMP_END_R)
	{
		return ANIM_ID_PLAYER_JUMP_END_R;
	}
	return ANIM_ID_PLAYER_IDLE_L;
}

// 功能：根据动画资源组和动画表现状态，解析出资源层 AnimationId。
AnimationId getAnimationId(AnimationSetId setId, AnimationState state)
{
	if (setId == ANIM_SET_PLAYER1)
	{
		return getPlayerAnimationId(state);
	}
	else if (setId == ANIM_SET_PLAYER2)
	{
		return ANIM_ID_PLAYER2_STATIC;
	}
	else if (setId == ANIM_SET_PLAYER3)
	{
		return ANIM_ID_PLAYER3_STATIC;
	}
	else if (setId == ANIM_SET_PLAYER4)
	{
		return ANIM_ID_PLAYER4_STATIC;
	}
	else if (setId == ANIM_SET_COIN_GOLD)
	{
		return ANIM_ID_COIN_GOLD;
	}
	else if (setId == ANIM_SET_COIN_SILVER)
	{
		return ANIM_ID_COIN_SILVER;
	}
	else if (setId == ANIM_SET_COIN_COPPER)
	{
		return ANIM_ID_COIN_COPPER;
	}

	return ANIM_ID_COUNT;
}
