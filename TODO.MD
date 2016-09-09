The TODO list...
================

### Swap over dear imgui render to use render api
>* ~~Swap over vertex buffer gen to render api~~ **Done**
>* ~~Add state to material parameters~~ **Done**
>* ~~Add inline api for creating vertex buffers on the fly~~ **Done**
>* ~~Swap over custom shader program to material resource~~ **Done**
>* Set Texture inputs/overrides via material interface

### Get material parameter inputs working
>* *get default material parameters loading and set-able*...**Doing**
>* material parameters need to create bgfx Uniforms
>* get material setup parameters loading and set-able
>* allow material setup to combine inputs from base material and its own inputs
>* add flush to materia setup to set these parameters

### Get Textures loading
>* fixup texture processor for [builder.py](https://github.com/JoJo2nd/hart/blob/master/data/builder/builder.py)
>* define class for texture, needed to be object to load/unload. May need one per dim i.e. 1D, 2D, 3D, Cube
>* get texture1D loading working...May not need
>* get texture2D loading working
>* get texture3D loading working
>* get textureCube loading working...May not need

### Load a collection of game objects
>* ???

### Save a collection of game objects
>* Strip the current serialise function from objects
>* ???

### Load and render map exported from Tiled.
>* export and process map in builder.py
>* write gather prerequisites script

### Remove magic numbers
>* I have a habit of using ~0ul in a number of places. Need to replace these with constants.
