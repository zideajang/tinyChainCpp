#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <stdexcept> // For std::invalid_argument
#include <iostream>  // For std::ostream

namespace tinychain {

class Message {
public:
    std::string role;
    std::string content;

    /**
     * Constructor for Message instances.
     *
     * @param role    Role of the message sender ("system", "assistant", "user")
     * @param content Message content
     * @throws std::invalid_argument if role is null or empty
     */
    Message(const std::string& role, const std::string& content);

    // Getters
    const std::string& getRole() const;
    const std::string& getContent() const;

    // Optional: Overload for printing (for debugging)
    friend std::ostream& operator<<(std::ostream& os, const Message& msg);

    // Optional: Equality comparison
    bool operator==(const Message& other) const;
    bool operator!=(const Message& other) const;
};

} // namespace tinychain

#endif // MESSAGE_HPP