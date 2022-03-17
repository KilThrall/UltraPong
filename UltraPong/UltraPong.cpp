// UltraPong.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <SDL.h>
#include <string>
#include <SDL_image.h>

using namespace std;


typedef struct Vector2 {
    int x;
    int y;
};
typedef struct Vector2Float {
    float x;
    float y;
};

typedef struct KeyControls {
    SDL_Scancode positive;
    SDL_Scancode negative;
    Vector2 position;
    Vector2 size;
};

void Menu();

bool CanPlayFrame();

void Draw(SDL_Texture* texture, int x, int y, Vector2 imageSize, SDL_Renderer* renderer);

void ControlPlayer(int index);

void DrawPlayer(int index);

void ControlBall();

void ControlAI();

SDL_Texture* LoadTexture(string filename, SDL_Renderer* renderer);

void GotPoint(int index);

void ResetBall();

const Uint8* keyboardState;
SDL_Renderer* renderer;
Vector2 mapSize = { 750,300 };
Vector2 screenSize = { 1280,720 };

int points[] = { 0,0,0,0 };

int timeSinceLastFrame = 0;
int timeForFrame = 600000;

Vector2 ballSize = { 20,20 };
Vector2Float ballDir = { 1,0.7f };
Vector2 ballPos = { 15,15 };
float ballSpeed = 2;
SDL_Texture* ballTexture;
int totalBounces = 0;
int bouncesForSpeed = 7;

int AiSpeed = 0;

int playerSpeed = 2;
SDL_Texture* playerTexture;
KeyControls playersControls[] = { {
        SDL_SCANCODE_W, SDL_SCANCODE_S, {0,1}, {15, 100}
},{
        SDL_SCANCODE_K, SDL_SCANCODE_I, {mapSize.x,1}, {15, 100}
},{
        SDL_SCANCODE_F, SDL_SCANCODE_G, {1,0}, {100, 15}
},{
        SDL_SCANCODE_N, SDL_SCANCODE_M, {1,mapSize.y}, {100, 15}
},
};


void Update()
{
    SDL_PumpEvents();
    keyboardState = SDL_GetKeyboardState(NULL);
}

int main(int argc, char* args[])
{
    bool playing = true;
    int totalPlayers = 1;
    SDL_Init(SDL_INIT_EVERYTHING);

    Menu();

    SDL_Window* window = SDL_CreateWindow("Ultra Pong", mapSize.x, mapSize.y, screenSize.x, screenSize.y, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    ballTexture = LoadTexture("graphics/ball.png", renderer);
    playerTexture = LoadTexture("graphics/line.png", renderer);
    
    while (playing) {

        if (CanPlayFrame()) {
            Update();
            SDL_RenderClear(renderer);

            if (keyboardState[SDL_SCANCODE_P]) {
                playing = false;
            }

            for (size_t i = 0; i < totalPlayers; i++)
            {
                ControlPlayer(i);
            }

            ControlBall();

            if (totalPlayers == 1) {
                ControlAI();
            }
            SDL_RenderPresent(renderer);
        }
    }

    return 0;
}

void Menu() {
    AiSpeed = 0;
    while (AiSpeed<=0||AiSpeed>20)
    {
        cout << "Select difficulty (1-20)"<<endl;
        cin >> AiSpeed;
    }
    
}

bool CanPlayFrame() {
    timeSinceLastFrame++;
    if (timeSinceLastFrame >= timeForFrame) {
        timeSinceLastFrame = 0;
        return true;
    }
    return false;
}

void ControlBall() {

    ballPos.x += ballDir.x * ballSpeed;
    ballPos.y += ballDir.y * ballSpeed;

    if (ballPos.x < mapSize.x / 100) {
        totalBounces++;
        if (ballPos.y + ballSize.y >= playersControls[0].position.y &&
            ballPos.y  <= playersControls[0].position.y + playersControls[0].size.y)
        {
            ballPos.x = mapSize.x / 100;
            ballDir.x = -ballDir.x;
        }
        else {
            GotPoint(-1);
        }
        
    }
    else if (ballPos.x >= mapSize.x-mapSize.x/100) {
        ballPos.x = mapSize.x - mapSize.x / 100;
        ballDir.x = -ballDir.x;
        totalBounces++;
    }

    if (ballPos.y < mapSize.y / 100) {
        ballPos.y = mapSize.y / 100;
        ballDir.y = -ballDir.y;
        totalBounces++;
    }
    else if (ballPos.y >= mapSize.y - mapSize.y / 100) {
        ballPos.y = mapSize.y - mapSize.y / 100;
        ballDir.y = -ballDir.y;
        totalBounces++;
    }

    if (totalBounces >= bouncesForSpeed) {
        ballSpeed *= 1.3f;
        totalBounces = 0;
    }

    Draw(ballTexture, ballPos.x,ballPos.y, ballSize, renderer);
}

void ControlPlayer(int index) {
    if (keyboardState[playersControls[index].negative]) {
        if (index < 2) {
            playersControls[index].position.y+= playerSpeed;
        }
        else {
            playersControls[index].position.x += playerSpeed;
        }
    }
    else if (keyboardState[playersControls[index].positive]) {
        if (index < 2) {
            playersControls[index].position.y -= playerSpeed;
        }
        else {
            playersControls[index].position.x -= playerSpeed;
        }
    }

    DrawPlayer(index);
}

void DrawPlayer(int index) {
    if (playersControls[index].position.x < 0) {
        playersControls[index].position.x = 0;
    }
    else if (playersControls[index].position.x + playersControls[index].size.x >= mapSize.x) {
        playersControls[index].position.x = mapSize.x - playersControls[index].size.x;
    }

    if (playersControls[index].position.y < 0) {
        playersControls[index].position.y = 0;
    }
    else if (playersControls[index].position.y + playersControls[index].size.y >= mapSize.y) {
        playersControls[index].position.y = mapSize.y - playersControls[index].size.y;
    }

    Draw(playerTexture, playersControls[index].position.x, playersControls[index].position.y,
        playersControls[index].size, renderer);
}

void GotPoint(int index) {
    if (index == -1) {
        points[1]++;
    }
    else {
        points[index]++;
    }
    ResetBall();
}

void ResetBall() {
    srand(time(NULL));
    ballSpeed = 2;

    ballPos.x = rand() % (mapSize.x-mapSize.x/5) + mapSize.x/10;
    ballPos.y = rand() % (mapSize.y-mapSize.y/5) + mapSize.y / 10;

    ballDir.x = -ballDir.x;
    ballDir.y = -ballDir.y;

    Draw(ballTexture, ballPos.x, ballPos.y, ballSize, renderer);
    SDL_RenderPresent(renderer);

    SDL_Delay(2000);
}

void ControlAI() {
    int dir = ballPos.y - playersControls[1].position.y ;
    if (dir != 0) {
        dir = dir / abs(dir);
    }
    bool initiallyHigher = playersControls[1].position.y > ballPos.y;

    playersControls[1].position.y += dir * AiSpeed;
    if (initiallyHigher && playersControls[1].position.y <= ballPos.y) {
        playersControls[1].position.y = ballPos.y;
    }
    else if (!initiallyHigher && playersControls[1].position.y >= ballPos.y) {
        playersControls[1].position.y = ballPos.y;
    }

    DrawPlayer(1);
}

void Draw(SDL_Texture* texture, int x, int y, Vector2 imageSize, SDL_Renderer* renderer) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;

    if (imageSize.x != 0) {
        rect.w = imageSize.x;
        rect.h = imageSize.y;
    }
    else {
        SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    }
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
}

SDL_Texture* LoadTexture(string filename, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());

    return texture;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
