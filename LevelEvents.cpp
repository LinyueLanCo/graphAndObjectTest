#include "LevelEvents.h"
#include "Level.h"
#include "Input.h"
#include <cmath>

void level1_InitEvent(Level& level)
{
    // 关卡特定逻辑：寻找 Checkpoint (旗杆) 并调整其碰撞箱偏移量
    auto& entityManager = level.getEntityManager();
    for (size_t idx : entityManager.getActiveIndices())
    {
        if (entityManager.getEntities()[idx].getEntityType() == CHECKPOINT)
        {
            entityManager.getEntities()[idx].getCollisionBox().setOffset(0.0, -30.0);
        }
    }
}

void level1_UpdateEvent(Level& level, InputManager& input)
{
    auto& entityManager = level.getEntityManager();
    auto& entities = entityManager.getEntities();
    auto& dialogueBox = level.getDialogueBox();
    auto& localizationManager = level.getLocalizationManager();

    auto& timerManager = level.getTimerManager();

    // 1. 事件联动：当旗子激活时，注册一个 180 帧 (3 秒) 的计时器
    for (size_t idx : entityManager.getActiveIndices())
    {
        Entity& ent = entities[idx];
        if (ent.getEntityType() == CHECKPOINT && ent.flagActivatedJustNow)
        {
            ent.flagActivatedJustNow = false; // 消费掉这个标记，重置它

            // 启动一个 180 帧 (大约 3 秒) 的计时器
            timerManager.setTimer("checkpoint_spawn_banana", 30.0);
        }
    }

    // 轮询检测触旗延迟计时器是否完成
    if (timerManager.isFinished("checkpoint_spawn_banana"))
    {
        // 寻找红旗物体的当前位置坐标来生成香蕉
        double spawnX = 1500.0;
        double spawnY = 200.0;
        for (size_t idx : entityManager.getActiveIndices())
        {
            if (entities[idx].getEntityType() == CHECKPOINT)
            {
                spawnX = entities[idx].getX();
                spawnY = entities[idx].getY() + 64.0;
                break;
            }
        }

        entityManager.queueSpawnEntity(
            spawnX,
            spawnY,
            "Banana"
        );

        // 伴随弹窗提示 CHECKPOINT REACHED. PROGRESS SAVED.
        std::string text = localizationManager.getString("checkpoint_hit");
        
        DialogueConfig config = DialogueConfig::Default();
        config.autoClose = true;
        config.autoCloseDuration = 30.0; // 3 秒后自动收回
        
        dialogueBox.startDialogue(text, config);
        dialogueBox.showDialogueWithOffset(0, 400);
    }

    // 2. 关卡临时交互事件：玩家站在旗杆附近按 E 键可以看路牌
    Entity* player = entityManager.getEntity(level.getControlledPlayerId());
    if (player)
    {
        Entity* checkpoint = nullptr;
        for (size_t idx : entityManager.getActiveIndices())
        {
            if (entities[idx].getEntityType() == CHECKPOINT)
            {
                checkpoint = &entities[idx];
                break;
            }
        }
        bool nearSign = false;
        if (checkpoint)
        {
            nearSign = (fabs(player->getX() - checkpoint->getX()) < 100.0);
        }

        if (nearSign)
        {
            if (input.isKeyPressed('E'))
            {
                if (dialogueBox.getState() == UI_HIDDEN)
                {
                    std::string text = localizationManager.getString("signpost_level1_intro");
                    dialogueBox.startDialogue(text);
                    dialogueBox.showDialogueWithOffset(0, 400);
                }
                else
                {
                    dialogueBox.advance();
                }
            }
        }
        else
        {
            // 远离后自动隐藏 —— 仅当当前对话框显示的是路牌警示语时才自动关闭
            if (dialogueBox.getState() != UI_HIDDEN && dialogueBox.getState() != UI_HIDING)
            {
                if (dialogueBox.getFullText() == localizationManager.getString("signpost_level1_intro"))
                {
                    dialogueBox.hideDialogueWithOffset(0, 400);
                }
            }
        }
    }
}
