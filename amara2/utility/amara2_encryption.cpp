namespace Amara {
    std::string xorEncryptDecrypt(const std::string& input, const std::string& key) {
        if (key.empty()) {
            throw std::invalid_argument("Key must not be empty");
        }

        std::string output;
        output.reserve(input.size());

        for (size_t i = 0; i < input.size(); ++i) {
            output.push_back(input[i] ^ key[i % key.size()]);
        }

        return output;
    }
    
    std::string encrypt(const std::string& plaintext, const std::string& key) {
        return xorEncryptDecrypt(plaintext, key);
    }

    std::string decrypt(const std::string& ciphertext, const std::string& key) {
        return xorEncryptDecrypt(ciphertext, key);
    }
}