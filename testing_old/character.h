    #ifndef CHARACTER_H
    #define CHARACTER_H

    // #include "common.h"
    #include <deque> // use for fast interpet of input buffer for command attack, allow 2 or more inputs to be recognized
                    // in a short time window and efficient use of the input buffer.
    #include <SDL.h>
    #include <SDL_image.h>
    #include <unordered_map>
    #include <string>
    #include <vector>
    #include <functional>
    #include <algorithm>
    #include <SDL_ttf.h>

    struct InputBuffer;
    struct INITSDL;

    struct Character {
        SDL_Renderer* renderer;
        std::unordered_map<std::string, SDL_Texture*> actionTextures;
        std::string currentAction;
        int x, y;
        bool flip; // Flips if rendering player 2
        SDL_Rect hurtbox; // Always active
        SDL_Rect hitbox;  // Active only during attack
        Uint32 lastHitTime = 0; // Time of the last hit (in milliseconds)
        Uint32 attackStartTime = 0; // Time when the attack action started
        std::string weaponType = "Epee"; // Default weapon type
        std::unordered_map<std::string, std::deque<Uint32>> inputTimestamps; // Track input timestamps

        SDL_Point position; // Player's position (x, y)
        float velocityX = 0; // Horizontal velocity
        float velocityY = 0; // Vertical velocity (optional, for future use)
        float dashDuration; // Duration of the dash (seconds)
        Uint32 actionStartTime = 0; // Tracks when the current action started
        // Animation-related variables
        std::vector<SDL_Texture*> forwardAnimationFrames; // Store forward movement frames
        int currentFrameIndex = 0; // Track the current frame
        Uint32 lastFrameTime = 0; // Track the time of the last frame update
        const int frameDelay = 100; // Delay between frames in milliseconds
        SDL_Rect positionRect = {0, 0, 300, 300}; // Position and size of the character
        SDL_Rect parryLowHitbox; // Hitbox for the parry_low action
        bool parryLowHitboxActive = false; // Tracks whether the hitbox is active
        int spriteHeight = 300; // Default height of the player's sprite
        int spriteWidth = 300; // Default width of the player's sprite
        std::vector<std::string> animationFrames; // Stores the file paths of animation frames
        Uint32 frameStartTime = 0;                // Start time of the current frame


        

        // Constructor
        Character(SDL_Renderer* renderer, int startX, int startY, bool flip = false)
            : x(startX), y(startY), velocityX(0), renderer(renderer), currentAction("idle"), flip(flip), position({x, y}), dashDuration(0), currentTexture(nullptr), lastParryTime(0) {}
        
        // Member functions
        void loadTexture(const std::string& action, const char* filename);
        void setAction(const std::string& action);
        void handleInput(const SDL_Event& event, const std::unordered_map<SDL_Keycode, std::string>& keyMappings);
        void processInputBuffer(InputBuffer& buffer);
        void move(int windowWidth, int windowHeight, float deltaTime);
        void render(INITSDL& sdlContext);
        void cleanup();
        void reset();
        std::string getCurrentAction() const;

        // Animation functions
        void loadAnimationFrames(const std::string& animationName, const std::vector<std::string>& framePaths, SDL_Renderer* renderer);
        void playMovementAnimation(SDL_Renderer* renderer, bool reverse);    
        void cleanupAnimationFrames();

        // Hurtbox and hitbox management
        void activateHitbox();
        void deactivateHitbox();
        void updateCollisionBoxes();
        bool checkCollision(const SDL_Rect& otherBox) const;
        void handleCollision(Character& opponent);
        void updateState(Character& opponent);
        void resetToIdle();
        void preventOverlap(Character& opponent);
        void updatePosition(Character& opponent);
        void setWeaponType(const std::string& type);
        void initializeHurtbox();
        void initializePosition(bool isPlayer1, int windowWidth, int windowHeight);

        // Command inputs handle
        void trackInput(const std::string& input, Uint32 timestamp);
        void addMove(const std::string& name, const std::vector<std::string>& inputSequence, const std::function<void()>& action);
        void interpretInputs(const std::deque<std::string>& inputBuffer);
        // void initializeMovelist(); outdated function
        void addCommandAttack(const std::vector<std::string>& sequence, const std::function<void()>& action);
        void interpretCommand();
        void updatePosition(float deltaTime);
        void renderCurrentAction(SDL_Renderer* renderer);

        // Game period and rendering logic
        void manageGamePeriods(Uint32& periodStartTime, int& currentPeriod, bool& suddenDeath, bool& running);
        void renderGameInfo(SDL_Renderer* renderer, TTF_Font* font, int currentPeriod, Uint32 elapsedTime);
        void loadActionTexturesForParry(const std::string& action, SDL_Renderer* renderer);
        float dashDistanceRemaining = 0; // Distance remaining for the "Dash Thrust"
        std::deque<std::pair<std::string, Uint32>> inputHistory; // Track recent inputs and their timestamps
        const Uint32 frameWindow = 50; // 3 frames at 60 FPS (1000ms / 60 * 3)


        std::string currentAnimation;  // Current animation state
        SDL_Texture* currentTexture;

        // Track the last time the parry was executed (in milliseconds)
        Uint32 lastParryTime;
        bool isParrying = false;
        Uint32 parryStartTime = 0;        
        void handleParry(InputBuffer& inputBuffer);
    };

    // Declare the global function to load key mappings
    void loadKeyMappings(const std::string& filename);

    // Declare global key mappings for Player 1 and Player 2
    extern std::unordered_map<SDL_Keycode, std::string> player1KeyMappings;
    extern std::unordered_map<SDL_Keycode, std::string> player2KeyMappings;

    struct Command {
        std::vector<std::string> sequence; // Input sequence (e.g., {"down", "attack"})
        std::string action;                // Action to trigger (e.g., "special_attack")
        Uint32 maxTimeGap;                 // Maximum time gap between inputs (in milliseconds)
    };

    struct InputHistory {
        std::deque<std::pair<std::string, Uint32>> inputs; // Input and timestamp
        size_t maxSize = 10; // Limit the size of the history
    
        void addInput(const std::string& input) {
            Uint32 currentTime = SDL_GetTicks();
            inputs.push_back({input, currentTime});
            if (inputs.size() > maxSize) {
                inputs.pop_front(); // Remove the oldest input if history exceeds maxSize
            }
        }
    
        void clear() {
            inputs.clear();
        }
    };
    void processInput(const SDL_Event& event, InputHistory& history, const std::vector<Command>& commands, Character& character, const std::unordered_map<SDL_Keycode, std::string>& keyMappings);
    bool matchCommand(const InputHistory& history, const Command& command);
    void debugInputHistory(const InputHistory& history);
    extern std::vector<Command> player1Commands;
    extern std::vector<Command> player2Commands;


    #endif