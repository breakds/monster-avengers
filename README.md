# monster-avengers - The MH4U/G Armor Set Search Tool

## Introduction
This is a hobby project of mine, which impelements an algorithm and a web based UI that finds armor sets to satisfy certain requirements in the game [Monster Hunter 4 Ultimate](http://www.monsterhunter.com/) (Japanese Version: [Monster Hunter 4G](http://www.capcom.co.jp/monsterhunter/4G/)). 

Monster Hunter has an unique skill system, and in order to active a certain skill, we need to craft a combination of armors including a weapon, a helmet, a body armor, a waist armor, an arm armor, a legs armor and an amulet, whose total contribution to the desired skill exceeds a skill points level. Finding the right set of armor pieces that activates the required skills is difficult, as there are ~ 150 different skill trees, ~ 600 candidate armors for each of the pieces, and each armor contributes to several skills, positively or negatively. What makes the problem more interesting is that armors may have slots, where different decorations contributing to different skills can be plugged in. 

monster-avengers is such a tool to efficiently find the right armor combinations given the desired set of skills. With a carefully designed algorithm, it 

1. supports fast search from all 3,000+ different armors in the databse.
2. outputs armor combinations with decorations.
3. consideres Torso up armors.
4. supports adding custom amulets in the query.
5. considers the number of slots on the weapon.
6. supports filtering on the search result to exclude certain armor pieces.
7. communicates with a simple lisp-based query language.
8. suggests skills that are still achievable based the current selected skills.

It also comes with a web based UI.

## Supported Interfaces

### Web App

There is currently one instance of this tool running on a server with very limited computational resource. You can find it at [http://mh4u.breakds.org](http://mh4u.breakds.org).

### C# API

Located in [csharp/winbind_wrapper](https://github.com/breakds/monster-avengers/tree/master/csharp/winbind_wrapper). A C# wrapper class implemented via DLLImport.

## Codebase

For those who is interested in either the algorithm or the implementaion, you can find a brief description below. Note that the code is still not very well documented yet, and I am working on adding the comments and documentations to the code as much as possible with the limited time budget.

The code is mainly divided into 2 parts:

1. [cpp/](https://github.com/breakds/monster-avengers/tree/master/cpp) A C++ implementation of the search algorithm, where we have
  * [lisp/](https://github.com/breakds/monster-avengers/tree/master/cpp/lisp) provides the parser and reader of lisp objects, in which format query and database entries are stored.
  * [data/](https://github.com/breakds/monster-avengers/tree/master/cpp/data) defines the data structs that are involved, e.g. the skills, the armors, the jewels ,etc.
  * [core/](https://github.com/breakds/monster-avengers/tree/master/cpp/core) implements the search algorithm, where you can find the member method `ArmorUp::Search` is the main entry point to the algorithm.
2. [lisp/ui/](https://github.com/breakds/monster-avengers/tree/master/lisp/ui) implements the web based UI for this armor search tool. This webapp is written in Common Lisp based on the web framework [reaLispic](https://github.com/breakds/realispic). Specifically,
  * [widgets/](https://github.com/breakds/monster-avengers/tree/master/lisp/ui/widgets) contains the definitions of all the widgets in the web. The code should be very straightforward if you are familiar with [React.js](http://facebook.github.io/react/).
  * [backend.lisp](https://github.com/breakds/monster-avengers/tree/master/lisp/ui/backend.lisp) defines the RPC that talks to the C++ backend to handle queries. This is a super naive implementation that does the message passing via files on disk. 
 
### Build and Run

The C++ code is managed by CMake. To build the C++ binary:
````
cd cpp/
mkdir build
cd build
cmake ..
````

Now, you can either run `make` to build the binary `serve_query`, or `ccmake .` to configure the Makefile generation. You can toggle the build type to be `RELEASE` to build a much more efficient binary.


To run the web server that serves the UI, you need a lisp implementation (e.g. [SBCL](http://kiranico.com/en/mh4u)), and [quicklisp](http://www.quicklisp.org/). After having them configured, you can place the project folder (or a symbolic link to the folder) under `quicklisp/local-projects`, and start `sbcl`. At the Common Lisp top-level, run
````lisp
(ql:quickload 'monster-avengers)
(monster-avengers.simple-web:armor-tools :start)
````

to start the web server. You should now be able to access the web app locally at `localhost:16384/app?lang=en`. Note that this project depends on several of other projects, which should be downloaded at placed at `quicklisp/local-projects`:
* [basicl](https://github.com/breakds/basicl)
* [stefil](https://github.com/breakds/stefil)
* [reaLispic](https://github.com/breakds/realispic)
* [struct-wrapper](https://github.com/breakds/struct-wrapper)

## Limitations and Futrue Work

There are currently some limitations in the current implementations, and I will work on improve over them. Those that I am aware of are:

1. The webapp does not support custom guild armors.
  * The algorithm supports guild armors, of course.
  * I am a terribly bad designer and I cannot think of a way to add such options to the page, without making it look very complicated. 
  * This functionality will be added soon.
2. The communication between backend and web server are file-based
  * This is going to be replaced by a real RPC framework.
3. The webapp does not store any information to personalization features, such as personal armor sets library, personal amulets library, etc.
  * To be honest, I am not experienced enough to implement this efficiently and securely. Will add those features in the future, but with no promise.
4. Hunter progress options.
  * I think it is a good idea to filter out armors based the player's progress. I am lacking the data to implement this feature.

## Acknoledgement

1. Credit to Ping An, the author of [Ping's Dex](https://sites.google.com/site/pingsdex/pingsmh4gdex). His effort on composing the data are source to many other related projects.
1. Credit to the project [MonsterHunter4UDatabase](https://github.com/kamegami13/MonsterHunter4UDatabase), where all the data (armors, skills and decorations) are from their database. Completing the database requires a lot of work and without their effort, I would not have accomplished this.
2. Credit to Cassandra Qi, my wife and my Monster Hunter game partner, for working on the art and design of the UI.
3. Thanks to my friends who inspired me, encouraged me, disccussed the ideas with me and helped with the tests:
  * Frank Xiao Lin
  * Zommie Dev
  * Hui Guo
  * [KIRANICO](http://kiranico.com/en/mh4u)
  * Epona Yunlu Guo
  * Wybaby

## License

All the codes in this project are under the MIT License.



