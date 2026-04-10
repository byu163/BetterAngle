#include <iostream>
#include <string>

int main() {
    std::string iniContent = "MouseX=0.05";
    std::string key = "MouseX";
    size_t pos = iniContent.find(key + "=");
    if (pos != std::string::npos) {
        size_t end = iniContent.find_first_of("\r\n", pos);
        std::cout << "end: " << end << std::endl;
        size_t count = end - (pos + key.length() + 1);
        std::cout << "count: " << count << std::endl;
        try {
            std::string val = iniContent.substr(pos + key.length() + 1, count);
            std::cout << "val: " << val << std::endl;
        } catch (const std::exception& e) {
            std::cout << "exception: " << e.what() << std::endl;
        }
    }
    return 0;
}
