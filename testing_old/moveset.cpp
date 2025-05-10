#include "moveset.h"
#include <iostream>

void INITMOVESET::initialize(Character& player1, Character& player2) {
    // Shared list of moves
    std::vector<SpecialAttack> sharedMoves;

    sharedMoves.push_back(SpecialAttack(
        "Dash Thrust",
        {"right", "right"}, // Default input sequence (Player 1 perspective)
        [&player1, &player2](int playerID) {
            if (playerID == 1) {
                std::cout << "Player 1 executes Dash Thrust!" << std::endl;
                player1.dashDistanceRemaining = 50; // Dash forward 50 pixels
            } else if (playerID == 2) {
                std::cout << "Player 2 executes Dash Thrust!" << std::endl;
                player2.dashDistanceRemaining = 50; // Dash forward 50 pixels
            }
        }
    ));



    // Adapt moves for Player 1
    for (const auto& move : sharedMoves) {
        player1SpecialAttacks.push_back(SpecialAttack(
            move.name,
            move.inputSequence, // Use the default input sequence
            [move](int playerID) { move.execute(playerID); } // Pass playerID to the execute function
        ));
    }

    // Adapt moves for Player 2
    for (const auto& move : sharedMoves) {
        // Flip the input sequence for Player 2
        std::vector<std::string> flippedInputSequence = move.inputSequence;
        for (auto& input : flippedInputSequence) {
            if (input == "right") input = "left";
            else if (input == "left") input = "right";
        }

        player2SpecialAttacks.push_back(SpecialAttack(
            move.name,
            flippedInputSequence,
            [move](int playerID) { move.execute(playerID); } // Pass playerID to the execute function
        ));
    }

    std::cout << "Special attacks initialized for both players." << std::endl;
}