#ifndef MENU_H
#define MENU_H

#include <map>
#include <functional>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "character.h"
#include "common.h"

struct MENU {
    std::map<int, std::string> menuOptions;
    std::map<int, std::function<void()>> menuActions;

    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Color textColor = {0, 0, 0, 255};
    mutable std::vector<SDL_Rect> buttonRects; //mutable to able to modify renderMenu, which is declared as const
    // DO NOT, UNDER ANY CIRCUMSTANCE, REPLACE OR MODIFY LINE 21. ANY MODIFICATIONS WILL LEAD TO SEGMENTATION FAULT ERROR
    MENU(); // Default constructor
    MENU(const std::map<int, std::string>& options, const std::map<int, std::function<void()>>& actions, SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor); // Parameterized constructor

    void displayMenu() const;
    void handleMenu() const;

    void renderMenu() const; // Render the menu using SDL
    void presentMenu() const; // Present the rendered menu

    void checkButtonClick(int mouseX, int mouseY) const; // Check for mouse clicks on buttons
};

struct PAUSEMENU {
    std::vector<std::string> options = {"Resume", "Return to main menu", "Quit"};
    std::unordered_map<int, std::function<void()>> actions;
    int selectedOption = 0;
    void initializeActions(bool& paused, bool& running, bool& inMenu);
    void render(SDL_Renderer* renderer, TTF_Font* font);
    void checkButtonClick(int mouseX, int mouseY, bool& paused, bool& running);
};

#endif // MENU_H
