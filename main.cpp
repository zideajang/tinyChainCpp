#include "LocalOllamaLLM.hpp" // 包含 LocalOllamaLLM 类的定义
#include <iostream>           // 用于输入输出（如 cout, cerr）
#include <vector>             // 用于 std::vector 容器
#include "json.hpp"           // 用于直接在 main 函数中创建 JSON 对象（来自 nlohmann/json 库）

int main() {
    try {
        // 实例化 LocalOllamaLLM，传入所需的模型名称。
        // 例如，对于 qwen3:8b 模型，使用 "qwen:8b"（或者你本地拉取的精确标签）。
        // 使用 llama3.2 的示例：
        tinychain::LocalOllamaLLM llm("llama3.2");
        // 使用 qwen3:8b 的示例（如果需要，请取消注释并使用此行）：
        // tinychain::LocalOllamaLLM llm("qwen:8b");

        std::cout << "--- 基础聊天示例（使用 stream 调度器）---\n";
        // 创建一个包含用户消息的 Message 向量
        std::vector<tinychain::Message> chat_messages = {
            {"user", "天空为什么是蓝色的？"} // 用户角色和消息内容
        };
        // 调用 LLM 的 stream 方法发送消息并获取响应
        nlohmann::json chat_response = llm.stream(chat_messages);
        // .dump(2) 用于将 JSON 输出格式化为易读的（缩进为 2 个空格）
        std::cout << "聊天响应：\n" << chat_response.dump(2) << "\n\n";

        std::cout << "--- 带工具的聊天示例（获取当前天气）---\n";
        // 手动创建工具的 JSON 模式，遵循 Ollama 的格式。
        // 在实际应用中，你可能有一个 ToolManager 类来管理这些工具定义，就像你的 Java 示例一样。
        nlohmann::json tools_array = nlohmann::json::array(); // 创建一个 JSON 数组来存放工具
        nlohmann::json weather_tool_schema = { // 定义天气工具的 JSON 模式
            {"type", "function"}, // 工具类型是函数
            {"function", { // 函数的具体定义
                {"name", "get_current_weather"}, // 函数名称
                {"description", "获取一个地点的当前天气"}, // 函数描述
                {"parameters", { // 函数参数定义
                    {"type", "object"}, // 参数类型是 JSON 对象
                    {"properties", { // 参数的属性
                        {"location", {{"type", "string"}, {"description", "要获取天气的地点，例如：旧金山，CA"}}}, // 地点参数
                        {"format", {{"type", "string"}, {"description", "返回天气的格式，例如：'celsius' (摄氏) 或 'fahrenheit' (华氏)"}, {"enum", {"celsius", "fahrenheit"}}}} // 格式参数
                    }},
                    {"required", {"location", "format"}} // 必需的参数
                }}
            }}
        };
        tools_array.push_back(weather_tool_schema); // 将天气工具添加到工具数组中

        // 创建包含用户查询消息的 Message 向量
        std::vector<tinychain::Message> tool_messages = {
            {"user", "巴黎今天天气怎么样？"} // 用户询问天气
        };

        // 创建一个包含 "tools" 数组的上下文 JSON 对象
        nlohmann::json tool_context = {
            {"tools", tools_array}
        };

        // 调用 LLM 的 stream 方法，传入消息和工具上下文
        nlohmann::json tool_response = llm.stream(tool_messages, &tool_context);
        std::cout << "工具响应：\n" << tool_response.dump(2) << "\n\n";

        std::cout << "--- 带结构化输出的聊天示例 ---\n";
        // 定义所需结构化输出的 JSON 模式
        nlohmann::json format_schema = {
            {"type", "object"}, // 输出类型是 JSON 对象
            {"properties", { // 对象的属性
                {"age", {{"type", "integer"}}}, // 年龄属性，整数类型
                {"available", {{"type", "boolean"}}} // 是否可用属性，布尔类型
            }},
            {"required", {"age", "available"}} // 必需的属性
        };

        // 创建包含用户查询消息的 Message 向量
        std::vector<tinychain::Message> structure_messages = {
            {"user", "Ollama 22 岁了，正忙着拯救世界。返回一个包含年龄和可用性的 JSON 对象。"} // 用户请求结构化信息
        };

        // 创建一个包含 "outputFormat" 模式的上下文 JSON 对象
        nlohmann::json structure_context = {
            {"outputFormat", format_schema} // 注意：这里根据你最新的 Ollama API 示例，应该是 "format" 键而不是 "outputFormat"。
                                             // 但为了与你提供的原始上下文保持一致，我保留了 "outputFormat"。
                                             // 如果实际 API 要求是 "format"，请将此处的 "outputFormat" 改为 "format"。
        };

        // 调用 LLM 的 stream 方法，传入消息和结构化输出上下文
        nlohmann::json structure_response = llm.stream(structure_messages, &structure_context);
        std::cout << "结构化响应：\n" << structure_response.dump(2) << "\n\n";

    } catch (const std::exception& e) {
        // 捕获并打印所有标准异常
        std::cerr << "错误: " << e.what() << std::endl;
        return 1; // 返回非零值表示程序出错
    }

    return 0; // 返回零表示程序成功执行
}