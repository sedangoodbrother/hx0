#ifndef MOVESET_H
#define MOVESET_H

#include <vector>
#include <string>
#include <functional>
#include <deque>
#include "character.h" // Include Character definition

struct SpecialAttack {
    std::string name; // Name of the attack
    std::vector<std::string> inputSequence; // Input sequence to trigger the attack
    std::function<void(int)> execute; // Function to execute the attack, parameterized by player ID

    // Constructor
    SpecialAttack(const std::string& name, const std::vector<std::string>& inputSequence, const std::function<void(int)>& execute)
        : name(name), inputSequence(inputSequence), execute(execute) {}
};

struct INITMOVESET {
    std::vector<SpecialAttack> player1SpecialAttacks;
    std::vector<SpecialAttack> player2SpecialAttacks;

    void initialize(Character& player1, Character& player2);
};

#endif // MOVESET_H