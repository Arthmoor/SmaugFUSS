#FUSSAREA
#AREADATA
Version      1
Name         Redferne's Residence~
Author       Diku~
WeatherX     0
WeatherY     0
Ranges       20 30 0 60
Economy      0 1246000
#ENDAREADATA

#MOBILE
Vnum       7900
Keywords   grand knight paladin~
Short      the Grand Knight of paladins~
Long       The Grand Knight is standing here, waiting for someone to help.
~
Desc       The Knight is standing here, smiling at you. He is dressed all in
white, blue and silver. He looks VERY strong, as he stands here, ready to
help the innocent.
~
Race       human~
Class      warrior~
Position   standing~
DefPos     standing~
Specfun    spec_cast_cleric~
Gender     male~
Actflags   npc stayarea prototype~
Affected   detect_invis sanctuary~
Stats1     1000 45 -5 -9 2000 100000
Stats2     20 10 700
Stats3     2 6 14
Stats4     0 0 4 0 0
Attribs    13 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common~
Speaking   common~
Bodyparts  head arms legs heart guts hands feet ear eye~
Attacks    kick trip bash weaken~
Defenses   parry dodge disarm~
#ENDMOBILE

#OBJECT
Vnum     7909
Keywords treasure coins~
Type     money~
Short    a huge treasure~
Long     A huge treasure is lying here, looking very valuable.~
WFlags   take~
Values   -100 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    treasure~
ExDesc       Looks like a LOT of coins.  One of them has writing on it.
~
#ENDEXDESC

#EXDESC
ExDescKey    writing letters~
ExDesc       It reads: "Hi Conan.  No more coins from here, pal. :) Signed, Redferne".
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     7910
Keywords chest~
Type     container~
Short    a wooden chest~
Long     A wooden chest stands in the corner.~
WFlags   take~
Values   100 15 7911 0 0 0
Stats    400 100 10 0 0
#EXDESC
ExDescKey    chest~
ExDesc       It is a robust chest made from short, heavy planks that have been fastened
together with tenons.  It is equipped with a simple brass lock.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     7911
Keywords key~
Type     key~
Short    a small brass key~
Long     A small brass key lies here.~
WFlags   take~
Values   7910 0 0 0 0 0
Stats    1 10 1 0 0
#EXDESC
ExDescKey    key~
ExDesc       It is a small, simple brass key with no inscriptions or marks of any kind.
~
#ENDEXDESC

#ENDOBJECT

#ROOM
Vnum     7900
Name     Outside Redferne's residence~
Sector   inside~
Flags    nomob private nosummon noastral~
Desc     A huge cloud forms the plateau on which you are now standing. The wind here
is absolutely quiet and the sun is shining warmly upon you. From under the
cloud you can hear the faint sounds of Darkhaven. Right before you to the
north lies the grand Mansion of Naris, the residence of the Greater God
Redferne.
~
#EXIT
Direction north~
ToRoom    7901
Desc      You see a HUGE arched gate leading into this magnificent building.
~
Keywords  gate door huge arched~
Flags     isdoor closed pickproof~
#ENDEXIT

#EXIT
Direction down~
ToRoom    7918
Desc      You see a Huge Chain that anchors the Mansion of Naris to the ground.
~
#ENDEXIT

Reset D 0 7900 0 1
Reset M 0 7900 1 7900
  Reset E 1 7216 99 16
  Reset E 1 7217 99 5
  Reset E 1 7218 99 7
  Reset E 1 7219 99 9
  Reset E 1 7220 99 6
  Reset E 1 7221 99 11
  Reset E 1 7223 99 10
  Reset E 1 7222 99 8
  Reset E 1 7224 99 12
  Reset G 1 7911 99
#MUDPROG
Progtype  act_prog~
Arglist   p has lost~
Comlist   mpforce 0.$n quit
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     7901
Name     The Southern end of the hall~
Sector   inside~
Flags    nomob indoors private~
Desc     You are standing in a vast hall that is dimly lit, but wherefrom the
light comes, is a mystery. The walls seem to radiate warmth and give
the pleasant feeling of being welcome here. A large portrait is hanging
on one of the walls. A large wooden staircase leads up into the tower.
To the east there is a high passage away from the hall. This ends
shortly after by a tall oak door. The enormous hall extends further
north from here. To the south you can see a huge, and VERY heavy-
looking iron-wrought door. It looks like this is the only exit from
this magnificent old house. To the west you can see a large ashen door.
~
#EXIT
Direction north~
ToRoom    7904
Desc      The hallway continues that way. You can see more doorways under the
wooden staircase in that direction.
~
#ENDEXIT

#EXIT
Direction east~
ToRoom    7910
Desc      You see there a tall oak door. It looks quite tightly closed to you.
On it little runes are chiselled into the wood.
~
Keywords  oak tall east oakdoor talldoor eastdoor~
Flags     isdoor closed pickproof~
#ENDEXIT

#EXIT
Direction south~
ToRoom    7900
Desc      Here you see a REAL door. It would be more proper to call this a
"GATE", rather than a "door". It's really HUGE! On it hangs a large
sign with very large letters spelling : "EMERGENCY EXIT".
~
Keywords  gate~
Flags     isdoor closed pickproof~
#ENDEXIT

#EXIT
Direction west~
ToRoom    7902
Desc      This looks like a "door" in the meaning of the word. The ashen wood
is painted in a peculiar yellow colour. Small letters are written
with black on it.
~
Keywords  ashen ash door west yellow~
Flags     isdoor closed pickproof~
#ENDEXIT

#EXIT
Direction up~
ToRoom    7909
Desc      You see the staircase extending upwards into the very high tower.
It ends in what seems like a large bedroom up there.
~
#ENDEXIT

Reset D 0 7901 2 1
Reset D 0 7901 3 1
Reset D 0 7901 1 1
#EXDESC
ExDescKey    runes~
ExDesc       These runes are utterly strange to you, but you are in luck today: Under
the runes you can just make out a sentence in Common. It goes : " Stay out,
if you treasure your life. That is if you are mortal.".
~
#ENDEXDESC

#EXDESC
ExDescKey    letters~
ExDesc       They read "LIBRARY".
~
#ENDEXDESC

#ENDROOM

#ROOM
Vnum     7902
Name     Redferne's Library~
Sector   inside~
Flags    nomob indoors private~
Desc     This is truly a magnificent place! Books and scrolls lie together on every
shelf. A large globe, with the map of the Mud-world upon it, stands in the
dimly lit north-western corner of the room. Two large and comfortable-
looking leather arm-chairs adorn the center of the library together with
a huge oak desk. Dim light radiates from an enormous crystal chandelier
hanging down from the ceiling approximately 10 feet off the floor. To the
east there is a great old ashen door. A large glass window is set in the
west wall.
~
#EXIT
Direction east~
ToRoom    7901
Desc      You can see an old ashen door, painted in a peculiar yellow colour.
~
Keywords  door ash ashen east~
Flags     isdoor closed pickproof~
#ENDEXIT

Reset D 0 7902 1 1
#EXDESC
ExDescKey    globe world map~
ExDesc       You see a large world map stretch out on the enormous globe. It has towns
drawn in every spot available for such. In the middle of the map you can
spot a large town with the name 'DARKHAVEN' written over it. The rest is
mountains, woods, plains and water.
~
#ENDEXDESC

#EXDESC
ExDescKey    darkhaven town city~
ExDesc       You see a small speck with woods on the west from it, plains to the east
from it and mountains to the north from it. To the south from it you can
see a thin trail lead to a large castle. Finally you notice a rather large
river pour in from the east and go through Darkhaven in the middle.
~
#ENDEXDESC

#EXDESC
ExDescKey    chairs leather arm-chairs~
ExDesc       These two chair are exactly alike one another. They look incredibly
comfortable. They're both made from old leather, and yet they seem so worn
that they can be nothing but a perfect place for a long needed rest.
~
#ENDEXDESC

#EXDESC
ExDescKey    window glass~
ExDesc       These windows are really BIG! They reach from about 20 inches above the
floor to approximately 10 inches under the ceiling. If you try and
"look out", you might see what might lie beyond these windows.
~
#ENDEXDESC

#EXDESC
ExDescKey    out outside beyond~
ExDesc       The clouds muster and form the ground on which this entire building is set.
Through the thinnest of the clouds you can just make out DARKHAVEN with all
it's magnificent activity.
~
#ENDEXDESC

#EXDESC
ExDescKey    river~
ExDesc       You see a large and winding river cut through the landscape, starting at an
enormous inland lake, it seeps through Darkhaven and finally ends up in the
Grand Sea on the West-coast of the land.
~
#ENDEXDESC

#ENDROOM

#ROOM
Vnum     7903
Name     The Artifact room of Naris~
Sector   inside~
Flags    nomob indoors private~
Desc     This is gloomy and dark room with only a faint light radiating from the
walls. A bunch of funny-looking items fill the center of the room. There are
no furniture here what-so-ever. The only way out seems to be west, through
the low steel door.
~
Reset O 0 7909 10 7903
#ENDROOM

#ROOM
Vnum     7904
Name     The Northern end of the hall~
Sector   inside~
Flags    nomob indoors private~
Desc     You are standing in the northern end of the huge hall. This part is under
the grand wooden staircase so the light seems a little less bright here,
but this doesn't bother your sight at all. To the north lies the kitchen.
To the south lies the Southern end of the hall. To the east there is a huge
metal door. To the west there is a large aspenwood door.
~
#EXIT
Direction north~
ToRoom    7906
#ENDEXIT

#EXIT
Direction east~
ToRoom    7913
Desc      You see a huge metal door. From it a foul stench emanates. The smell is the
most awful experience in your entire life. A thought seeps through this
terrible stench and into your mind : "Monsters", you feel BAD about opening
that door.
~
Keywords  steel metal huge~
Flags     isdoor closed pickproof~
#ENDEXIT

#EXIT
Direction south~
ToRoom    7901
Desc      The rest of the hall lies in that direction, and so does the exit.
~
#ENDEXIT

#EXIT
Direction west~
ToRoom    7905
Desc      The door has "SITTING ROOM" written on it. It is made from Aspenwood and
is beautifully carven with small elves as main issue of sculpture.
~
Keywords  aspen asp~
Flags     isdoor closed pickproof~
#ENDEXIT

Reset D 0 7904 1 1
Reset D 0 7904 3 1
#ENDROOM

#ROOM
Vnum     7905
Name     The Sitting room of Naris~
Sector   inside~
Flags    nomob indoors private~
Desc     You are standing in the middle of a really comfortable place. The walls are
decorated with paintings of smiling Kings and Queens. The most attractive
picture is one of a Prince in shiny armour. By one of the walls there is an
old arm-chair. The only exit is through the aspenwood door to the east.
~
#EXIT
Direction east~
ToRoom    7904
Desc      The door seems to be a very HEAVY door, carved completely from the trunk of
an Aspen tree.
~
Keywords  asp aspen door heavy~
Flags     isdoor closed pickproof~
#ENDEXIT

Reset D 0 7905 1 1
#EXDESC
ExDescKey    chair arm-chair~
ExDesc       This is truly a wonderful relic of the past. In it is a large cushion.
~
#ENDEXDESC

#ENDROOM

#ROOM
Vnum     7906
Name     The Kitchen of Naris~
Sector   inside~
Flags    nomob indoors private~
Desc     This must be the place of food and drink. You can hear the faint noise of 
rats feasting on meat and bread from somewhere undeterminable. The sound
makes you feel the agony of HUNGER. The only visible exit is south to the
Northern end of the hall. To the north there is a larder, and to the east
stands a fridge.
~
#EXIT
Direction north~
ToRoom    7907
Desc      It's dark in there,  but the sounds from there are unmistakably from rats.
~
Keywords  larder wooden cupboard~
Flags     isdoor closed pickproof~
#ENDEXIT

#EXIT
Direction east~
ToRoom    7908
Desc      You can see the fridge from here. In it are drinks all over.
~
Keywords  fridge~
Flags     isdoor closed pickproof~
#ENDEXIT

#EXIT
Direction south~
ToRoom    7904
Desc      You can see the northern end of the hall.
~
#ENDEXIT

Reset D 0 7906 1 1
Reset D 0 7906 0 1
#ENDROOM

#ROOM
Vnum     7907
Name     The Larder~
Sector   inside~
Flags    nomob indoors private~
Desc     You can see food all over. Among the heaps of food you notice HUGE rats
scuttering around, nibbling pieces off the heaps here and there.
~
#EXIT
Direction south~
ToRoom    7906
Keywords  door wooden larder exit~
Flags     isdoor closed pickproof~
#ENDEXIT

Reset D 0 7907 2 1
#ENDROOM

#ROOM
Vnum     7908
Name     The Fridge~
Sector   inside~
Flags    nomob indoors private~
Desc     This place is LOADED with drink, water and booze.
~
#EXIT
Direction west~
ToRoom    7906
Keywords  door exit fridge~
Flags     isdoor closed pickproof~
#ENDEXIT

Reset D 0 7908 3 1
Reset O 0 6013 10 7908
#ENDROOM

#ROOM
Vnum     7909
Name     On the stairs~
Sector   inside~
Flags    nomob indoors private~
Desc     You can see up and down the stairway. It seems to take forever, either going
up OR down. It's just a seemingly insurmountable climb, either way. Up is the 
bedroom of Redferne, and Down leads to the Southern end of the hall.
~
#EXIT
Direction up~
ToRoom    7911
#ENDEXIT

#EXIT
Direction down~
ToRoom    7901
#ENDEXIT

#ENDROOM

#ROOM
Vnum     7910
Name     The Treasure room~
Sector   inside~
Flags    nomob indoors private~
Desc     This place is gloomy.  The only visible exit is west, through the oak door.
~
#EXIT
Direction west~
ToRoom    7901
Desc      You see a tall oak door.
~
Keywords  door oak tall~
Flags     isdoor closed pickproof~
#ENDEXIT

Reset D 0 7910 3 1
Reset O 0 7910 1 7910
  Reset P 0 7298 1 7910
  Reset P 0 7208 6 7910
#ENDROOM

#ROOM
Vnum     7911
Name     Redferne's Bedroom~
Sector   inside~
Flags    nomob indoors private~
Desc     This is a snugly set bedroom with all the necessities for a romantic evening.
A large fireplace adorns the east wall, and sizzling away is the wood that
is ablaze within.  The bed is enormous, covering at least HALF of this great
room.  This stretches at least 300 square feet, so can you imagine the BED?
A large staircase leads down to the hall.  A couple of doors open up to a
broad balcony to the south. 
~
#EXIT
Direction south~
ToRoom    7912
Desc      The sun seems to shine out there, warmly and comforting.
~
Keywords  doors~
Flags     isdoor closed~
#ENDEXIT

#EXIT
Direction down~
ToRoom    7909
#ENDEXIT

Reset D 0 7911 2 1
#ENDROOM

#ROOM
Vnum     7912
Name     The Balcony of Redferne's Residence~
Sector   inside~
Flags    nomob private~
Desc     You see a splendid view of most of this world. The valleys stretch as
far as the eye reaches to the south from here. Down below you can see
the entrance of this magnificent building. To the north are the doors
to Redferne's Bedroom.
~
#EXIT
Direction north~
ToRoom    7911
Desc      You see the comfortable bedroom of Naris, the mansion of Redferne the Greater
God.
~
Keywords  doors~
Flags     isdoor closed pickproof~
#ENDEXIT

Reset D 0 7912 0 1
#ENDROOM

#ROOM
Vnum     7913
Name     The Monster Pen~
Sector   inside~
Flags    indoors~
Desc     This looks like the cage in which a large carnivore is being kept. Judging by
the state the walls are in, this could very well be a large AGGRESSIVE animal. 
This makes you pretty insecure, this stating your feeling about the place, 
quite mildly.
~
#EXIT
Direction west~
ToRoom    7904
Desc      This looks like the only exit from here.
~
Keywords  door cage metal pen~
Flags     isdoor closed pickproof~
#ENDEXIT

Reset D 0 7913 3 1
#ENDROOM

#ROOM
Vnum     7914
Name     On the Huge Chain~
Sector   mountain~
Flags    nomob~
Desc     This place has quite a good view over Darkhaven. Your possibilities of movement
seems to extend only to up and down. Unless you want to let go of the secure
and seemingly unmoveable chain, then those are the directions you should take
from here.
~
#EXIT
Direction up~
ToRoom    7916
Desc      Up the way you see the chain disappearing into the clouds above.
~
#ENDEXIT

#EXIT
Direction down~
ToRoom    21291
Desc      Down below you see the huge chain anchored to the ground in the Road Crossing.
~
#ENDEXIT

#ENDROOM

#ROOM
Vnum     7916
Name     On the Great Chain of Naris~
Sector   mountain~
Flags    nomob~
Desc     You are approximately on the middle of the treacherous Chain. One false step
and death will come and collect you quickly. The chain leads upwards and down.
No way are you going to descend now ... you've only just begun your climb.
Besides it could cost you your life.  Look down and you'll see why.
~
#EXIT
Direction up~
ToRoom    7918
Desc      There seems to be only one way from here and that's up.
~
#ENDEXIT

#ENDROOM

#ROOM
Vnum     7918
Name     The Mighty Chain of Naris~
Sector   mountain~
Flags    nomob~
Desc     This place offers a splendid view of the WHOLE COUNTRY SIDE!!! A magnificent
light shines upon you and the way up through the clouds is opened. Up there
you can now see the Mansion of Naris, Residence of the Greater God Redferne.
The chain beneath you seems to evaporate in the mustering clouds that
surround you by now.
~
#EXIT
Direction up~
ToRoom    7900
Desc      You see the sunny top of the clouds. Beyond these, the Mansion towers before
your eyes. Beautiful!!
~
#ENDEXIT

#ENDROOM

#ENDAREA
