#pragma once
#pragma warning( disable : 4005 )

#define WORKING_DIR std::string("Resources/")


#define PRINT_ALL

       





#ifdef PRINT_ALL
    #define PRINT_BOARD_TO_CONSOLE 1        
    #define STEP_BY_STEP 1
    #define PRINT_WINNER_PER_ROUND 1

    #define PRINT_THINK_MAP 1
#else
    #define PRINT_BOARD_TO_CONSOLE 0        //prints the board after each step to the console (twice a round)
    #define STEP_BY_STEP 0                  //Waits for user input after each step (twice a round)
    #define PRINT_WINNER_PER_ROUND 0        //prints the winner of each round to the console

    #define PRINT_THINK_MAP 0               //prints the throughts of the AI's to the console
#endif


//Weitere improvements:
//
//  - gegnerische felder blockieren, wenn der Gegner mit dem n�chsten zug gewinnen k�nnte!
//
