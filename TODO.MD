The TODO list...
================

Completed task are marked with strikethrough 

### ~~Swap over dear imgui render to use render api~~
>* ~~Swap over vertex buffer gen to render api~~
>* ~~Add state to material parameters~~
>* ~~Add inline api for creating vertex buffers on the fly~~
>* ~~Swap over custom shader program to material resource~~
>* ~~Set Texture inputs/overrides via material interface~~

### Get material parameter inputs working
>* get default material parameters loading and set-able
>* ~~material parameters need to create bgfx Uniforms~~
>* ~~get material setup parameters loading and set-able *from runtime*~~
>* ~~allow material setup to combine inputs from base material and its own inputs~~
>* ~~add flush to material setup to set these parameters~~
>* add support for texture resources to be passed to material setup at runtime and have material setup take a reference to the resource. Currently, the resource link mush be setup up by asset links in resource manager (a.k.a. prerequisites)
>* ~~update gather prerequisites script for materials to handle texture resource inputs~~

### Add Support for reloading resources
>* Allow clients to register callbacks with the resource system. Callbacks are used to notify when a resource changes. These callbacks should be debug only.
>* Add system to detect when an asset changes (file watcher?). 
>* On file change reload the asset. This should be done as a synchronous, blocking method on the main thread and before a tick.
>* Fix up current resources to support reload.

### Get Resource Unloading working
>* Create the unload queue and process it. Then add test case.

### Get Textures loading
>* ~~fixup texture processor for builder.py~~
>* ~~define class for texture, needed to be object to load/unload. May need one per dim i.e. 1D, 2D, 3D, Cube~~
>* ~~get texture1D loading working...May not need~~
>* ~~get texture2D loading working~~
>* ~~get texture3D loading working~~
>* ~~get textureCube loading working...May not need~~

### Load a collection of game objects
>* ~~Define game object in assets.~~
>* ~~Add script to parse prerequisites from entity/game object~~
>* ~~Added builder process script to deal with whatever final game object format will be~~
>* ~~Load game object in runtime.~~
>* Load game object in runtime alongside save data to allow restoring a saved game.

### Save a collection of game objects
>* ~~Strip the current serialise function from objects~~
>* ???

### Load a animated sprite and render on screen
>* ~~Process sprite frames and anim data from asset~~
>* ~~Create component to reference, instance and play anims on sprite.~~
>* ~~Create sprite rendering functions and data~~

### Load a tileset exported from Tiled
>* ~~write builder pipeline/script to generate asset~~
>* ~~Load asset file in runtime~~

### Load and render map exported from Tiled.
>* ~~export and process map in builder.py~~
>* ~~load level asset~~
>* ~~render level asset~~
>* link entities to levels that loaded them

### Add basic collision systems
>* ~~Add basic AABB tests (AABBxAABB, RayxAABB, MovingAABBs)~~
>* Add collision system for board phase pass (collect objects near player/NPC/moving objects)
>* Add **simple** physics system for collision reaction (verlet integration for solve & response)
>* Add callback system for other collision types (phantoms/trigger zones/NPC interactions)

### Add simple string collection resource
>* for translations (but will be english only ATM), 
>* use CSV for format
>* add generated header enum for reading table of values
>* support hash table for debug builds to speed up iteration (no extra compiles...)

### Add Player entity
>* with sprite rendering & animation
>* saving & loading support

### Player input
>* If going with a flashback style system; This system is grid based i.e. an action like step forward will move the character 1 grid space left or right. 
>* Define the grid convention. How big is a grid unit (width & height in pixels)? Will all animation be locked to that grid? How tall and wide is the main player character in the gird. This will likely define the size for most enemies. Flashback/Another World might be solid references here.
>* Create a test sprite of just a single colour block which matches the main player character. Add some debug rendering to display a grid (a test level with other things like jumps, ledges, etc can come later...) for testing movement with. Add a number of test animations to the sprite (with letters on the frames to show what animation is playing). You will find you need to extend the sprite animation system as you work with it to fit your needs. One likely extention is to add events to animation. These events could be used
 to signal back to the game code when an animation hits a given point such as the end of the animation, when a foot hits the ground or gun is fired so a sound can be player or when some gameplay logic needs to check for something (e.g. checking inputs on a controller and updating when and what the next animation is to play)
>* Play Flashback or another world and examin how their movement works. What feels right about it? What could be better?
>* Define (as best as possible) how the input system will work.
>* Create a player class with an input component. Begin writing code to implement input and character movement. Systems to read keyboard & joypad buttons already exist. Iterate on this until it feels good enough.

### Remove magic numbers
>* I have a habit of using ~0ul in a number of places. Need to replace these with constants.

### Move enums to seperate header files
>* including fbs ones. Wrap them in their own fbs/headers and typedef.

### Add keyboard support to Dear ImGUI

### Add keyboard & gamepad support (via events for game hook up)

### Add debug menus
>* ~~Add register function for debug menus~~
>* ~~Add debug menu for resource manager. Should allow testing loading/unloading assets (outside of currently loaded assets). Should display **ALL** assets that can be loaded along side assets that currently loaded (plus refcount, friendly name, etc)~~
