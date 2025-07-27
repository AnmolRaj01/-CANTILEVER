#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream> // Required for std::ostringstream
#include <map>
#include <algorithm>
#include <ctime>   // Required for std::time, std::localtime, std::strftime, std::time_t
#include <limits>  // Required for numeric_limits
#include <cstddef> // Required for NULL

// --- Helper Functions ---

// Function to get current timestamp as a string
std::string getCurrentTimestamp() {
    std::time_t now = std::time(NULL); // Replaced nullptr with NULL
    char buf[100];
    // Use std::localtime and std::strftime for formatting the time
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}

// Function to sanitize string for file storage (replace delimiters)
// This prevents issues when reading data back using '|' as a delimiter.
std::string sanitizeString(const std::string& str) {
    std::string sanitized = str;
    std::replace(sanitized.begin(), sanitized.end(), '|', '_'); // Replace '|' with '_'
    std::replace(sanitized.begin(), sanitized.end(), '\n', ' '); // Replace newline with space
    return sanitized;
}

// Function to desanitize string for display (if needed, though not strictly necessary for this simple case)
// For this simple app, we just replace '|' with '_' during sanitization.
// Desanitization might involve replacing '_' back to '|' if it was a meaningful delimiter,
// but for user-entered text, it's usually fine to leave it as is.
std::string desanitizeString(const std::string& str) {
    return str;
}

// --- User Class ---
class User {
public:
    std::string username;
    std::string password;
    std::string bio;

    // Constructor to initialize a User object
    User(std::string username, std::string password, std::string bio)
        : username(username), password(password), bio(bio) {}

    // Default constructor for when loading from file (needed for std::vector operations)
    User() = default;

    // Method to display user profile information
    void displayProfile() const {
        std::cout << "--------------------------------\n";
        std::cout << "Profile: " << username << "\n";
        std::cout << "Bio: " << desanitizeString(bio) << "\n";
        std::cout << "--------------------------------\n";
    }
};

// --- Post Class ---
class Post {
public:
    std::string postId;
    std::string username;
    std::string content;
    std::string timestamp;

    // Constructor to initialize a Post object
    Post(std::string postId, std::string username, std::string content, std::string timestamp)
        : postId(postId), username(username), content(content), timestamp(timestamp) {}

    // Default constructor for when loading from file (needed for std::vector operations)
    Post() = default;

    // Method to display a post's content, author, and timestamp
    void displayPost() const {
        std::cout << "[" << timestamp << "] " << username << ": " << desanitizeString(content) << "\n";
    }
};

// Custom comparator for sorting Posts by timestamp in descending order
struct ComparePostsByTimestamp {
    bool operator()(const Post& a, const Post& b) const {
        return a.timestamp > b.timestamp;
    }
};

// --- SocialNetwork Class ---
class SocialNetwork {
private:
    // Added space between > and > for older compilers (digraph issue)
    std::vector<User> users;
    std::map<std::string, std::vector<std::string> > friendships; // Added space here
    std::vector<Post> posts;
    std::string loggedInUser; // Stores the username of the currently logged-in user

    // File names for data persistence
    const std::string USERS_FILE = "users.txt";
    const std::string FRIENDSHIPS_FILE = "friendships.txt";
    const std::string POSTS_FILE = "posts.txt";

    // Helper to find a user by username in the 'users' vector
    // Made const to be callable from const member functions
    User* findUser(const std::string& username) const { // Added const here
        for (size_t i = 0; i < users.size(); ++i) { // Changed to index-based loop
            if (users[i].username == username) {
                // Need to cast away constness if returning a non-const pointer
                // This is generally discouraged, but necessary if the return type cannot be const User*
                // For this simple app, we'll return a non-const pointer, assuming it's only used for read/write in non-const contexts
                // or where the User object itself is not modified through this pointer in a const context.
                // A better design might be to return const User* or a copy if no modification is intended.
                return const_cast<User*>(&users[i]);
            }
        }
        return NULL; // Replaced nullptr with NULL
    }

    // Helper to check if a user with the given username already exists
    bool userExists(const std::string& username) const { // Added const here
        return findUser(username) != NULL; // Replaced nullptr with NULL
    }

    // Helper to generate a unique post ID using stringstream (more compatible than std::to_string for some compilers)
    std::string generatePostId() {
        std::ostringstream oss;
        oss << "post_" << (posts.size() + 1);
        return oss.str();
    }

public:
    // Constructor: loads data from files when the SocialNetwork object is created
    SocialNetwork() : loggedInUser("") {
        loadData();
    }

    // Destructor: saves data to files when the SocialNetwork object is destroyed
    ~SocialNetwork() {
        saveData();
    }

    // --- Data Management (Load/Save) ---

    // Loads user, friendship, and post data from respective text files
    void loadData() {
        // Load Users
        // Using .c_str() for compatibility with older compilers for ifstream constructor
        std::ifstream userFile(USERS_FILE.c_str());
        if (userFile.is_open()) {
            std::string line;
            while (std::getline(userFile, line)) {
                std::stringstream ss(line);
                std::string username, password, bio;
                std::getline(ss, username, '|');
                std::getline(ss, password, '|');
                std::getline(ss, bio); // Bio is the rest of the line
                users.push_back(User(username, password, bio)); // Changed to push_back
            }
            userFile.close();
            std::cout << "Users loaded from " << USERS_FILE << "\n";
        } else {
            std::cout << "No existing users file found. Creating new one.\n";
        }

        // Load Friendships
        std::ifstream friendshipFile(FRIENDSHIPS_FILE.c_str());
        if (friendshipFile.is_open()) {
            std::string line;
            while (std::getline(friendshipFile, line)) {
                std::stringstream ss(line);
                std::string user1, user2;
                std::getline(ss, user1, '|');
                std::getline(ss, user2); // user2 is the rest of the line
                friendships[user1].push_back(user2);
                friendships[user2].push_back(user1); // Friendships are mutual
            }
            friendshipFile.close();
            std::cout << "Friendships loaded from " << FRIENDSHIPS_FILE << "\n";
        } else {
            std::cout << "No existing friendships file found. Creating new one.\n";
        }

        // Load Posts
        std::ifstream postFile(POSTS_FILE.c_str());
        if (postFile.is_open()) {
            std::string line;
            while (std::getline(postFile, line)) {
                std::stringstream ss(line);
                std::string postId, username, timestamp, content;
                std::getline(ss, postId, '|');
                std::getline(ss, username, '|');
                std::getline(ss, timestamp, '|');
                std::getline(ss, content); // Content is the rest of the line
                posts.push_back(Post(postId, username, content, timestamp)); // Changed to push_back
            }
            postFile.close();
            std::cout << "Posts loaded from " << POSTS_FILE << "\n";
        } else {
            std::cout << "No existing posts file found. Creating new one.\n";
        }
    }

    // Saves user, friendship, and post data to respective text files
    void saveData() const {
        // Save Users
        // Using .c_str() for compatibility with older compilers for ofstream constructor
        std::ofstream userFile(USERS_FILE.c_str());
        if (userFile.is_open()) {
            for (size_t i = 0; i < users.size(); ++i) { // Changed to index-based loop
                userFile << users[i].username << "|" << users[i].password << "|" << sanitizeString(users[i].bio) << "\n";
            }
            userFile.close();
            std::cout << "Users saved to " << USERS_FILE << "\n";
        } else {
            std::cerr << "Error: Could not open " << USERS_FILE << " for writing.\n";
        }

        // Save Friendships (avoid duplicates by only saving one direction, e.g., A|B but not B|A)
        std::ofstream friendshipFile(FRIENDSHIPS_FILE.c_str());
        if (friendshipFile.is_open()) {
            std::map<std::string, std::vector<std::string> > savedPairs; // Added space here
            for (std::map<std::string, std::vector<std::string> >::const_iterator pair_it = friendships.begin(); pair_it != friendships.end(); ++pair_it) { // Added space here
                const std::string& user1 = pair_it->first;
                for (size_t i = 0; i < pair_it->second.size(); ++i) { // Changed to index-based loop
                    const std::string& user2 = pair_it->second[i];
                    // Check if the reverse pair (user2, user1) has already been saved
                    // This prevents saving A|B and B|A as separate entries if they are mutual
                    if (savedPairs.find(user2) == savedPairs.end() ||
                        std::find(savedPairs[user2].begin(), savedPairs[user2].end(), std::string(user1)) == savedPairs[user2].end()) { // Explicitly construct std::string
                        friendshipFile << user1 << "|" << user2 << "\n";
                    }
                }
                savedPairs[user1] = pair_it->second; // Mark user1's friends as saved
            }
            friendshipFile.close();
            std::cout << "Friendships saved to " << FRIENDSHIPS_FILE << "\n";
        } else {
            std::cerr << "Error: Could not open " << FRIENDSHIPS_FILE << " for writing.\n";
        }

        // Save Posts
        std::ofstream postFile(POSTS_FILE.c_str());
        if (postFile.is_open()) {
            for (size_t i = 0; i < posts.size(); ++i) { // Changed to index-based loop
                postFile << posts[i].postId << "|" << posts[i].username << "|" << posts[i].timestamp << "|" << sanitizeString(posts[i].content) << "\n";
            }
            postFile.close();
            std::cout << "Posts saved to " << POSTS_FILE << "\n";
        } else {
            std::cerr << "Error: Could not open " << POSTS_FILE << " for writing.\n";
        }
    }

    // --- User Management ---

    // Allows a new user to register with a unique username, password, and bio
    void registerUser() {
        std::string username, password, bio;
        std::cout << "Enter desired username: ";
        std::cin >> username;
        // Clear the input buffer after reading username to prevent issues with subsequent getline
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (userExists(username)) {
            std::cout << "Username already taken. Please choose another.\n";
            return;
        }

        std::cout << "Enter password: ";
        std::cin >> password;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer

        std::cout << "Enter a short bio (e.g., 'Loves coding'): ";
        std::getline(std::cin, bio); // Use getline for bio as it can contain spaces

        users.push_back(User(username, password, bio)); // Changed to push_back
        saveData(); // Save immediately after a new user is registered
        std::cout << "User " << username << " registered successfully!\n";
    }

    // Allows an existing user to log in
    bool loginUser() {
        std::string username, password;
        std::cout << "Enter username: ";
        std::cin >> username;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer

        std::cout << "Enter password: ";
        std::cin >> password;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer

        User* user = findUser(username);
        if (user && user->password == password) {
            loggedInUser = username;
            std::cout << "Welcome, " << loggedInUser << "!\n";
            return true;
        } else {
            std::cout << "Invalid username or password.\n";
            return false;
        }
    }

    // Logs out the current user
    void logoutUser() {
        loggedInUser = "";
        std::cout << "Logged out successfully.\n";
    }

    // Checks if any user is currently logged in
    bool isLoggedIn() const {
        return !loggedInUser.empty();
    }

    // Returns the username of the currently logged-in user
    std::string getLoggedInUser() const {
        return loggedInUser;
    }

    // --- Friend Management ---

    // Allows the logged-in user to add another user as a friend
    void addFriend() {
        if (!isLoggedIn()) {
            std::cout << "Please log in to add friends.\n";
            return;
        }

        std::string friendUsername;
        std::cout << "Enter username of person to add as friend: ";
        std::cin >> friendUsername;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer

        if (friendUsername == loggedInUser) {
            std::cout << "You cannot add yourself as a friend.\n";
            return;
        }

        if (!userExists(friendUsername)) {
            std::cout << "User " << friendUsername << " does not exist.\n";
            return;
        }

        // Check if already friends to avoid duplicates
        std::map<std::string, std::vector<std::string> >::iterator it = friendships.find(loggedInUser); // Added space here
        if (it != friendships.end()) {
            if (std::find(it->second.begin(), it->second.end(), std::string(friendUsername)) != it->second.end()) { // Explicitly construct std::string
                std::cout << "You are already friends with " << friendUsername << ".\n";
                return;
            }
        }

        // Add friendship (mutual relationship)
        friendships[loggedInUser].push_back(friendUsername);
        friendships[friendUsername].push_back(loggedInUser);
        saveData(); // Save immediately after adding a friend
        std::cout << "You are now friends with " << friendUsername << "!\n";
    }

    // Allows the logged-in user to remove a friend
    void removeFriend() {
        if (!isLoggedIn()) {
            std::cout << "Please log in to remove friends.\n";
            return;
        }

        std::string friendUsername;
        std::cout << "Enter username of person to remove from friends: ";
        std::cin >> friendUsername;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer

        std::map<std::string, std::vector<std::string> >::iterator it = friendships.find(loggedInUser); // Added space here
        if (it == friendships.end()) {
            std::cout << "You have no friends to remove.\n";
            return;
        }

        // Remove from current user's friend list
        std::vector<std::string>& currentUsersFriends = it->second; // Changed to explicit reference
        // Use std::remove-erase idiom to remove the friend
        std::vector<std::string>::iterator friendIt = std::remove(currentUsersFriends.begin(), currentUsersFriends.end(), std::string(friendUsername)); // Explicitly construct std::string
        if (friendIt == currentUsersFriends.end()) {
            std::cout << "You are not friends with " << friendUsername << ".\n";
            return;
        }
        currentUsersFriends.erase(friendIt, currentUsersFriends.end());

        // Remove from target user's friend list (if they exist in the map)
        std::map<std::string, std::vector<std::string> >::iterator targetIt = friendships.find(friendUsername); // Added space here
        if (targetIt != friendships.end()) {
            std::vector<std::string>& targetUsersFriends = targetIt->second; // Changed to explicit reference
            std::vector<std::string>::iterator targetFriendIt = std::remove(targetUsersFriends.begin(), targetUsersFriends.end(), std::string(loggedInUser)); // Explicitly construct std::string
            targetUsersFriends.erase(targetFriendIt, targetUsersFriends.end());
        }

        saveData(); // Save immediately after removing a friend
        std::cout << "You have removed " << friendUsername << " from your friends.\n";
    }

    // Displays the list of friends for a given username
    void viewFriends(const std::string& username) const {
        User* user = findUser(username); // Now calling const findUser
        if (!user) {
            std::cout << "User " << username << " not found.\n";
            return;
        }

        std::cout << "--------------------------------\n";
        std::cout << username << "'s Friends:\n";
        std::map<std::string, std::vector<std::string> >::const_iterator it = friendships.find(username); // Added space here
        if (it != friendships.end() && !it->second.empty()) {
            for (size_t i = 0; i < it->second.size(); ++i) { // Changed to index-based loop
                std::cout << "- " << it->second[i] << "\n";
            }
        } else {
            std::cout << "(No friends yet)\n";
        }
        std::cout << "--------------------------------\n";
    }

    // --- Post Management ---

    // Allows the logged-in user to create and post a new message
    void postMessage() {
        if (!isLoggedIn()) {
            std::cout << "Please log in to post messages.\n";
            return;
        }

        std::string content;
        std::cout << "Enter your message (max 255 chars, no newlines): ";
        std::getline(std::cin, content); // Use getline for message content to capture spaces

        if (content.length() > 255) {
            content = content.substr(0, 255); // Truncate if too long
            std::cout << "Message truncated to 255 characters.\n";
        }

        std::string postId = generatePostId();
        posts.push_back(Post(postId, loggedInUser, content, getCurrentTimestamp())); // Changed to push_back
        saveData(); // Save immediately after posting a message
        std::cout << "Message posted successfully!\n";
    }

    // Views the profile and posts of a specified user
    void viewProfile(const std::string& username) const {
        User* user = findUser(username); // Now calling const findUser
        if (user) {
            user->displayProfile(); // Display user's bio
            std::cout << "Posts by " << username << ":\n";
            bool hasPosts = false;
            for (size_t i = 0; i < posts.size(); ++i) { // Changed to index-based loop
                if (posts[i].username == username) {
                    posts[i].displayPost(); // Display each post by this user
                    hasPosts = true;
                }
            }
            if (!hasPosts) {
                std::cout << "(No posts yet)\n";
            }
        } else {
            std::cout << "User " << username << " not found.\n";
        }
    }

    // Displays the social feed for the logged-in user (their posts and their friends' posts)
    void viewFeed() const {
        if (!isLoggedIn()) {
            std::cout << "Please log in to view your feed.\n";
            return;
        }

        std::cout << "--------------------------------\n";
        std::cout << "Your Social Feed:\n";
        std::cout << "--------------------------------\n";

        std::vector<std::string> usersInFeed;
        usersInFeed.push_back(loggedInUser); // Always include own posts in the feed

        // Add friends' usernames to the list of users whose posts should appear in the feed
        std::map<std::string, std::vector<std::string> >::const_iterator it = friendships.find(loggedInUser); // Added space here
        if (it != friendships.end()) {
            for (size_t i = 0; i < it->second.size(); ++i) { // Changed to index-based loop
                usersInFeed.push_back(it->second[i]);
            }
        }

        std::vector<Post> feedPosts;
        // Collect all relevant posts
        for (size_t i = 0; i < posts.size(); ++i) { // Changed to index-based loop
            if (std::find(usersInFeed.begin(), usersInFeed.end(), std::string(posts[i].username)) != usersInFeed.end()) { // Explicitly construct std::string
                feedPosts.push_back(posts[i]);
            }
        }

        // Sort posts by timestamp in descending order (most recent first)
        std::sort(feedPosts.begin(), feedPosts.end(), ComparePostsByTimestamp()); // Using custom comparator struct

        if (feedPosts.empty()) {
            std::cout << "No posts in your feed yet. Add friends or post something!\n";
        } else {
            for (size_t i = 0; i < feedPosts.size(); ++i) { // Changed to index-based loop
                feedPosts[i].displayPost();
            }
        }
        std::cout << "--------------------------------\n";
    }
};

// --- Main Application Loop ---

// Displays the main menu options when no user is logged in
void displayMainMenu() {
    std::cout << "\n--- Main Menu ---\n";
    std::cout << "1. Register\n";
    std::cout << "2. Login\n";
    std::cout << "3. Exit\n";
    std::cout << "Enter choice: ";
}

// Displays the menu options when a user is logged in
void displayLoggedInMenu() {
    std::cout << "\n--- Logged In Menu ---\n";
    std::cout << "1. View My Profile\n";
    std::cout << "2. View My Friends\n";
    std::cout << "3. Add Friend\n";
    std::cout << "4. Remove Friend\n";
    std::cout << "5. Post Message\n";
    std::cout << "6. View Social Feed\n";
    std::cout << "7. Search User Profile\n";
    std::cout << "8. Logout\n";
    std::cout << "Enter choice: ";
}

// Main function where the application execution begins
int main() {
    SocialNetwork app; // Create an instance of the SocialNetwork application
    std::string choice;

    // Main application loop
    while (true) {
        if (!app.isLoggedIn()) {
            // Display main menu if no user is logged in
            displayMainMenu();
            std::cin >> choice;
            // Clear the input buffer to prevent issues with subsequent getline calls
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (choice == "1") {
                app.registerUser();
            } else if (choice == "2") {
                app.loginUser();
            } else if (choice == "3") {
                std::cout << "Exiting Simple Social Network. Goodbye!\n";
                break; // Exit the loop and terminate the program
            } else {
                std::cout << "Invalid choice. Please try again.\n";
            }
        } else {
            // Display logged-in menu if a user is logged in
            displayLoggedInMenu();
            std::cin >> choice;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer

            if (choice == "1") {
                app.viewProfile(app.getLoggedInUser());
            } else if (choice == "2") {
                app.viewFriends(app.getLoggedInUser());
            } else if (choice == "3") {
                app.addFriend();
            } else if (choice == "4") {
                app.removeFriend();
            } else if (choice == "5") {
                app.postMessage();
            } else if (choice == "6") {
                app.viewFeed();
            } else if (choice == "7") {
                std::string targetUsername;
                std::cout << "Enter username to search: ";
                std::cin >> targetUsername;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer
                app.viewProfile(targetUsername);
            } else if (choice == "8") {
                app.logoutUser();
            } else {
                std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }

    return 0; // Indicate successful program execution
}
