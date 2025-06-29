#include "LocalOllamaLLM.hpp"
#include <iostream>
#include <stdexcept>

namespace tinychain { // <--- 重要！把 WriteCallback 的定义放在这里

// libcurl 的全局初始化和清理。
// 这个静态对象确保 curl_global_init() 在 main() 之前被调用，
// 以及 curl_global_cleanup() 在 main() 退出后被调用。
class CurlGlobalInitializer {
public:
    CurlGlobalInitializer() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    ~CurlGlobalInitializer() {
        curl_global_cleanup();
    }
};

static CurlGlobalInitializer curl_initializer; // 在全局作用域创建静态对象

// curl 回调函数，用于将响应数据写入我们的 MemoryStruct。
// 即使在命名空间内，因为它是给 C 库用的回调，依然需要 extern "C"。
extern "C" {
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    // 现在 reinterpret_cast<LocalOllamaLLM::MemoryStruct*> 可以访问了，
    // 因为 WriteCallback 是 LocalOllamaLLM 的友元，且都在 tinychain 命名空间内。
    reinterpret_cast<LocalOllamaLLM::MemoryStruct*>(userp)->buffer.append(static_cast<char*>(contents), realsize);
    return realsize;
}
} // 结束 extern "C" 块

// --- LocalOllamaLLM 类的其他成员函数的实现保持不变 ---

LocalOllamaLLM::LocalOllamaLLM(const std::string& modelName, const std::string& ollamaApiUrl)
    : modelName(modelName), ollamaApiUrl(ollamaApiUrl) {
    if (modelName.empty()) {
        throw std::invalid_argument("Model name cannot be empty.");
    }
    if (ollamaApiUrl.empty()) {
        throw std::invalid_argument("Ollama API URL cannot be empty.");
    }
}

LocalOllamaLLM::~LocalOllamaLLM() {
    // curl_global_cleanup() 由 CurlGlobalInitializer 的析构函数处理
}

nlohmann::json LocalOllamaLLM::buildBaseRequestBody(const std::vector<Message>& messages) const {
    nlohmann::json request_body;
    request_body["model"] = modelName;

    nlohmann::json messages_array = nlohmann::json::array();
    for (const auto& msg : messages) {
        messages_array.push_back({
            {"role", msg.getRole()},
            {"content", msg.getContent()}
        });
    }
    request_body["messages"] = messages_array;
    request_body["stream"] = false;
    request_body["thinking"] = false;

    return request_body;
}

nlohmann::json LocalOllamaLLM::performRequest(const nlohmann::json& request_body) const {
    CURL *curl;
    CURLcode res;
    // 现在这里直接使用 MemoryStruct 即可，因为它在同一个命名空间和类作用域下
    MemoryStruct chunk;

    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize cURL.");
    }

    curl_easy_setopt(curl, CURLOPT_URL, ollamaApiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string request_json_str = request_body.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_json_str.c_str());

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::string error_msg = "curl_easy_perform() failed: ";
        error_msg += curl_easy_strerror(res);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error(error_msg);
    }

    nlohmann::json response_json;
    try {
        response_json = nlohmann::json::parse(chunk.buffer);
    } catch (const nlohmann::json::parse_error& e) {
        std::string error_msg = "Failed to parse JSON response: ";
        error_msg += e.what();
        error_msg += "\nRaw response: ";
        error_msg += chunk.buffer;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error(error_msg);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response_json;
}

nlohmann::json LocalOllamaLLM::chat(const std::vector<Message>& messages) const {
    nlohmann::json request_body = buildBaseRequestBody(messages);
    return performRequest(request_body);
}

nlohmann::json LocalOllamaLLM::chatWithTool(const std::vector<Message>& messages, const nlohmann::json& tools) const {
    nlohmann::json request_body = buildBaseRequestBody(messages);
    request_body["tools"] = tools;
    return performRequest(request_body);
}

nlohmann::json LocalOllamaLLM::chatWithStructure(const std::vector<Message>& messages, const nlohmann::json& outputFormat) const {
    nlohmann::json request_body = buildBaseRequestBody(messages);

    nlohmann::json format_wrapper;
    format_wrapper["type"] = "json";
    format_wrapper["schema"] = outputFormat;
    request_body["format"] = format_wrapper;

    nlohmann::json options;
    options["temperature"] = 0.0;
    request_body["options"] = options;

    return performRequest(request_body);
}

nlohmann::json LocalOllamaLLM::stream(const std::vector<Message>& messages, const nlohmann::json* context) const {
    if (context != nullptr) {
        if (context->contains("tools") && context->at("tools").is_array()) {
            return chatWithTool(messages, context->at("tools"));
        } else if (context->contains("outputFormat") && context->at("outputFormat").is_object()) {
            return chatWithStructure(messages, context->at("outputFormat"));
        }
    }
    return chat(messages);
}

} // namespace tinychain <--- 重要！确保整个命名空间闭合