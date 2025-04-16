namespace Amara {
    class String {
    public:
        static std::string toLower(const std::string& str) {
            std::string lowerStr = str;
            std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
            return lowerStr;
        }

        static std::string toUpper(const std::string& str) {
            std::string upperStr = str;
            std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
            return upperStr;
        }

        static bool contains(const std::string& str, const std::string& substr) {
            return str.find(substr) != std::string::npos;
        }
        
        static std::string trim(const std::string& str) {
            size_t first = str.find_first_not_of(' ');
            size_t last = str.find_last_not_of(' ');
            return (first == std::string::npos) ? "" : str.substr(first, (last - first + 1));
        }

        static std::string replace(const std::string& str, const std::string& from, const std::string& to) {
            std::string result = str;
            size_t start_pos = 0;
            while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
                result.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }
            return result;
        }

        static std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
            std::vector<std::string> tokens;
            size_t start = 0;
            size_t end = str.find(delimiter);
            while (end != std::string::npos) {
                tokens.push_back(str.substr(start, end - start));
                start = end + delimiter.length();
                end = str.find(delimiter, start);
            }
            tokens.push_back(str.substr(start, end));
            return tokens;
        }

        static std::string join(const std::vector<std::string>& strings, const std::string& delimiter) {
            std::ostringstream oss;
            for (size_t i = 0; i < strings.size(); ++i) {
                oss << strings[i];
                if (i != strings.size() - 1) {
                    oss << delimiter;
                }
            }
            return oss.str();
        }

        static std::string format(const std::string& format, ...) {
            va_list args;
            va_start(args, format);
            size_t size = std::snprintf(nullptr, 0, format.c_str(), args) + 1; // Extra space for '\0'
            if (size <= 0) {
                va_end(args);
                return ""; // Error occurred
            }
            std::unique_ptr<char[]> buf(new char[size]);
            std::vsnprintf(buf.get(), size, format.c_str(), args);
            va_end(args);
            return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
        }

        static std::string fromHex(const std::string& hex) {
            std::string result;
            for (size_t i = 0; i < hex.length(); i += 2) {
                std::string byteString = hex.substr(i, 2);
                char byte = (char)(int)strtol(byteString.c_str(), nullptr, 16);
                result.push_back(byte);
            }
            return result;
        }

        static std::u32string utf8_to_utf32(const std::string& utf8) {
            std::u32string utf32;
            size_t i = 0;
            size_t len = utf8.size();
            
            while (i < len) {
                char32_t codepoint = 0;
                unsigned char c = utf8[i];
        
                // Determine the number of bytes in this character
                int num_bytes = 0;
                if (c < 0x80) {
                    num_bytes = 1;
                    codepoint = c;
                } else if ((c & 0xE0) == 0xC0) {
                    num_bytes = 2;
                    codepoint = c & 0x1F;
                } else if ((c & 0xF0) == 0xE0) {
                    num_bytes = 3;
                    codepoint = c & 0x0F;
                } else if ((c & 0xF8) == 0xF0) {
                    num_bytes = 4;
                    codepoint = c & 0x07;
                } else {
                    throw std::runtime_error("Invalid UTF-8: Unexpected leading byte.");
                }
        
                // Ensure enough bytes exist
                if (i + num_bytes > len) {
                    throw std::runtime_error("Invalid UTF-8: Truncated character.");
                }
        
                // Decode remaining bytes
                for (int j = 1; j < num_bytes; ++j) {
                    unsigned char next = utf8[i + j];
                    if ((next & 0xC0) != 0x80) {
                        throw std::runtime_error("Invalid UTF-8: Missing continuation byte.");
                    }
                    codepoint = (codepoint << 6) | (next & 0x3F);
                }
        
                // Reject overlong encodings
                if ((num_bytes == 2 && codepoint < 0x80) ||
                    (num_bytes == 3 && codepoint < 0x800) ||
                    (num_bytes == 4 && codepoint < 0x10000)) {
                    throw std::runtime_error("Invalid UTF-8: Overlong encoding detected.");
                }
        
                // Reject UTF-16 surrogate halves (U+D800 to U+DFFF)
                if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
                    throw std::runtime_error("Invalid UTF-8: Surrogate codepoints are not valid in UTF-32.");
                }
        
                // Reject codepoints beyond Unicode range (> U+10FFFF)
                if (codepoint > 0x10FFFF) {
                    throw std::runtime_error("Invalid UTF-8: Codepoint out of Unicode range.");
                }
        
                utf32.push_back(codepoint);
                i += num_bytes;
            }
        
            return utf32;
        }

        static std::string utf32_to_utf8(const std::u32string& utf32) {
            std::string utf8;
            for (char32_t codepoint : utf32) {
                // Check for invalid codepoints (surrogates or out of range)
                if (codepoint > 0x10FFFF) {
                    throw std::runtime_error("Invalid UTF-32: Codepoint out of Unicode range.");
                }
                if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
                    throw std::runtime_error("Invalid UTF-32: Surrogate codepoints are not valid.");
                }

                if (codepoint <= 0x7F) {
                    // 1-byte sequence (ASCII)
                    utf8 += static_cast<char>(codepoint);
                } else if (codepoint <= 0x7FF) {
                    // 2-byte sequence
                    utf8 += static_cast<char>(0xC0 | (codepoint >> 6));            // 110xxxxx
                    utf8 += static_cast<char>(0x80 | (codepoint & 0x3F));         // 10xxxxxx
                } else if (codepoint <= 0xFFFF) {
                    // 3-byte sequence
                    utf8 += static_cast<char>(0xE0 | (codepoint >> 12));           // 1110xxxx
                    utf8 += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));  // 10xxxxxx
                    utf8 += static_cast<char>(0x80 | (codepoint & 0x3F));         // 10xxxxxx
                } else { // codepoint <= 0x10FFFF
                    // 4-byte sequence
                    utf8 += static_cast<char>(0xF0 | (codepoint >> 18));           // 11110xxx
                    utf8 += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)); // 10xxxxxx
                    utf8 += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));  // 10xxxxxx
                    utf8 += static_cast<char>(0x80 | (codepoint & 0x3F));         // 10xxxxxx
                }
            }
            return utf8;
        }
    };
}