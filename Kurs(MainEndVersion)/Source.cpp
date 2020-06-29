#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
using namespace std;

//Прорисовка змеи
void renderPlayer(SDL_Renderer* renderer, SDL_Rect player, int x, int y, int scale, vector<int> tailX, vector<int> tailY, int tailLength);
//Прорисовка еды
void renderFood(SDL_Renderer* renderer, SDL_Rect food);
//Прорисовка счёта
void renderScore(SDL_Renderer* renderer, int tailLength, int scale, int wScale);
//Проверка на столкновение
bool checkCollision(int foodx, int foody, int playerx, int playery);
//Создание еды и возвращение её координат
pair<int, int> getFoodSpawn(vector<int> tailX, vector<int> tailY, int playerX, int playerY, int scale, int wScale, int tailLength);
//Конец игры
void gameOver(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength);
//Победа
void youWin(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength);

int main(int argc, char* argv[]) {
	
	// Инициализация всех библиотек SDL
	SDL_Init(SDL_INIT_EVERYTHING);

	// Инициализация библиотеки SDL_mixer (нужна для звука) и проверка на корректность.
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
		cout << "Error: " << Mix_GetError() << endl;

	// Инициализация библиотеки SDL_TTF (нужна для шрифтов) и проверка на корректность.
	if (TTF_Init() < 0)
		cout << "Error: " << TTF_GetError() << endl;

	//Основное окно которое будет показано, основной рендерер который будет отображать текстуры и основное событие
	SDL_Window* window;	
	SDL_Renderer* renderer;
	SDL_Event event;

	//Фоновая музыка и звуки
	int volume = MIX_MAX_VOLUME / 6;
	Mix_Music* bgm = Mix_LoadMUS("src/music/bgm.mp3");
	Mix_Chunk* soundEffect = Mix_LoadWAV("src/music/eat.wav");
	Mix_Chunk* deathEffect = Mix_LoadWAV("src/music/death.wav");
	Mix_Chunk* winEffect = Mix_LoadWAV("src/music/win.wav");

	//Громкость звука
	Mix_VolumeMusic(volume);
	Mix_VolumeChunk(soundEffect, volume);
	Mix_VolumeChunk(deathEffect, volume);
	Mix_VolumeChunk(winEffect, volume);

	// Квадрат которым управляет игрок и его изначальные координаты
	SDL_Rect player;
	player.x = 0;
	player.y = 0;
	player.h = 0;
	player.w = 0;

	// Длина хвоста (увеличивается каждый раз, когда игрок съедает еду)
	int tailLength = 0;

	// Вектора которые хранят в себе позиции всех блоков которые представляют собой тело змеи
	vector<int> tailX;
	vector<int> tailY;

	// Длина и ширина клеток тела змеи и еды и размерность поля (должны быть одного размера)
	int mainSize = 24;
	int snakeBody = mainSize;
	int scale = mainSize;
	int wScale = mainSize;

	// Переменные позиций игрока
	int x = 0;
	int y = 0;
	int prevX = 0;
	int prevY = 0;

	// Управление змеёй
	bool up = false;
	bool down = false;
	bool right = false;
	bool left = false;
	//Переменные контроля управления и рисования
	bool inputThisFrame = false;
	bool redo = false;

	// Еда и её начальные значения
	SDL_Rect food;
	food.w = scale;
	food.h = scale;
	food.x = 0;
	food.y = 0;
	//foodLoc отвечает за расположение новой еды на экране
	pair<int, int> foodLoc = getFoodSpawn(tailX, tailY, x, y, scale, wScale, tailLength);
	food.x = foodLoc.first;
	food.y = foodLoc.second;

	// ПОказывает окно с этими настройками и начинает его рендер
	window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scale * wScale + 1, scale * wScale + 1, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	//Таймер передвижения змеи (fps)
	float time = SDL_GetTicks() / 100;

	// Основной игрвой цикл
	while (true) {
		//Эта скорость (75) отвечает за скорость обновления блоков
		float newTime = SDL_GetTicks() / 75; 
		float delta = newTime - time;
		time = newTime;

		inputThisFrame = false;

		//Фоновая музыка
		if (!Mix_PlayingMusic())
			Mix_PlayMusic(bgm, -1);		
		
		// Проверка на победу (если длина змеи 5 то игрок выиграл) (обычно нужно ставить mainSize*mainSize - 1
		if (tailLength >= 5) {
			//Остановка фоновой музыки
			Mix_HaltMusic();
			//Звук победы
			Mix_PlayChannel(-1, winEffect, 0);
			//Вызов победной функции и освобождение памяти
			youWin(renderer, event, scale, wScale, tailLength);
			x = 0;
			y = 0;
			up = false;
			left = false;
			right = false;
			down = false;
			tailX.clear();
			tailY.clear();
			tailLength = 0;
			redo = false;
			//Возвращаем значения -100,-100 для redo (перерисовки)
			foodLoc = getFoodSpawn(tailX, tailY, x, y, scale, wScale, tailLength);

			if (food.x == -100 && food.y == -100) {
				redo = true;
			}

			food.x = foodLoc.first;
			food.y = foodLoc.second;
		}

		// Управление (если произошло событие)
		if (SDL_PollEvent(&event)) {

			// Закрытие программы на крестик (если событие - выход то программа закрывается
			if (event.type == SDL_QUIT) {
				exit(0);
			}

			// При нажатии клавиши
			if (event.type == SDL_KEYDOWN && inputThisFrame == false) {

				// Проверка на то, какая кнопка нажата и выполнение соответсвтующего действия
				if (down == false && event.key.keysym.scancode == SDL_SCANCODE_UP) {
					up = true;
					left = false;
					right = false;
					down = false;
					inputThisFrame = true;
				}
				else if (right == false && event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					up = false;
					left = true;
					right = false;
					down = false;
					inputThisFrame = true;
				}
				else if (up == false && event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					up = false;
					left = false;
					right = false;
					down = true;
					inputThisFrame = true;
				}
				else if (left == false && event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					up = false;
					left = false;
					right = true;
					down = false;
					inputThisFrame = true;
				}
			}
		}

		// Предыдущие координаты позиции головы
		prevX = x;
		prevY = y;

		//Проверка нажатой клавиши и направление змеи в выбранную сторону
		if (up) {
			y -= delta * scale;
		}
		else if (left) {
			x -= delta * scale;
		}
		else if (right) {
			x += delta * scale;
		}
		else if (down) {
			y += delta * scale;
		}
		//Перерисовка
		if (redo == true) {

			redo = false;
			foodLoc = getFoodSpawn(tailX, tailY, x, y, scale, wScale, tailLength);
			food.x = foodLoc.first;
			food.y = foodLoc.second;

			if (food.x == -100 && food.y == -100) {
				redo = true;
			}

		}

		// Проверка столкновения, столкнулась ли змея с едой
		if (checkCollision(food.x, food.y, x, y)) {
			//Звук поедания
			Mix_PlayChannel(-1, soundEffect, 0);
			// Появление новой еды при столкновении
			foodLoc = getFoodSpawn(tailX, tailY, x, y, scale, wScale, tailLength);
			food.x = foodLoc.first;
			food.y = foodLoc.second;

			if (food.x == -100 && food.y == -100) {
				redo = true;
			}

			tailLength++;
		}

		// Работает только в кадрах где голова змеи была перемещена
		if (delta * scale == snakeBody) {

			// Обновление длины хвоста и позиции
			if (tailX.size() != tailLength) {
				tailX.push_back(prevX);
				tailY.push_back(prevY);
			}

			//Цикл для всех блоков тела змеи и передвижение их вперед
			//Это обноволяет блоки от конца к началу (от хвоста до головы не включительно)
			for (int i = 0; i < tailLength; i++) {

				if (i > 0) {
					tailX[i - 1] = tailX[i];
					tailY[i - 1] = tailY[i];
				}
			}

			if (tailLength > 0) {
				tailX[tailLength - 1] = prevX;
				tailY[tailLength - 1] = prevY;
			}
		}

		// Конец игры если игрок столкнулся с препятствием и перезагрука игры
		for (int i = 0; i < tailLength; i++) {

			if (x == tailX[i] && y == tailY[i]) {
				//Остановка фоновой музыки
				Mix_HaltMusic();
				//Звук смерти
				Mix_PlayChannel(-1, deathEffect, 0);
				gameOver(renderer, event, scale, wScale, tailLength);
				x = 0;
				y = 0;
				up = false;
				left = false;
				right = false;
				down = false;
				tailX.clear();
				tailY.clear();
				tailLength = 0;
				redo = false;

				foodLoc = getFoodSpawn(tailX, tailY, x, y, scale, wScale, tailLength);
				if (food.x == -100 && food.y == -100) {
					redo = true;
				}

				food.x = foodLoc.first;
				food.y = foodLoc.second;
			}
		}

		// Конец игры если игрок ударился о края карты и перезагрузка игры
		if (x < 0 || y < 0 || x > scale * wScale - scale || y > scale * wScale - scale) {
			//Остановка фоновой музыки
			Mix_HaltMusic();
			//Звук смерти
			Mix_PlayChannel(-1, deathEffect, 0);
			gameOver(renderer, event, scale, wScale, tailLength);
			x = 0;
			y = 0;
			up = false;
			left = false;
			right = false;
			down = false;
			tailX.clear();
			tailY.clear();
			tailLength = 0;
			redo = false;
			foodLoc = getFoodSpawn(tailX, tailY, x, y, scale, wScale, tailLength);
			food.x = foodLoc.first;
			food.y = foodLoc.second;

			if (food.x == -100 && food.y == -100) {
				redo = true;
			}
		}

		// Прорисовка всех текстур
		renderFood(renderer, food);
		renderPlayer(renderer, player, x, y, scale, tailX, tailY, tailLength);
		renderScore(renderer, tailLength, scale, wScale);

		SDL_RenderDrawLine(renderer, 0, 0, 0, scale * scale);
		SDL_RenderDrawLine(renderer, 0, snakeBody * snakeBody, snakeBody * snakeBody, snakeBody * snakeBody);
		SDL_RenderDrawLine(renderer, snakeBody * snakeBody, snakeBody * snakeBody, snakeBody * snakeBody, 0);
		SDL_RenderDrawLine(renderer, snakeBody * snakeBody, 0, 0, 0);

		// Вывод на экран
		SDL_RenderPresent(renderer);

		// Выбор цвета и заполнение этим цветом поверхности
		SDL_SetRenderDrawColor(renderer, 83, 181, 173, 1);
		SDL_RenderClear(renderer);
	}
	
	SDL_DestroyWindow(window);
	Mix_FreeMusic(bgm);
	Mix_FreeChunk(soundEffect);
	Mix_FreeChunk(deathEffect);
	Mix_FreeChunk(winEffect);

	bgm = nullptr;
	soundEffect = nullptr;
	deathEffect = nullptr;
	winEffect = nullptr;

	Mix_Quit();
	TTF_Quit();
	SDL_Quit();

	return 0;
}

void renderPlayer(SDL_Renderer* renderer, SDL_Rect player, int x, int y, int scale, vector<int> tailX, vector<int> tailY, int tailLength) {
	//Выбор цвета
	SDL_SetRenderDrawColor(renderer, 230, 237, 9, 1);
	player.w = scale;
	player.h = scale;

	// Получение координат всех блоков хвоста и прорисовка их
	for (int i = 0; i < tailLength; i++) {
		player.x = tailX[i];
		player.y = tailY[i];
		SDL_RenderFillRect(renderer, &player);
	}

	player.x = x;
	player.y = y;
	//Прорисовка головы
	SDL_RenderFillRect(renderer, &player);
}

void renderFood(SDL_Renderer* renderer, SDL_Rect food) {
	//Выбор цвета
	SDL_SetRenderDrawColor(renderer, 255, 3, 3, 1);
	//Прорисовка еды
	SDL_RenderFillRect(renderer, &food);
}

void renderScore(SDL_Renderer* renderer, int tailLength, int scale, int wScale) {
	//Цвет текста
	SDL_Color scoreColor = { 0, 0, 0 };
	
	// Подключение стороннего шрифта
	TTF_Font* font = TTF_OpenFont((char*)"src/fonts/arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}
	//Отображение счета вверху экрана
	SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), scoreColor);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);
	//Размер текстуры отображающей счёт и её координаты
	SDL_Rect scoreRect;
	scoreRect.w = 100;
	scoreRect.h = 30;
	scoreRect.x = ((scale * wScale) / 2) - (scoreRect.w / 2);
	scoreRect.y = 0;
	SDL_RenderCopy(renderer, scoreMessage, NULL, &scoreRect);

	TTF_CloseFont(font);
}

bool checkCollision(int foodx, int foody, int playerx, int playery) {
	//Если игрок стокнулся с едой
	if (playerx == foodx && playery == foody) {
		return true;
	}

	return false;
}

// Появление еды на поле (не на теле змеи) (Работает странно)
pair<int, int> getFoodSpawn(vector<int> tailX, vector<int> tailY, int playerX, int playerY, int scale, int wScale, int tailLength) {
	bool valid = false;
	int x = 0;
	int y = 0;
	srand(time(0));
	x = scale * (rand() % wScale);
	y = scale * (rand() % wScale);
	valid = true;

	// Проверка всех блоков хвоста и блока головы
	for (int i = 0; i < tailLength; i++) {

		if ((x == tailX[i] && y == tailY[i]) || (x == playerX && y == playerY)) {
			valid = false;
		}

	}
	//Если еда создается на ненужном блоке
	if (!valid) {
		pair<int, int> foodLoc;
		foodLoc = make_pair(-100, -100);
		return foodLoc;
	}

	pair<int, int> foodLoc;
	foodLoc = make_pair(x, y);

	return foodLoc;
}

void gameOver(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength) {
	SDL_Color gameOverColor = { 255, 0, 0 };
	SDL_Color retryColor = { 255, 255, 255 };
	SDL_Color scoreColor = { 0, 0, 0 };

	// Подключение стороннего шрифта
	TTF_Font* font = TTF_OpenFont((char*)"src/fonts/arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}
	//Отображение финальных надписей на экране
	SDL_Surface* gameover = TTF_RenderText_Solid(font, "Game Over", gameOverColor);
	SDL_Surface* retry = TTF_RenderText_Solid(font, "Enter - retry, Esc - exit", retryColor);
	SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), scoreColor);
	SDL_Texture* gameoverMessage = SDL_CreateTextureFromSurface(renderer, gameover);
	SDL_Texture* retryMessage = SDL_CreateTextureFromSurface(renderer, retry);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);
	SDL_Rect gameoverRect;
	SDL_Rect retryRect;
	SDL_Rect scoreRect;
	gameoverRect.w = 200;
	gameoverRect.h = 100;
	gameoverRect.x = ((scale * wScale) / 2) - (gameoverRect.w / 2);
	gameoverRect.y = ((scale * wScale) / 2) - (gameoverRect.h / 2) - 50;
	retryRect.w = 300;
	retryRect.h = 50;
	retryRect.x = ((scale * wScale) / 2) - ((retryRect.w / 2));
	retryRect.y = (((scale * wScale) / 2) - ((retryRect.h / 2)) + 150);
	scoreRect.w = 100;
	scoreRect.h = 30;
	scoreRect.x = ((scale * wScale) / 2) - (scoreRect.w / 2);
	scoreRect.y = 0;
	SDL_RenderCopy(renderer, gameoverMessage, NULL, &gameoverRect);
	SDL_RenderCopy(renderer, retryMessage, NULL, &retryRect);
	SDL_RenderCopy(renderer, scoreMessage, NULL, &scoreRect);

	TTF_CloseFont(font);

	// Игра работает пока не нажмется кнопка
	while (true) {
		SDL_RenderPresent(renderer);

		if (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				exit(0);
			}

			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
				return;
			}

			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				exit(0);
			}
		}
	}
}

void youWin(SDL_Renderer* renderer, SDL_Event event, int scale, int wScale, int tailLength) {
	SDL_Color Red = { 255, 0, 0 };
	SDL_Color White = { 255, 255, 255 };
	SDL_Color Black = { 0, 0, 0 };
	SDL_Color Yellow = { 255, 255, 0 };

	// Подключение стороннего шрифта
	TTF_Font* font = TTF_OpenFont((char*)"src/fonts/arial.ttf", 10);
	if (font == NULL) {
		cout << "Font loading error" << endl;
		return;
	}
	//Отображение финальных надписей на экране
	SDL_Surface* gameover = TTF_RenderText_Solid(font, "You won!", Yellow);
	SDL_Surface* retry = TTF_RenderText_Solid(font, "Enter - retry, Esc - exit", White);
	SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(tailLength * 10)).c_str(), Black);
	SDL_Texture* gameoverMessage = SDL_CreateTextureFromSurface(renderer, gameover);
	SDL_Texture* retryMessage = SDL_CreateTextureFromSurface(renderer, retry);
	SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);
	SDL_Rect gameoverRect;
	SDL_Rect retryRect;
	SDL_Rect scoreRect;
	gameoverRect.w = 200;
	gameoverRect.h = 100;
	gameoverRect.x = ((scale * wScale) / 2) - (gameoverRect.w / 2);
	gameoverRect.y = ((scale * wScale) / 2) - (gameoverRect.h / 2) - 50;
	retryRect.w = 300;
	retryRect.h = 50;
	retryRect.x = ((scale * wScale) / 2) - ((retryRect.w / 2));
	retryRect.y = (((scale * wScale) / 2) - ((retryRect.h / 2)) + 150);
	scoreRect.w = 100;
	scoreRect.h = 30;
	scoreRect.x = ((scale * wScale) / 2) - (scoreRect.w / 2);
	scoreRect.y = 0;
	SDL_RenderCopy(renderer, gameoverMessage, NULL, &gameoverRect);
	SDL_RenderCopy(renderer, retryMessage, NULL, &retryRect);
	SDL_RenderCopy(renderer, scoreMessage, NULL, &scoreRect);

	TTF_CloseFont(font);

	// Игра работает пока не нажмется кнопка
	while (true) {
		SDL_RenderPresent(renderer);

		if (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				exit(0);
			}

			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
				return;
			}

			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				exit(0);
			}

		}

	}

}