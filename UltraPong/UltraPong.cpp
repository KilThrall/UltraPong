// UltraPong.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <SDL.h>
#include <string>
#include <SDL_image.h>
#include <SDL_ttf.h>

using namespace std;


struct Vector2 {
    int x;
    int y;
};
struct Vector2Float {
    float x;
    float y;
};

struct PlayerData {
    SDL_Scancode positiveKey;
    SDL_Scancode negativeKey;
    Vector2 position;
    Vector2 size;
    int points;
    SDL_Surface* controlsSurface;
};

enum class GameMode {
    SINGLEPLAYER = 0,
    SINGLEPLAYER_PREDICTIVE = 1,
    MULTIPLAYER = 2,
    SINGLEPLAYER_3D = 3
};

void SetPointsSurfaces();

GameMode SelectMode();

int AIMenu();

int MultiplayerMenu();

void SetTextSurface(int playerNumber);

bool CanPlayFrame();

void Draw(SDL_Texture* texture, int x, int y, Vector2 imageSize, SDL_Renderer* renderer);

void ControlPlayer(int index);

void DrawPlayer(int index);

void ControlBall(int totalPlayers);

void ControlAI();

SDL_Texture* LoadTexture(string filename, SDL_Renderer* renderer);

void GotScored(int index);

void ResetBall();

void UpdateTexts(int totalPlayers);

bool HasSomeoneWon(int totalPlayers);


const Uint8* keyboardState;
SDL_Renderer* renderer;
TTF_Font* gameFont;
Vector2 mapSize = { 750,300 };
Vector2 screenSize = { 1280,720 };
bool is3D = false;
int textHeight = 60;

char scoreChars[] = { '0','1', '2', '3', '4', '5' };
SDL_Surface* pointsSurface[6];

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
PlayerData playersControls[] = { {
        SDL_SCANCODE_W, SDL_SCANCODE_S, {0,1}, {15, 100}, 0
},{
        SDL_SCANCODE_I, SDL_SCANCODE_K, {mapSize.x,1}, {15, 100}, 0
},{
        SDL_SCANCODE_F, SDL_SCANCODE_G, {1,0}, {100, 15}, 0
},{
        SDL_SCANCODE_N, SDL_SCANCODE_M, {1,mapSize.y}, {100, 15}, 0
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
    TTF_Init();
    gameFont = TTF_OpenFont("fonts/pixelated.ttf", 24);
    SetPointsSurfaces();

    GameMode gameMode = SelectMode();

    if (gameMode == GameMode::MULTIPLAYER) {
        totalPlayers = MultiplayerMenu();
    }
    else {
        AiSpeed = AIMenu();
        if (gameMode == GameMode::SINGLEPLAYER_3D) {
            is3D = true;
        }
    }

    

    SDL_Window* window = SDL_CreateWindow("Ultra Pong", mapSize.x, mapSize.y+ textHeight, screenSize.x, screenSize.y, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    ballTexture = LoadTexture("graphics/ball.png", renderer);
    playerTexture = LoadTexture("graphics/line.png", renderer);
    
    while (playing) {

        if (CanPlayFrame()) {
            playing = !HasSomeoneWon(totalPlayers);
            Update();
            SDL_RenderClear(renderer);

            if (keyboardState[SDL_SCANCODE_P]) {
                playing = false;
            }

            for (size_t i = 0; i < totalPlayers; i++)
            {
                ControlPlayer(i);
            }

            if (totalPlayers == 1) {
                ControlAI();
            }

            ControlBall(totalPlayers);

            UpdateTexts(totalPlayers);
            
            SDL_RenderPresent(renderer);
        }
        
    }
    system("cls");

    if (!HasSomeoneWon(totalPlayers)) {
        cout << "How can you all be such pussies to end the game early?" << endl;
    }
    else {
        int totalWinners = 0;
        int checkingPlayers = totalPlayers;
        if (checkingPlayers == 1) {
            checkingPlayers = 2; //Made to check on single player
        }
        for (size_t i = 0; i < checkingPlayers; i++)
        {
            if (playersControls[i].points >= 5) {
                totalWinners++;
            }
        }
        switch (totalWinners) {
        case 1:
            for (size_t i = 0; i < checkingPlayers; i++)
            {
                if (playersControls[i].points >= 5) {
                    if (i == 1 && totalPlayers == 1) {
                        cout << "Did you really just lose to an AI?" << endl;
                    }
                    else {
                        cout << "Congratulations to player " << i << " for breaking everyone else's asses'" << endl;
                    }
                   
                }
            }
            break;
        case 2:
            cout << "Two winners? really? I am too lazy to even figure out who won. GG" << endl;
            break;
        case 3:
            cout << "Who the f* got his ass beaten so badly to have everyone else win but him?" << endl;
            break;
        case 4:
            cout << "How? Just how did all of you win? Was there a point to this all?" << endl;
            break;
        }
    }

    return 0;
}

void SetPointsSurfaces() {
    SDL_Color color = { 0,255,0 };
    pointsSurface[0] = TTF_RenderText_Blended(gameFont, "0", color);
    pointsSurface[1] = TTF_RenderText_Blended(gameFont, "1", color);
    pointsSurface[2] = TTF_RenderText_Blended(gameFont, "2", color);
    pointsSurface[3] = TTF_RenderText_Blended(gameFont, "3", color);
    pointsSurface[4] = TTF_RenderText_Blended(gameFont, "4", color);
    pointsSurface[5] = TTF_RenderText_Blended(gameFont, "5", color);
}

GameMode SelectMode() {
    int value = 0;
    while (value <= 0 || value > 4)
    {
        system("cls");
        cout << "Select game mode: " << endl;
        cout << "1: Singleplayer" << endl;
        cout << "2: Singleplayer with predictive AI (possibly impossible with dif over 5) (WIP, does nothing, for now)" << endl;
        cout << "3: Multiplayer" << endl;
        cout << "4: 3D??" << endl;
        cin >> value;
    }
    value--;
    return static_cast<GameMode>(value);
}

int MultiplayerMenu() {
    int value = 0;
    while (value <= 1 || value > 4)
    {
        system("cls");
        cout << "Select amount of players (2-4)" << endl;
        cin >> value;
    }
    
    for (int i = 0; i < value; i++)
    {
        SetTextSurface(i);
    }
    return value;
}

void SetTextSurface(int playerNumber) {
    SDL_Color color = { 255,255,255 };
    switch (playerNumber) {
    case -1:
        playersControls[1].controlsSurface = TTF_RenderText_Blended(gameFont, "AI:", color);
        break;
    case 0:
        playersControls[playerNumber].controlsSurface = TTF_RenderText_Blended(gameFont, "Player 1(w,s):", color);
        break;
    case 1:
        playersControls[playerNumber].controlsSurface = TTF_RenderText_Blended(gameFont, "Player 2(i,k):", color);
        break;
    case 2:
        playersControls[playerNumber].controlsSurface = TTF_RenderText_Blended(gameFont, "Player 3(f,g):", color);
        break;
    case 3:
        playersControls[playerNumber].controlsSurface = TTF_RenderText_Blended(gameFont, "Player 4(n,m):", color);
        break;
    }
}

int AIMenu() {
    int value = 0;
    while (value <=0|| value >20)
    {
        system("cls");
        cout << "Select difficulty (1-20)"<<endl;
        cin >> value;
    }
    SetTextSurface(0);
    SetTextSurface(-1); //AI
    return value;
}

bool CanPlayFrame() {
    timeSinceLastFrame++;
    if (timeSinceLastFrame >= timeForFrame) {
        timeSinceLastFrame = 0;
        return true;
    }
    return false;
}

void ControlBall(int totalPlayers) {

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
            
            GotScored(0);
        }
        
    }
    else if (ballPos.x + ballSize.x > mapSize.x-mapSize.x/100) {
        totalBounces++;
        if (ballPos.y + ballSize.y >= playersControls[1].position.y &&
            ballPos.y <= playersControls[1].position.y + playersControls[1].size.y) 
        {
            ballPos.x = mapSize.x - mapSize.x / 95 - ballSize.x;
            ballDir.x = -ballDir.x;
        }
        else {
            if (totalPlayers == 1) {
                GotScored(-1);
            }
            else {
                GotScored(1);
            }
        }
    }

    if (ballPos.y < mapSize.y / 100) {

        totalBounces++;
        if ((ballPos.x + ballSize.x >= playersControls[2].position.x &&
            ballPos.x <= playersControls[2].position.x + playersControls[2].size.x) ||
            totalPlayers<3)
        {
            ballPos.y = mapSize.y / 100;
            ballDir.y = -ballDir.y;
        }
        else {
            GotScored(2);
        }

        
        
    }
    else if (ballPos.y + ballSize.y > mapSize.y - mapSize.y / 100) {

        totalBounces++;
        if ((ballPos.x + ballSize.x >= playersControls[3].position.x &&
            ballPos.x <= playersControls[3].position.x + playersControls[3].size.x) ||
            totalPlayers < 4) 
        {
            ballPos.y = mapSize.y - mapSize.y / 100 - ballSize.y;
            ballDir.y = -ballDir.y;
        }
        else {
            GotScored(3);
        }
    }

    if (totalBounces >= bouncesForSpeed) {
        ballSpeed *= 1.3f;
        totalBounces = 0;
    }

    Draw(ballTexture, ballPos.x,ballPos.y, ballSize, renderer);
}

void ControlPlayer(int index) {
    if (keyboardState[playersControls[index].negativeKey]) {
        if (index < 2) {
            playersControls[index].position.y+= playerSpeed;
        }
        else {
            playersControls[index].position.x += playerSpeed;
        }
    }
    else if (keyboardState[playersControls[index].positiveKey]) {
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

    if (!is3D) {
        Draw(playerTexture, playersControls[index].position.x, playersControls[index].position.y,
            playersControls[index].size, renderer);
    }
    else if (index != 0) {
        Draw(playerTexture, playersControls[index].position.x, playersControls[index].position.y,
            playersControls[index].size, renderer);
    }
    
}

void GotScored(int index) {
    
    if (index == -1) {
        playersControls[0].points++;
    }
    else {
        for (size_t i = 0; i < 4; i++)
        {
            if (i != index) {
                playersControls[i].points++;
            }
        }
    }
    
    ResetBall();
}

int GetTotalPoints(int index) {
    if (index == -1) {
        return playersControls[0].points;
    }
    
    int total = 0;
    for (size_t i = 0; i < 4; i++)
    {
        if (i != index) {
            total += playersControls[index].points;
        }
    }
    return total;
    
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
    if (!is3D) {
        rect.x = x;
        rect.y = y;

        if (imageSize.x != 0) {
            rect.w = imageSize.x;
            rect.h = imageSize.y;
        }
        else {
            SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
        }
    }
    else {
        int fixedWidth = mapSize.x / 3;
        Vector2 distance = { x - playersControls[0].position.x,  playersControls[0].position.y - y };
        rect.w = fixedWidth*50 / distance.x;
        rect.h = rect.w * (imageSize.y / imageSize.x);
        rect.x = fixedWidth - (float)rect.w;
     /*   int posY = 0;
        if (distance.y > 0) {
            rect.y = 
        }*/
        rect.y = y - distance.y;
        
    }
    
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
}

SDL_Texture* LoadTexture(string filename, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());

    return texture;
}

void UpdateTexts(int totalPlayers) {
    SDL_Texture* textTexture;
    SDL_Texture* pointsTexture;
    SDL_Rect rect;
    rect.w = 100;
    rect.h = textHeight/2;
    if (totalPlayers == 1) {
        totalPlayers = 2; //made to render AI texts
    }
    SDL_Rect pointsRect;
    pointsRect.h = rect.h;
    pointsRect.w = rect.w / 5;
    
    for (size_t i = 0; i < totalPlayers; i++)
    {
        textTexture = SDL_CreateTextureFromSurface(renderer, playersControls[i].controlsSurface);
        rect.x = rect.w * i;
        rect.y = mapSize.y;
        
        pointsTexture = SDL_CreateTextureFromSurface(renderer, pointsSurface[playersControls[i].points]);
        pointsRect.x = rect.x;
        pointsRect.y = mapSize.y + textHeight / 2;

        SDL_RenderCopy(renderer, textTexture, NULL, &rect);
        SDL_RenderCopy(renderer, pointsTexture, NULL, &pointsRect);
    }
}

bool HasSomeoneWon(int totalPlayers) {
    if (totalPlayers == 1) {
        totalPlayers = 2; //Made to check on single player
    }
    for (size_t i = 0; i < totalPlayers; i++)
    {
        if (playersControls[i].points >= 5) {
            return true;
        }
    }
    return false;
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
