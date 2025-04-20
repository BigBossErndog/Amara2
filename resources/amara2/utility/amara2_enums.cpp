namespace Amara {
    enum class GraphicsEnum {
        None,
        Render2D,
        OpenGL,
        Vulkan,
        DirectX,
        DirectX_Legacy,
        VulkanMetalDirectX
    };

    enum class ScreenModeEnum {
        Windowed,
        BorderlessWindowed,
        Fullscreen,
        BorderlessFullscreen
    };

    enum class AssetEnum {
        None,
        Surface,
        Image,
        Spritesheet,
        Audio,
        Font,
        TMXTilemap,
        ShaderProgram
    };

    enum class BlendMode {
        None = 0,
        Alpha = 1,
        Additive = 2,
        Multiply = 3,
        PremultipliedAlpha = 4
    };

    enum class AlignmentEnum {
        Left = 0,
        Center = 1,
        Right = 2,
        Top = 3,
        Middle = 4,
        Bottom = 5
    };

    enum class WrapModeEnum {
        ByCharacter,
        ByWord
    };

    void bindLua_Enums(sol::state& lua) {
        lua.new_enum("Graphics",
            "None", Amara::GraphicsEnum::None,
            "Render2D", Amara::GraphicsEnum::Render2D,
            "OpenGL", Amara::GraphicsEnum::OpenGL,
            "Vulkan", Amara::GraphicsEnum::Vulkan,
            "DirectX", Amara::GraphicsEnum::DirectX,
            "DirectX_Legacy", Amara::GraphicsEnum::DirectX_Legacy,
            "VulkanMetalDirectX", Amara::GraphicsEnum::VulkanMetalDirectX
        );

        lua.new_enum("ScreenMode",
            "Windowed", Amara::ScreenModeEnum::Windowed,
            "BorderlessWindowed", Amara::ScreenModeEnum::BorderlessWindowed,
            "Fullscreen", Amara::ScreenModeEnum::Fullscreen,
            "BorderlessFullscreen", Amara::ScreenModeEnum::BorderlessFullscreen
        );

        lua.new_enum("BlendMode",
            "None", Amara::BlendMode::None,
            "Alpha", Amara::BlendMode::Alpha,
            "Additive", Amara::BlendMode::Additive,
            "Multiply", Amara::BlendMode::Multiply,
            "PremultipliedAlpha", Amara::BlendMode::PremultipliedAlpha
        );

        lua.new_enum("Align",
            "Left", Amara::AlignmentEnum::Left,
            "Center", Amara::AlignmentEnum::Center,
            "Right", Amara::AlignmentEnum::Right,
            "Top", Amara::AlignmentEnum::Top,
            "Middle", Amara::AlignmentEnum::Middle,
            "Bottom", Amara::AlignmentEnum::Bottom
        );

        lua.new_enum("WrapMode",
            "ByCharacter", Amara::WrapModeEnum::ByCharacter,
            "ByWord", Amara::WrapModeEnum::ByWord
        );
    }
}