namespace Amara {
    class Encryption {
    public:
        /*
         * Really basic encryption for the sake of obfuscation and not security.
         * Using the TEA encryption algorithm.
         */ 
        static constexpr uint8_t ENCRYPTION_HEADER[] = {'_', 'A', 'R', 'A'}; // Never change this.
        
        static std::array<uint32_t, 4> hash(const std::string& keyStr) {
            std::array<uint32_t, 4> key = {0, 0, 0, 0};
            uint8_t keyBytes[16] = {0};
            size_t len = std::min<size_t>(keyStr.size(), 16);
            std::memcpy(keyBytes, keyStr.data(), len);
        
            for (int i = 0; i < 4; ++i) {
                key[i] = (static_cast<uint32_t>(keyBytes[i * 4 + 0])      ) |
                         (static_cast<uint32_t>(keyBytes[i * 4 + 1]) <<  8) |
                         (static_cast<uint32_t>(keyBytes[i * 4 + 2]) << 16) |
                         (static_cast<uint32_t>(keyBytes[i * 4 + 3]) << 24);
            }
        
            return key;
        }

        static void tea_encrypt(uint32_t* v, const std::string& keyStr) {
            std::array<uint32_t, 4> k = Encryption::hash(keyStr);
            uint32_t v0 = v[0], v1 = v[1];
            uint32_t sum = 0, delta = 0x9e3779b9;
            for (int i = 0; i < 32; ++i) {
                sum += delta;
                v0 += ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
                v1 += ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
            }
            v[0] = v0;
            v[1] = v1;
        }

        static bool is_buffer_encrypted(unsigned char* buffer, size_t size) {
            if (size < sizeof(ENCRYPTION_HEADER)) {
                return false;
            }

            if (std::memcmp(buffer, ENCRYPTION_HEADER, sizeof(ENCRYPTION_HEADER)) != 0) {
                return false;
            }

            return true;
        }

        static bool is_string_encrypted(const std::string& str) {
            if (str.size() < sizeof(ENCRYPTION_HEADER)) {
                return false;
            }

            if (std::memcmp(str.data(), ENCRYPTION_HEADER, sizeof(ENCRYPTION_HEADER)) != 0) {
                return false;
            }
            
            return true;
        }
        
        static void tea_decrypt(uint32_t* v, const std::string& keyStr) {
            std::array<uint32_t, 4> k = Encryption::hash(keyStr);
            uint32_t v0 = v[0], v1 = v[1];
            uint32_t sum = 0xC6EF3720, delta = 0x9e3779b9;
            for (int i = 0; i < 32; ++i) {
                v1 -= ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
                v0 -= ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
                sum -= delta;
            }
            v[0] = v0;
            v[1] = v1;
        }

        static void encryptBuffer(unsigned char* buffer, size_t& size, const std::string& keyStr) {
            size_t paddedSize = (size + 7) & ~7; // round up to multiple of 8
            std::vector<unsigned char> data(paddedSize, 0);
            std::memcpy(data.data(), buffer, size);

            for (size_t i = 0; i < paddedSize; i += 8) {
                tea_encrypt(reinterpret_cast<uint32_t*>(&data[i]), keyStr);
            }

            size_t encryptedSize = paddedSize + sizeof(ENCRYPTION_HEADER);
            unsigned char* encryptedData = new unsigned char[encryptedSize];
            
            std::memcpy(encryptedData, ENCRYPTION_HEADER, sizeof(ENCRYPTION_HEADER)); 
            std::memcpy(encryptedData + sizeof(ENCRYPTION_HEADER), data.data(), paddedSize);

            std::memcpy(buffer, encryptedData, encryptedSize);
            size = encryptedSize;
            delete[] encryptedData;
        }
        
        static void decryptBuffer(unsigned char* buffer, size_t& size, const std::string& keyStr) {
            if (!Encryption::is_buffer_encrypted(buffer, size)) {
                debug_log("Warning: Attempted to decrypt non-encrypted data (type buffer).");
                return;
            }

            unsigned char* data = buffer + sizeof(ENCRYPTION_HEADER);
            size -= sizeof(ENCRYPTION_HEADER); // Adjust size to remove header
        
            for (size_t i = 0; i < size; i += 8) {
                Encryption::tea_decrypt(reinterpret_cast<uint32_t*>(&data[i]), keyStr);
            }

            size_t newSize = size;
            while (newSize > 0 && data[newSize - 1] == 0) {
                --newSize;
            }

            size = newSize;
        }

        static std::string encryptString(const std::string& input, const std::string& keyStr) {
            size_t size = input.size();
            unsigned char* buffer = new unsigned char[size];
            std::memcpy(buffer, input.data(), size);

            Encryption::encryptBuffer(buffer, size, keyStr);

            std::string encrypted(reinterpret_cast<char*>(buffer), size);
            
            delete[] buffer;
            return encrypted;
        }

        static std::string decryptString(const std::string& encrypted, const std::string& keyStr) {
            size_t size = encrypted.size();
            unsigned char* buffer = new unsigned char[size];
            std::memcpy(buffer, encrypted.data(), size);

            if (!Encryption::is_buffer_encrypted(buffer, size)) {
                debug_log("Error: Attempted to decrypt non-encrypted data (type string).");
                delete[] buffer;
                return "";
            }

            Encryption::decryptBuffer(buffer, size, keyStr);

            std::string decrypted(reinterpret_cast<char*>(buffer), size);
        
            delete[] buffer;
            return decrypted;
        }
    };
}