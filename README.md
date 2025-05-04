# MeshParser

**MeshParser** is the repository for the **MeshUnpacker**, a command-line tool for converting between Shadow of War `.mesh` files and `.fbx` files.

## Features

- Converts between `.mesh` and `.fbx` file formats
- Supports static meshes
- Preserves:
    - Vertex positions
    - Texture coordinates (UVs)
    - Normals
    - Vertex colors

## Installation

To install the **MeshUnpacker** simply download the latest release from the [Releases](https://github.com/Shadow-of-War-Modding-Community/MeshParser/releases) section and extract it to a location of your choice.

## Usage

### Batch File Usage
Following the installation is two batch files named `Unpack.bat` and `Repack.bat` which allows you to select the files through the file explorer instead of having to use the command-line.

#### Convert MESH File to FBX File
> 1. Launch the **`Unpack.bat`** file.
> 2. Select the **mesh file** that you want to convert.
> 
> An **output file** named **`out.fbx`** should then appear in the same directory as the **mesh file**.

#### Convert FBX File to original MESH File
> 1. Launch the **`Repack.bat`** file.
> 2. Select the **fbx file** that you want to convert.
> 3. Select the **original mesh file**.
> 
> An **output file** named **`out.mesh`** should then appear in the same directory as the **fbx file**.

### Command-Line Usage
You can follow these usage instructions If you prefer to use the command-line.

#### Command-Line Arguments
> - **First argument**: **`unpack`** or **`repack`**.
> - **Second argument**: **`.mesh file`** you want to unpack or **`.fbx file`** you want to repack.
> - **If first argument is `repack`**:
>  - **Third argument**: Original **`.mesh file`** to get merged with the **`.fbx file`** passed in the second argument.

#### Convert MESH File to FBX File
> Open **CMD** and run this command:
> ```PATH\\TO\\YOUR\\MeshUnpacker.exe unpack PATH\\TO\\YOUR\\FILE.mesh```
> An **output file** named **`out.fbx`** should then appear in the same directory as the **mesh file**.
> 
> **NOTE:** Adjust the paths according to your setup.

#### Convert FBX File to original MESH File
> Open **CMD** and run this command:
> ```PATH\\TO\\YOUR\\MeshUnpacker.exe repack PATH\\TO\\YOUR\\FILE.fbx PATHT\\TO\\YOUR\\FILE.mesh```
> An **output file** named **`out.mesh`** should then appear in the same directory as the **fbx file**.
>
> **NOTE:** Adjust the paths according to your setup.

## Authors

- [survivalizeed](https://github.com/survivalizeed)
- [xNightswitch](https://github.com/xNightswitch)

## Credits

- [Assimp](https://github.com/assimp/assimp) - Loads 3D file formats into a shared, in-memory format
- [Christian Rau](rauy@users.sourceforge.net) - IEEE 754-based half-precision floating point library
