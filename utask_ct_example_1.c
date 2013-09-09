// Jakub Zdroik (c) 2013 MIT license
//
// utask_ct_example_1 program using uTaskCT
// 
// Program is mockup of 
// - processing input from ADC, 
// - processing commands from stdin 
// - controlling GSM modem 
// - dispaching long tasks (type to stdin: test 1000)
// - interaction between tasks (not sending via gsm while command in progress)
//
// !!!requires utask lib as dir in parrent directory!!!
// compilation:
// #gcc -I../utask ../utask/utask.c  utask_ct_example_1.c -o utask_ct_example_1 -lpthread 
// run:
// ./utask_ct_example_1

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "utask.h"
#include "utask_ct.h"

#include <signal.h>
#include <unistd.h>
#include <sys/select.h>

static int pipe_fds[2];

pthread_t timer_thread;
pthread_t stdin_read_thread;

volatile int run = 1;

#define TIPS 1000

#define D1S		TIPS
#define D3S		(3*D1S)
#define D5S		(5*D1S)
#define D500MS	(D1S/2)
#define D100MS  (D1S/10)
#define D10MS   (D1S/100)

// needed to run uTask 
// this is usually implemented as interrupt function on uC 
void* timer_thread_fun(void* arg)
{
	while (run)
	{
		utask_sleep_process();
		usleep(1e6 / TIPS);
	}
	return;
}

// this is helper to receive commands from STDIN
#define MAX_COMMAND_LENGTH 256

char command[MAX_COMMAND_LENGTH];
int command_size = 0;
int command_action = 0;

void* stdin_read_thread_fun(void* arg)
{

  char read_char;
  fd_set descriptor_set;
  FD_ZERO(&descriptor_set); 
  FD_SET(STDIN_FILENO, &descriptor_set); 
  FD_SET(pipe_fds[0], &descriptor_set);

  while(run)
  {
      if (select(FD_SETSIZE, &descriptor_set, NULL, NULL, NULL) < 0) {
          // select() error
      }

      if (FD_ISSET(STDIN_FILENO, &descriptor_set)) {
          // read byte from stdin
          if (command_action == 0){

          	read(STDIN_FILENO, &read_char, 1);

          	if (read_char == '\n'){

          		command_action = 1;

          	} else {

          		if (command_size < MAX_COMMAND_LENGTH){
		          	command[command_size] = read_char;
		          	command_size+=1;
		          	command[command_size] = 0;
	          	}

          	}
          }

      }

      //if (FD_ISSET(pipe_fds[0], &descriptor_set))
          // Special event. break
			// break;
  }

	return;
}


int ws_action = 0; 

// ADC task
void CT_task(task_adc){

	static int i;
	static int counter = 0;

	CT_start();

	i = 0;
	printf("ADC start %d \n", counter);

	CT(1);
	printf("get ADC[%d] \n",i);
	i +=1;

	if (i >= 10) { CT_goto(2); }
	CT_set_sleep(D100MS);
	CT_goto_previous(2);

	printf("ADC end \n");

	if (ws_action == 0 ){
		ws_action = 1;
	} else {
		printf("not sending data via gsm, some operation in progress\n");
	}

	counter += 1;
 	CT_set_sleep(D5S);

	CT_end_and_goto_start();
}


void CT_task(task_gsm_controller){

	CT_start();
		if(CT_is_timedout()) {
			printf("GSM IDLE timed out\n");
			CT_wait_neq(&ws_action, 0, D1S);
		} else { 
			CT_goto(1);
		}
	CT_goto_previous(1);

		printf("GSM POWER ON\n");
		ws_action = 2;
		CT_set_sleep(D1S);

	CT(2);

		printf("GSM SEND AT\n");
		CT_set_sleep(D3S);

	CT(3);

		printf("GSM GPRS CONNECT\n");
		CT_set_sleep(D3S);

	CT(4);

		printf("GSM TCP CONNECT\n");
		CT_set_sleep(D3S);

	CT(5);

		printf("GSM TCP SEND\n");
		CT_set_sleep(D3S);

	CT(6);

		printf("GSM POWER OFF\n");

		CT_set_sleep(D1S);
		ws_action = 0;

	CT_end_and_goto_start();

}

// Command dispach task
//help - list commands
//help [command] help about command 
//reset
void CT_task(task_command_dispach){

	static int i;

	CT_start();

		i = 0;
		command_action = 0;
  	command_size = 0 ;
  	command[command_size] = 0;

		CT_wait_neq(&command_action, 0, D1S);

	CT(1);

		if(CT_is_timedout()) {
			//printf("Command dispacher IDLE timed out\n");
			CT_goto(CT_START);
		} else {
			command_action = 2;
			if (sscanf(command, "%*s %d", &i) == 1){
				printf("command doing: %s \n",command);
				CT_goto(2);
			} else if(strncmp(command,"help",4) == 0){

				printf("help: \ntype: command [number]\n");
				CT_goto(CT_START);

			} else {
				printf("command error: %s \ntype help\n",command);
				CT_goto(CT_START);
			}
		}

	CT(2);

		printf("sub command i = %d \n", i);

		i -= 1;

		if ( i <=  0 ) {
			printf("sub command end\n");
			CT_goto(CT_START);
		}

	CT_set_sleep(D100MS);
	CT_goto_previous(4);
	CT_end_and_goto_start();
}


void signal_handler(int sig)
{
	fprintf(stderr,"Got signal!\n");
	run = 0;
}

void mcu_sleep(utask_timer_t s)
{
	if (s == 0)
		return;
	//	printf("%dT\n",s);
	usleep((1e6 * s / TIPS) / 2 + 1000);
}

int main()
{

	utask_init();

	signal(SIGINT,  signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);

  pipe(pipe_fds);

	pthread_create(&timer_thread,NULL,timer_thread_fun,NULL);
  pthread_create(&stdin_read_thread,NULL,stdin_read_thread_fun,NULL);

	utask_add(task_adc);
	utask_add(task_gsm_controller);
	utask_add(task_command_dispach);

	utask_put_mcu_to_sleep = mcu_sleep;
	
	while (run)
	{
		utask_schedule();
	}
	
	pthread_join(timer_thread,NULL);
	pthread_join(stdin_read_thread,NULL);

  close(pipe_fds[1]);

	return 0;
}

