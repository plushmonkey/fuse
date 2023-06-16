# Fuse
Fuse is an injection library for Continuum. It works by making use of Continuum's automatic loading of the direct sound dll.

## Loader
By providing a custom `dsound.dll`, Continuum will load it and begin executing the Fuse loader.  
The Fuse loader will look in the Continuum directory for a file named `fuse.cfg` and attempt to load the libraries listed in each line of the file.

### Build
Use the provided MSVC solution to build in x86 release mode.

### Setup
- Copy `dsound.dll` and `fuse.dll` in the `Release` folder into your Continuum directory.
- Create a file called fuse.cfg in your Continuum directory.
- List each dll you want to load in the cfg file separated by lines.

## Fuse library
In addition to the loader, there is a library provided for hooking into the gameplay loop and reading common data structures such as player data and weapon data.  
Use the hello_world project as an example of using the library.

## Examples
#### hello_world
A very simple example of injecting into the main update loop and rendering some text.

#### multicont
Overrides the mutex checks so multiple Continuum clients can be launched at once.

#### menu_controller
Sets the active profile index while on the menu screen. It gets this index from launching Continuum with an index argument.
