namespace Amara {
    class Text: public Amara::Node {
    public:
        std::string text;
        std::u32string converted_text;

        Amara::FontAsset* font = nullptr;
        
        int progress = 0;

        Text(): Amara::Node() {
            set_base_node_id("Text");
        }

        void setText(std::string str) {
            text = str;

            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
            converted_text = converter.from_bytes(str);  // Convert UTF-8 string to UTF-32

            if (font) font->packGlyphsFromString(converted_text);
        }

        bool setFont(std::string key) {
            font = nullptr;
            if (!Props::assets->has(key)) {
                debug_log("Error: Asset \"", key, "\" was not found.");
                return false;
            }

            font = Props::assets->get(key)->as<Amara::FontAsset*>();

            if (font == nullptr) {
                debug_log("Error: Asset \"", key, "\" is not a valid font asset.");
                return false;
            }

            if (!converted_text.empty()) {
                font->packGlyphsFromString(converted_text);
            }

            return true;
        }
    };
}