#include <iostream> // Required for input/output operations (cout, cin)
#include <cstdlib>  // Required for random number generation (rand, srand)
#include <ctime>    // Required for time functions (time) to seed the random number generator
#include <limits>   // Required for numeric_limits to clear input buffer
#include <ios>      // Required for streamsize

int main() {
    // Seed the random number generator using the current time.
    // This ensures that a different sequence of random numbers is generated each time the program runs.
    srand(static_cast<unsigned int>(time(0)));

    int randomNumber = 0;
    int guess = 0;
    int attempts = 0;
    int maxAttempts = 0;
    int minRange = 1;
    int maxRange = 100;
    int difficultyChoice = 0;
    char playAgain = 'y';

    // Main game loop: continues as long as the user wants to play
    while (playAgain == 'y' || playAgain == 'Y') {
        // Reset game parameters for a new round
        attempts = 0;
        guess = 0;
        minRange = 1;
        maxRange = 100;
        maxAttempts = 0; // Will be set based on difficulty

        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Welcome to the Number Guessing Game!" << std::endl;
        std::cout << "------------------------------------------" << std::endl;

        // Difficulty selection loop
        while (true) {
            std::cout << "\nChoose a difficulty level:" << std::endl;
            std::cout << "1. Easy (1-50, 7 attempts)" << std::endl;
            std::cout << "2. Medium (1-100, 10 attempts)" << std::endl;
            std::cout << "3. Hard (1-200, 12 attempts)" << std::endl;
            std::cout << "4. Custom (You set the range and attempts)" << std::endl;
            std::cout << "Enter your choice (1-4): ";

            std::cin >> difficultyChoice;

            // Input validation for difficulty choice
            if (std::cin.fail() || difficultyChoice < 1 || difficultyChoice > 4) {
                std::cout << "Invalid input. Please enter a number between 1 and 4." << std::endl;
                std::cin.clear(); // Clear the error flag
                // Ignore remaining characters in the input buffer up to the newline
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else {
                break; // Valid input, exit loop
            }
        }

        // Set game parameters based on difficulty choice
        switch (difficultyChoice) {
            case 1: // Easy
                minRange = 1;
                maxRange = 50;
                maxAttempts = 7;
                break;
            case 2: // Medium
                minRange = 1;
                maxRange = 100;
                maxAttempts = 10;
                break;
            case 3: // Hard
                minRange = 1;
                maxRange = 200;
                maxAttempts = 12;
                break;
            case 4: // Custom
                while (true) {
                    std::cout << "Enter the minimum number for the range: ";
                    std::cin >> minRange;
                    if (std::cin.fail()) {
                        std::cout << "Invalid input. Please enter a number." << std::endl;
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    } else {
                        break;
                    }
                }
                while (true) {
                    std::cout << "Enter the maximum number for the range: ";
                    std::cin >> maxRange;
                    if (std::cin.fail() || maxRange <= minRange) {
                        std::cout << "Invalid input. Please enter a number greater than the minimum." << std::endl;
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    } else {
                        break;
                    }
                }
                while (true) {
                    std::cout << "Enter the maximum number of attempts: ";
                    std::cin >> maxAttempts;
                    if (std::cin.fail() || maxAttempts <= 0) {
                        std::cout << "Invalid input. Please enter a positive number." << std::endl;
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    } else {
                        break;
                    }
                }
                break;
        }

        // Generate a random number within the chosen range
        // (rand() % (maxRange - minRange + 1)) generates a number from 0 to (maxRange - minRange)
        // Adding minRange shifts it to the desired range [minRange, maxRange]
        randomNumber = (rand() % (maxRange - minRange + 1)) + minRange;

        std::cout << "\nI have picked a number between " << minRange << " and " << maxRange << "." << std::endl;
        std::cout << "You have " << maxAttempts << " attempts to guess it." << std::endl;
        std::cout << "Good luck!" << std::endl;
        std::cout << "------------------------------------------" << std::endl;

        // Inner game loop: continues until the user guesses the correct number or runs out of attempts
        while (guess != randomNumber && attempts < maxAttempts) {
            std::cout << "Attempt " << attempts + 1 << "/" << maxAttempts << ". Enter your guess: ";
            std::cin >> guess; // Read the user's guess

            // Input validation for the guess
            if (std::cin.fail()) {
                std::cout << "Invalid input. Please enter a number." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue; // Skip the rest of the loop and ask for input again
            }

            attempts++; // Increment the attempt counter

            // Provide feedback based on the guess
            if (guess > randomNumber) {
                std::cout << "Too high! Try again." << std::endl;
            } else if (guess < randomNumber) {
                std::cout << "Too low! Try again." << std::endl;
            } else {
                // Correct guess
                std::cout << "\nCongratulations! You guessed the number " << randomNumber;
                std::cout << " in " << attempts << " attempts!" << std::endl;
            }
        }

        // Check if the user ran out of attempts
        if (guess != randomNumber) {
            std::cout << "\nSorry, you ran out of attempts! The number was " << randomNumber << "." << std::endl;
        }

        // Ask the user if they want to play another round
        std::cout << "\nDo you want to play again? (y/n): ";
        std::cin >> playAgain; // Read user's choice
        std::cout << std::endl; // Add a newline for better formatting
    }

    std::cout << "Thanks for playing! Goodbye." << std::endl;

    return 0; // Indicate successful program execution
}

