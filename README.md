# deferred-renderer


## How to build

### Linux
Pretty much the same as the Windows instructions.

- Install the dependencies (might need other packages depending on distribution)
  - Arch Linux
    ```
    pacman -S cmake make gcc
    ```
  - Debian/Ubuntu
    ```
    apt-get install cmake make gcc
    ```
    
- Build
  ```
  mkdir build && cd build
  cmake -DCMAKE_BUILD_TYPE=Release .. -G "Unix Makefiles"
  make -j4
  ```

### Windows (MSYS2 mingw64)
- Install the dependencies with the following command
    ```
    pacman -S mingw-w64-x86_64-cmake make mingw-w64-x86_64-gcc
    ```
    
- Create a build directory, generate the make files, and make the project
  ```
  mkdir build && cd build
  cmake -DCMAKE_BUILD_TYPE=Release .. -G "Unix Makefiles"
  make -j4
  ```
  
  
### macOS
You can build on macOS but it won't run... 
Apple still only supports OpenGL version 4.1, which came out in 2010.



## How to run

- Execute `deferred` or `deferred.exe`



