#include "common.h" // Include common.h for global variables
#include "character.h"
#include "menu.h"
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <filesystem> // For checking the working directory
#include <fstream>
std::string weaponType = "Epee"; // Default weapon type

// Winning conditions
 int player1Points = 5; // Player 1's points
 int player2Points = 5; // Player 2's points
 int player1Score = 0; // Define the variable
 int player2Score = 0; // Define the variable
bool gameOver = false; // Flag to indicate if the game is over
std::string winner = ""; // Stores the winner ("Player 1", "Player 2", or "Draw")

int main(int argc, char* argv[]) {
    // Redirect std::cout to both console and game_log.txt
    std::ofstream logFile("game_log.txt", std::ios::app);
    if (!logFile) {
        std::cerr << "Failed to open game_log.txt for logging." << std::endl;
        return -1;
    }
    std::streambuf* consoleBuf = std::cout.rdbuf();
    std::streambuf* logBuf = logFile.rdbuf();
    std::ostream dualOutput(consoleBuf);
    dualOutput.rdbuf(logBuf);
    std::cout.rdbuf(dualOutput.rdbuf());

    Uint32 lastInputTimePlayer1 = 0; // Timestamp of the last input for Player 1
    Uint32 lastInputTimePlayer2 = 0; // Timestamp of the last input for Player 2
    const Uint32 inputCooldown = 50; // Cooldown duration in milliseconds
    bool paused = false; // Track whether the game is paused
    bool pauseMenuNeedsUpdate = true; // Set to true when the pause menu needs to be re-rendered (prevent flickering)

    // Print the current working directory for debugging
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

    // Construct paths dynamically
    std::string fontPath = std::filesystem::current_path().string() + "/font/Arian LT Light.ttf";
    std::string backgroundPath = std::filesystem::current_path().string() + "/assets/background.png";

    // Check if the font file exists
    if (!std::filesystem::exists(fontPath)) {
        std::cerr << "Error: File " << fontPath << " does not exist." << std::endl;
        return -1;
    }

    // Check if the background texture file exists
    if (!std::filesystem::exists(backgroundPath)) {
        std::cerr << "Error: File " << backgroundPath << " does not exist." << std::endl;
        return -1;
    }

    // Load key mappings from input.txt
    loadKeyMappings("input.txt");

    INITSDL app("OFFencing", SCREEN_WIDTH, SCREEN_HEIGHT, fontPath);


    // Load background texture
    SDL_Texture* backgroundTexture = LOADTEXTURE(backgroundPath.c_str(), app.renderer);
    if (!backgroundTexture) {
        ERRORMSG("Failed to load background texture", SDL_GetError());
        return -1;
    }

    // Initialize and display the menu
    SDL_Color textColor = {255, 255, 255, 255}; // White color for text
    bool inMenu = true; // Track whether the game is in the menu state

    MENU menu = {
        { // menuOptions
            {1, "Start with Epee"},
            {2, "Start with Sabre"},
            {3, "Exit"}
        },
        { // menuActions
            {1, [&inMenu]() { 
                std::cout << "Starting the game with Epee...\n"; 
                weaponType = "Epee"; // Set weapon type
                inMenu = false; // Exit menu state and transition to the game
            }},
            {2, [&inMenu]() { 
                std::cout << "Starting the game with Sabre...\n"; 
                weaponType = "Sabre"; // Set weapon type
                inMenu = false; // Exit menu state and transition to the game
            }},
            {3, []() { 
                std::cout << "Exiting the game...\n"; 
                exit(0); // Exit the game
            }}
        },
        app.renderer,   // Pass the SDL_Renderer
        app.font,       // Pass the TTF_Font
        SDL_Color{0, 0, 0, 255} // Black text color
    };
    PAUSEMENU pause; //create pause menu

    // Render the menu once before entering the game loop
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(app.renderer);

    // Render the background for the menu
    SDL_Rect menuBackgroundRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(app.renderer, backgroundTexture, nullptr, &menuBackgroundRect);

    menu.renderMenu();
    menu.presentMenu();

    // Create characters
    Character player1(app.renderer, 50, SCREEN_HEIGHT - 300, false); // Player 1 starts at (100, 400)
    player1.setWeaponType(weaponType);
    player1.loadTexture("idle", "assets/player_idle.png");
    player1.loadTexture("attack", "assets/player_attack.png");
    player1.loadTexture("parry_low", "assets/player_parry_down.png");
    player1.loadTexture("parry_mid", "assets/player_parry_mid.png");
    player1.loadTexture("parry_high", "assets/player_parry_up.png");
    player1.loadAnimationFrames("strike_lowhigh", {"assets/player_strike_higher1.png", "assets/player_strike_higher2.png"}, app.renderer);
    player1.loadAnimationFrames("strike_highlow", {"assets/player_strike_lower1.png", "assets/player_strike_lower2.png"}, app.renderer);
    player1.initializeHurtbox();

    Character player2(app.renderer, SCREEN_WIDTH - 350, SCREEN_HEIGHT - 300, true); // Player 2 starts at (600, 400)
    player2.setWeaponType(weaponType);
    player2.loadTexture("idle", "assets/player_idle.png");
    player2.loadTexture("attack", "assets/player_attack.png");
    player2.loadTexture("parry_low", "assets/player_parry_down.png");
    player2.loadTexture("parry_mid", "assets/player_parry_mid.png");
    player2.loadTexture("parry_high", "assets/player_parry_up.png");
    player2.loadAnimationFrames("strike_lowhigh", {"assets/player_strike_higher1.png", "assets/player_strike_higher2.png"}, app.renderer);
    player2.loadAnimationFrames("strike_highlow", {"assets/player_strike_lower1.png", "assets/player_strike_lower2.png"}, app.renderer);
    player2.initializeHurtbox();

    std::deque<std::string> player1InputBuffer; // Stores recent inputs for Player 1
    std::deque<std::string> player2InputBuffer; // Stores recent inputs for Player 2
    // player1.initializeMovelist(); outdated functions
    // player2.initializeMovelist();

    // Load movement animation frames
    player1.loadAnimationFrames("forward", {
        "assets/player_mov1.png",
        "assets/player_mov2.png",
        "assets/player_mov3.png"
    }, app.renderer);

    player2.loadAnimationFrames("forward", {
        "assets/player_mov1.png",
        "assets/player_mov2.png",
        "assets/player_mov3.png"
    }, app.renderer);

    // Input buffers for both players
    InputBuffer player1Buffer;
    InputBuffer player2Buffer;

    // Game loopx
    const int FPS = 60;
    const int frameDelay = 1000 / FPS; // 16.67ms per frame
    int frameTime;

    bool running = true;
    SDL_Event event;

    // Initialize the pause menu actions
    pause.initializeActions(paused, running, inMenu);
    
    int frameCount = 0;
    Uint32 lastTime = SDL_GetTicks();
    int fps = 0;

    // Add timer and period management
    Uint32 periodStartTime = SDL_GetTicks();
    int currentPeriod = 1; // Start with the first period
    bool suddenDeath = false; // Track if the game is in sudden-death mode
    
    Uint32 frameStart, frameEnd;
    float deltaTime = 0.0f;

    InputHistory player1InputHistory;
    InputHistory player2InputHistory;

    while (running) {
        frameStart = SDL_GetTicks(); // Start of the frame

        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
    
            if (inMenu) {
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;
    
                    // Check for button clicks in the main menu
                    menu.checkButtonClick(mouseX, mouseY);
                }
                continue; // Skip the rest of the game loop while in the menu
            }
    
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
                paused = !paused; // Toggle the paused state
            }
    
            if (paused) {
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;
    
                    // Reuse the existing checkButtonClick function from menu.cpp
                    pause.checkButtonClick(mouseX, mouseY, paused, running);
                } else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_UP) {
                        pause.selectedOption = (pause.selectedOption - 1 + pause.options.size()) % pause.options.size();
                    } else if (event.key.keysym.sym == SDLK_DOWN) {
                        pause.selectedOption = (pause.selectedOption + 1) % pause.options.size();
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                        pause.actions[pause.selectedOption](); // Call the corresponding action
                    }
                }
                continue; // Skip the rest of the game loop
            }

            if (!inMenu) {
                processInput(event, player1InputHistory, player1Commands, player1, player1KeyMappings);
                processInput(event, player2InputHistory, player2Commands, player2, player2KeyMappings);
            }
    
            if (!inMenu) {
                // Set weapon type and initialize hurtboxes for both players
                player1.setWeaponType(weaponType);
                player1.initializeHurtbox();
                player2.setWeaponType(weaponType);
                player2.initializeHurtbox();

                // Player 1 input
                if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    bool isKeyDown = (event.type == SDL_KEYDOWN);
                    auto it = player1KeyMappings.find(event.key.keysym.sym);
                    if (it != player1KeyMappings.end()) {
                        const std::string& action = it->second;
                        
                        Uint32 currentTime = SDL_GetTicks();
                        if (isKeyDown && (currentTime - lastInputTimePlayer1 >= inputCooldown)) {
                            player1InputBuffer.push_back(action); // Add input to Player 1's buffer
                            lastInputTimePlayer1 = currentTime; // Update the last input time
                            std::cout << "Player 1 Action: " << action << std::endl; // Debug: Log action
                        }

                        if (action == "up") player1Buffer.up = isKeyDown;
                        else if (action == "down") player1Buffer.down = isKeyDown;
                        else if (action == "left") player1Buffer.left = isKeyDown;
                        else if (action == "right") player1Buffer.right = isKeyDown;
                        else if (action == "attack") player1Buffer.attack = isKeyDown;

                        std::cout << "Player 1 Input: " << action << " = " << isKeyDown << std::endl; // Debug log

                    }
                }
    
                    // Player 2 input
                if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    bool isKeyDown = (event.type == SDL_KEYDOWN);
                    auto it2= player2KeyMappings.find(event.key.keysym.sym);
                    if (it2 != player2KeyMappings.end()) {
                        const std::string& action = it2->second;

                        Uint32 currentTime = SDL_GetTicks();
                        if (isKeyDown && (currentTime - lastInputTimePlayer2 >= inputCooldown)) {
                            player2InputBuffer.push_back(action); // Add input to Player 2's buffer
                            lastInputTimePlayer2 = currentTime; // Update the last input time
                            std::cout << "Player 2 Action: " << action << std::endl; // Debug: Log action
                        }

                        if (action == "up") player2Buffer.up = isKeyDown;
                        else if (action == "down") player2Buffer.down = isKeyDown;
                        else if (action == "left") player2Buffer.left = isKeyDown; // Ensure "left" is handled
                        else if (action == "right") player2Buffer.right = isKeyDown;
                        else if (action == "attack") player2Buffer.attack = isKeyDown;

                        std::cout << "Player 1 Input: " << action << " = " << isKeyDown << std::endl; // Debug log

                        if (isKeyDown) {
                            player2InputBuffer.push_back(action); // Add input to Player 2's buffer
                            std::cout << "Player 2 Action: " << action << std::endl; // Debug: Log action
                        }
                    }
                }
            }
            if (!inMenu) {
                // Update input buffers
                processPlayerInput(event, player1InputBuffer, player1KeyMappings);
                processPlayerInput(event, player2InputBuffer, player2KeyMappings);
            }
        }

        
        Uint32 currentTimestamp = SDL_GetTicks();

        // Process inputs for Player 1
        player1.processInputBuffer(player1Buffer);

        // Process inputs for Player 2
        player2.processInputBuffer(player2Buffer);


        // Update the state of Player 1 and Player 2
        player1.updateState(player2);
        player2.updateState(player1);

        // Limit the size of the input buffer for Player 1
        if (player1InputBuffer.size() > 10) {
            player1InputBuffer.pop_front();
        }

        if (player1.checkCollision(player2.hurtbox)) {
            // Check if Player 2 is not performing a parry action
            if (player2.getCurrentAction() != "parry_high" &&
                player2.getCurrentAction() != "parry_low" &&
                player2.getCurrentAction() != "parry_mid") {
                handleRoundEnd(player2, player1, player1, player2, player1Points, player2Points, periodStartTime);
            } else {
                std::cout << "Player 2 successfully parried Player 1's attack!" << std::endl;
            }
        } else if (player2.checkCollision(player1.hurtbox)) {
            // Check if Player 1 is not performing a parry action
            if (player1.getCurrentAction() != "parry_high" &&
                player1.getCurrentAction() != "parry_low" &&
                player1.getCurrentAction() != "parry_mid") {
                handleRoundEnd(player1, player2, player1, player2, player1Points, player2Points, periodStartTime);
            } else {
                std::cout << "Player 1 successfully parried Player 2's attack!" << std::endl;
            }
        }
    
        if (player1Points <= 0) {
            winner = "Player 2";
            running = false; // End the game
        } else if (player2Points <= 0) {
            winner = "Player 1";
            running = false; // End the game
        } else if (SDL_GetTicks() - periodStartTime >= 180000) { // Timer runs out (3 minutes)
            if (player1Points > player2Points) {
                winner = "Player 1";
            } else if (player2Points > player1Points) {
                winner = "Player 2";
            } else {
                winner = "Draw";
            }
            running = false; // End the game
        }
        
        // Limit the size of the input buffer for Player 2
        if (player2InputBuffer.size() > 10) {
            player2InputBuffer.pop_front();
        }
    
        // Update Player 1's position
        if (player1.dashDistanceRemaining > 0) {
            float dashStep = 100 * deltaTime; // Adjust Dash Thrust speed to 100 pixels per second
            dashStep = std::min(dashStep, player1.dashDistanceRemaining); // Clamp dashStep
            player1.position.x += dashStep; // Move forward
            player1.dashDistanceRemaining -= dashStep; // Reduce the remaining distance
            if (player1.dashDistanceRemaining <= 0) {
                player1.dashDistanceRemaining = 0; // Ensure it doesn't go negative
                std::cout << "Player 1 dash ended." << std::endl;
            }
        } else {
            // Normal movement logic
            player1.move(SCREEN_WIDTH, SCREEN_HEIGHT, deltaTime);
        }

        // Update Player 2's position
        if (player2.dashDistanceRemaining > 0) {
            float dashStep = 100 * deltaTime; // Adjust Dash Thrust speed to 100 pixels per second
            dashStep = std::min(dashStep, player2.dashDistanceRemaining); // Clamp dashStep
            player2.position.x += dashStep; // Move forward
            player2.dashDistanceRemaining -= dashStep; // Reduce the remaining distance
            if (player2.dashDistanceRemaining <= 0) {
                player2.dashDistanceRemaining = 0; // Ensure it doesn't go negative
                std::cout << "Player 2 dash ended." << std::endl;
            }
        } else {
            // Normal movement logic
            player2.move(SCREEN_WIDTH, SCREEN_HEIGHT, deltaTime);
        }
        // Main menu rendering

        if (inMenu) {
            // Reset game-specific variables when returning to the main menu
            player1.reset(); // Reset Player 1
            player2.reset(); // Reset Player 2
            periodStartTime = SDL_GetTicks(); // Reset the timer
            currentPeriod = 1; // Reset the period
            suddenDeath = false; // Reset sudden-death mode

            // Render the background for the menu
            SDL_Rect menuBackgroundRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(app.renderer, backgroundTexture, nullptr, &menuBackgroundRect);

            // Render the main menu
            menu.renderMenu();
            SDL_RenderPresent(app.renderer); // Present the updated frame
            continue; // Skip the rest of the game loop
        }
    
        // Pause menu rendering
        if (paused) {
            // Render the game background as the pause menu background
            SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(app.renderer, backgroundTexture, nullptr, &destRect);
    
            // Render a semi-transparent overlay
            SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND); // Enable blending
            SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128); // Black with 50% transparency
            SDL_RenderFillRect(app.renderer, &destRect);
    
            // Render the pause menu on top of the background
            pause.render(app.renderer, app.font); // Render the pause menu
            SDL_RenderPresent(app.renderer); // Present the updated frame
            continue; // Skip the rest of the game loop
        }

        player1.move(SCREEN_WIDTH, SCREEN_HEIGHT, deltaTime);
        player2.move(SCREEN_WIDTH, SCREEN_HEIGHT, deltaTime);

        if (!inMenu) {
            // Game rendering logic
            SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255); // Black background
            SDL_RenderClear(app.renderer); // Clear the screen only when not paused
    
            // Render the game background
            SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(app.renderer, backgroundTexture, nullptr, &destRect);
    
            // Process inputs
            player1.processInputBuffer(player1Buffer);
            player2.processInputBuffer(player2Buffer);

            // player1.handleParry(player1Buffer);
            // player1.renderCurrentAction(app.renderer); // Render Player 1's current action
            // player2.handleParry(player2Buffer);
            // player2.renderCurrentAction(app.renderer); // Render Player 2's current action
            // Update game logic
            player1.updatePosition(player2); // Prevent overlap
            player2.updatePosition(player1); // Prevent overlap

            // Ensure hurtboxes are updated after movement
            player1.initializeHurtbox();
            player2.initializeHurtbox();

            // Play movement animations or render idle state
            if (player1Buffer.right) {
                player1.playMovementAnimation(app.renderer, false); // Player 1 moves forward (right)
            } else if (player1Buffer.left) {
                player1.playMovementAnimation(app.renderer, true); // Player 1 moves backward (left)
            } else {
                player1.render(app);
            }

            if (player2Buffer.left) {
                player2.playMovementAnimation(app.renderer, false); // Player 2 moves forward (left)
            } else if (player2Buffer.right) {
                player2.playMovementAnimation(app.renderer, true); // Player 2 moves backward (right)
            } else {
                player2.render(app);
            }
    
            // Render FPS counter
            renderFPSCounter(app.font, app.renderer, fps);
    
            // Render scores, timer, and period
            player1.manageGamePeriods(periodStartTime, currentPeriod, suddenDeath, running);
            player1.renderGameInfo(app.renderer, app.font, currentPeriod, SDL_GetTicks() - periodStartTime);    
            SDL_RenderPresent(app.renderer); // Present the game frame
        }
    
        // Frame rate control
        frameEnd = SDL_GetTicks(); // End of the frame
        deltaTime = (frameEnd - frameStart) / 1000.0f; // Calculate deltaTime in seconds

        frameTime = frameEnd - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime); // Delay to maintain consistent frame rate
        }
    
        // FPS calculation
        frameCount++;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1000) { // Update FPS every second
            fps = frameCount;
            frameCount = 0;
            lastTime = currentTime;
            std::cout << "FPS:" << " " << fps << std::endl;
        }
    }

    renderWinningScreen(app.renderer, app.font, winner);

    // Cleanup
    SDL_DestroyTexture(backgroundTexture);
    player1.cleanupAnimationFrames();
    player2.cleanupAnimationFrames();
    player1.cleanup();
    player2.cleanup();
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}