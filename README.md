# Amara2
A light-weight 2D game engine with Lua scripting.
Built in C++ using SDL3.

*Note: Amara2 is currently only available for Windows only.*

### Features
- Setting up a new project is easy as heck.
- Go from downloading Amara2, creating a project and running a test of your game in a matter of seconds.
- Game architecture inspired by my favorite elements of the Phaser framework and the Godot Engine.
- Code-focused game engine that attempts to unify all existing tools a game developer may use into a cohesive experience.
- A charming pixel-art aesthetic inspired by popular indie developer tools such as Aseprite.

### Supported Tools
- Code Editors such as VS Code, Atom, Sublime.
- Tiled .tmx file support.

## Setup
### Windows
Download the windows zip file from the latest release:

https://github.com/BigBossErndog/Amara2/releases

Run "*Amara2.exe*".

#### - *Building your project*
Amara2 requires VS Build Tools from Microsoft to build your final game.
You don't have to install it until you need to build your game.

Download the installer from:

https://visualstudio.microsoft.com/visual-cpp-build-tools/

When you run the installer, select "Desktop development with C++".

Once installation is completed, you are ready to use Amara2.

## Nodes
Here is a compehensive list of what nodes are available:
- Node : The base node object.
- Group : A copy of Node for use of grouping Nodes as one entity.
- CopyNode : Used to re-render a node in another location.
- NodePool : Used to pool nodes for reuse.
- Text : Used to render text from a ttf.
- Camera : Used by a Scene to render elements. View can be controlled with scroll and zoom.
- Scene : Used to organize your game's various states.
- Action : The base action, used to reuse behaviors across various types of nodes.
- WaitAction : Used to pause an amount of time before performing a function or another action (typically accessed via node.wait).
- Tween : An action used to tween the values of a node (typically accessed via node.tween).
- StateMachine : Used to manage various states of a node.
- DelayedNode : Used to delay the creation of a node.
- PeriodicAction : Used to perform a repeated action with a specified time between each call.
- ProcessNode : Used to execute a System program as a sub-process and manage output.
- TextInput : Used to track user keyboard input.
- Loader : Used to load assets into the game (typically access via node.load).
- AutoProgress : Used to progress a Text node (i.e. text appearing character by character).
- Sprite : Used to display an image and animate from a spritesheet.
- Animation : Action to make a Sprite play an animation from a spritesheet (typically access via sprite:play)
- FillRect : Used to display a colored rectangle.
- Container : Used to crop view of child nodes within a rectangle.
- TextureContainer : Used to crop view of child nodes in a textured container that can be further manipulated with rotation and tints.
- ShaderContainer : (OpenGL graphics only) A TextureContainer used to perform multiple render passes.
- TilemapLayer : Used to display a layer from a tilemap.
- Tilemap : Used to manage the various elements of a tilemap (e.g. layers and objects from a Tiled .tmx file)
- NineSlice : Used to display a textured box, typically for GUI. https://en.wikipedia.org/wiki/9-slice_scaling
- Audio : A node used to manage the playing of an audio.
- AudioPool : If a sound is supposed to be layered, or a different sound to be played each time then use an AudioPool.
- AudioGroup : Used to group audio. Also used to manage the volume of a collection of audio.
- AudioMaster : The root audio node used by the World class. Add your audio nodes as a child to world.audio.
- Transition : Base node used to create your own transition between scenes.
- FillTransition : Used to fade in and out between scenes.
- World : The big manager node used to contain the game and manage the window.
