#include "scheduler.h"

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint8_t SCH_count = 0;
static uint32_t next_task_id = 1;

static uint32_t Get_New_Task_ID(void) {
    uint32_t id = next_task_id++;
    if (next_task_id == NO_TASK_ID) next_task_id = 1;
    return id;
}

void SCH_Init(void) {
    SCH_count = 0;
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask = 0;
        SCH_tasks_G[i].Delay = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe = 0;
        SCH_tasks_G[i].TaskID = NO_TASK_ID;
    }
}

/* Bắt buộc O(1): chỉ xử lý task đầu danh sách */
void SCH_Update(void) {
    if (SCH_count == 0) return;
    if (SCH_tasks_G[0].Delay > 0) {
        SCH_tasks_G[0].Delay--;
    }
    if (SCH_tasks_G[0].Delay == 0) {
        SCH_tasks_G[0].RunMe = 1;
    }
}

static uint32_t SCH_Insert_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD, uint32_t existingID) {
    if (SCH_count >= SCH_MAX_TASKS || pFunction == 0) {
        return NO_TASK_ID;
    }

    uint8_t pos = 0;
    uint32_t rem_delay = DELAY;

    while (pos < SCH_count && rem_delay >= SCH_tasks_G[pos].Delay) {
        rem_delay -= SCH_tasks_G[pos].Delay;
        pos++;
    }

    for (uint8_t i = SCH_count; i > pos; i--) {
        SCH_tasks_G[i] = SCH_tasks_G[i - 1];
    }

    if (pos < SCH_count) {
        SCH_tasks_G[pos + 1].Delay -= rem_delay;
    }

    uint32_t assignedID = (existingID != NO_TASK_ID) ? existingID : Get_New_Task_ID();
    SCH_tasks_G[pos].pTask = pFunction;
    SCH_tasks_G[pos].Delay = rem_delay;
    SCH_tasks_G[pos].Period = PERIOD;
    SCH_tasks_G[pos].RunMe = (pos == 0 && rem_delay == 0) ? 1 : 0;
    SCH_tasks_G[pos].TaskID = assignedID;

    SCH_count++;
    return assignedID;
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD) {
    return SCH_Insert_Task(pFunction, DELAY, PERIOD, NO_TASK_ID);
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (SCH_count == 0 || taskID == NO_TASK_ID) return 0;

    uint8_t pos = 0;
    while (pos < SCH_count && SCH_tasks_G[pos].TaskID != taskID) {
        pos++;
    }
    if (pos == SCH_count) return 0;

    if (pos + 1 < SCH_count) {
        SCH_tasks_G[pos + 1].Delay += SCH_tasks_G[pos].Delay;
    }

    for (uint8_t i = pos; i < SCH_count - 1; i++) {
        SCH_tasks_G[i] = SCH_tasks_G[i + 1];
    }

    SCH_count--;
    SCH_tasks_G[SCH_count].pTask = 0;
    SCH_tasks_G[SCH_count].TaskID = NO_TASK_ID;
    return 1;
}

void SCH_Dispatch_Tasks(void) {
    while (SCH_count > 0 && SCH_tasks_G[0].RunMe > 0) {
        sTask currentTask = SCH_tasks_G[0];

        for (uint8_t i = 0; i < SCH_count - 1; i++) {
            SCH_tasks_G[i] = SCH_tasks_G[i + 1];
        }
        SCH_count--;
        SCH_tasks_G[SCH_count].pTask = 0;
        SCH_tasks_G[SCH_count].TaskID = NO_TASK_ID;

        if (SCH_count > 0 && SCH_tasks_G[0].Delay == 0) {
            SCH_tasks_G[0].RunMe = 1;
        }

        if (currentTask.pTask != 0) {
            (*currentTask.pTask)();
        }

        if (currentTask.Period > 0) {
            SCH_Insert_Task(currentTask.pTask, currentTask.Period, currentTask.Period, currentTask.TaskID);
        }
    }
}
