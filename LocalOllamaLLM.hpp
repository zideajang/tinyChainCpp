#ifndef LOCAL_OLLAMA_LLM_HPP
#define LOCAL_OLLAMA_LLM_HPP

#include "Message.hpp"
#include <string>
#include <vector>
#include <stdexcept>

#include "json.hpp"
#include <curl/curl.h>

namespace tinychain { // 注意：这是我们的命名空间

// 前向声明 WriteCallback 函数。
// 即使它现在会定义在 tinychain 命名空间内，由于是给 C 库用的回调，
// 仍然需要 extern "C" 来确保 C 链接兼容性。
extern "C" {
    size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
}

class LocalOllamaLLM {
private:
    std::string modelName;
    std::string ollamaApiUrl;

    // 结构体用于存储 curl 响应数据
    struct MemoryStruct {
        std::string buffer;
    };

    // <--- 在这里添加 friend 声明！
    // 声明 WriteCallback 是 LocalOllamaLLM 的友元函数，这样它就能访问 private 成员了。
    // 因为 WriteCallback 会在 tinychain 命名空间内定义，所以这里的 friend 声明会正确匹配。
    friend size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    // 辅助方法，执行实际的 HTTP POST 请求
    nlohmann::json performRequest(const nlohmann::json& request_body) const;

    // 辅助方法，构建通用的基础请求体 JSON
    nlohmann::json buildBaseRequestBody(const std::vector<Message>& messages) const;

public:
    // 构造函数
    LocalOllamaLLM(const std::string& modelName, const std::string& ollamaApiUrl = "http://localhost:11434/api/chat");

    // 析构函数
    ~LocalOllamaLLM();

    // 核心聊天方法
    nlohmann::json chat(const std::vector<Message>& messages) const;

    // 带工具的聊天方法
    nlohmann::json chatWithTool(const std::vector<Message>& messages, const nlohmann::json& tools) const;

    // 带结构化输出的聊天方法
    nlohmann::json chatWithStructure(const std::vector<Message>& messages, const nlohmann::json& outputFormat) const;

    // Stream 函数（调度器）
    nlohmann::json stream(const std::vector<Message>& messages, const nlohmann::json* context = nullptr) const;
};

} // namespace tinychain

#endif // LOCAL_OLLAMA_LLM_HPP