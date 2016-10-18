The TODO list...
================

### ~~Swap over dear imgui render to use render api~~
>* ~~Swap over vertex buffer gen to render api~~ **Done**
>* ~~Add state to material parameters~~ **Done**
>* ~~Add inline api for creating vertex buffers on the fly~~ **Done**
>* ~~Swap over custom shader program to material resource~~ **Done**
>* ~~Set Texture inputs/overrides via material interface~~ **Done**

### Get material parameter inputs working
>* get default material parameters loading and set-able
>* ~~material parameters need to create bgfx Uniforms~~ **Done**
>* ~~get material setup parameters loading and set-able *from runtime*~~ **Done**
>* ~~allow material setup to combine inputs from base material and its own inputs~~ **Done**
>* ~~add flush to material setup to set these parameters~~ **Done**
>* add support for texture resources to be passed to material setup at runtime and have material setup take a reference to the resource. Currently, the resource link mush be setup up by asset links in resource manager (a.k.a. prerequisites)
>* ~~update gather prerequisites script for materials to handle texture resource inputs~~ **Done**

### Get Resource Unloading working
>* Create the unload queue and process it. Then add test case.

### Get Textures loading
>* ~~fixup texture processor for [builder.py](https://github.com/JoJo2nd/hart/blob/master/data/builder/builder.py)~~ **Done**
>* ~~define class for texture, needed to be object to load/unload. May need one per dim i.e. 1D, 2D, 3D, Cube~~ **Done**
>* ~~get texture1D loading working...May not need~~ **Done**
>* ~~get texture2D loading working~~ **Done**
>* ~~get texture3D loading working~~ **Done**
>* ~~get textureCube loading working...May not need~~ **Done**

### Load a collection of game objects
>* ~~Define game object in assets.~~ **Done**
>* ~~Add script to parse prerequisites from entity/game object~~ **Done**
>* ~~Added builder process script to deal with whatever final game object format will be~~ **Done**
>* ~~Load game object in runtime.~~ **Done**
>* Load game object in runtime alongside save data to allow restoring a saved game.

### Save a collection of game objects
>* ~~Strip the current serialise function from objects~~ **Done**
>* ???

### Load a animated sprite and render on screen
>* Process sprite frames and anim data from asset
>* Create component to reference, instance and play anims on sprite.
>* Create sprite rendering functions and data

### Load a tileset exported from Tiled
>* ~~write builder pipeline/script to generate asset~~ **Done but to be dropped**
>* ~~Load asset file in runtime~~ **Done to allow loading embedded tilesets in levels**

### Load and render map exported from Tiled.
>* ~~export and process map in builder.py~~ **Done**
>* ~~load level asset~~ **Done**
>* ~~render level asset~~ **Done**
>* link entities to levels that loaded them

### Add basic collision systems
>* ~~Add basic AABB tests (AABBxAABB, RayxAABB, MovingAABBs)~~ **Done**
>* Add collision system for board phase pass (collect objects near player/NPC/moving objects)
>* Add **simple** physics system for collision reaction (verlet integration for solve & response)
>* Add callback system for other collision types (phantoms/trigger zones/NPC interactions)

### Add simple string collection resource
>* for translations (but will be english only ATM), 
>* use CSV for format
>* add generated header enum for reading table of values
>* support hash table for debug builds to speed up iteration (no extra compiles...)

### Add Player entity
>* player entity components/tasks:
    * player input component (handles input mapping)
    * animated sprite component (creates sprite, updates anim state, adds to render lists)
    * physics component (defines solid region, has movement forces applied, reacts to collision, contains world position(!!))
    * more..?
>* with sprite rendering & animation
>* with saving

### Add Shop entity NPC
>* with test dialog
>* with saving purchase history (items go out of stock)
>* use test menu built in ImGUI

### Add test NPC with debug choices
>* ala FFVIII debug menu. Allows setup of debug levels/stages/etc.
>* use test menu built in ImGUI.

### Add menu system
>* replace any in-game ImGUI menus
>* Style in a classic JRPG style (borrow one to start with)

### Remove magic numbers
>* I have a habit of using ~0ul in a number of places. Need to replace these with constants.

### Move enums to seperate header files
>* including fbs ones. Wrap them in their own fbs/headers and typedef.

### Add keyboard support to Dear ImGUI

### Add keyboard & gamepad support (via events for game hook up)

### Add debug menus
>* ~~Add register function for debug menus~~
>* ~~Add debug menu for resource manager.~~ Should allow testing loading/unloading assets (outside of currently loaded assets). ~~Should display **ALL** assets that can be loaded along side assets that currently loaded (plus refcount, friendly name, etc)~~