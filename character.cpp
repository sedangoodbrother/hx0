#include "character.h"
#include "common.h"
#include <SDL_image.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <functional>
#include <SDL_ttf.h>


std::unordered_map<SDL_Keycode, std::string> player1KeyMappings;
std::unordered_map<SDL_Keycode, std::string> player2KeyMappings;
/**
 * A mapping of SDL_Keycode values to corresponding action names for Player 1/2.
 * 
 * This unordered map is used to associate specific keyboard keys (represented by SDL_Keycode)
 * with their respective action names (as strings) for Player 1/2's controls. It allows for
 * customizable key bindings and efficient lookup of actions based on key presses.
 * 
 * SDL_Keycode is an enumeration provided by the SDL library to represent keyboard keys.
 */

 void loadKeyMappings(const std::string& filename) {
    std::ifstream file(filename);
    std::string action, key;

    while (file >> action >> key) {
        SDL_Keycode keycode = SDL_GetKeyFromName(key.c_str());
        if (keycode != SDLK_UNKNOWN) {
            if (action.rfind("P1_", 0) == 0) {
                player1KeyMappings[keycode] = action.substr(3); // Remove "P1_" prefix
            } else if (action.rfind("P2_", 0) == 0) {
                player2KeyMappings[keycode] = action.substr(3); // Remove "P2_" prefix
            } else {
                std::cerr << "Invalid action prefix in input file: " << action << std::endl;
            }
        } else {
            std::cerr << "Invalid key name in input file: " << key << std::endl;
        }
    }
}

void Character::loadTexture(const std::string& action, const char* filename) {
    SDL_Texture* texture = LOADTEXTURE(filename, renderer);
    if (texture) {
        actionTextures[action] = texture;
    } else {
        SDL_Log("Failed to load texture for action '%s': %s", action.c_str(), IMG_GetError());
    }
}

void Character::setAction(const std::string& action) {
    if (action == "strike_lowhigh" || action == "strike_highlow") {
        // Skip texture loading for these actions as they use animations
        currentAction = action;
        std::cout << "Action set to: " << currentAction << " (animation only)" << std::endl;
        return;
    }

    if (actionTextures.find(action) == actionTextures.end()) {
        // Load the texture dynamically if not already loaded
        std::string texturePath = "assets/player_" + action + ".png";
        loadTexture(action, texturePath.c_str());
    }

    if (actionTextures.find(action) != actionTextures.end()) {
        currentAction = action;
        std::cout << "Action set to: " << currentAction << std::endl;

        if (action == "parry_low" || action == "parry_high" || action == "parry_mid") {
            std::cout << action << " activated!" << std::endl;
            deactivateHitbox(); // Deactivate other hitboxes
            actionStartTime = SDL_GetTicks(); // Record the start time of the action

            // Set the current texture for rendering
            currentTexture = actionTextures[action];

            // Activate the parry hitbox
            parryLowHitboxActive = true;

            // Calculate the hitbox position based on the action and player's facing direction
            int hitboxWidth = 50;  // Width of the parry hitbox
            int hitboxHeight = 50; // Height of the parry hitbox

            if (action == "parry_low") {
                // Position for parry_low
                if (flip) {
                    parryLowHitbox = {
                        positionRect.x - hitboxWidth, // Left of the player's texture
                        positionRect.y + positionRect.h / 2, // Lower bottom-half of the texture
                        hitboxWidth,
                        hitboxHeight
                    };
                } else {
                    parryLowHitbox = {
                        positionRect.x + positionRect.w, // Right of the player's texture
                        positionRect.y + positionRect.h / 2, // Lower bottom-half of the texture
                        hitboxWidth,
                        hitboxHeight
                    };
                }
            } else if (action == "parry_high") {
                // Position for parry_high (opposite of parry_low)
                if (flip) {
                    parryLowHitbox = {
                        positionRect.x - hitboxWidth, // Left of the player's texture
                        positionRect.y, // Upper half of the texture
                        hitboxWidth,
                        hitboxHeight
                    };
                } else {
                    parryLowHitbox = {
                        positionRect.x + positionRect.w, // Right of the player's texture
                        positionRect.y, // Upper half of the texture
                        hitboxWidth,
                        hitboxHeight
                    };
                }
            } else if (action == "parry_mid") {
                // Position for parry_mid (centered vertically)
                if (flip) {
                    parryLowHitbox = {            
                        hurtbox.x - hitboxWidth, // Left of the hurtbox
                        hurtbox.y + (hurtbox.h - hitboxHeight) / 2, // Center vertically relative to the hurtbox
                        hitboxWidth,
                        hitboxHeight
                    };
                } else {
                    parryLowHitbox = {
                        hurtbox.x + hurtbox.w, // Right of the hurtbox
                        hurtbox.y + (hurtbox.h - hitboxHeight) / 2, // Center vertically relative to the hurtbox
                        hitboxWidth,
                        hitboxHeight
                    };
                }
            }

            if (action == "strike_lowhigh" || action == "strike_highlow") {
                std::cout << action << " activated!" << std::endl;
                deactivateHitbox(); // Deactivate other hitboxes
                actionStartTime = SDL_GetTicks(); // Record the start time of the action
    
                // Set the animation frames
                if (action == "strike_lowhigh") {
                    animationFrames = {"assets/player_strike_higher1.png", "assets/player_strike_higher2.png"};
                } else if (action == "strike_highlow") {
                    animationFrames = {"assets/player_strike_lower1.png", "assets/player_strike_lower2.png"};
                }
    
                currentFrameIndex = 0; // Start with the first frame
                frameStartTime = SDL_GetTicks(); // Record the start time of the first frame
            }

        } else {
            // Deactivate the parry hitbox for other actions
            parryLowHitboxActive = false;
        }
    } else {
        std::cerr << "Action texture not found for: " << action << std::endl;
    }
}

void Character::handleInput(const SDL_Event& event, const std::unordered_map<SDL_Keycode, std::string>& keyMappings) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        auto it = keyMappings.find(event.key.keysym.sym);
        if (it != keyMappings.end()) {
            const std::string& action = it->second;
            if (event.type == SDL_KEYDOWN) {
                if (action == "left") velocityX = -6; // Move left
                else if (action == "right") velocityX = 6; // Move right
                else if (action == "attack") setAction("attack");
            } else if (event.type == SDL_KEYUP) {
                if (action == "left" || action == "right") velocityX = 0; // Stop movement
                else if (action == "attack") setAction("idle");
            }
        }
    }
}

// void Character::initializePosition(bool isPlayer1, int windowWidth, int windowHeight) {
//     if (isPlayer1) {
//         // Player 1 starts on the left side of the screen
//         x = 50; // Offset from the left border
//         y = windowHeight - 300; // Bottom of the screen
//     } else {
//         // Player 2 starts on the right side of the screen
//         x = windowWidth - 350; // Ensure the entire texture is visible
//         y = windowHeight - 300; // Bottom of the screen
//     }
// }
void Character::move(int windowWidth, int windowHeight, float deltaTime) {
    // Debug: Log velocity and position before updating
    std::cout << "Before move: x = " << x << ", velocityX = " << velocityX << std::endl;

    // Update position based on velocity
    x += velocityX * deltaTime;

    // Ensure the character stays within the window bounds
    if (x < 0) x = 0;
    if (x + positionRect.w > windowWidth) x = windowWidth - positionRect.w;

    // Update the positionRect to reflect the new position
    positionRect.x = x;
    positionRect.y = y; // Y-axis remains constant

    // Debug: Log position after updating
    std::cout << "After move: x = " << x << std::endl;
}

void Character::render(INITSDL& sdlContext) {
    // Render the current action's texture
    if (actionTextures.find(currentAction) != actionTextures.end()) {
        SDL_Texture* texture = actionTextures[currentAction];
        SDL_Rect destRect = {x, y, positionRect.w, positionRect.h};
        SDL_RenderCopyEx(sdlContext.renderer, texture, nullptr, &destRect, 0, nullptr, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }

    // Render the parry_low hitbox if active
    if (parryLowHitboxActive) {
        SDL_SetRenderDrawColor(sdlContext.renderer, 0, 255, 0, 128); // Green color with transparency
        SDL_RenderFillRect(sdlContext.renderer, &parryLowHitbox);
    }

    // Debug: Render hurtbox
    SDL_SetRenderDrawColor(sdlContext.renderer, 255, 0, 0, 128); // Red for hurtbox
    SDL_RenderDrawRect(sdlContext.renderer, &hurtbox);

    // Debug: Render hitbox
    SDL_SetRenderDrawColor(sdlContext.renderer, 0, 255, 0, 128); // Green for hitbox
    SDL_RenderDrawRect(sdlContext.renderer, &hitbox);
}

void Character::cleanup() {
    for (auto& pair : actionTextures) {
        SDL_DestroyTexture(pair.second);
    }
    actionTextures.clear();
}

void Character::setWeaponType(const std::string& type) {
    weaponType = type;
}

void Character::initializeHurtbox() {
    int textureWidth = 300;  // Width of the player's texture
    int textureHeight = 300; // Height of the player's texture

    if (weaponType == "Epee") {
        // Default hurtbox dimensions
        if (!flip) {
            // Player 1: Place hurtbox to the left of the center
            hurtbox = {
                x + (textureWidth / 4) - 75, // Offset to the left of the center
                y + (textureHeight - 200) / 2, // Center vertically
                150, // Width of the hurtbox
                200  // Height of the hurtbox
            };
        } else {
            // Player 2: Place hurtbox to the right of the center
            hurtbox = {
                x + (textureWidth * 3 / 4) - 75, // Offset to the right of the center
                y + (textureHeight - 200) / 2, // Center vertically
                150, // Width of the hurtbox
                200  // Height of the hurtbox
            };
        }
    } else if (weaponType == "Sabre") {
        // Sabre hurtbox: half the height, same width, higher on the Y-axis
        if (!flip) {
            // Player 1: Place hurtbox to the left of the center
            hurtbox = {
                x + (textureWidth / 4) - 75, // Offset to the left of the center
                y + (textureHeight - 150) / 2 - 25, // Center vertically and move higher
                150, // Width of the hurtbox
                150  // Height of the hurtbox
            };
        } else {
            // Player 2: Place hurtbox to the right of the center
            hurtbox = {
                x + (textureWidth * 3 / 4) - 75, // Offset to the right of the center
                y + (textureHeight - 150) / 2 - 25, // Center vertically and move higher
                150, // Width of the hurtbox
                150  // Height of the hurtbox
            };
        }
    }
}

void Character::activateHitbox() {
    if (flip) {
        hitbox = {x - 50, y + 100, 50, 50}; // Example dimensions for left-facing attack
    } else {
        hitbox = {x + 150, y + 100, 50, 50}; // Example dimensions for right-facing attack
    }
}

void Character::deactivateHitbox() {
    hitbox = {0, 0, 0, 0}; // Reset hitbox dimensions
}

void Character::updateCollisionBoxes() {
    int textureWidth = 300;  // Width of the player's texture
    int textureHeight = 300; // Height of the player's texture

    // Update hurtbox position dynamically
    if (weaponType == "Epee") {
        if (!flip) {
            hurtbox = {
                x + (textureWidth / 4) - 75,
                y + (textureHeight - 200) / 2,
                150,
                200
            };
        } else {
            hurtbox = {
                x + (textureWidth * 3 / 4) - 75,
                y + (textureHeight - 200) / 2,
                150,
                200
            };
        }
    } else if (weaponType == "Sabre") {
        if (!flip) {
            hurtbox = {
                x + (textureWidth / 4) - 75,
                y + (textureHeight - 150) / 2 - 25,
                150,
                150
            };
        } else {
            hurtbox = {
                x + (textureWidth * 3 / 4) - 75,
                y + (textureHeight - 150) / 2 - 25,
                150,
                150
            };
        }
    }

    // Debug: Log hurtbox position
    std::cout << "Hurtbox Position: (" << hurtbox.x << ", " << hurtbox.y << ")" << std::endl;

    // Update hitbox based on the current action
    if (currentAction == "attack") {
        activateHitbox(); // Activate and position the hitbox during attack
    } else {
        deactivateHitbox(); // Deactivate the hitbox for non-attack actions
    }
}
bool Character::checkCollision(const SDL_Rect& otherBox) const {
    return SDL_HasIntersection(&hitbox, &otherBox);
}

void Character::handleCollision(Character& opponent) {
    // Check if the hitboxes of both players intersect
    if (SDL_HasIntersection(&hitbox, &opponent.hitbox)) {
        std::cout << "Hitbox collision detected between players!" << std::endl;

        // Apply knockback to both players
        if (!flip) { // Player 1 is facing right
            x -= 6; // Knock Player 1 to the left
            opponent.x += 6; // Knock Player 2 to the right
        } else { // Player 1 is facing left
            x += 6; // Knock Player 1 to the right
            opponent.x -= 6; // Knock Player 2 to the left
        }

        // Ensure players stay within screen boundaries
        if (x < 0) x = 0;
        if (x + positionRect.w > SCREEN_WIDTH) x = SCREEN_WIDTH - positionRect.w;

        if (opponent.x < 0) opponent.x = 0;
        if (opponent.x + opponent.positionRect.w > SCREEN_WIDTH) opponent.x = SCREEN_WIDTH - opponent.positionRect.w;

        // Update the positionRect for both players
        positionRect.x = x;
        opponent.positionRect.x = opponent.x;
    }
}

void Character::updateState(Character& opponent) {
    Uint32 currentTime = SDL_GetTicks();

    // Handle strike_lowhigh and strike_highlow logic
    if (currentAction == "strike_lowhigh" || currentAction == "strike_highlow") {
        // Transition to the next frame after 500ms
        if (currentTime - frameStartTime > 500 && currentFrameIndex < animationFrames.size() - 1) {
            currentFrameIndex++;
            frameStartTime = currentTime; // Reset the frame start time
        }

        // Update the hitbox based on the current frame
        if (currentAction == "strike_lowhigh") {
            if (currentFrameIndex == 0) {
                // Frame 1: Hitbox occupies the lower half of the texture
                hitbox = {
                    positionRect.x,
                    positionRect.y + positionRect.h / 2,
                    positionRect.w,
                    positionRect.h / 2
                };
            } else if (currentFrameIndex == 1) {
                // Frame 2: Hitbox's Y-axis value is the center of the texture
                hitbox = {
                    positionRect.x,
                    positionRect.y + positionRect.h / 2 - hitbox.h / 2,
                    positionRect.w,
                    hitbox.h
                };
            }
        } else if (currentAction == "strike_highlow") {
            if (currentFrameIndex == 0) {
                // Frame 1: Hitbox is the same as Frame 2 of strike_lowhigh
                hitbox = {
                    positionRect.x,
                    positionRect.y + positionRect.h / 2 - hitbox.h / 2,
                    positionRect.w,
                    hitbox.h
                };
            } else if (currentFrameIndex == 1) {
                // Frame 2: Hitbox is the same as Frame 1 of strike_lowhigh
                hitbox = {
                    positionRect.x,
                    positionRect.y + positionRect.h / 2,
                    positionRect.w,
                    positionRect.h / 2
                };
            }
        }

        // Reset to idle after the animation ends
        if (currentTime - actionStartTime > 1000) { // 1 second duration
            resetToIdle();
        }
    }

    // Reset the current action to "idle" after a specific duration for other actions
    if (currentAction != "idle" && currentTime - actionStartTime > 1000) { // 1000 ms = 1 second
        resetToIdle();
        parryLowHitboxActive = false; // Deactivate the hitbox if active
    }
}
void Character::processInputBuffer(InputBuffer& buffer) {
    // Handle "left" input
    if (buffer.left) {
        velocityX = -6; // Move left
        buffer.leftFrames = 0;
    } else if (buffer.leftFrames < 2) {
        buffer.leftFrames++;
    } else {
        buffer.left = false; // Reset input after 2 frames
        if (!buffer.right) velocityX = 0; // Stop horizontal movement if no other input is active
    }

    // Handle "right" input
    if (buffer.right) {
        velocityX = 6; // Move right
        buffer.rightFrames = 0;
    } else if (buffer.rightFrames < 2) {
        buffer.rightFrames++;
    } else {
        buffer.right = false; // Reset input after 2 frames
        if (!buffer.left) velocityX = 0; // Stop horizontal movement if no other input is active
    }

    // Handle "up" input
    if (buffer.up) {
        // For jumping or other actions if needed
    }

    // Handle "down" input
    if (buffer.down) {
        // For crouching or other actions if needed
    }

    // Handle "attack" input
    if (buffer.attack) {
        setAction("attack");
        buffer.attackFrames = 0;
    } else if (buffer.attackFrames < 2) {
        buffer.attackFrames++;
    } else {
        buffer.attack = false; // Reset input after 2 frames
    }

    // Handle parry input (check combinations)
    // handleParry(buffer);

    // Debug: Log the updated velocities
    std::cout << "Updated velocityX: " << velocityX << std::endl;
}


void Character::resetToIdle() {
    // Ensure the current action has completed before resetting to idle
    Uint32 currentTime = SDL_GetTicks();
    if (currentAction != "idle" && currentTime - actionStartTime > 1000) { // 1 second duration
        setAction("idle");
    }
}

// Use player1Score and player2Score as needed
void Character::manageGamePeriods(Uint32& periodStartTime, int& currentPeriod, bool& suddenDeath, bool& running) {
    Uint32 elapsedTime = SDL_GetTicks() - periodStartTime;
    if (currentPeriod <= 3 && elapsedTime >= 180000) { // 3 minutes per period
        if (currentPeriod < 3) {
            currentPeriod++;
            periodStartTime = SDL_GetTicks(); // Reset timer for the next period
            std::cout << "Period " << currentPeriod << " started." << std::endl;
        } else {
            if (player1Score == player2Score) {
                suddenDeath = true; // Enter sudden-death mode
                std::cout << "Sudden death started." << std::endl;
            } else {
                running = false; // End the game
                std::cout << "Game over!" << std::endl;
            }
        }
    }

    // Handle sudden death
    if (suddenDeath && (player1Score > player2Score || player2Score > player1Score)) {
        running = false; // End the game when a player scores in sudden death
        std::cout << "Game over!" << std::endl;
    }
}

// Add rendering logic for scores, period, and timer
void Character::renderGameInfo(SDL_Renderer* renderer, TTF_Font* font, int currentPeriod, Uint32 elapsedTime) {
    SDL_Color scoreColor = {255, 255, 255, 255}; // White color for text

    // Render scores
    std::string scoreText = "P1: " + std::to_string(player1Points) + " P2: " + std::to_string(player2Points);
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), scoreColor);
    if (scoreSurface) {
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_FreeSurface(scoreSurface);
        if (scoreTexture) {
            int screenWidth;
            SDL_GetRendererOutputSize(renderer, &screenWidth, nullptr); // Get screen width
            SDL_Rect scoreRect = {
                (screenWidth - scoreSurface->w) / 2, // Center horizontally
                10,                                 // Top margin
                scoreSurface->w,
                scoreSurface->h
            };
            SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);
            SDL_DestroyTexture(scoreTexture);
        }
    }

    // Render current period
    std::string periodText = "Period: " + std::to_string(currentPeriod);
    SDL_Surface* periodSurface = TTF_RenderText_Solid(font, periodText.c_str(), scoreColor);
    if (periodSurface) {
        SDL_Texture* periodTexture = SDL_CreateTextureFromSurface(renderer, periodSurface);
        SDL_FreeSurface(periodSurface);
        if (periodTexture) {
            SDL_Rect periodRect = {10, 40, periodSurface->w, periodSurface->h};
            SDL_RenderCopy(renderer, periodTexture, nullptr, &periodRect);
            SDL_DestroyTexture(periodTexture);
        }
    }

    // Render timer
    std::string timerText = "Time: " + std::to_string((180000 - elapsedTime) / 1000) + "s";
    SDL_Surface* timerSurface = TTF_RenderText_Solid(font, timerText.c_str(), scoreColor);
    if (timerSurface) {
        SDL_Texture* timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);
        SDL_FreeSurface(timerSurface);
        if (timerTexture) {
            SDL_Rect timerRect = {10, 70, timerSurface->w, timerSurface->h};
            SDL_RenderCopy(renderer, timerTexture, nullptr, &timerRect);
            SDL_DestroyTexture(timerTexture);
        }
    }
}

void Character::preventOverlap(Character& opponent) {
    if (SDL_HasIntersection(&hurtbox, &opponent.hurtbox)) {
        // If this player is moving right and collides with the opponent
        if (velocityX > 0 && x + hurtbox.w > opponent.x) {
            x = opponent.x - hurtbox.w; // Stop this player to the left of the opponent
        }
        // If this player is moving left and collides with the opponent
        else if (velocityX < 0 && x < opponent.x + opponent.hurtbox.w) {
            x = opponent.x + opponent.hurtbox.w; // Stop this player to the right of the opponent
        }
    }
}

void Character::updatePosition(Character& opponent) {

    // Calculate next position
    int nextX = x + velocityX;

    // Create a temporary hurtbox for the next position
    SDL_Rect nextHurtbox = {nextX + 25, y + 50, hurtbox.w, hurtbox.h};

    // Check for collision with the opponent's hurtbox
    if (SDL_HasIntersection(&nextHurtbox, &opponent.hurtbox)) {
        // Stop horizontal movement if collision is detected
        if (velocityX > 0 && nextX + hurtbox.w > opponent.x) {
            velocityX = 0;
        } else if (velocityX < 0 && nextX < opponent.x + opponent.hurtbox.w) {
            velocityX = 0;
        }
    }

    // Update position based on velocity
    x += velocityX;

    // Ensure the player stays within the screen boundaries
    if (x < 0) x = 0;
    if (x + 150 > SCREEN_WIDTH) x = SCREEN_WIDTH - 150;

    // Update collision boxes and state
    updateCollisionBoxes();
    updateState(opponent); // Pass the opponent to updateState
}

void Character::loadAnimationFrames(const std::string& animationName, const std::vector<std::string>& framePaths, SDL_Renderer* renderer) {
    for (const auto& path : framePaths) {
        SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
        if (texture) {
            if (animationName == "forward") {
                forwardAnimationFrames.push_back(texture);
            }
        } else {
            std::cerr << "Failed to load texture: " << path << " - " << SDL_GetError() << std::endl;
        }
    }
}

void Character::playMovementAnimation(SDL_Renderer* renderer, bool reverse) { // more dynamic movement animation
    if (SDL_GetTicks() - lastFrameTime > frameDelay) {
        if (reverse) {
            currentFrameIndex = (currentFrameIndex - 1 + forwardAnimationFrames.size()) % forwardAnimationFrames.size();
        } else {
            currentFrameIndex = (currentFrameIndex + 1) % forwardAnimationFrames.size();
        }
        lastFrameTime = SDL_GetTicks();
    }

    // Update the hurtbox dynamically
    updateCollisionBoxes();

    // Render the current animation frame
    positionRect.x = x; // Update positionRect with the current position
    positionRect.y = y;

    SDL_RendererFlip flipType = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, forwardAnimationFrames[currentFrameIndex], nullptr, &positionRect, 0, nullptr, flipType);

    // Render the hurtbox for debugging
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128); // Red for hurtbox
    SDL_RenderDrawRect(renderer, &hurtbox);
}

void Character::cleanupAnimationFrames() {
    for (auto texture : forwardAnimationFrames) {
        SDL_DestroyTexture(texture);
    }
    forwardAnimationFrames.clear();
}

void Character::reset() {
    // Reset position directly
    if (!flip) {
        // Player 1 starts on the left side of the screen
        x = 50; // Offset from the left border
        y = SCREEN_HEIGHT - 300; // Bottom of the screen
    } else {
        // Player 2 starts on the right side of the screen
        x = SCREEN_WIDTH - 350; // Ensure the entire texture is visible
        y = SCREEN_HEIGHT - 300; // Bottom of the screen
    }

    // Reset hurtbox
    initializeHurtbox();

    // Reset action to idle
    resetToIdle();

    // Reset animation frame index
    currentFrameIndex = 0;

    // Reset velocity
    velocityX = 0;

    // Deactivate hitbox
    deactivateHitbox();
}

void Character::trackInput(const std::string& input, Uint32 timestamp) {
    // Keep track of the last few timestamps for the input
    auto& timestamps = inputTimestamps[input];
    timestamps.push_back(timestamp);

    // Limit the size of the deque to avoid excessive memory usage
    if (timestamps.size() > 2) {
        timestamps.pop_front();
    }
}

// void Character::updatePosition(float deltaTime) {
//     if (dashDuration > 0) {
//         position.x += velocity * deltaTime; // Update position based on velocity
//         dashDuration -= deltaTime;         // Reduce dash duration
//         if (dashDuration <= 0) {
//             velocity = 0; // Stop the dash
//         }
//     }
// }

// void Character::reset() {
//     position = {0, 0};
//     velocity = 0;
//     dashDuration = 0;
// }

// bool InputBuffer::hasCombination(const std::string& dir1, const std::string& dir2) const {
//     // Check if the directional inputs match the specified combination
//     if (dir1 == "up" && dir2 == "attack") {
//         return up && attack; // Check if both 'up' and 'attack' are pressed
//     }
//     else if (dir1 == "down" && dir2 == "attack") {
//         return down && attack; // Check if both 'down' and 'attack' are pressed
//     }
//     else if (dir1 == "left" && dir2 == "attack") {
//         return left && attack; // Check if both 'left' and 'attack' are pressed
//     }
//     else if (dir1 == "right" && dir2 == "attack") {
//         return right && attack; // Check if both 'right' and 'attack' are pressed
//     }

//     return false; // If no valid combination is found, return false
// }


// void Character::handleParry(InputBuffer& buffer) {
//     Uint32 currentTime = SDL_GetTicks();

//     if (currentTime - lastParryTime > 500) {
//         if (buffer.parryDownInput(flip)) {
//             if (currentAction != "parry_down") {
//                 setAction("parry_down");
//                 lastParryTime = currentTime;
//                 std::cout << "Down Parry activated!\n";
//                 // Dynamically load the texture only if it's not already loaded
//                 if (!actionTextures["parry_down"]) {
//                     loadActionTexturesForParry("parry_down", renderer); // Loading texture
//                 }
//                 currentTexture = actionTextures["parry_down"];
//                 activateHitbox();
//             }
//         }
//         else if (buffer.parryUpInput(flip)) {
//             if (currentAction != "parry_up") {
//                 setAction("parry_up");
//                 lastParryTime = currentTime;
//                 std::cout << "Up Parry activated!\n";
//                 // Dynamically load the texture only if it's not already loaded
//                 if (!actionTextures["parry_up"]) {
//                     loadActionTexturesForParry("parry_up", renderer); // Loading texture
//                 }
//                 currentTexture = actionTextures["parry_up"];
//                 activateHitbox();
//             }
//         }
//         else if (buffer.parryMidInput(flip)) {
//             if (currentAction != "parry_mid") {
//                 setAction("parry_mid");
//                 lastParryTime = currentTime;
//                 std::cout << "Mid Parry activated!\n";
//                 // Dynamically load the texture only if it's not already loaded
//                 if (!actionTextures["parry_mid"]) {
//                     loadActionTexturesForParry("parry_mid", renderer); // Loading texture
//                 }
//                 currentTexture = actionTextures["parry_mid"];
//                 activateHitbox();
//             }
//         }
//     }
// }

// void Character::loadActionTexturesForParry(const std::string& action, SDL_Renderer* renderer) {
//     // Load textures specifically for parry actions
//     if (action == "parry_up") {
//         actionTextures["parry_up"] = LOADTEXTURE("assets/player_parry_up.png", renderer);
//     } else if (action == "parry_down") {
//         actionTextures["parry_down"] = LOADTEXTURE("assets/player_parry_down.png", renderer);
//     } else if (action == "parry_mid") {
//         actionTextures["parry_mid"] = LOADTEXTURE("assets/player_parry_mid.png", renderer);
//     }
// }

// void Character::renderCurrentAction(SDL_Renderer* renderer) {
//     // Check if there is a texture for the current action
//     if (actionTextures.count(currentAction) == 0) return;

//     // Directly get the texture for the current action (not a vector of textures)
//     SDL_Texture* texture = actionTextures[currentAction];
//     if (texture == nullptr) return;

//     // Set the destination rectangle to render the texture
//     SDL_Rect destRect = {
//         static_cast<int>(position.x),
//         static_cast<int>(position.y),
//         300,
//         300
//     };

//     // Handle flipping based on the player's side (flip)
//     SDL_RendererFlip sdlFlip = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

//     // Render the texture using SDL_RenderCopyEx
//     SDL_RenderCopyEx(renderer, texture, nullptr, &destRect, 0.0, nullptr, sdlFlip);
// }

std::vector<Command> player1Commands = {
    {"parry_low", {"up", "left", "attack"}, 1000}, // Up, Back, Attack within 1 second
    {"parry_high", {"down", "left", "attack"}, 1000}, // Down, Back, Attack within 1 second
    {"parry_mid", {"left", "attack"}, 1000}, // Back, Attack within 1 second
    {"strike_lowhigh", {"down", "attack"}, 1000}, // Strike Low-High for Player 1
    {"strike_highlow", {"up", "attack"}, 1000},   // Strike High-Low for Player 1
};

std::vector<Command> player2Commands = {
    {"parry_low", {"up", "right", "attack"}, 1000}, // Up, Forward, Attack within 1 second
    {"parry_high", {"down", "right", "attack"}, 1000}, // Down, Forward, Attack within 1 second
    {"parry_mid", {"right", "attack"}, 1000}, // Forward, Attack within 1 second
    {"strike_lowhigh", {"down", "attack"}, 1000},  // Strike Low-High for Player 2
    {"strike_highlow", {"up", "attack"}, 1000},    // Strike High-Low for Player 2
};

bool matchCommand(const InputHistory& history, const Command& command) {
    std::cout << "Matching command: " << command.name << std::endl;
    std::cout << "Required inputs: ";
    for (const auto& input : command.requiredInputs) {
        std::cout << input << " ";
    }
    std::cout << std::endl;

    if (history.inputs.empty()) {
        std::cout << "Input history is empty." << std::endl;
        return false;
    }

    for (const auto& input : command.requiredInputs) {
        auto it = std::find_if(history.inputs.begin(), history.inputs.end(), [&](const auto& pair) {
            return pair.first == input;
        });

        if (it == history.inputs.end()) {
            std::cout << "Input missing for command: " << command.name << ", required: " << input << std::endl;
            return false;
        }
    }

    auto earliest = std::min_element(history.inputs.begin(), history.inputs.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    auto latest = std::max_element(history.inputs.begin(), history.inputs.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    if (latest->second - earliest->second > command.maxTimeGap) {
        std::cout << "Inputs for command " << command.name << " are too far apart in time." << std::endl;
        return false;
    }

    std::cout << "Command matched: " << command.name << std::endl;
    return true;
}

void processInput(const SDL_Event& event, InputHistory& history, const std::vector<Command>& commands, Character& character, const std::unordered_map<SDL_Keycode, std::string>& keyMappings) {
    if (event.type == SDL_KEYDOWN) {
        auto it = keyMappings.find(event.key.keysym.sym);
        if (it != keyMappings.end()) {
            const std::string& input = it->second;
            history.addInput(input); // Add input to the player's history

            // Check for matching commands
            bool commandMatched = false;
            for (const auto& command : commands) {
                if (matchCommand(history, command)) {
                    std::cout << "Matched Command: " << command.name << std::endl;
                    character.setAction(command.name); // Trigger the command action
                    commandMatched = true;
                }
            }

            // Clear history only if a command was matched
            if (commandMatched) {
                history.clear();
            }
        }
    }
}

void debugInputHistory(const InputHistory& history) {
    std::cout << "Input History: ";
    for (const auto& [input, timestamp] : history.inputs) {
        std::cout << input << " ";
    }
    std::cout << std::endl;
}

std::string Character::getCurrentAction() const {
    return currentAction;
}