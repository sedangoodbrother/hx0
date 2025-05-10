#include <iostream>
#include <map>
#include <functional>
#include <vector>
#include "menu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

MENU::MENU(const std::map<int, std::string>& options, const std::map<int, std::function<void()>>& actions, SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor)
    : menuOptions(options), menuActions(actions), renderer(renderer), font(font), textColor(textColor) {}

void MENU::renderMenu() const {
    static std::vector<SDL_Texture*> cachedTextures; // Cache textures
    static bool texturesCreated = false;

    if (!texturesCreated) {
        int yOffset = 50; // Initial vertical offset for menu items
        for (const auto& [key, value] : menuOptions) {
            SDL_Surface* surface = TTF_RenderText_Solid(font, value.c_str(), textColor);
            if (!surface) {
                std::cerr << "Failed to create surface for menu option '" << value << "': " << TTF_GetError() << std::endl;
                continue;
            }

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (!texture) {
                std::cerr << "Failed to create texture for menu option '" << value << "': " << SDL_GetError() << std::endl;
                SDL_FreeSurface(surface);
                continue;
            }

            SDL_Rect dstRect = {100, yOffset, surface->w + 10, surface->h + 10}; // Button size slightly larger than text
            buttonRects.push_back(dstRect); // Store button rectangle
            cachedTextures.push_back(texture);
            SDL_FreeSurface(surface);

            yOffset += 50; // Increment vertical offset for the next menu item
        }
        texturesCreated = true;
    }

    for (size_t i = 0; i < cachedTextures.size(); ++i) {
        SDL_Texture* texture = cachedTextures[i];
        SDL_Rect buttonRect = buttonRects[i];

        // Render button background
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Light gray
        SDL_RenderFillRect(renderer, &buttonRect);

        // Render button text
        SDL_Rect textRect = {buttonRect.x + 5, buttonRect.y + 5, buttonRect.w - 10, buttonRect.h - 10};
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
    }
}

void MENU::presentMenu() const {
    SDL_RenderPresent(renderer);
}

// Commented out as these methods are intended for debugging
/*
void MENU::displayMenu() const {
    std::cout << "Displaying menu options:" << std::endl;
    for (const auto& [key, value] : menuOptions) {
        std::cout << key << ": " << value << std::endl;
    }
}

void MENU::handleMenu() const {
    int choice;
    std::cout << "Enter your choice: ";
    std::cin >> choice;

    auto it = menuActions.find(choice);
    if (it != menuActions.end()) {
        it->second(); // Execute the corresponding action
    } else {
        std::cout << "Invalid choice." << std::endl;
    }
}
*/

void MENU::checkButtonClick(int mouseX, int mouseY) const {
    for (size_t i = 0; i < buttonRects.size(); ++i) {
        const SDL_Rect& buttonRect = buttonRects[i];
        if (mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
            mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h) {
            auto it = menuActions.find(i + 1); // Button index corresponds to menu option key
            if (it != menuActions.end()) {
                it->second(); // Execute the corresponding action
            }
        }
    }
}

void PAUSEMENU::initializeActions(bool& paused, bool& running, bool& inMenu) {
    actions[0] = [&paused]() { paused = false; }; // Resume
    actions[1] = [&paused, &inMenu]() {
        // Reset game state and return to the main menu
        paused = false;
        inMenu = true; // Transition back to the main menu
        std::cout << "Returning to the main menu...\n";
    };
    actions[2] = [&running]() { running = false; }; // Quit
}

void PAUSEMENU::render(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color textColor = {0, 0, 0, 255}; // Black text color
    SDL_Color highlightColor = {255, 0, 0, 255}; // Red text color for the selected option

    for (size_t i = 0; i < options.size(); ++i) {
        // Button dimensions
        int buttonWidth = 200;
        int buttonHeight = 50;
        int buttonX = SCREEN_WIDTH / 2 - buttonWidth / 2;
        int buttonY = SCREEN_HEIGHT / 2 - (int(options.size()) * buttonHeight) / 2 + i * (buttonHeight + 10);

        // Render the button background
        SDL_Rect buttonRect = {buttonX, buttonY, buttonWidth, buttonHeight};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White button
        SDL_RenderFillRect(renderer, &buttonRect);

        // Render the button border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black border
        SDL_RenderDrawRect(renderer, &buttonRect);

        // Render the option text
        SDL_Color currentTextColor = (i == selectedOption) ? highlightColor : textColor;
        SDL_Surface* surface = TTF_RenderText_Blended(font, options[i].c_str(), currentTextColor); // Use Blended for better quality
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) {
                int textWidth, textHeight;
                SDL_QueryTexture(texture, nullptr, nullptr, &textWidth, &textHeight);
                SDL_Rect textRect = {
                    buttonX + (buttonWidth - textWidth) / 2,
                    buttonY + (buttonHeight - textHeight) / 2,
                    textWidth,
                    textHeight
                };
                SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                SDL_DestroyTexture(texture);
            }
        }
    }
}

void PAUSEMENU::checkButtonClick(int mouseX, int mouseY, bool& paused, bool& running) {
    for (size_t i = 0; i < options.size(); ++i) {
        int buttonWidth = 200;
        int buttonHeight = 50;
        int buttonX = SCREEN_WIDTH / 2 - buttonWidth / 2;
        int buttonY = SCREEN_HEIGHT / 2 - (int(options.size()) * buttonHeight) / 2 + i * (buttonHeight + 10);

        SDL_Rect buttonRect = {buttonX, buttonY, buttonWidth, buttonHeight};
        if (mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
            mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h) {
            actions[i](); // Execute the corresponding action
            break;
        }
    }
}
