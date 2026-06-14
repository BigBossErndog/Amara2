#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

// CRC32 implementation (needed for ZIP)
static uint32_t crc32_table[256];
static bool crc32_initialized = false;

static void init_crc32_table() {
    if (crc32_initialized) return;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        crc32_table[i] = c;
    }
    crc32_initialized = true;
}

static uint32_t crc32(const uint8_t* data, size_t len) {
    init_crc32_table();
    uint32_t c = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++)
        c = crc32_table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFF;
}

// Write little-endian integers
static void write_u16(std::vector<uint8_t>& buf, uint16_t v) {
    buf.push_back(v & 0xFF);
    buf.push_back((v >> 8) & 0xFF);
}
static void write_u32(std::vector<uint8_t>& buf, uint32_t v) {
    buf.push_back(v & 0xFF);
    buf.push_back((v >> 8) & 0xFF);
    buf.push_back((v >> 16) & 0xFF);
    buf.push_back((v >> 24) & 0xFF);
}

struct ZipEntry {
    std::string name;
    std::vector<uint8_t> data;
    uint32_t crc;
    uint32_t offset;        // offset of local file header in file
    bool store;             // true = no compression (STORED), false = DEFLATE
};

class ZipAppender {
public:
    // Open existing ZIP and read its central directory
    bool open(const fs::path& path) {
        zip_path = path;

        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f.is_open()) return false;

        size_t file_size = f.tellg();
        f.seekg(0);

        existing_data.resize(file_size);
        f.read(reinterpret_cast<char*>(existing_data.data()), file_size);
        f.close();

        // Find end of central directory record (EOCD)
        // Signature: 0x06054b50
        int eocd_offset = -1;
        for (int i = (int)file_size - 22; i >= 0; i--) {
            if (existing_data[i]   == 0x50 && existing_data[i+1] == 0x4B &&
                existing_data[i+2] == 0x05 && existing_data[i+3] == 0x06) {
                eocd_offset = i;
                break;
            }
        }
        if (eocd_offset < 0) return false;

        // Parse EOCD
        num_existing_entries = read_u16(eocd_offset + 8);
        central_dir_size     = read_u32(eocd_offset + 12);
        central_dir_offset   = read_u32(eocd_offset + 16);

        return true;
    }

    // Add a file entry (store = no compression, required for .so files)
    void add(const std::string& zip_name,
             const uint8_t* data, size_t data_len,
             bool store = false)
    {
        ZipEntry entry;
        entry.name  = zip_name;
        entry.data  = std::vector<uint8_t>(data, data + data_len);
        entry.crc   = crc32(data, data_len);
        entry.store = store;
        entries.push_back(std::move(entry));
    }

    void add(const std::string& zip_name, const std::string& data, bool store = false) {
        add(zip_name, reinterpret_cast<const uint8_t*>(data.data()), data.size(), store);
    }

    void add_file(const std::string& zip_name, const fs::path& file_path, bool store = false) {
        std::ifstream f(file_path, std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)),
                                   std::istreambuf_iterator<char>());
        add(zip_name, data.data(), data.size(), store);
    }

    // Write the final ZIP
    bool finalize() {
        std::vector<uint8_t> out;

        // 1. Copy existing data up to (but not including) the central directory
        out.insert(out.end(),
                   existing_data.begin(),
                   existing_data.begin() + central_dir_offset);

        // 2. Write new local file headers + data
        for (auto& entry : entries) {
            entry.offset = (uint32_t)out.size();
            write_local_header(out, entry);
            out.insert(out.end(), entry.data.begin(), entry.data.end());
        }

        // 3. Write central directory (existing entries first)
        uint32_t new_central_dir_offset = (uint32_t)out.size();

        // Copy existing central directory entries
        out.insert(out.end(),
                   existing_data.begin() + central_dir_offset,
                   existing_data.begin() + central_dir_offset + central_dir_size);

        // Write new central directory entries
        for (const auto& entry : entries)
            write_central_header(out, entry);

        // 4. Write EOCD
        uint32_t total_entries    = num_existing_entries + (uint32_t)entries.size();
        uint32_t new_central_size = (uint32_t)out.size() - new_central_dir_offset;
        write_eocd(out, total_entries, new_central_size, new_central_dir_offset);

        // 5. Write to file
        std::ofstream f(zip_path, std::ios::binary);
        if (!f.is_open()) return false;
        f.write(reinterpret_cast<char*>(out.data()), out.size());
        return true;
    }

private:
    fs::path zip_path;
    std::vector<uint8_t> existing_data;
    std::vector<ZipEntry> entries;
    uint32_t central_dir_offset = 0;
    uint32_t central_dir_size   = 0;
    uint16_t num_existing_entries = 0;

    uint16_t read_u16(size_t offset) {
        return existing_data[offset] | (existing_data[offset+1] << 8);
    }
    uint32_t read_u32(size_t offset) {
        return existing_data[offset]        |
               (existing_data[offset+1] << 8)  |
               (existing_data[offset+2] << 16) |
               (existing_data[offset+3] << 24);
    }

    void write_local_header(std::vector<uint8_t>& buf, const ZipEntry& e) {
        uint16_t method = e.store ? 0 : 8; // 0=STORED, 8=DEFLATE
        write_u32(buf, 0x04034B50);  // local file header signature
        write_u16(buf, 20);          // version needed
        write_u16(buf, 0);           // flags
        write_u16(buf, method);      // compression method
        write_u16(buf, 0);           // last mod time
        write_u16(buf, 0);           // last mod date
        write_u32(buf, e.crc);
        write_u32(buf, (uint32_t)e.data.size()); // compressed size (same as uncompressed for STORED)
        write_u32(buf, (uint32_t)e.data.size()); // uncompressed size
        write_u16(buf, (uint16_t)e.name.size()); // file name length
        write_u16(buf, 0);                        // extra field length
        buf.insert(buf.end(), e.name.begin(), e.name.end());
    }

    void write_central_header(std::vector<uint8_t>& buf, const ZipEntry& e) {
        uint16_t method = e.store ? 0 : 8;
        write_u32(buf, 0x02014B50);  // central directory signature
        write_u16(buf, 20);          // version made by
        write_u16(buf, 20);          // version needed
        write_u16(buf, 0);           // flags
        write_u16(buf, method);
        write_u16(buf, 0);           // last mod time
        write_u16(buf, 0);           // last mod date
        write_u32(buf, e.crc);
        write_u32(buf, (uint32_t)e.data.size());
        write_u32(buf, (uint32_t)e.data.size());
        write_u16(buf, (uint16_t)e.name.size());
        write_u16(buf, 0);           // extra field length
        write_u16(buf, 0);           // file comment length
        write_u16(buf, 0);           // disk number start
        write_u16(buf, 0);           // internal attributes
        write_u32(buf, 0);           // external attributes
        write_u32(buf, e.offset);    // offset of local header
        buf.insert(buf.end(), e.name.begin(), e.name.end());
    }

    void write_eocd(std::vector<uint8_t>& buf,
                    uint32_t total_entries,
                    uint32_t central_dir_size,
                    uint32_t central_dir_offset)
    {
        write_u32(buf, 0x06054B50);  // EOCD signature
        write_u16(buf, 0);           // disk number
        write_u16(buf, 0);           // disk with central dir
        write_u16(buf, (uint16_t)total_entries);
        write_u16(buf, (uint16_t)total_entries);
        write_u32(buf, central_dir_size);
        write_u32(buf, central_dir_offset);
        write_u16(buf, 0);           // comment length
    }
};