#include "Os.h"
#include "Os_Arch.h"

static volatile TickType Os_Counter_SysTick_Value = 0u;

typedef struct
{
  uint8_t active;
  TickType expiry;
  TickType cycle;
} Os_Alarm_t;
static Os_Alarm_t Os_Alarm_10ms = {0u, 0u, 0u};
static AppModeType Os_AppMode = OSDEFAULTAPPMODE;

typedef void (*Os_AlarmCallback)(void);
static Os_AlarmCallback Os_Alarm_10ms_cb = 0;

__attribute__((weak)) void StartupHook(void) {}
__attribute__((weak)) void ShutdownHook(StatusType error) { (void)error; }
__attribute__((weak)) void ErrorHook(StatusType error) { (void)error; }

void DisableAllInterrupts(void) { __asm volatile("cpsid i"); }
void EnableAllInterrupts(void) { __asm volatile("cpsie i"); }

StatusType StartOS(AppModeType mode)
{
  Os_AppMode = mode;

  DisableAllInterrupts();
  Os_Counter_SysTick_Value = 0u;
  Os_Alarm_10ms.active = 0u;
  Os_Alarm_10ms.expiry = 0u;
  Os_Alarm_10ms.cycle = 0u;

  StartupHook();

  Os_Arch_InitTick((uint32_t)OS_TICK_HZ);
  (void)SetRelAlarm(ALARM_10MS, (TickType)OS_10MS_TICKS, (TickType)OS_10MS_TICKS);

  EnableAllInterrupts();
  return E_OK;
}

void ShutdownOS(StatusType error)
{
  DisableAllInterrupts();
  (void)CancelAlarm(ALARM_10MS);
  ShutdownHook(error);
}

StatusType Os_ConnectAlarm(AlarmType alarm, void (*cb)(void))
{
  if (alarm != ALARM_10MS)
    return E_OS_ID;
  DisableAllInterrupts();
  Os_Alarm_10ms_cb = cb;
  EnableAllInterrupts();
  return E_OK;
}
StatusType Os_DisconnectAlarm(AlarmType alarm)
{
  if (alarm != ALARM_10MS)
    return E_OS_ID;
  DisableAllInterrupts();
  Os_Alarm_10ms_cb = 0;
  EnableAllInterrupts();
  return E_OK;
}

StatusType GetCounterValue(CounterType counter, TickRefType value)
{
  if (!value)
    return E_OS_VALUE;
  if (counter != COUNTER_SYSTICK)
    return E_OS_ID;
  DisableAllInterrupts();
  *value = Os_Counter_SysTick_Value;
  EnableAllInterrupts();
  return E_OK;
}

StatusType IncrementCounter(CounterType counter)
{
  if (counter != COUNTER_SYSTICK)
    return E_OS_ID;
  Os_Counter_SysTick_Value++;
  return E_OK;
}

StatusType SetRelAlarm(AlarmType alarm, TickType offset, TickType cycle)
{
  if (alarm != ALARM_10MS)
    return E_OS_ID;
  if (offset == 0u)
    return E_OS_VALUE;
  DisableAllInterrupts();
  Os_Alarm_10ms.active = 1u;
  Os_Alarm_10ms.cycle = cycle;
  Os_Alarm_10ms.expiry = Os_Counter_SysTick_Value + offset;
  EnableAllInterrupts();
  return E_OK;
}

StatusType CancelAlarm(AlarmType alarm)
{
  if (alarm != ALARM_10MS)
    return E_OS_ID;
  DisableAllInterrupts();
  Os_Alarm_10ms.active = 0u;
  Os_Alarm_10ms.cycle = 0u;
  Os_Alarm_10ms.expiry = 0u;
  EnableAllInterrupts();
  return E_OK;
}

void Os_TickISR(void)
{
  (void)IncrementCounter(COUNTER_SYSTICK);

  if (Os_Alarm_10ms.active != 0u)
  {
    TickType now = Os_Counter_SysTick_Value;
    if ((TickType)(now - Os_Alarm_10ms.expiry) < (TickType)0x80000000u)
    {
      if (Os_Alarm_10ms_cb)
      {
        Os_Alarm_10ms_cb();
      }
      if (Os_Alarm_10ms.cycle != 0u)
      {
        Os_Alarm_10ms.expiry += Os_Alarm_10ms.cycle;
      }
      else
      {
        Os_Alarm_10ms.active = 0u;
      }
    }
  }
}
