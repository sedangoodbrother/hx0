#include "common.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <deque>


SDL_Texture* LOADTEXTURE(const char* filename, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, filename);
    if (!texture) {
        SDL_Log("Failed to load texture %s: %s", filename, IMG_GetError());
    }
    return texture;
}

void ERRORMSG(const char* msg, const char* err) {
    SDL_Log("%s: %s", msg, err);
}

void RENDERTEXTURE(SDL_Texture* texture, int x, int y, SDL_Renderer* renderer) {
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
}

void RENDERSCALEDTEXTURE(SDL_Texture* texture, int x, int y, int width, int height, SDL_Renderer* renderer) {
    SDL_Rect dest = {x, y, width, height};
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
}

// Define InputBuffer structure
InputBuffer::InputBuffer()
    : up(false), down(false), left(false), right(false), attack(false),
      upFrames(0), downFrames(0), leftFrames(0), rightFrames(0), attackFrames(0) {}

void renderFPSCounter(TTF_Font* font, SDL_Renderer* renderer, int fps) {
    SDL_Color color = {255, 255, 255, 255}; // White color
    std::string fpsText = "FPS: " + std::to_string(fps);
    SDL_Surface* surface = TTF_RenderText_Solid(font, fpsText.c_str(), color);
    if (!surface) {
        SDL_Log("Failed to create FPS surface: %s", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Failed to create FPS texture: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    int screenWidth;
    SDL_GetRendererOutputSize(renderer, &screenWidth, nullptr); // Get the screen width

    SDL_Rect destRect = {
        screenWidth - surface->w - 10, // Align to the top-right corner with a 10px margin
        10,                           // Top margin
        surface->w,
        surface->h
    };

    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// Add input to the buffer
void InputBuffer::addInput(const std::string& input) {
    // Add the input to the buffer
    std::cout << "Input added to buffer: " << input << std::endl;
}

// Clear the buffer
void InputBuffer::clear() {
    up = down = left = right = attack = false;
    upFrames = downFrames = leftFrames = rightFrames = attackFrames = 0;
}

void interpretInputs(std::deque<std::string>& inputBuffer, const std::vector<SpecialAttack>& specialAttacks, int playerID, Character& player) {
    Uint32 currentTime = SDL_GetTicks();

    // Add recent inputs to the player's input history with timestamps
    while (!inputBuffer.empty()) {
        player.inputHistory.push_back({inputBuffer.front(), currentTime});
        inputBuffer.pop_front();

        // Limit the size of the input history to avoid excessive memory usage
        if (player.inputHistory.size() > 20) { // Adjust size as needed
            player.inputHistory.pop_front();
        }
    }

    // Check for special attacks
    for (const auto& attack : specialAttacks) {
        const auto& requiredInputs = attack.inputSequence;
        size_t matchIndex = 0;

        for (const auto& [input, timestamp] : player.inputHistory) {
            if (input == requiredInputs[matchIndex]) {
                matchIndex++;

                // Check if all inputs match
                if (matchIndex == requiredInputs.size()) {
                    // Check if the inputs were pressed within the 3-frame window
                    Uint32 timeDifference = timestamp - player.inputHistory[player.inputHistory.size() - requiredInputs.size()].second;
                    if (timeDifference <= player.frameWindow) {
                        std::cout << "Matched Special Attack: " << attack.name << std::endl;
                        attack.execute(playerID); // Trigger the special attack
                        break;
                    }
                }
            }
        }
    }
}

// Define global variables for player scores
// int player1Score = 0;
// int player2Score = 0;

void processPlayerInput(SDL_Event& event, std::deque<std::string>& inputBuffer, const std::unordered_map<SDL_Keycode, std::string>& keyMappings) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        bool isKeyDown = (event.type == SDL_KEYDOWN);
        auto it = keyMappings.find(event.key.keysym.sym);
        if (it != keyMappings.end()) {
            const std::string& action = it->second;
            if (isKeyDown) {
                inputBuffer.push_back(action); // Add input to the buffer
                if (inputBuffer.size() > 10) {
                    inputBuffer.pop_front(); // Limit buffer size to 10
                }
                std::cout << "Input added: " << action << std::endl;
            }
        }
    }
}



bool InputBuffer::parryUpInput(bool flip) const {
    return flip ? (up && right) : (up && left); // back + up
}

bool InputBuffer::parryDownInput(bool flip) const {
    return flip ? (down && right) : (down && left); // back + down
}

bool InputBuffer::parryMidInput(bool flip) const {
    return left && right; // both directions
}

void renderWinningScreen(SDL_Renderer* renderer, TTF_Font* font, const std::string& winner) {
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    std::string message;

    if (winner == "Draw") {
        message = "It's a Draw!";
    } else {
        message = winner + " Wins!";
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, message.c_str(), textColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (texture) {
            int screenWidth, screenHeight;
            SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
            SDL_Rect destRect = {
                (screenWidth - surface->w) / 2,
                (screenHeight - surface->h) / 2,
                surface->w,
                surface->h
            };
            SDL_RenderCopy(renderer, texture, nullptr, &destRect);
            SDL_DestroyTexture(texture);
        }
    }
}

#include "common.h"
#include "character.h"

void handleRoundEnd(Character& winner, Character& loser, Character& player1, Character& player2, int& player1Points, int& player2Points, Uint32& periodStartTime) {
    // Deduct a point from the losing player
    if (&loser == &player1) {
        player1Points--; // Deduct a point from Player 1
    } else if (&loser == &player2) {
        player2Points--; // Deduct a point from Player 2
    }

    // Reset players to their initial positions
    player1.reset();
    player2.reset();

    // Reset the timer for the next round
    periodStartTime = SDL_GetTicks();
}