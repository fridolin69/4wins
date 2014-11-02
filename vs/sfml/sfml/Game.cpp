#include "Game.h"
#include "Meeple.h"
#include "MeepleBag.h"
#include "BoardState.h"
#include "I_Player.h"

#include <assert.h>
#include <string>
#include <iostream>


using namespace std;

Game::Game(I_Player& player1, I_Player& player2) : player1(player1), player2(player2){
    bag[0] = new MeepleBag(MeepleColor::WHITE);
    bag[1] = new MeepleBag(MeepleColor::BLACK);
    board = new BoardState();
}


Game::~Game(){
    delete bag[0];
    delete bag[1];
    delete board;
}



//Game Loop for one game, until there is a winner or the board is full
GameWinner::Enum Game::runGame(){
    cout << "Running the game..." << endl;
    for (;;){
        runGameCycle(player1, player2, 0);
        if (board->checkWinSituation()){    //player2 won
            return GameWinner::PLAYER_2;
        }

        runGameCycle(player2,player1, 1);
        if (board->checkWinSituation()){    //player1 won
            return GameWinner::PLAYER_1;
        }


        if (board->isFull()){
            return GameWinner::TIE;
        }
    }
}


//a have round cycle, where a player chooses a meeple, and the other player sets it
void Game::runGameCycle(I_Player& player, I_Player& opponent, int playerNr){   
    const Meeple& toSet = player.selectOpponentsMeeple(*bag[playerNr], *bag[(playerNr+1)%2], *board);
    cout << "Player " << playerNr+1 << " chose meeple \"" << toSet.toString() << '\"' << endl;
    Meeple* meeple = bag[(playerNr + 1) % 2]->removeMeeple(toSet);   //remove meeple from opponent's bag          
    
    BoardPos pos = opponent.selectMeeplePosition(*bag[(playerNr + 1) % 2], *bag[playerNr], *board, *meeple);
    assert(pos.x < 4 && pos.y < 4);
    cout << "Player " << (playerNr+1)%2+1 << " sets meeple to (" << static_cast<int>(pos.x) << 'x' << static_cast<int>(pos.y) << ')' << endl;
    board->setMeeple(pos, *meeple);   
    board->print(cout);   
        
    cin.ignore();
}