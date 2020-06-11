// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 2 starter file.
// Daniel Valvano
// February 20, 2016

#include <stdint.h>
#include "os.h"
#include "../inc/CortexM.h"
#include "../inc/BSP.h"

// function definitions in osasm.s
void StartOS(void);
//EDN: Prototype for the PTTTs
uint32_t Counter; //EDN

uint32_t task1Rate = 0;//rate for 1st task
uint32_t task2Rate = 0;//rate for 2nd task

void (*function1)(void); //Periodic function1, EDN pointer to user function
void (*function2)(void); //Periodic function2, EDN pointer to user function


//EDN: GLOBAL VARIABLES
uint32_t mail;  // shared data
int32_t signal=0; // semaphore
int32_t recv=0;  // semaphore
uint32_t lost=0; //Data lost when Event thread produces and it is not consumed


threadblkType threadblocks[NUMTHREADS];
threadblkType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];


// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: systick, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
  // initialize any global variables as needed
  
}

void SetInitialStack(int i){
  //***YOU IMPLEMENT THIS FUNCTION*****
threadblocks[i].sp = &Stacks[i][STACKSIZE-16]; // thread stack pointer 
  Stacks[i][STACKSIZE-1] = 0x01000000; // Thumb bit 
  Stacks[i][STACKSIZE-3] = 0x14141414; // R14 
  Stacks[i][STACKSIZE-4] = 0x12121212; // R12 
  Stacks[i][STACKSIZE-5] = 0x03030303; // R3 
  Stacks[i][STACKSIZE-6] = 0x02020202; // R2 
  Stacks[i][STACKSIZE-7] = 0x01010101; // R1 
  Stacks[i][STACKSIZE-8] = 0x00000000; // R0 
  Stacks[i][STACKSIZE-9] = 0x11111111; // R11 
  Stacks[i][STACKSIZE-10] = 0x10101010; // R10 
  Stacks[i][STACKSIZE-11] = 0x09090909; // R9 
  Stacks[i][STACKSIZE-12] = 0x08080808; // R8 
  Stacks[i][STACKSIZE-13] = 0x07070707; // R7 
  Stacks[i][STACKSIZE-14] = 0x06060606; // R6 
  Stacks[i][STACKSIZE-15] = 0x05050505; // R5 
  Stacks[i][STACKSIZE-16] = 0x04040404; // R4 
}

//******** OS_AddThreads ***************
// Add four main threads to the scheduler
// Inputs: function pointers to four void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*funcThread0)(void),
                  void(*funcThread1)(void),
                  void(*funcThread2)(void)){
// initialize TCB circular list
// initialize RunPt
// initialize four stacks, including initial PC
//***YOU IMPLEMENT THIS FUNCTION*****
int32_t status; 
  status = StartCritical(); 
  threadblocks[0].next = &threadblocks[1]; // 0 points to 1 
  threadblocks[1].next = &threadblocks[2]; // 1 points to 2 
  threadblocks[2].next = &threadblocks[3]; // 2 points to 0 
  //threadblocks[3].next = &threadblocks[0]; // 2 points to 0 
  
	SetInitialStack(0); Stacks[0][STACKSIZE-2] = (int32_t)(funcThread0); // PC
  SetInitialStack(1); Stacks[1][STACKSIZE-2] = (int32_t)(funcThread1); // PC
  SetInitialStack(2); Stacks[2][STACKSIZE-2] = (int32_t)(funcThread2); // PC
  //SetInitialStack(3); Stacks[3][STACKSIZE-2] = (int32_t)(funcThread3); // PC
  RunPt = &threadblocks[0];        // thread 0 will run first 
  EndCritical(status); 
																		
  return 1;               // successful
}

//******** OS_AddThreads3 ***************
// add three foregound threads to the scheduler
// This is needed during debugging and not part of final solution
// Inputs: three pointers to a void/void foreground tasks
// Outputs: 1 if successful, 0 if this thread can not be added
int OS_AddThreads3(void(*funcTask0)(void),
                 void(*funcTask1)(void),
                 void(*funcTask2)(void)){ 
// initialize TCB circular list (same as RTOS project)
// initialize RunPt
// initialize four stacks, including initial PC
  //***YOU IMPLEMENT THIS FUNCTION*****
  int32_t status; 
  status = StartCritical(); 
  threadblocks[0].next = &threadblocks[1]; // 0 points to 1 
  threadblocks[1].next = &threadblocks[2]; // 1 points to 2 
  threadblocks[2].next = &threadblocks[0]; // 2 points to 0 

  SetInitialStack(0); Stacks[0][STACKSIZE-2] = (int32_t)(funcTask0); // PC
  SetInitialStack(1); Stacks[1][STACKSIZE-2] = (int32_t)(funcTask1); // PC
  SetInitialStack(2); Stacks[2][STACKSIZE-2] = (int32_t)(funcTask2); // PC
  RunPt = &threadblocks[0];        // thread 0 will run first 
  EndCritical(status); 
									 
									 
  return 1;               // successful
}
                 
//******** OS_AddPeriodicEventThreads ***************
// Add two background periodic event threads
// Typically this function receives the highest priority
// Inputs: pointers to a void/void event thread function2
//         periods given in units of OS_Launch (Lab 2 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal


int OS_AddPeriodicEventThreads(void(*thread1)(void), uint32_t period1,
  void(*thread2)(void), uint32_t period2){
  //***YOU IMPLEMENT THIS FUNCTION*****
		function1=thread1;
		task1Rate=period1;
		function2=thread2;
		task2Rate=period2;
  return 1;
}


// runs every ms
uint32_t scheduleCounter =0;
void Scheduler(void){ // every time slice
  // run any periodic event threads if needed
  // implement round robin scheduler, update RunPt
  //***YOU IMPLEMENT THIS FUNCTION*****

scheduleCounter = scheduleCounter+1;
	if((scheduleCounter % task1Rate) == 0){ // every 1ms,
	(*function1)();
		//PeriodUserTask1();
	}
	if((scheduleCounter % task2Rate) == 0){ // every 100ms
	(*function2)();
	Counter = 0;
	}
	RunPt = RunPt->next;  // Round Robin
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
  //***YOU IMPLEMENT THIS FUNCTION*****
*semaPt = value;
}

// ******** OS_Wait ************
// Decrement semaphore
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *s){
  DisableInterrupts();
  while((*s) == 0){
    EnableInterrupts(); // interrupts can occur here
    DisableInterrupts();
  }
  (*s) = (*s) - 1;
  EnableInterrupts();
} 

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *s){
//***YOU IMPLEMENT THIS FUNCTION*****
DisableInterrupts();
  (*s) = (*s) + 1;
  EnableInterrupts();
}

// ******** OS_MailBox_Init ************
// Initialize communication channel
// Producer is an event thread, consumer is a main thread
// Inputs:  none
// Outputs: none
void OS_MailBox_Init(void){
  // include data field and semaphore
	mail = 0; //EDN:  MAke the data field to be zero, clear whatever is in there originally if there is
  //***YOU IMPLEMENT THIS FUNCTION*****
OS_InitSemaphore(&signal,0);  //EDN: meaning there is no data to be sent initially
}

// ******** OS_MailBox_Send ************
// Enter data into the MailBox, do not spin/block if full
// Use semaphore to synchronize with OS_MailBox_Recv
// Inputs:  data to be sent
// Outputs: none
// Errors: data lost if MailBox already has data
void OS_MailBox_Send(uint32_t data){
  //***YOU IMPLEMENT THIS FUNCTION*****
  mail = data;
  if(signal){
    lost++;
  }else{
    OS_Signal(&signal);
  }
}

// ******** OS_MailBox_Recv ************
// retreive mail from the MailBox
// Use semaphore to synchronize with OS_MailBox_Send
// Lab 2 spin on semaphore if mailbox empty
// Lab 3 block on semaphore if mailbox empty
// Inputs:  none
// Outputs: data retreived
// Errors:  none
uint32_t OS_MailBox_Recv(void){ uint32_t theData;
  //***YOU IMPLEMENT THIS FUNCTION*****  
  OS_Wait(&signal);
  theData = mail; // read mail
//  OS_Signal(&Ack);  //EDN: Since Task 1 cannot spin, this signal is actually useless for task one, hence it *Ack is will just be accumulating for nothing, except if it is called by another Task that can spin 
  return theData;
}
