#pragma once
#include "I_Player.h"

//This player is controled by the user (with the mouse)
class Player : public I_Player{
public:
    virtual const Meeple& selectOpponentsMeeple(const MeepleBag& ownBag, const MeepleBag& opponentBag, const Board& board);
    virtual BoardPos selectMeeplePosition(const MeepleBag& ownBag, const MeepleBag& opponentBag, const Board& board, const Meeple& meepleToSet);
};

