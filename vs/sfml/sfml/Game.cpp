#include "Game.h"

#include <vector>
#include <assert.h>
#include <string>
//#include <iostream>
#include <math.h>

#include <SFML/System.hpp>
#include <SFML/System/Clock.hpp>

#include "Meeple.h"
#include "MeepleBag.h"
#include "Board.h"

#include "RBoard.h"
#include "RBag.h"
#include "RMeeple.h"

#include "I_Player.h"
#include "ThreadController.h"

#include "config.h"
#include "helper.h"

#include "ParticleSystem.h"
#include "ResourceLoader.h"
//
//const sf::Color STANDARD_GLOW = sf::Color::Yellow;
//const sf::Color SELECTED_GLOW = sf::Color::Red;


Game::Game(sf::RenderWindow& window, Player& player1, Player& player2, ResourceLoader& resourceLoader)
	: window(&window), resourceLoader(&resourceLoader)
	, runGameSwitch(true), pressedLeftMouse(false)
	, releasedLeftMouse(false), rMeepleToSet(nullptr)
	, activePlayerIndex(0), STANDARD_GLOW(sf::Color::Yellow)
	, SELECTED_GLOW(sf::Color::Red), drawEndScreen(false)
	, hoveredButtonPtr(nullptr), buttonColor(sf::Color(119, 110, 0, 255))
	{
    
	assert(&player1 != &player2);   //hehehe, that won't crash this game

	players[0] = &player1;
	players[0]->logicalMeepleBag = new MeepleBag(MeepleColor::WHITE);
	players[0]->rbag = new RBag();

	players[1] = &player2;
	players[1]->logicalMeepleBag = new MeepleBag(MeepleColor::BLACK);
	players[1]->rbag = new RBag();

	logicalBoard = new Board();	

	board = new RBoard(*resourceLoader.getTexture(ResourceLoader::BOARD_TEX)
		, *resourceLoader.getTexture(ResourceLoader::FIELD_TEX)
		, *resourceLoader.getTexture(ResourceLoader::FIELD_TEX));

    gameStates[0] = new GameState(players[0]->logicalMeepleBag, players[1]->logicalMeepleBag, logicalBoard);
    gameStates[1] = new GameState(players[1]->logicalMeepleBag, players[0]->logicalMeepleBag, logicalBoard);
    
	initMeeples();

	color4MGlow[0] = 0.1f;
	color4MGlow[1] = 0.25f;
	color4MGlow[2] = 0.35f;
	color4MGlow[3] = 0.45f;

	sf::Vector2f buttonSize(150.f, 150.f);
	sf::Vector2f buttonOrigin(75.f, 75.f);


	restartButton.setTexture(resourceLoader.getTexture(ResourceLoader::RELOAD_BTN_TEX));
	restartButton.setFillColor(buttonColor);
	restartButton.setSize(buttonSize);
	restartButton.setOrigin(buttonOrigin);
	restartButton.setPosition(WINDOW_WIDTH_TO_CALCULATE / 2.f -90.f , WINDOW_HEIGHT_TO_CALCULATE / 2.f);


	exitButton.setTexture(resourceLoader.getTexture(ResourceLoader::EXIT_BTN_TEX));
	exitButton.setFillColor(buttonColor);
	exitButton.setSize(buttonSize);
	exitButton.setOrigin(buttonOrigin);
	exitButton.setPosition(WINDOW_WIDTH_TO_CALCULATE / 2.f +90.f, WINDOW_HEIGHT_TO_CALCULATE / 2.f);
	


}


Game::~Game(){
    delete gameStates[1];
    delete gameStates[0];
	delete players[1]->logicalMeepleBag;
	delete players[0]->logicalMeepleBag;
	delete logicalBoard;
	delete board;				//	deletes all fields
	delete players[1]->rbag;	//	deletes all rendermeeples for p2
	delete players[0]->rbag;	//	deletes all rendermeeples for p1
}


void Game::reset(){
	players[0]->logicalMeepleBag->reset();
	players[1]->logicalMeepleBag->reset();

	logicalBoard->reset();
	players[0]->rbag->reset();
	players[1]->rbag->reset();
}



//Game Loop for one game, until there is a winner or the board is full
void Game::runGame(){
	background.setTexture(resourceLoader->getTexture(ResourceLoader::BACKGROUND_TEX));
	background.setSize(sf::Vector2f(static_cast<float>(WINDOW_WIDTH_TO_CALCULATE), static_cast<float>(WINDOW_HEIGHT_TO_CALCULATE)));
	background.setPosition(0, 0);

	sf::Clock clock;
    float elapsedTime = 0;
	
	meepleToSet = nullptr;
	glowMeepleTmp = nullptr;
	rMeepleToSet = nullptr;
	posMeepleTo = { 42, 42 };
	dragMeeple = false;

	LoopState loopState = INIT_STATE;



	particleSystem = new ParticleSystem(*resourceLoader->getTexture(ResourceLoader::STAR_SPRITE_TEX), sf::Vector2u(2, 2));


    ////FOR DUST-CLOUDS:
    //ParticleBuilder* builder = new ParticleBuilder({ 300, 300 }, { 5, 30 }, { 50, 150 }, { 290, 320 }, { 500, 2000 }, { 300, 500 });
    //builder->setRotation();
    //builder->setGravity(120, 90);
    //particleSystem->newParticleCloud(20, *builder);
    
    ////FOR MOUSE-CLICKS:
    //ParticleBuilder* mbBuilder = new ParticleBuilder({ 300, 300 }, { 5, 30 }, { 50, 150 });
    //mbBuilder->setRotation({ 0.1, 3.5 });
    //mbBuilder->setGravity(120, 90);

    endScreenParticleBuilder = new ParticleBuilder({ 0, static_cast<float>(WINDOW_HEIGHT_TO_CALCULATE) }, { 5, 30 });
    endScreenParticleBuilder->setPath({ 10, 200 }, { 275, 350 })
                            ->setGravity(30)
                            ->setRotation({ 100, 600 }, { -1, 3 })
                            ->setFadeoutSpeed({ 35, 65 });      
        

	while (runGameSwitch && window->isOpen()){
        elapsedTime = clock.getElapsedTime().asSeconds();
	    float fps = 1.f / elapsedTime;
		clock.restart();

		//text.setFont(font);
		//text.setString("");
		//text.setCharacterSize(50); // in pixels, not points!
		//text.setColor(sf::Color::Black);
		//text.setStyle(sf::Text::Bold /*| sf::Text::Underlined*/);
		
       /* mbBuilder->setPosition(convertedMousePos);
        if (pressedLeftMouse){
            particleSystem->newParticleCloud(5, *mbBuilder);
        }*/
        
		pollEvents();

		switch (loopState)
		{
		case INIT_STATE:
			//todo das stimmt no ned ganz human iplayer und tc !!!!
			loopState = players[activePlayerIndex]->type == Player::HUMAN ? HUMAN_SELECT_MEEPLE : TC_START_SELECT_MEEPLE;
			//goto ?
			break;

		case I_PLAYER_SELECT_MEEPLE:
			loopState = i_playerSelectMeeple();
			//todo: goto??
			break;

		case HUMAN_SELECT_MEEPLE:
			loopState =  humanSelectMeeple();
			break;

		case TC_START_SELECT_MEEPLE:
			loopState = tcStartSelectMeeple();
			break;

		case TC_WAIT_FOR_SELECTED_MEEPLE:
			loopState = tcWaitForSelectedMeeple();
			break;
			//fall through not possible

		case HIGHLIGHT_SELECTED_MEEPLE:
			loopState = highlightSelectedMeeple();
			break;

		case I_PLAYER_SELECT_MEEPLE_POSITION:
			loopState = i_playerSelectMeeplePosition();
			//todo: goto??
			break;

		case HUMAN_SELECT_MEEPLE_POSITION:
			loopState = humanSelectMeeplePosition();
			break;

		case TC_START_SELECT_MEEPLE_POSITION:
			loopState = tcStartSelectMeeplePosition();
			//no .... intentional fall through
			break;

		case TC_WAIT_FOR_SELECTED_MEEPLE_POSITION:
			loopState = tcWaitForSelectedMeeplePosition();
			//goto?
			break;
		case MOVE_MEEPLE_TO_SELECTED_POSITION:
			loopState = MoveMeepleToSelectedPosition();
			break;

		case CHECK_END_CONDITION:
			loopState = checkEndCondition();
			break;

		case DISPLAY_END_SCREEN:
			loopState = displayEndscreen();
			break;
		}

		std::string title("4Wins by Jakob M., Sebastian S. and Simon D.   @");
		title.append(std::to_string(fps));
		title.append(" fps");
		window->setTitle(title);

		window->clear(sf::Color::White);
		window->draw(background);
		
		board->draw(*window);

		//window->draw(text);
             
		std::sort(meeplesToDrawAndSort.begin(), meeplesToDrawAndSort.end(), [](RMeeple* a, RMeeple* b){return a->getYPos() < b->getYPos(); });
		for (std::vector<RMeeple*>::iterator it = meeplesToDrawAndSort.begin(); it != meeplesToDrawAndSort.end(); ++it){
			(*it)->draw(*window);
		}

		particleSystem->update(elapsedTime);
		particleSystem->draw(*window);

		if (drawEndScreen)
		{
			window->draw(exitButton);
			window->draw(restartButton);
		}

		window->display();
	}

    delete endScreenParticleBuilder;

}

sf::Color Game::rainbow(float progress) const
{
	/*convert to rainbow RGB*/
	float a = (1.f - progress) / 0.2f;
	uint8_t X = static_cast<uint8_t>(floor(a));
	float Y = floor(255 * (a - X));
	switch (X)
	{
	case 0:
		return sf::Color(255, static_cast<uint8_t>( Y), 0, 255);

	case 1:
		return sf::Color(static_cast<uint8_t>(255 - Y) , 255, 0, 255);

	case 2:
		return sf::Color(0, 255, static_cast<uint8_t>(Y), 255);

	case 3:
		return	sf::Color(0, static_cast<uint8_t>(255 - Y), 255, 255);

	case 4:
		return sf::Color(static_cast<uint8_t>(Y), 0, 255, 255);

	case 5:
		return sf::Color(255, 0, 255, 255);
	default:
		return sf::Color::Blue;
	}
}

//return meeple pos and then set i
void Game::initMeeples(){
	//cout << "init meeples" << endl;
	for (int i = 0; i < 16; ++i){

		unsigned int meepleIndex;
		unsigned int bagInd = 0;
		if (i < 8){
			meepleIndex = i;
		}
		else{
			meepleIndex = i - 8;
			bagInd = 1;
		}
		const Meeple* meeple = players[bagInd]->logicalMeepleBag->getMeeple(meepleIndex);

		RMeeple* rmeeple = new RMeeple(*meeple,
			*resourceLoader->getTexture(ResourceLoader::MEEPLE_SPRITE),
			*resourceLoader->getTexture(ResourceLoader::MEEPLE_GLOW_SPRITE));

		meeplesToDrawAndSort.push_back(rmeeple);
		players[bagInd]->rbag->addRMeeple(rmeeple);
	}
}






void Game::pollEvents(){
	pressedLeftMouse = false;
	releasedLeftMouse = false;

	sf::Event event;
	sf::Vector2i mousepos = sf::Mouse::getPosition(*window);
	convertedMousePos = window->mapPixelToCoords(mousepos);
	while (window->pollEvent(event)){
		switch (event.type){

			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left){
					pressedLeftMouse = true;
				}
				break;

			case sf::Event::MouseButtonReleased:
				switch (event.mouseButton.button){
				case sf::Mouse::Left:
					releasedLeftMouse = true;
					break;

				default:
					break;
				}
				break;

			case sf::Event::MouseMoved:
				convertedMousePos = window->mapPixelToCoords(mousepos);
				break;

			case sf::Event::Resized:
				handleResizeWindowEvent(window);
				break;

			case sf::Event::Closed:
				window->close();
				break;

			default:
				break;
		}
	}
}


void Game::switchPlayers(){
	++activePlayerIndex;
	activePlayerIndex %= 2;
}

Game::LoopState Game::i_playerSelectMeeple(){
	assert(players[activePlayerIndex]->type == Player::I_PLAYER);
	meepleToSet = &players[activePlayerIndex]->player->selectOpponentsMeeple(*gameStates[activePlayerIndex]);
	return HIGHLIGHT_SELECTED_MEEPLE;
}
Game::LoopState Game::humanSelectMeeple(){
		
			assert(players[activePlayerIndex]->type == Player::HUMAN);
			//setString(std::string("Player " + std::to_string(activePlayerIndex + 1) + " choose Meeple"));

			if (releasedLeftMouse){
				rMeepleToSet = players[(activePlayerIndex + 1) % 2]->rbag->getRMeepleAtPosition(convertedMousePos);
				if (rMeepleToSet != nullptr)
				{
					rMeepleToSet->setGlow(&SELECTED_GLOW);
					sf::Color yellow(sf::Color::Yellow);
					rMeepleToSet->setGlow(&yellow);
					meepleToSet = rMeepleToSet->getLogicalMeeple();

					switchPlayers();
					glowMeepleTmp = nullptr;
					if (players[activePlayerIndex]->type == Player::TC){
						return TC_START_SELECT_MEEPLE_POSITION;
					}
					else if (players[activePlayerIndex]->type == Player::HUMAN) {
						return HUMAN_SELECT_MEEPLE_POSITION;
					}
					else{
						return I_PLAYER_SELECT_MEEPLE_POSITION;
					}
				}
			}
			if (glowMeepleTmp != nullptr)
			{
				glowMeepleTmp->setGlow(nullptr);
			}
			glowMeepleTmp = players[(activePlayerIndex + 1) % 2]->rbag->getRMeepleAtPosition(convertedMousePos);
			if (glowMeepleTmp != nullptr)
			{
				glowMeepleTmp->setGlow(&STANDARD_GLOW);
			}
			return HUMAN_SELECT_MEEPLE;

}
Game::LoopState Game::tcStartSelectMeeple(){
	assert(players[activePlayerIndex]->type == Player::TC);
	players[activePlayerIndex]->controller->run_selectOpponentsMeeple(*gameStates[activePlayerIndex]);
	return TC_WAIT_FOR_SELECTED_MEEPLE;
}
Game::LoopState Game::tcWaitForSelectedMeeple(){
	assert(players[activePlayerIndex]->type == Player::TC);
	if (!players[activePlayerIndex]->controller->isResultAvailable()){
		return TC_WAIT_FOR_SELECTED_MEEPLE;
	}
	meepleToSet = &players[activePlayerIndex]->controller->getOpponentsMeeple();
	return HIGHLIGHT_SELECTED_MEEPLE;
}
Game::LoopState Game::highlightSelectedMeeple(){
	assert(players[activePlayerIndex]->type == Player::TC);
	assert(meepleToSet != nullptr);

	switchPlayers();

	rMeepleToSet = players[activePlayerIndex]->rbag->isPassedMeepleInUnused(meepleToSet);
	assert(rMeepleToSet != nullptr);
	rMeepleToSet->setGlow(&SELECTED_GLOW);
	//highlight meeple (with glow animation? light -> heavy glow)
	//search meeple in meeplebag  -> glow

	//===todo auf 
	return players[activePlayerIndex]->type == Player::HUMAN ? HUMAN_SELECT_MEEPLE_POSITION : TC_START_SELECT_MEEPLE_POSITION;

}
Game::LoopState Game::i_playerSelectMeeplePosition(){
	assert(players[activePlayerIndex]->type == Player::I_PLAYER);
	posMeepleTo = players[activePlayerIndex]->player->selectMeeplePosition(*gameStates[activePlayerIndex], *meepleToSet);
	return MOVE_MEEPLE_TO_SELECTED_POSITION;
}
Game::LoopState Game::humanSelectMeeplePosition(){
	assert(players[activePlayerIndex]->type == Player::HUMAN);
	assert(rMeepleToSet != nullptr);
	//clicked meeple -> start to drag
	if (pressedLeftMouse && rMeepleToSet->containsPosition(convertedMousePos)){
		lastValidPosition = rMeepleToSet->getPosition();
		mousePosRelativeToMeepleBoundary = rMeepleToSet->getMousePosRelativeToMeepleBoundary(convertedMousePos);
		dragMeeple = true;
	}

	if (dragMeeple){ // todo checken ob !releaseleftmous braucht
		sf::Vector2f test(convertedMousePos.x - mousePosRelativeToMeepleBoundary.x, convertedMousePos.y - mousePosRelativeToMeepleBoundary.y);
		rMeepleToSet->setPosition(test);
		sf::Vector2f lookupPos = rMeepleToSet->getGlobalOrigin();
		board->setHoveredField(board->getBoardPosForPosititon(lookupPos));
	}

	if (releasedLeftMouse && dragMeeple){
		dragMeeple = false;
		sf::Vector2f lookupPos = rMeepleToSet->getGlobalOrigin();
		BoardPos pos = board->getBoardPosForPosititon(lookupPos);
		if ((pos.x < 4 && pos.y < 4) && logicalBoard->isFieldEmpty(pos)){

			sf::FloatRect fieldBounds = board->getFieldGlobalBounds(pos);
			sf::Vector2f newPosition(fieldBounds.left + fieldBounds.width / 2.f, fieldBounds.top + fieldBounds.height / 2.f);
			rMeepleToSet->setPosition(newPosition);


			players[activePlayerIndex]->rbag->changeRMeepleToUsed(*rMeepleToSet);

			Meeple* placeMe = players[activePlayerIndex]->logicalMeepleBag->removeMeeple(*meepleToSet);
			logicalBoard->setMeeple(pos, *placeMe);
			rMeepleToSet->setGlow(nullptr);
			rMeepleToSet = nullptr;
			board->setHoveredField({ 42, 42 });
			return CHECK_END_CONDITION;
		}
		else{
			rMeepleToSet->setPosition(lastValidPosition);
		}
	}
	return HUMAN_SELECT_MEEPLE_POSITION;
}
Game::LoopState Game::tcStartSelectMeeplePosition(){
	assert(players[activePlayerIndex]->type == Player::TC);
	players[activePlayerIndex]->controller->run_selectMeeplePosition(*gameStates[activePlayerIndex], *meepleToSet);
	return TC_WAIT_FOR_SELECTED_MEEPLE_POSITION;

}
Game::LoopState Game::tcWaitForSelectedMeeplePosition(){
	assert(players[activePlayerIndex]->type == Player::TC);
	if (!players[activePlayerIndex]->controller->isResultAvailable()){
		return TC_WAIT_FOR_SELECTED_MEEPLE_POSITION;
	}
	posMeepleTo = players[activePlayerIndex]->controller->getMeeplePosition();

	return MOVE_MEEPLE_TO_SELECTED_POSITION;

}
Game::LoopState Game::MoveMeepleToSelectedPosition(){
	assert(players[activePlayerIndex]->type == Player::TC);
	assert(rMeepleToSet != nullptr);
	assert(posMeepleTo.x < 4 && posMeepleTo.y < 4);

	sf::FloatRect fieldBounds = board->getFieldGlobalBounds(posMeepleTo);
	sf::Vector2f newPosition(fieldBounds.left + fieldBounds.width / 2.f, fieldBounds.top + fieldBounds.height / 2.f);

	rMeepleToSet->setPosition(newPosition);
	rMeepleToSet->setGlow(nullptr);

	Meeple* placeMe = players[activePlayerIndex]->logicalMeepleBag->removeMeeple(*meepleToSet);
	logicalBoard->setMeeple(posMeepleTo, *placeMe);

	players[activePlayerIndex]->rbag->changeRMeepleToUsed(*rMeepleToSet);

	return CHECK_END_CONDITION;

}
Game::LoopState Game::checkEndCondition(){
	#if PRINT_BOARD_TO_CONSOLE
		std::cout << "Player " << activePlayerIndex + 1 << " chose meeple \"" << meepleToSet->toString() << '\"' << std::endl;
		std::cout << "Player " << (activePlayerIndex + 1) % 2 + 1 << " sets meeple to " << meepleToSet->toString() << std::endl;
		logicalBoard->print(std::cout);
	#endif
	#if PRINT_WINNER_PER_ROUND
		std::cout << "Player " << activePlayerIndex + 1 << " wins!" << std::endl;
	#endif

	meepleToSet = nullptr;
	rMeepleToSet = nullptr;

	const WinCombination* combi = logicalBoard->checkWinSituation();
	if (combi != nullptr){    //player2 won
		for (uint8_t i = 0; i < 4; ++i){
			RMeeple* temp = players[activePlayerIndex]->rbag->isPassedMeepleInUsed(combi->meeples[i]);
			if (temp == nullptr)
			{
				winningCombiRMeeples[i] = players[(activePlayerIndex + 1) % 2]->rbag->isPassedMeepleInUsed(combi->meeples[i]);
			}
			else
			{
				winningCombiRMeeples[i] = temp;
			}
			assert(winningCombiRMeeples[i] != nullptr);
		}
		return DISPLAY_END_SCREEN;
	}
	else if (activePlayerIndex == 1 && logicalBoard->isFull()){
	#if PRINT_WINNER_PER_ROUND
			std::cout << "Tie! There is no winner." << std::endl;
	#endif
		return DISPLAY_END_SCREEN;
	}
	else{
		//switchPlayers();// if not switched the other player starts... 
		return INIT_STATE;
	}
}
Game::LoopState Game::displayEndscreen(){
	assert(winningCombiRMeeples[0] != nullptr && winningCombiRMeeples[1] != nullptr &&winningCombiRMeeples[2] != nullptr &&winningCombiRMeeples[3] != nullptr);

	if (drawEndScreen != true || rand() % 100 < 50){
		int particle_count = 1;
		endScreenParticleBuilder->setPosition({ 0, static_cast<float>(WINDOW_HEIGHT_TO_CALCULATE) }, { 5, 30 })
			->setPath({ 10, 200 }, { 275, 350 });
		particleSystem->newParticleCloud(particle_count, *endScreenParticleBuilder);
		endScreenParticleBuilder->setPosition({ static_cast<float>(WINDOW_WIDTH_TO_CALCULATE), static_cast<float>(WINDOW_HEIGHT_TO_CALCULATE) })
			->setPath({ 10, 200 }, { 190, 265 });
		particleSystem->newParticleCloud(particle_count, *endScreenParticleBuilder);
		endScreenParticleBuilder->setPosition({ 0, 0 })
			->setPath({ 10, 200 }, { -15, 89 });
		particleSystem->newParticleCloud(particle_count, *endScreenParticleBuilder);
		endScreenParticleBuilder->setPosition({ static_cast<float>(WINDOW_WIDTH_TO_CALCULATE), 0 })
			->setPath({ 10, 200 }, { 89, 195 });
		particleSystem->newParticleCloud(particle_count, *endScreenParticleBuilder);
	}
	drawEndScreen = true;

	if (meepleGlowAnimationClock.getElapsedTime().asSeconds() > 0.03f){

		for (uint8_t i = 0; i < 4; ++i){
			sf::Color c = rainbow(color4MGlow[i]);
			winningCombiRMeeples[i]->setGlow(&c);

			color4MGlow[i] += 0.01f;
			if (color4MGlow[i] > 1){
				color4MGlow[i] = 0;
			}
		}
		meepleGlowAnimationClock.restart();
	}

	// glow winning combination in rainbowcolor -> nyan cat -> nyan song


	if (hoveredButtonPtr != nullptr)
	{
		hoveredButtonPtr->setFillColor(buttonColor);
		hoveredButtonPtr = nullptr;
	}
	if (restartButton.getGlobalBounds().contains(convertedMousePos))
	{
		restartButton.setFillColor(sf::Color::Magenta);
		hoveredButtonPtr = &restartButton;
	}

	if (exitButton.getGlobalBounds().contains(convertedMousePos))
	{
		exitButton.setFillColor(sf::Color::Magenta);
		hoveredButtonPtr = &exitButton;
	}

	if (releasedLeftMouse && restartButton.getGlobalBounds().contains(convertedMousePos))
	{
		reset();
		for (uint8_t i = 0; i < 4; ++i){

			winningCombiRMeeples[i]->setGlow(nullptr);
			winningCombiRMeeples[i] = nullptr;
		}
		switchPlayers(); // TODO check this.. 
		drawEndScreen = false;
		return INIT_STATE;
	}
	if (releasedLeftMouse && exitButton.getGlobalBounds().contains(convertedMousePos))
	{
		runGameSwitch = false;
	}
	return DISPLAY_END_SCREEN;
}

//void Game::setString(std::string message)
//{
//	text.setString(message);
//
//	sf::FloatRect textRect = text.getLocalBounds();
//	text.setOrigin(textRect.left + textRect.width / 2.0f,
//		25.0f);
//	text.setPosition(sf::Vector2f(WINDOW_WIDTH_TO_CALCULATE / 2.0f, 35.0f));
//}