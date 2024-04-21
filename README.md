# asabru-engine
This project is a shared static library used in the ChistaDATA Database Proxy ( chista-asabru ) 


This project contains code for the ChistaDATA database proxy socket engine.

### Prerequisite

Make
`https://www.gnu.org/software/make`

CMake
`https://cmake.org/install/`

### Build

1. First create a build folder :

```
mkdir build
cd build
```

2. The `asabru-engine` project has a dependency on [asabru-commons](https://github.com/ChistaDATA/asabru-commons)  . This dependency can be installed in 3 ways.

* Based on release version on [Github](https://github.com/ChistaDATA/asabru-commons/releases). 

```
cmake ..
```

*  Based on commit ID on [Github](https://github.com/ChistaDATA/asabru-commons/commits/main/).

```
cmake -DASABRU_COMMONS_BUILD=GIT_TAG -DASABRU_COMMONS_GIT_TAG=<COMMIT_ID> ..
```

* Based on Local build directory.

```
cmake -DASABRU_COMMONS_BUILD=LOCAL_DIR .. 
```

3. Finally to create the build run the following command 

```
make
```