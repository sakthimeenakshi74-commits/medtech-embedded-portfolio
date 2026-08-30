#include <stdio.h>
#include <stdint.h>

/* ========================================================================== */
/*                                 ENUMERATIONS                               */
/* ========================================================================== */

/* Defines all possible operating states of the infusion pump */
typedef enum{
    STATE_IDLE,       /* Pump is stopped and waiting */
    STATE_INFUSING,   /* Pump motor is actively delivering fluid */
    STATE_ALARM,      /* Safety hazard triggered; pump is securely halted */
    MAX_STATE         /* Row boundary marker for the dispatch lookup table */
}PumpState;

/* Defines all incoming events that can cause a state transition */
typedef enum{
    EV_START,          /* Command to begin infusion */
    EV_STOP,           /* Command to stop infusion */
    EV_FAULT,          /* Safety sensor alert (e.g., occlusion, air bubble) */
    EV_SILENCE_ALARM,  /* Command to clear or acknowledge an active alarm */
    MAX_EVENT          /* Column boundary marker for the dispatch lookup table */
}PumpEvent;

/* ========================================================================== */
/*                             FUNCTION POINTERS & HANDLERS                  */
/* ========================================================================== */

/* Function pointer type definition for state change actions */
typedef void (*StateAction) (void);

/* Concrete action handlers to execute during state transitions */
void handleStart(void){
    printf("[FSM] Action: Motor engaged. Infusion started.\n");
}
void handleStop(void){
    printf("[FSM] Action: Motor stopped. System idle.\n");
}
void handleFault(void){
    printf("[FSM] CRITICAL: System Fault! Sounding alarm.\n");
}
void handleAlarmSilence(void){
    printf("[FSM] Action: Alarm Silencing.\n");
}
void handleNoop(void){
    printf("[FSM] Warning: Event ignored or invalid for current state.\n");
}

/* ========================================================================== */
/*                            TRANSITION STRUCTURE & MATRIX                   */
/* ========================================================================== */

/* Structure representing a single cell rule inside the 2D matrix */
typedef struct{
    StateAction action;  /* Function to run upon triggering the transition */
    PumpState nextState; /* Target state the system will enter next */
}Transition_t;

/* The 2D Dispatch Table stored in Flash memory (ROM) via static const.
   Maps every State-Event pairing using modern C Designated Initializers. */
static const Transition_t DispatchMatrix[MAX_STATE][MAX_EVENT]={
    [STATE_IDLE]={
        [EV_START]={handleStart,STATE_INFUSING},
        [EV_STOP]={handleNoop,STATE_IDLE},
        [EV_FAULT]={handleFault,STATE_ALARM},
        [EV_SILENCE_ALARM]={handleAlarmSilence,STATE_IDLE}
    },
    [STATE_INFUSING]={
        [EV_START]={handleNoop,STATE_INFUSING},
        [EV_STOP]={handleStop,STATE_IDLE},
        [EV_FAULT]={handleFault,STATE_ALARM},
        [EV_SILENCE_ALARM]={handleAlarmSilence,STATE_IDLE}
    },
    [STATE_ALARM]={
        [EV_START]={handleNoop,STATE_ALARM},
        [EV_STOP]={handleNoop,STATE_ALARM},
        [EV_FAULT]={handleNoop,STATE_ALARM},
        [EV_SILENCE_ALARM]={handleAlarmSilence,STATE_IDLE}
    }
};

/* ========================================================================== */
/*                                CORE FSM ENGINE                             */
/* ========================================================================== */

/* The state transition engine. Resolves and runs rules instantly in O(1) time */
void ProcessEvent(PumpState *ActiveState,PumpEvent ActiveEvent){
    /* Look up the rule directly using state and event coordinates */
    Transition_t rule = DispatchMatrix[*ActiveState][ActiveEvent];
    
    /* Run the action if a valid handler function exists */
    if(rule.action!=NULL){
        rule.action();
    }
    
    /* Overwrite the persistent variable with the target next state */
    *ActiveState=rule.nextState;
}

/* ========================================================================== */
/*                                  TEST BENCH                                */
/* ========================================================================== */

int main()
{
    /* Initialize the system in an IDLE state */
    PumpState currentState=STATE_IDLE;
    printf("Initial State: IDLE\n\n");
    
    /* Test Case 1: Valid Start event from IDLE state */
    printf("Triggering: EV_START\n");
    ProcessEvent(&currentState,EV_START);
    printf("Current State: %d\n", currentState);
    
    /* Test Case 2: Invalid Start event from an already running state (INFUSING) */
    printf("Triggering: EV_START\n");
    ProcessEvent(&currentState,EV_START);
    printf("Current State: %d\n\n", currentState);
    
    /* Test Case 3: Critical hardware fault triggers while running */
    printf("Triggering: EV_FAULT\n");
    ProcessEvent(&currentState,EV_FAULT);
    printf("Current State: %d\n", currentState);
    
    /* Test Case 4: Recovering back to IDLE by silencing the alarm */
    printf("Triggering: EV_SILENCE_ALARM\n");
    ProcessEvent(&currentState,EV_SILENCE_ALARM);
    printf("Current State: %d\n", currentState);
}
