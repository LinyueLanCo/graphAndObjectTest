#include "AnimationClipManager.h"

// 功能：根据资源管理器中已加载的图片创建当前关卡的动画片段表。
void AnimationClipManager::init(ResourceManager& resources)
{
    clips.clear();

    clips[ANIM_ID_PLAYER_IDLE_L] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_IDLE_L), 8, 3, true);

    clips[ANIM_ID_PLAYER_IDLE_R] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_IDLE_R), 8, 3, true);

    clips[ANIM_ID_PLAYER_WALK_L] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_WALK_L), 8, 3, true);

    clips[ANIM_ID_PLAYER_WALK_R] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_WALK_R), 8, 3, true);

    clips[ANIM_ID_PLAYER_RUN_L] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_RUN_L), 8, 3, true);

    clips[ANIM_ID_PLAYER_RUN_R] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_RUN_R), 8, 3, true);

    clips[ANIM_ID_PLAYER_JUMP_START_L] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_START_L), 8, 2, false);

    clips[ANIM_ID_PLAYER_JUMP_START_R] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_START_R), 8, 2, false);

    clips[ANIM_ID_PLAYER_JUMP_LOOP_L] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_LOOP_L), 8, 3, true);

    clips[ANIM_ID_PLAYER_JUMP_LOOP_R] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_LOOP_R), 8, 3, true);

    clips[ANIM_ID_PLAYER_JUMP_END_L] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_END_L), 8, 2, false);

    clips[ANIM_ID_PLAYER_JUMP_END_R] =
        AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_END_R), 8, 2, false);
}

// 功能：根据动画资源 ID 获取动画片段描述。
AnimationClip AnimationClipManager::getClip(AnimationId id)
{
    if (clips.find(id) == clips.end())
    {
        return AnimationClip();
    }

    return clips[id];
}
