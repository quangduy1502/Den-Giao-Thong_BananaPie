#include "scheduler.h"

typedef struct {
    void (*pTask)(void);
    uint32_t Delay;
    uint32_t Period;
    uint32_t TaskID;
} sTask;

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint8_t SCH_count = 0;
static uint32_t next_task_id = 1;
static volatile uint32_t tick_count;
static uint32_t last_schedule_tick;

static uint32_t Get_New_Task_ID(void) {
    uint32_t id = next_task_id++;
    if (next_task_id == NO_TASK_ID) next_task_id = 1;
    return id;
}

void SCH_Init(void) {
    SCH_count = 0;
    tick_count = 0;
    last_schedule_tick = 0;
}

void SCH_Update(void) {
    /* TIM2 only records ticks. The foreground owns the task queue. */
    tick_count++;
}

uint32_t SCH_Get_Tick(void) {
    return tick_count;
}

static void SCH_Apply_Elapsed_Ticks(void) {
    uint32_t now = SCH_Get_Tick();
    uint32_t elapsed = now - last_schedule_tick;
    last_schedule_tick = now;

    /* Spend the elapsed ticks across the queue, leaving due tasks at zero. */
    for (uint8_t i = 0; i < SCH_count && elapsed > 0; i++) {
        if (elapsed < SCH_tasks_G[i].Delay) {
            SCH_tasks_G[i].Delay -= elapsed;
            break;
        }
        elapsed -= SCH_tasks_G[i].Delay;
        SCH_tasks_G[i].Delay = 0;
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
    SCH_tasks_G[pos].TaskID = assignedID;

    SCH_count++;
    return assignedID;
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD) {
    SCH_Apply_Elapsed_Ticks();
    return SCH_Insert_Task(pFunction, DELAY, PERIOD, NO_TASK_ID);
}

static void SCH_Remove_Task(uint8_t pos) {
    if (pos + 1 < SCH_count) {
        SCH_tasks_G[pos + 1].Delay += SCH_tasks_G[pos].Delay;
    }

    for (uint8_t i = pos; i < SCH_count - 1; i++) {
        SCH_tasks_G[i] = SCH_tasks_G[i + 1];
    }

    SCH_count--;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (SCH_count == 0 || taskID == NO_TASK_ID) return 0;

    SCH_Apply_Elapsed_Ticks();
    uint8_t pos = 0;
    while (pos < SCH_count && SCH_tasks_G[pos].TaskID != taskID) {
        pos++;
    }
    if (pos == SCH_count) return 0;

    SCH_Remove_Task(pos);
    return 1;
}

void SCH_Dispatch_Tasks(void) {
    SCH_Apply_Elapsed_Ticks();
    while (SCH_count > 0 && SCH_tasks_G[0].Delay == 0) {
        sTask currentTask = SCH_tasks_G[0];
        uint32_t started = last_schedule_tick;
        SCH_Remove_Task(0);
        currentTask.pTask();
        SCH_Apply_Elapsed_Ticks();

        if (currentTask.Period > 0) {
            /* Resume at the next period after an overrun. */
            uint32_t elapsed = last_schedule_tick - started;
            uint32_t delay = currentTask.Period - (elapsed % currentTask.Period);
            SCH_Insert_Task(currentTask.pTask, delay, currentTask.Period, currentTask.TaskID);
        }
    }
}
