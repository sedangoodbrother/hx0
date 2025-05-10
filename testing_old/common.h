// Declare and enumerate structs, class and constant variables
#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <filesystem>
#include <functional>
#include <vector>
#include <deque>
#include "moveset.h"
// #include "character.h"

struct Character;

// Declare constants
#define SCREEN_WIDTH 854
#define SCREEN_HEIGHT 480

// Declare functions
SDL_Texture* LOADTEXTURE(const char* filename, SDL_Renderer* renderer);
void ERRORMSG(const char* msg, const char* err);
void RENDERTEXTURE(SDL_Texture* texture, int x, int y, SDL_Renderer* renderer);
void RENDERSCALEDTEXTURE(SDL_Texture* texture, int x, int y, int width, int height, SDL_Renderer* renderer);
void renderFPSCounter(TTF_Font* font, SDL_Renderer* renderer, int fps);

// Declare structs
struct INITSDL {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    bool initialized = false;

    INITSDL(const std::string& title, int width, int height, const std::string& fontPath) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
            return;
        }

        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            std::cerr << "IMG_Init Error: " << IMG_GetError() << "\n";
            SDL_Quit();
            return;
        }

        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init Error: " << TTF_GetError() << "\n";
            IMG_Quit();
            SDL_Quit();
            return;
        }

        window = SDL_CreateWindow(title.c_str(),
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT,
                                  SDL_WINDOW_SHOWN);

        if (!window) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
            IMG_Quit();
            SDL_Quit();
            return;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if (!renderer) {
            std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            return;
        }

        font = TTF_OpenFont(fontPath.c_str(), 16); // Initialize font
        if (!font) {
            std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << "\n";
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            return;
        }

        initialized = true;
    }

    ~INITSDL() { // Destructor
        if (font) TTF_CloseFont(font); // Clean up font
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
};

struct InputBuffer {
    bool up, down, left, right, attack;
    int upFrames, downFrames, leftFrames, rightFrames, attackFrames;

    InputBuffer(); // Constructor
    void addInput(const std::string& input); // Add input to the buffer
    void clear(); // Clear the buffer
    bool hasCombination(const std::string& dir1, const std::string& dir2) const;
    bool parryUpInput(bool flip) const;
    bool parryDownInput(bool flip) const;
    bool parryMidInput(bool flip) const;
};

// Declare global variables for player scores
extern int player1Score;
extern int player2Score;

extern int player1Points; // Declare as extern
extern int player2Points; // Declare as extern

void interpretInputs(std::deque<std::string>& inputBuffer, const std::vector<SpecialAttack>& specialAttacks, int playerID, Character& player);
void processPlayerInput(SDL_Event& event, std::deque<std::string>& inputBuffer, const std::unordered_map<SDL_Keycode, std::string>& keyMappings);
void renderWinningScreen(SDL_Renderer* renderer, TTF_Font* font, const std::string& winner);
void handleRoundEnd(Character& winner, Character& loser, Character& player1, Character& player2, int& player1Points, int& player2Points, Uint32& periodStartTime);
#endif // COMMON_H