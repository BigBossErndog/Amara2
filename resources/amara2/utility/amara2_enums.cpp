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
        None = 0,
        Surface = 1,
        Image = 2,
        Spritesheet = 3,
        Audio = 4,
        Font = 5,
        TMXTilemap = 6,
        ShaderProgram = 7
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

    enum class Direction {
        NoDirection = 0,
        Up = 1,
        UpLeft = 2,
        UpRight = 4,
        Down = 8,
        DownLeft = 16,
        DownRight = 32,
        Left = 64,
        Right = 128
    };

    std::vector<Amara::Direction> DirectionsInOrder = { Right, DownRight, Down, DownLeft, Left, UpLeft, Up, UpRight };
    std::vector<Amara::Direction> CardinalDirections = { Right, Down, Left, Up };
    std::vector<Amara::Direction> OrdinalDirections = { DownLeft, UpLeft, UpRight, DownRight };

    void bind_lua_Enums(sol::state& lua) {
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

        lua.new_enum("Direction"
            "NoDirection", Amara::Direction::NoDirection,
            
            "Up", Amara::Direction::Up,
            "North", Amara::Direction::Up,

            "UpLeft", Amara::Direction::UpLeft,
            "NorthWest", Amara::Direction::UpLeft,

            "UpRight", Amara::Direction::UpRight,
            "NorthEast", Amara::Direction::UpRight,

            "Down", Amara::Direction::Down,
            "South", Amara::Direction::South,

            "DownLeft", Amara::Direction::DownLeft,
            "SouthWest", Amara::Direction::DownLeft,

            "DownRight", Amara::Direction::DownRight,
            "SouthEast", Amara::Direction::DownRight,

            "Left", Amara::Direction::Left,
            "West", Amara::Direction::Left,

            "Right", Amara::Direction::Right,
            "East", Amara::Direction::East
        );
    }
}