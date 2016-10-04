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
>* write builder pipeline/script to generate asset
>* Load asset file in runtime

### Load and render map exported from Tiled.
>* export and process map in builder.py
>* write gather prerequisites script

### Remove magic numbers
>* I have a habit of using ~0ul in a number of places. Need to replace these with constants.

### Move enums to seperate header files
>* including fbs ones. Wrap them in their own fbs/headers and typedef.

### Add keyboard support to Dear ImGUI

### Add debug menus
>* ~~Add register function for debug menus~~
>* ~~Add debug menu for resource manager.~~ Should allow testing loading/unloading assets (outside of currently loaded assets). ~~Should display **ALL** assets that can be loaded along side assets that currently loaded (plus refcount, friendly name, etc)~~