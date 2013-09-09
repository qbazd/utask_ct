// Jakub Zdroik (c) 2013 MIT license

#ifndef UTASK_CT_H
#define UTASK_CT_H

#define CT_START 0

// usage: void CT_task(test)
#define CT_task(name) name(utask_t *__current_task)

#define CT_start() \
  if (__current_task->istate == CT_START) {

#define CT(__ct_label) \
  { __current_task->istate = (__ct_label); return ;}} else if (__current_task->istate == (__ct_label)) {

#define CT_goto_previous(__ct_label) \
  ; return; } else if (__current_task->istate == (__ct_label)) {

#define CT_end_and_goto_start()  \
  {__current_task->istate = CT_START; return ;} } else {__current_task->istate = CT_START; return ;}

#define CT_end_and_finish() \
  utask_exit(__current_task); return ;}

#define CT_goto(__ct_label) \
  {__current_task->istate = (__ct_label); return;}

#define CT_quit() \
  {utask_exit(__current_task); return ;}

#define CT_is_timedout() \
  (__current_task->sleep == 0)

#define CT_set_sleep(_delay) \
  utask_sleep(__current_task, _delay)

#define CT_wait_for_change(_sem, _delay) \
  utask_wait_neq(__current_task, (_sem), *(_sem), (_delay));

#define CT_wait_eq(_sem, _value, _delay) \
  utask_wait_eq(__current_task, (_sem), (_value), (_delay));

#define CT_wait_neq(_sem, _value, _delay) \
  utask_wait_neq(__current_task, (_sem), (_value), (_delay));

#endif
