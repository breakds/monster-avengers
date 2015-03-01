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

It also comes with a web based UI.

## The Webapp

There is currently one instance of this tool running on a server with very limited computational resource. You can find it at [http://mh4u.breakds.org](http://mh4u.breakds.org).

## Codebase

For those who is interested in either the algorithm or the implementaion, you can find a brief description below. Note that the code is still not very well documented yet, and I am working on adding the comments and documentations to the code as much as possible with the limited time budget.

The code is mainly divided into 2 parts:

1. [cpp/](https://github.com/breakds/monster-avengers/tree/master/cpp) A C++ implementation of the search algorithm, where we have
  * [lisp/](https://github.com/breakds/monster-avengers/tree/master/cpp/lisp) provides the parser and reader of lisp objects, in which format query and database entries are stored.
  * [data/](https://github.com/breakds/monster-avengers/tree/master/cpp/data) defines the data structs that are involved, e.g. the skills, the armors, the jewels ,etc.
  * [core/](https://github.com/breakds/monster-avengers/tree/master/cpp/core) implements the search algorithm, where you can find the member method `ArmorUp::Search` is the main entry point to the algorithm.
2. [lisp/ui](https://github.com/breakds/monster-avengers/tree/master/lisp/ui) implements the web based UI for this armor search tool. This webapp is written in Common Lisp based on the web framework [reaLispic](https://github.com/breakds/realispic). Specifically,
  * [widgets/](https://github.com/breakds/monster-avengers/tree/master/lisp/ui/widgets) contains the definitions of all the widgets in the web. The code should be very straightforward if you are familiar with [React.js](http://facebook.github.io/react/).
  * [backend.lisp](https://github.com/breakds/monster-avengers/tree/master/lisp/ui/backend.lisp) defines the RPC that talks to the C++ backend to handle queries. This is a super naive implementation that does the message passing via files on disk. 





