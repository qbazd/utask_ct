uTaskCT
=======

uTaskCT, macros for uTask (utask.sourceforge.net) lib, simplifing tasks code.
uTaskCT aims to simplify writing algorythms in uTask.

uTask is very simple but efecive library for making async routines... feels like running things in parallel.
It's based on concept of cooroutines with finite state machines (FSM) in tasks.
It something like threads scheduler without nifty magick on CPU registers, stack, and so on, when switching context of threads.
It's multiplatform, multitasking without need of porting.


Simple example
==============

Simple task like written in pthreads:

```
void * one_time_task (void * x){
  int i;
  for (i =0; i< 10; i++){
    printf("one_time_task %d\n", i);
    sleep(1);
  }

  return NULL;
}
```

Above task written in uTaskCT:

```
void CT_task(one_time_task){
  static int i;

  CT_start; // start of thread FSM 

    i = 0; // init static var i 

  CT(1); // thread FSM label 1

    printf("one_time_task %d\n", i);
    i++;

    if (i < 10) {
      CT_sleep(1S); // set current thread sleep to 1S
      CT_goto(1); // goto thread FSM label 1 
    }

  CT_end_and_finish(2); // thread FSM label 2
}
```

Main diffrence is pthreads runs on full blown OS, uTaskCT runs where you do not have pthreads, ie: on 8bit uC.
The effect of above codes is compareble.

Digging into details one can found that running task in real scheduler ends up using mutexes, to access resources ie: stdout;
uTask allways runs one section of thread FSM atomicaly by design.

