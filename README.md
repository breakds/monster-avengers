# monster-avengers - The Monster Hunter 4G/4U Armor Set Search Tool

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





