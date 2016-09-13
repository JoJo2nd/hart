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
>* update gather prerequisites script for materials to handle texture resource inputs

### Get Resource Unloading working
>* Create the unload queue and process it. Then add test case.

### Get Textures loading
>* ~~fixup texture processor for [builder.py](https://github.com/JoJo2nd/hart/blob/master/data/builder/builder.py)~~ **Done**
>* ~~define class for texture, needed to be object to load/unload. May need one per dim i.e. 1D, 2D, 3D, Cube~~ **Done**
>* ~~get texture1D loading working...May not need~~ **Done**
>* ~~get texture2D loading working~~ **Done**
>* ~~get texture3D loading working~~ **Done**
>* ~~get textureCube loading working...May not need~~ **Done**

### Load a collection of game objects (Not 100% sure how this system will work yet...)
>* Define game object in assets. This could be a collection of components in flatbuffer json format? Leaning towards a template file in this format with extra json file overridding some properties of the template. dot asset file would link to the extra json file and define a UUID for object (as with other resources). That UUID is the object's name (forever, to enable save/load) and then other resoruces (e.g. levels? scripts?) link to the object by UUID (by prerequisites for hard links, resid_t/uuid_t for soft links)
>* Added builder process script to deal with whatever final game object format will be
>* Load game object in runtime.
>* Load game object in runtime alongside save data to allow restoring a saved game.

### Save a collection of game objects
>* ~~Strip the current serialise function from objects~~ **Done**
>* ???

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