# Amara2
A 2D game engine with Lua scripting.
Built in C++ using SDL3.

*Note: Amara2 is currently available for Windows only.*

[Click here for the latest release.](https://github.com/BigBossErndog/Amara2/releases)

[Click here for tutorials and documentation.](https://github.com/BigBossErndog/Amara2/wiki)

## Features
- Setting up a new project and running a test is easy as heck.
- Game architecture inspired by my favorite elements of the Phaser framework and the Godot Engine.
- Minimal GUI. Get the necessities while making your commonly used tools more cohesive.
- A charming pixel-art aesthetic inspired by popular indie developer tools such as Aseprite.
- Built using SDL3, so porting to other platforms shouldn't be too great a task for porting companies.

<img width="1920" height="1080" alt="Screenshot 2025-07-15 141959" src="https://github.com/user-attachments/assets/98571dfa-3573-4019-bfe9-5382c862b4e7" />

## What I'm Currently Working On
- Automated web export via emscripten
- Android export via Android Studio

## Community
Join the Discord community where you can ask questions, promote your work and interact with fellow users of Amara2!

https://discord.gg/wEfZN8zGnT

## Support Development
I have a ko-fi available for people who would like to donate.

I have many plans to support more tools and more target platforms, so your support would be greatly appreciated!

https://ko-fi.com/ernestplacido

## Philosophy
This is a game engine for people who dislike game engine GUIs. If you're like me, you may get overwhelmed by endless text fields and drop down menus.

Get the benefits of an easy-to-use game framework while also taking out all the frustrations of setting up a new project, making builds and running tests.

All your core necessities sit in small windows floating on top of other game dev programs you may be using, so you can use whichever tools you want without having to switch out of the engine.

Over time I want this program to tie in with all your favorite game dev tools, easy access to everything you need.

The front end is also built using the engine, all the lua scripts are in the repository for you to check out!

I'm committed to using this to create all my games from now on (1st July 2025).


## Supported Tools
- Code Editors such as VS Code, Atom, Sublime.
- Tiled .tmx file support.

## Supported Platform Exports
- Windows
- Web (HTML)
  
*More will be supported along the way.*

## Node System
Amara2 defines every entity in the game as nodes, all of which exist in a tree starting with your World. Extend existing nodes and add nodes as children to other nodes in this easy-to-use modular system. And define them all with easy to read Lua tables!

Here is an example of how you would write a scene in Amara2.

Example file: "*lua_scripts/MyScene.lua*"
```lua
Nodes:define("MyScene", "Scene", { -- define MyScene from already existing node Scene
    -- Three main functions of a node: onPreload, onCreate and onUpdate
    
    onPreload = function(self)

        -- Loading a font of size 10
        -- General rule of thumb, call a function with a colon.
        self.load:font("defaultFont", "fonts/PixelMplus10-Regular.ttf", 10)

        -- Loading a spritesheet with frame of size 32 x 32 px.
        self.load:spritesheet("mySpritesheet", "spritesheets/mySpritesheet.png", 32, 32) 
        
        self.animations:add({
            key = "waveHello",
            texture = "mySpritesheet",
            frameRate = 12,
            frames = { 1, 2, 3 }, -- frames start at 1 in the top-left of texture.
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
        
        self.props.mySprite = self:createChild("Sprite", {
            x = 0, y = 0, -- (0, 0) is the center of the screen / view.
            texture = "mySpritesheet",
            frame = 4, -- set starting frame, first frame (top-left) is 1
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
            self.props.mySprite:animate("waveHello")
        end
    end
})
```
You would then load it in within your *index.lua* or another script file via the following.
```lua
Nodes:load("MyScene.lua") -- searches for "MyScene.lua" in the folder "lua_scripts"

-- Additionally, you can omit ".lua" to search for either ".lua" or a compiled ".luac".
-- Handy for later when you are able to compile your lua scripts.
Nodes:load("MyScene") -- searches for "lua_scripts/MyScene.lua" or "lua_scripts/MyScene.luac"
```

You can now use this loaded Node from anywhere in your project.


## Documentation

[Click here for Tutorials and Documentation.](https://github.com/BigBossErndog/Amara2/wiki)

## Setup
### Windows
Download the windows zip file from the latest release:

https://github.com/BigBossErndog/Amara2/releases

Run ```Amara2.exe```.


#### - *Building your project*
Amara2 requires VS Build Tools from Microsoft to make a final build of your game.
You don't have to install it until you need to build your game.

Download the installer from:

https://visualstudio.microsoft.com/visual-cpp-build-tools/

When you run the installer, select ```Desktop development with C++```.

Once installation is completed, you are ready to build your game.

## Not-Yet-Frequently Asked Questions
### - Can I contribute code to development?

Sorry, I'm not accepting code contributions. I made this engine so I can spend more time making my own games than to manage an open source project. However I am happy for people to fork this repo and extend functionality! There is also a plugin system, so creating your own plugins and sharing them is also an option. If there are any forks or plugins that I like and can see becoming core functionality, I will be reaching out to see if we can make a deal to integrate it into the engine.

### - Why Amara'2'?
The first Amara was a C++ game framework (built on SDL2) that I created during university. It was used to make the game [Pry In The Void](https://store.steampowered.com/app/1234990/Pry_Into_The_Void/).

This new version vastly improves on it, while adding Lua scripting (so I don't have to compile the dang thing every time lol). The name itself is a reference that some people in my community may recognize from way back when.
