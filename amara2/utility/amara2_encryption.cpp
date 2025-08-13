namespace Amara {
    class Encryption {
    public:
        /*
         * Really basic encryption for the sake of obfuscation and not security.
         * Using the TEA encryption algorithm.
         * 
         * V2 Update: Refactored encryption and decryption functions to be more robust.
         * The old functions had memory management issues and incorrect size handling.
         * The new functions use std::vector to manage memory and return the correct sizes.
         * 
         * Note on padding: The current padding scheme is not robust and may fail for certain data.
         * It works by removing trailing null bytes, which could be part of the original data.
         * A more robust padding scheme like PKCS#7 should be used in the future.
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

        static bool is_buffer_encrypted(const unsigned char* buffer, size_t size) {
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

        static std::vector<unsigned char> encryptBuffer(const unsigned char* buffer, size_t size, const std::string& keyStr) {
            size_t paddedSize = (size + 7) & ~7; // round up to multiple of 8
            std::vector<unsigned char> data(paddedSize, 0);
            std::memcpy(data.data(), buffer, size);

            for (size_t i = 0; i < paddedSize; i += 8) {
                tea_encrypt(reinterpret_cast<uint32_t*>(&data[i]), keyStr);
            }

            size_t encryptedSize = paddedSize + sizeof(ENCRYPTION_HEADER);
            std::vector<unsigned char> encryptedData(encryptedSize);
            
            std::memcpy(encryptedData.data(), ENCRYPTION_HEADER, sizeof(ENCRYPTION_HEADER)); 
            std::memcpy(encryptedData.data() + sizeof(ENCRYPTION_HEADER), data.data(), paddedSize);

            return encryptedData;
        }
        
        static std::vector<unsigned char> decryptBuffer(const unsigned char* buffer, size_t size, const std::string& keyStr) {
            if (!Encryption::is_buffer_encrypted(buffer, size)) {
                debug_log("Warning: Attempted to decrypt non-encrypted data (type buffer).");
                return {};
            }

            const unsigned char* data = buffer + sizeof(ENCRYPTION_HEADER);
            size_t encrypted_content_size = size - sizeof(ENCRYPTION_HEADER);

            std::vector<unsigned char> decrypted_data(data, data + encrypted_content_size);

            for (size_t i = 0; i < encrypted_content_size; i += 8) {
                Encryption::tea_decrypt(reinterpret_cast<uint32_t*>(&decrypted_data[i]), keyStr);
            }

            size_t newSize = encrypted_content_size;
            while (newSize > 0 && decrypted_data[newSize - 1] == 0) {
                --newSize;
            }
            decrypted_data.resize(newSize);

            return decrypted_data;
        }

        static std::string encryptString(const std::string& input, const std::string& keyStr) {
            std::vector<unsigned char> encrypted = Encryption::encryptBuffer(reinterpret_cast<const unsigned char*>(input.data()), input.size(), keyStr);
            return std::string(reinterpret_cast<char*>(encrypted.data()), encrypted.size());
        }

        static std::string decryptString(const std::string& encrypted, const std::string& keyStr) {
            std::vector<unsigned char> decrypted_vector = decryptBuffer(
                reinterpret_cast<const unsigned char*>(encrypted.data()),
                encrypted.size(),
                keyStr
            );
            if (decrypted_vector.empty() && is_string_encrypted(encrypted)) {
                debug_log("Error: Failed to decrypt string.");
                return "";
            }
            return std::string(reinterpret_cast<char*>(decrypted_vector.data()), decrypted_vector.size());
        }
    };
}