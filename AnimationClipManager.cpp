#include "AnimationClipManager.h"

// 功能：根据资源管理器中已加载的图片创建当前关卡的动画片段表。
void AnimationClipManager::init(ResourceManager& resources)
{
    // 清空字典，释放以前保存的旧动画键值对
    clips.clear();

    // 如下直接使用 std::map::operator[] 关联各个动画 ID。
    // 在底层的红黑树结构中，如果 Key 不存在，它会自动分配一个新节点并调用默认构造，
    // 随后我们通过赋值，将具体的 AnimationClip 放入树节点中，复杂度均为 O(log N)。
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

    clips[ANIM_ID_PLAYER2_STATIC] =
        AnimationClip(resources.getImage2D(IMG_PLAYER2_STATIC), 1, 4, true);

    clips[ANIM_ID_PLAYER3_STATIC] =
        AnimationClip(resources.getImage2D(IMG_PLAYER3_STATIC), 1, 4, true);

    clips[ANIM_ID_PLAYER4_STATIC] =
        AnimationClip(resources.getImage2D(IMG_PLAYER4_STATIC), 1, 4, true);

    clips[ANIM_ID_COIN_GOLD] =
        AnimationClip(resources.getImage2D(IMG_COIN_GOLD), 5, 5, true);

    clips[ANIM_ID_COIN_SILVER] =
        AnimationClip(resources.getImage2D(IMG_COIN_SILVER), 5, 5, true);

    clips[ANIM_ID_COIN_COPPER] =
        AnimationClip(resources.getImage2D(IMG_COIN_COPPER), 5, 5, true);

    clips[ANIM_ID_APPLE] =
        AnimationClip(resources.getImage2D(IMG_APPLE), 17, 5, true);

    clips[ANIM_ID_BANANA] =
        AnimationClip(resources.getImage2D(IMG_BANANA), 17, 5, true);

    clips[ANIM_ID_MELON] =
        AnimationClip(resources.getImage2D(IMG_MELON), 17, 5, true);

    clips[ANIM_ID_ORANGE] =
        AnimationClip(resources.getImage2D(IMG_ORANGE), 17, 5, true);

    clips[ANIM_ID_PINEAPPLE] =
        AnimationClip(resources.getImage2D(IMG_PINEAPPLE), 17, 5, true);

    clips[ANIM_ID_STRAWBERRY] =
        AnimationClip(resources.getImage2D(IMG_STRAWBERRY), 17, 5, true);

    clips[ANIM_ID_KIWI] =
        AnimationClip(resources.getImage2D(IMG_KIWI), 17, 5, true);

    clips[ANIM_ID_CHERRY] =
        AnimationClip(resources.getImage2D(IMG_CHERRY), 17, 5, true);


    clips[ANIM_ID_COIN_COLLECTED] =
        AnimationClip(resources.getImage2D(IMG_COIN_COLLECTED), 6, 4, false);

    clips[ANIM_ID_CHECKPOINT_NO_FLAG] =
        AnimationClip(resources.getImage2D(IMG_CHECKPOINT_NO_FLAG), 1, 2, true);

    clips[ANIM_ID_CHECKPOINT_FLAG_OUT] =
        AnimationClip(resources.getImage2D(IMG_CHECKPOINT_FLAG_OUT), 26, 2, false);

    clips[ANIM_ID_CHECKPOINT_FLAG_IDLE] =
        AnimationClip(resources.getImage2D(IMG_CHECKPOINT_FLAG_IDLE), 10, 2, true);
    clips[ANIM_ID_ENDPOINT_IDLE] =
        AnimationClip(resources.getImage2D(IMAGE_ENDPOINT_IDLE), 1, 2, true);
    clips[ANIM_ID_ENDPOINT_PRESSED] =
        AnimationClip(resources.getImage2D(IMAGE_ENDPOINT_PRESSED), 8, 2, false);
}

// 功能：根据动画资源 ID 获取动画片段描述。
AnimationClip AnimationClipManager::getClip(AnimationId id)
{
    // 调用 std::map::find，在红黑树结构中进行对数级时间复杂度 O(log N) 的查找，
    // 快速验证想要获取的动画 ID 是否确实存在于字典中，避免直接使用 operator[] 导致意外插入空节点。
    if (clips.find(id) == clips.end())
    {
        return AnimationClip();
    }

    // 确定存在后，使用 operator[] 安全返回值。
    return clips[id];
}
