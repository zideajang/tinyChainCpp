#include "Message.hpp"
#include <iostream> // Already included in .hpp, but good practice for .cpp

namespace tinychain {

Message::Message(const std::string& role, const std::string& content) {
    if (role.empty()) {
        throw std::invalid_argument("Message role cannot be null or empty.");
    }
    // Content can be empty, so no check for content.empty() as per your Java comment
    this->role = role;
    this->content = content;
}

const std::string& Message::getRole() const {
    return role;
}

const std::string& Message::getContent() const {
    return content;
}

std::ostream& operator<<(std::ostream& os, const Message& msg) {
    os << "Message{role='" << msg.role << "', content='" << msg.content << "'}";
    return os;
}

bool Message::operator==(const Message& other) const {
    return role == other.role && content == other.content;
}

bool Message::operator!=(const Message& other) const {
    return !(*this == other);
}

} // namespace tinychain