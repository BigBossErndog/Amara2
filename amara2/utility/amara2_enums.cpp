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
        Invalid = 1,
        Windowed = 2,
        BorderlessWindowed = 3,
        Fullscreen = 4,
        BorderlessFullscreen = 5
    };

    enum class InputMode {
        None = 1,
        Keyboard = 2,
        Gamepad = 3,
        Mouse = 4,
        Touch = 5
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
        PremultipliedAlpha = 4,
        Mask = 5,
        Erase = 6
    };

    enum class AlignmentEnum {
        Invalid = 1,
        Left = 2,
        Center = 3,
        Right = 4,
        Top = 5,
        Middle = 6,
        Bottom = 7
    };

    enum class WrapModeEnum {
        Inavlid = 1,
        ByCharacter = 2,
        ByWord = 3
    };

    enum class TextInputEnum {
        None = 1,
        Text = 2,
        Backspace = 3
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

    std::vector<Amara::Direction> DirectionsInOrder = { Direction::Right, Direction::DownRight, Direction::Down, Direction::DownLeft, Direction::Left, Direction::UpLeft, Direction::Up, Direction::UpRight };
    std::vector<Amara::Direction> CardinalDirections = { Direction::Right, Direction::Down, Direction::Left, Direction::Up };
    std::vector<Amara::Direction> OrdinalDirections = { Direction::DownLeft, Direction::UpLeft, Direction::UpRight, Direction::DownRight };

    enum class ShapeEnum {
        None = 1,
        Rectangle = 2,
        Circle = 3,
        Quad = 4,
        Triangle = 6,
        Polygon = 7,
        Line = 8
    };

    enum class CursorEnum {
        Default = 1,
        Pointer = 2
    };

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
            "Invalid", Amara::ScreenModeEnum::Invalid,
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
            "PremultipliedAlpha", Amara::BlendMode::PremultipliedAlpha,
            "Mask", Amara::BlendMode::Mask,
            "Erase", Amara::BlendMode::Erase
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

        lua.new_enum("Direction",
            "NoDirection", Amara::Direction::NoDirection,
            
            "Up", Amara::Direction::Up,
            "North", Amara::Direction::Up,

            "UpLeft", Amara::Direction::UpLeft,
            "NorthWest", Amara::Direction::UpLeft,

            "UpRight", Amara::Direction::UpRight,
            "NorthEast", Amara::Direction::UpRight,

            "Down", Amara::Direction::Down,
            "South", Amara::Direction::Down,

            "DownLeft", Amara::Direction::DownLeft,
            "SouthWest", Amara::Direction::DownLeft,
            
            "DownRight", Amara::Direction::DownRight,
            "SouthEast", Amara::Direction::DownRight,

            "Left", Amara::Direction::Left,
            "West", Amara::Direction::Left,

            "Right", Amara::Direction::Right,
            "East", Amara::Direction::Right
        );

        lua.new_enum("Shape",
            "None", Amara::ShapeEnum::None,
            "Rectangle", Amara::ShapeEnum::Rectangle,
            "Circle", Amara::ShapeEnum::Circle,
            "Quad", Amara::ShapeEnum::Quad,
            "Triangle", Amara::ShapeEnum::Triangle,
            "Polygon", Amara::ShapeEnum::Polygon,
            "Line", Amara::ShapeEnum::Line
        );

        lua.new_enum("Cursor",
            "Default", Amara::CursorEnum::Default,
            "Pointer", Amara::CursorEnum::Pointer
        );
    }
}