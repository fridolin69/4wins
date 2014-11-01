#pragma once

class MeepleBag;
class BoardState;
class I_Player;



struct GameWinner{
    enum Enum{
        PLAYER_1,
        PLAYER_2,
        TIE
    };
};



//Contains all information about a game, and handles the game loop
class Game
{
private:
    MeepleBag* bag[2];
    BoardState* board;

    I_Player& player1;
    I_Player& player2;
public:
    Game(I_Player& player1, I_Player& player2); //Initialises the game with 2 players
    GameWinner::Enum runGame();                 //Runs the game, until it is over; returns the winner
    virtual ~Game();
};

