#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class I_Player;

struct GameSettings{
	I_Player* playerOne;
	I_Player* playerTwo;
};



class Menu
{
private:
	sf::RenderWindow* window;
	sf::RectangleShape button;

	bool startgame;
	void pollEvents();

public:
	explicit Menu(sf::RenderWindow* window);
	~Menu();

	//ich w�rd hier ein struct mit den playerdaten/spieleinstellungen 
	// zur�ckgeben und der gameloop zum initialisieren geben.
	//per value zur�ckgeben????
	GameSettings loop();


};

