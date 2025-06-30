# Amara2
A 2D game engine with Lua scripting.
Built in C++ using SDL3.

*Note: Amara2 is currently only available for Windows only.*

Get the latest version from:

https://github.com/BigBossErndog/Amara2/releases

## Features
- Setting up a new project and running a test is easy as heck.
- Game architecture inspired by my favorite elements of the Phaser framework and the Godot Engine.
- Minimal GUI. Get the necessities while making your commonly used tools more cohesive.
- A charming pixel-art aesthetic inspired by popular indie developer tools such as Aseprite.

## Philosophy
This is a game engine for people who dislike game engine GUIs. If you're like me, you may get overwhelmed by endless text fields and drop down menus.

Get the benefits of an easy-to-use game framework while also taking out all the frustrations of setting up a new project, making builds and running tests.

All your core necessities sit in small windows on top of your desktop over the program you're currently using, so you can use whichever tools you want without having to switch out of the engine. 

Over time I want this program to tie in with all your favorite game dev tools, easy access to everything you need.

![Screenshot 2025-06-30 213758](https://github.com/user-attachments/assets/e0a09742-6a23-4d5e-af4d-2cf07ab95655)


## Supported Tools
- Code Editors such as VS Code, Atom, Sublime.
- Tiled .tmx file support.

## Node System
Amara2 defines every entity in the game as nodes, all of which exist in a tree starting with your World. Extend existing nodes and add nodes as children to other nodes in this easy-to-use modular system. And define them all with easy to read Lua tables!

Here is an example of how you would write a scene in Amara2.

Example file: "*lua_scripts/MyScene.lua*"
```lua
local mySprite

return Nodes:define("MyScene", "Scene", { -- define MyScene from already existing node Scene
    onPreload = function(self)
        -- General rule of thumb, call a function with a color (: passes itself as first argument).

        -- Loading a font of size 10
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        -- Loading a spritesheet with frame of size 32 x 32 px.
        self.load:spritesheet("mySpritesheet", "spritesheets/mySpritesheet.png", 32, 32) 
        
        self.animations:add({
            key = "waveHello",
            texture = "mySpritesheet",
            frameRate = 12,
            frames = { 0, 1, 2 },
            loop = true
        })
    end,
    
    onCreate = function(self)
        self:createChild("Text", { -- pass a definition table to setup a node
            x = 0, y = -100,
            font = "defaultFont",
            text = "Hello!",
            color = Color.Green
        })
        
        mySprite = self:createChild("Sprite", {
            x = 0, y = 0, -- (0, 0) is the center of the screen / view.
            texture = "mySpritesheet",
            input = {
                active = true,
                onMouseDown = function(self)
                    -- Use node.func to call custom functions
                    self.func:jump()
                end
            },

            -- Add functions to definition table to create custom functions
            jump = function(self)
                self.y = 0
                self.tween:to({
                    y = -20,
                    yoyo = true,
                    duration = 0.2
                })
            end
        })
    end,

    onUpdate = function(self, deltaTime)
        -- Keyboard global object handles key input
        if Keyboard:justPressed(Key.Space) then
            mySprite:play("waveHello")
        end
    end
})
```

## Documentation
*TODO*

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

When you run the installer, select "*Desktop development with C++*".

Once installation is completed, you are ready to use Amara2.

## List Of Nodes
Here is a compehensive list of what nodes are available:
- *Node* : The base node object.
- *Group* : Used to grouping Nodes as one entity (by creating them as children of the Group).
- *CopyNode* : Used to re-render a node in another location.
- *NodePool* : Used to pool nodes for reuse.
- *Text* : Used to render text from a ttf.
- *Camera* : Used by a Scene to render elements. View can be controlled with scroll and zoom.
- *Scene* : Used to organize your game's various states.
- *Action* : The base action, used to reuse behaviors across various types of nodes.
- *WaitAction* : Used to pause an amount of time before performing a function or another action (typically accessed via node.wait).
- *Tween* : An action used to tween the values of a node (typically accessed via node.tween).
- *StateMachine* : Used to manage various states of a node.
- *DelayedNode* : Used to delay the creation of a node.
- *PeriodicAction* : Used to perform a repeated action with a specified time between each call.
- *ProcessNode* : Used to execute a System program as a sub-process and manage output.
- *TextInput* : Used to track user keyboard input.
- *Loader* : Used to load assets into the game (typically access via node.load).
- *AutoProgress* : Used to progress a Text node (i.e. text appearing character by character).
- *Sprite* : Used to display an image and animate from a spritesheet.
- *Animation* : Action to make a Sprite play an animation from a spritesheet (typically access via sprite:play)
- *FillRect* : Used to display a colored rectangle.
- *Container* : Used to crop view of child nodes within a rectangle.
- *TextureContainer* : Used to crop view of child nodes in a textured container that can be further manipulated with rotation and tints.
- *ShaderContainer* : (OpenGL graphics only) A TextureContainer used to perform multiple render passes.
- *TilemapLayer* : Used to display a layer from a tilemap.
- *Tilemap* : Used to manage the various elements of a tilemap (e.g. layers and objects from a Tiled .tmx file)
- *NineSlice* : Used to display a textured box, typically for GUI. https://en.wikipedia.org/wiki/9-slice_scaling
- *Audio* : A node used to manage the playing of an audio.
- *AudioPool* : If a sound is supposed to be layered, or a different sound to be played each time then use an AudioPool.
- *AudioGroup* : Used to group audio. Also used to manage the volume of a collection of audio.
- *AudioMaster* : The root audio node used by the World class. Add your audio nodes as a child to world.audio.
- *Transition* : Base node used to create your own transition between scenes.
- *FillTransition* : Used to fade in and out between scenes.
- *World* : The big node used to contain the game and manage the window.
