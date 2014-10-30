#FUSSAREA
#AREADATA
Version      1
Name         Spectral Gate~
Author       Brittany~
WeatherX     0
WeatherY     0
Ranges       0 60 0 60
Economy      0 45009000
ResetMsg     A bell rings far above as another passes beyond the Spectral Gates...~
#ENDAREADATA

#MOBILE
Vnum       100
Keywords   Samylla receptionist woman~
Short      Samylla~
Long       The Realms of Despair hostess is here to greet you.
~
Desc       This is a woman, that is obvious, yet she does not quite fit what you
are used to a female looking like.  You can not place what it is at
first.  As you look her over, you see she is tall and quite slim, and
her figure is one to attract eyes.  As you glance again over her face
you notice her face is quite white, almost brilliantly so.  Her eyes
are black as coal, no pupil distinguishable from the iris.  She smiles
as she watches your mind turn to take in her appearance, and that is
the last piece of missing information.  You notice the small fanged
eyeteeth, and realize this is a vampire standing before you.  What
world have you stepped into?
~
Race       human~
Class      vampire~
Position   standing~
DefPos     standing~
Gender     neuter~
Actflags   npc sentinel~
Affected   detect_invis detect_hidden sanctuary infrared truesight~
Stats1     0 50 0 0 0 0
Stats2     0 0 0
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    13 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common~
Speaking   common~
#MUDPROG
Progtype  act_prog~
Arglist   p arrives from above.~
Comlist   smile $n
say Welcome to the Realms of Despair reception chamber, $n.
say You have yet to enter the actual game, but will soon...
mpechoat $n Please take the time to read everything you see along the way.
mpechoat $n - use Newbiechat to speak to the Immortals for assistance:
mpechoat $n - type "new <message>" to use the channel.
mpechoat $n Please, look around -- just type 'look <object>'.  (look bust)
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p arrives from the north.~
Comlist   say Welcome back, $n.
say Perhaps you come to look around some more?
mpechoat $n I hope you viewed the various tapestries at the top of the stairs.
mpechoat $n They give you a brief look in all directions of our Realms.
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p flys in from above.~
Comlist   smile $n
say Welcome to the Realms of Despair reception chamber, $n.
say You have yet to enter the actual game, but will soon...
mpechoat $n Please take the time to read everything you see along the way.
mpechoat $n - use Newbiechat to speak to the Immortals for assistance:
mpechoat $n - type "new <message>" to use the channel.
mpechoat $n Please, look around -- just type 'look <object>'.  (look bust)
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p flys in from the north.~
Comlist   say Welcome back, $n.
say Perhaps you come to look around some more?
mpechoat $n I hope you viewed the various tapestries at the top of the stairs.
mpechoat $n They give you a brief look in all directions of our Realms.
~
#ENDPROG

#MUDPROG
Progtype  all_greet_prog~
Arglist   100~
Comlist   if race($n) == Halfling
smile $n
say Welcome to the Realms of Despair reception chamber, $n.
say You have yet to enter the actual game, but will soon...
mpechoat $n Please take the time to read everything you see along the way.
mpechoat $n - use Newbiechat to speak to the Immortals for assistance:
mpechoat $n - type "new <message>" to use the channel.
mpechoat $n Please, look around -- just type 'look <object>'.  (look bust)
mpechoat $n ... I hope you viewed the various tapestries up the staircase.
mpechoat $n ... They give you a brief look in all directions of our Realms.
else
endif
~
#ENDPROG

#ENDMOBILE

#MOBILE
Vnum       101
Keywords   mob mobi act~
Short      Act Mob~
Long       An Act Mob is here, but can not be seen by Pre-Auths.
~
Race       human~
Class      warrior~
Position   standing~
DefPos     standing~
Gender     neuter~
Actflags   npc sentinel secretive~
Affected   invisible detect_invis infrared hide~
Stats1     0 1 0 0 0 0
Stats2     0 0 0
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    13 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common~
Speaking   common~
#MUDPROG
Progtype  act_prog~
Arglist   p has entered the game.~
Comlist   mpoload 10311 1
give eye 0.$n
mpforce 0.$n wear eye
mpoload 123 1
give burlap 0.$n
mea $n A burlap sack is delivered into your hands, type LOOK SACK to use.
mea $n Familiar, yet somehow different, your being has changed...
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p arrives from below.~
Comlist   mpechoat $n Your eyes are quickly drawn to the tapestries.
mpforce 0.$n look tapestries
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p flys in from below.~
Comlist   mpechoat $n Your eyes are quickly drawn to the tapestries.
mpforce 0.$n look tapestries
~
#ENDPROG

#MUDPROG
Progtype  all_greet_prog~
Arglist   100~
Comlist   if race($n) == Halfling
mpechoat $n Your eyes are quickly drawn to the tapestries.
mpforce 0.$n look tapestries
endif
~
#ENDPROG

#ENDMOBILE

#MOBILE
Vnum       102
Keywords   Xouwasi dwarven dwarf man teacher~
Short      Xouwasi~
Long       The statistics teacher sits here, guiding you towards your future.
~
Desc       This is a man who eyes speak volumes, mostly of knowledge and friendship.
Most dwarves are distrustful of strangers, but this one seems to thrive
on greeting and teaching the new travelers into these Realms.  His face
is covered in wrinkles, both from the sun and smiling.  His chin sprouts
a proud goatee, and his clothes are bright to the point of being gaudy.
His boots are a black leather, polished to a shine.  Overall, he makes
quite a striking picture.
~
Race       dwarf~
Class      druid~
Position   standing~
DefPos     standing~
Gender     neuter~
Actflags   npc sentinel prototype~
Affected   detect_invis detect_hidden sanctuary infrared truesight~
Stats1     0 1 0 0 0 0
Stats2     0 0 0
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    13 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common dwarven~
Speaking   common dwarven~
#MUDPROG
Progtype  act_prog~
Arglist   p from the south.~
Comlist   bow $n
  say You appear to be a visitor, $n.
  mpechoat $n - Type "look <item>" or "look <name>" to see things here.
  mpechoat $n - Type 'eq' to check your equipment (items you are wearing).
  mpechoat $n ... to see your unused belongings, type 'inventory' or 'i'
  mpechoat $n ... to display your statistics sheet, type 'score' or 'ol'
  say Type 'look tree'...
~
#ENDPROG

#MUDPROG
Progtype  all_greet_prog~
Arglist   100~
Comlist   if race($n) == halfling
bow $n
say You appear to be a visitor, $n.
mpechoat $n - Type "look <item>" or "look <name>" to see things here.
mpechoat $n - Type 'eq' to check your equipment (items you are wearing).
mpechoat $n ... to see your unused belongings, type 'inventory' or 'i'
mpechoat $n ... to display your statistics sheet, type 'score' or 'ol'
say Type 'look tree'...
endif
~
#ENDPROG

#ENDMOBILE

#MOBILE
Vnum       103
Keywords   serpent snake~
Short      a serpent~
Long       A serpent springs from a coil deep within the grass and strikes!
~
Desc       A scaly, slimy serpent, so black it shines.  Its beady eyes watch your every
movement, looking for an opening to strike!
~
Race       snake~
Class      vampire~
Position   standing~
DefPos     standing~
Gender     neuter~
Actflags   npc sentinel aggressive~
Affected   detect_invis infrared~
Stats1     0 3 0 75 500 0
Stats2     1 1 25
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    13 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common dwarven ogre orcish trollese reptile dragon halfling clan~
Speaking   common dwarven ogre orcish trollese reptile dragon halfling clan~
Bodyparts  head guts eye tail~
#MUDPROG
Progtype  fight_prog~
Arglist   50~
Comlist   mpechoat $n $I sinks its fangs painfully into your flesh!
mpechoaround $n $I sinks its fangs into $n's flesh!
~
#ENDPROG

#MUDPROG
Progtype  death_prog~
Arglist   100~
Comlist   mpoload 108 1
mpechoat $n A piece of meat is torn from the serpent as it dies.
~
#ENDPROG

#ENDMOBILE

#MOBILE
Vnum       104
Keywords   dragon hatchling~
Short      the dragon hatchling~
Long       A dragon hatchling peers at you, huge and darkly menacing.
~
Desc       The head of this dragon is identical to the one you viewed earlier, the
difference being that this one is (thankfully) only about tenth the size.
This dragon, though a baby, looks like a tough battle indeed.
~
Race       dragon~
Class      warrior~
Position   standing~
DefPos     standing~
Gender     neuter~
Actflags   npc sentinel~
Stats1     0 2 0 75 1000 0
Stats2     20 0 0
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    13 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common elvish orcish trollese insectoid spiritual magical clan gnome~
Speaking   common elvish orcish trollese insectoid spiritual magical clan gnome~
Bodyparts  head arms legs feet tail~
#MUDPROG
Progtype  rand_prog~
Arglist   7~
Comlist   mpe The dragon gives a deep rumbling growl, just to let you know he is here.
sac corpse
sac shield
sac bucket
~
#ENDPROG

#MUDPROG
Progtype  death_prog~
Arglist   100~
Comlist   mpoload 112 1
mpecho $I snarls as thick blood fills its throat and it collapses.
mpecho A loud crack can be heard in the shadows against the southern wall.
mpecho A new dragon hatchling emerges from inside a hidden egg.
mpmload 104
mpechoat $n Remember...
mpechoat $n You cannot advance beyond level 1 until you have passed the
mpechoat $n runed gate and entered the actual game.
~
#ENDPROG

#ENDMOBILE

#MOBILE
Vnum       105
Keywords   mob mobii act~
Short      Act Mob~
Long       An Act Mob is here, but can not be seen by Pre-Auths.
~
Race       human~
Class      warrior~
Position   standing~
DefPos     standing~
Gender     neuter~
Actflags   npc sentinel secretive prototype~
Affected   invisible detect_invis infrared hide~
Stats1     0 1 0 0 0 0
Stats2     0 0 0
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    18 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common~
Speaking   common~
#MUDPROG
Progtype  act_prog~
Arglist   P arrives from the west.~
Comlist   mpoload 113 1
mpoload 114 1
mpat 120 give armour Quercusi
mpat 120 give golden Quercusi
mpoload 116 1
mpoload 117 1
mpat 123 mppurge
mpat 123 drop box
mpat 123 put token box
mpat 123 close box
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p flys in from the west.~
Comlist   mpoload 113 1
mpoload 114 1
mpat 120 give armour Quercusi
mpat 120 give golden Quercusi
mpoload 116 1
mpoload 117 1
mpat 123 mppurge
mpat 123 drop box
mpat 123 put token box
mpat 123 close box
~
#ENDPROG

#MUDPROG
Progtype  all_greet_prog~
Arglist   100~
Comlist   if race($n) == Halfling
mpoload 113 1
mpoload 114 1
mpat 120 give armour Quercusi
mpat 120 give golden Quercusi
mpoload 116 1
mpoload 117 1
mpat 123 mppurge
mpat 123 drop box
mpat 123 put token box
mpat 123 close box
else
endif
~
#ENDPROG

#MUDPROG
Progtype  rand_prog~
Arglist   2~
Comlist   mpat 199 drop all
~
#ENDPROG

#ENDMOBILE

#MOBILE
Vnum       106
Keywords   Quercusi Q man~
Short      Quercusi~
Long       A wise man is here, rocking himself on a beautiful chair.
~
Desc       This man looks like he has been in the Realms for hundreds of years.
His kind and friendly face is wrinkled with time, yet he appears to
be in top physical shape, and his eyes sparkle with wit and wisdom.
~
Race       human~
Class      warrior~
Position   standing~
DefPos     standing~
Gender     male~
Actflags   npc sentinel prototype~
Affected   detect_invis sanctuary infrared truesight~
Stats1     0 50 0 0 0 0
Stats2     0 0 0
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    25 13 13 25 13 13 13
Saves      0 0 0 0 0
Speaks     common~
Speaking   common~
#MUDPROG
Progtype  all_greet_prog~
Arglist   100~
Comlist   smile $n
say Hello, $n, welcome to my home.
mpechoat $n Do you have the following items?
mpechoat $n ... a weapon from a chest in a tree.
mpechoat $n ... a Shield of Scales from the dragon hatchling in the cave.
mpechoat $n ... a ring from the end of your chosen path.
mpechoat $n ... also a feather, a bucket and the meat of a serpent.
mpechoat $n If you missed any of these and would like to retrieve them, or
if ispkill($n)
  mpechoat $n if you would like to return to choose the Peaceful path, simply
else
  mpechoat $n if you would like to return to choose the Deadly path, simply
endif
mpechoat $n give me your ring (type 'give ring man').
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p leaves north.~
Comlist   mpat 119 mpecho $I leans out the door...
mpat 119 give key $n
mpat 119 give armour $n
mpat 119 mpechoat $n Good luck in your adventures, $n...
close door
~
#ENDPROG

#MUDPROG
Progtype  give_prog~
Arglist   ring light lightring~
Comlist   mpechoat $n I shall now return you to Xouwasi at the tree.
mpechoaround $n $n disappears in a magical flash.
mptransfer 0.$n 103
mpat 103 mpforce 0.$n south
mpjunk lightring
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p flys north.~
Comlist   mpat 119 mpecho $I leans out the door...
mpat 119 give key $n
mpat 119 give armour $n
mpat 119 mpechoat $n Good luck in your adventures, $n...
close door
~
#ENDPROG

#MUDPROG
Progtype  give_prog~
Arglist   ring shadow shadowring~
Comlist   mpechoat $n I shall now return you to Xouwasi at the tree.
mpechoaround $n $n disappears in a magical flash.
mptransfer 0.$n 103
mpat 103 mpforce 0.$n south
mpjunk shadowring
~
#ENDPROG

#MUDPROG
Progtype  rand_prog~
Arglist   2~
Comlist   mpat 199 drop all
mpat 199 get armour
mpat 199 get armour
mpat 199 get key
mpat 199 get key
mpat 199 get key
mpat 199 get key
mpat 199 get armour
mpat 199 get armour
mpat 199 mppurge
~
#ENDPROG

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   if race($n) == Halfling
give key $n
give armour $n
mpechoat $n Good luck in your adventures, $n...
else
endif
~
#ENDPROG

#MUDPROG
Progtype  act_prog~
Arglist   p floats north.~
Comlist   mpat 119 mpecho $I leans out the door...
mpat 119 give key $n
mpat 119 give armour $n
mpat 119 mpechoat $n Good luck in your adventures, $n...
close door
~
#ENDPROG

#ENDMOBILE

#MOBILE
Vnum       107
Keywords   mob mobiii act~
Short      Act Mob~
Long       An Act Mob is here, but can not be seen by Pre-Auths.
~
Race       human~
Class      warrior~
Position   standing~
DefPos     standing~
Gender     neuter~
Actflags   npc sentinel secretive~
Affected   invisible detect_invis infrared hide~
Stats1     0 1 0 0 0 0
Stats2     0 0 0
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    13 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common~
Speaking   common~
#MUDPROG
Progtype  act_prog~
Arglist   p pulls a rope.~
Comlist   mpechoat $n High overhead you hear a large bell thunder in response.
mpechoaround $n High overhead, a large bell thunders in response.
mpechoat $n If nothing happens, type LOOK ROPE, and pull again in 30 seconds.
mpapplyb $n
push rope
~
#ENDPROG

#ENDMOBILE

#MOBILE
Vnum       199
Keywords   spacermob~
Short      a newly created spacermob~
Long       Some god abandoned a newly created spacermob here.
~
Race       human~
Class      warrior~
Position   standing~
DefPos     standing~
Gender     neuter~
Actflags   npc prototype~
Stats1     0 1 0 0 0 0
Stats2     0 0 0
Stats3     0 0 0
Stats4     0 0 0 0 0
Attribs    13 13 13 13 13 13 13
Saves      0 0 0 0 0
Speaks     common~
Speaking   common~
#ENDMOBILE

#OBJECT
Vnum     100
Keywords candelabra~
Type     light~
Short    a candelabra with six black candles~
Long     Six black candles burn brightly in this brass candelabra~
WFlags   take~
Values   0 0 0 0 0 0
Stats    100 10 1 0 0
#ENDOBJECT

#OBJECT
Vnum     101
Keywords spring~
Type     fountain~
Short    a magical spring~
Long     A magical spring sprinkles water from the corner of the room.~
Flags    magic~
Values   100000 100000 0 0 0 0
Stats    5000 10 1 0 0
#ENDOBJECT

#OBJECT
Vnum     104
Keywords chest~
Type     container~
Short    a weapon's chest~
Long     A weapons chest rests against the northern wall.~
Values   1000 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    chest~
ExDesc       This chest is made from a thick carved oak, it is sturdy but not locked.
You feel you must examine it closer to see what it might contain.
~
#ENDEXDESC

#MUDPROG
Progtype  exa_prog~
Arglist   100~
Comlist   if class ($n) == Mage
or class ($n) == Thief
or class ($n) == Vampire
or class ($n) == Augurer
  mpoload 125 1
  put 'menacing granitic blade' chest
  mpforce $n get granitic chest
  mpforce $n wield granitic
else
if class ($n) == Cleric
or class ($n) == Druid
  mpoload 126 1
  put 'adamantium mace' chest
  mpforce $n get adamantium chest
  mpforce $n wield mace
else
if class ($n) == Warrior
or class ($n) == Ranger
or class ($n) == Paladin
  mpoload 127 1
  put 'heavy broadsword broad sword' chest
  mpforce $n get broadsword chest
  mpforce $n wield broadsword
endif
endif
endif
~
#ENDPROG

#ENDOBJECT

#OBJECT
Vnum     105
Keywords Weapon~
Type     weapon~
Short    A finely honed weapon lies here, waiting to be carried into battle.~
Long     You see a finely honed dagger here.~
Flags    magic~
WFlags   wield~
Values   12 1 6 11 0 0
Stats    2 0 0 0 0
Affect       -1 -1 1 18 0
Affect       -1 -1 2 19 0
#EXDESC
ExDescKey    dagger~
ExDesc       This weapon looks to be of decent quality and should serve you well in your
upcoming battles.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     106
Keywords ring light lightring~
Type     armor~
Short    a ring of light~
Long     A small circle of light lies here.~
Flags    magic~
WFlags   take finger~
Values   5 5 0 0 0 0
Stats    2 0 0 0 0
Affect       -1 -1 5 13 0
Affect       -1 -1 1 2 0
#EXDESC
ExDescKey    ring light~
ExDesc       This ring is a symbol of your chosen peaceful nature.  It will aid you
until you reach level 10.  May it always find you in good health.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     107
Keywords ring shadow shadowring~
Type     armor~
Short    a ring of shadows~
Long     A small circle of shadows lies here.~
Flags    magic~
WFlags   take finger~
Values   5 5 0 0 0 0
Stats    2 0 0 0 0
Affect       -1 -1 5 13 0
Affect       -1 -1 1 2 0
Affect       -1 -1 1 18 0
#EXDESC
ExDescKey    ring shadows~
ExDesc       This ring is a symbol of your chosen deadly nature.  It will aid you until
you reach level 10.  May it always find you in good health and fast reflex.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     108
Keywords snakemeat snake meat~
Type     food~
Short    a piece of meat~
Long     A piece of meat from a snake~
Flags    organic~
WFlags   take~
Values   25 0 0 0 0 0
Stats    1 0 0 0 0
#ENDOBJECT

#OBJECT
Vnum     109
Keywords feather purple~
Type     wand~
Short    a purple feather~
Long     A bright purple feather lies here.~
Flags    magic organic deathrot~
WFlags   take hold~
Values   3 2 2 -1 0 0
Stats    4 0 0 0 0
Spells   'magic missile'
#EXDESC
ExDescKey    feather purple~
ExDesc       Rather heavy for a feather, it looks to actually be a wand of sorts.  Perhaps
you should hold it and zap <target>.  Because this feather contains an attack
spell, this is NOT something you would want to cast on another player, unless
you are both pkill characters.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     110
Keywords bucket water tin~
Type     drinkcon~
Short    a tin bucket~
Long     A bucket full of water sits here, waiting for someone to carry it away.~
Flags    metal~
WFlags   take~
Values   20 10 0 0 0 0
Stats    4 0 0 0 0
#EXDESC
ExDescKey    tin bucket~
ExDesc       This is a sturdy bucket made to store drinking water.  You can 'drink tin'
or 'drink water'.  You may refill the bucket at fountains or streams by
typing 'fill bucket'.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     111
Keywords pond~
Type     fountain~
Short    the pond~
Long     A shallow pond of crystal clear water~
Flags    prototype~
Values   100000 1000000 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    pond~
ExDesc       The waters are shallow and clear, and surprisingly cold.  Several varieties
of fish swim beneath the surface, but you can best make out a pool of lapis-
toned fish.  You have no doubt you can drink from this water source, and may
wish to fill your water bucket before continuing your journey.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     112
Keywords shield dragonscales scales~
Type     armor~
Short    a shield of dragonscales~
Long     A shield of scales lie here, marred with deep burns.~
Flags    dark organic~
WFlags   take shield~
Values   4 3 0 0 0 0
Stats    5 0 0 0 0
Affect       -1 -1 1 18 0
Affect       -1 -1 1 19 0
#EXDESC
ExDescKey    shield dragonscales scales~
ExDesc       This shield shall protect you well in your travels.  Wear it in good health.
~
#ENDEXDESC

#MUDPROG
Progtype  drop_prog~
Arglist   100~
Comlist   mpforce $n sac $o
~
#ENDPROG

#ENDOBJECT

#OBJECT
Vnum     113
Keywords armour breast plate~
Type     armor~
Short    a thick plate of breast armour~
Long     A breast plate lies here, unused and forgotten.~
WFlags   take body~
Values   4 7 0 0 0 0
Stats    10 0 0 0 0
Affect       -1 -1 -3 17 0
#ENDOBJECT

#OBJECT
Vnum     114
Keywords key golden~
Type     key~
Short    a huge golden key~
Long     A large golden key lies here, it looks to open something important.~
WFlags   take~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    golden key~
ExDesc       This key looks to open something very important. Maybe the gate to
your future. To use a key, just UNLOCK <door>. Like UNLOCK GATE.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     115
Keywords rope~
Type     lever~
Short    a rope~
Long     A rope hangs from high above, waiting to be pulled.~
Flags    organic~
Values   29 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    rope~
ExDesc       If you are ready to pass beyond the Spectral Gates, you need only pull this
rope to notify the Immortals that you are ready to enter the game!
 
If your name has been approved, you will be sent into the actual game.
 
If you receive a message stating your name has been denied by the Immortals,
you must choose a NEW, original medieval name.  To do this, you type:
 
"name <newname>"                 For example:  name Tsythia
 
Then pull the rope to announce your new name to the Immortals.  Give them
about 30 seconds to approve or deny the new name.  If you have not gotten
a response, pull the rope again.
 
The rope is on a 30 second timer, meaning you can pull it as often as you
wish, but the Immortals will only be notified once every 30 seconds.  The
Immortals are often busy and may not see you the first time around, so
please be patient - one will be with you as quickly as possible.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     116
Keywords box~
Type     container~
Short    the box~
Long     A blackened wooden box sets in the shadows atop a small table.~
WFlags   take~
Values   11111 1 0 0 0 0
Stats    1100 0 0 0 0
#ENDOBJECT

#OBJECT
Vnum     117
Keywords earring token~
Type     armor~
Short    a token of a Goddesses affection~
Long     An earring lies here, tarnished and forgotten.~
Flags    glow magic~
WFlags   take ears~
Values   4 2 0 0 0 0
Stats    5 0 0 0 0
Affect       -1 -1 2097152 26 0
#MUDPROG
Progtype  wear_prog~
Arglist   100~
Comlist   if level($n) < 16
mpechoat $n As you wear this earring you feel your feet rise off the ground.
mpechoar $n As $n clasps $O on $s ear, $e begins to float above the ground.
else
mpechoat $n The clasp on the earring will no longer stay closed on your ear.
mpechoaround $n $n struggles with $O but cannot get it to stay on $s ear.
mpforce $n remove token
endif
~
#ENDPROG

#ENDOBJECT

#OBJECT
Vnum     119
Keywords gownq~
Type     armor~
Short    a flowing white gown~
Long     Some god dropped a newly created gown here.~
WFlags   take body~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
Affect       -1 -1 5 1 0
Affect       -1 -1 5 2 0
#ENDOBJECT

#OBJECT
Vnum     120
Keywords peaceful sign~
Type     furniture~
Short    the sign of the peaceful path~
Long     A large sign floats before the trees alongside the pathway.~
Flags    glow~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    sign~
ExDesc       The Gods hope your stay on the game will be a pleasant one.  However,
to insure the enjoyment of others, there are two major Rules which you
should be made aware of:
 
1) Thou shalt neither attack nor steal from a player who has chosen
   Non-Pkill Status.  Thou shalt not engage in pkill as long as you
   remain peaceful.
2) Thou shalt not spam, use profanity, harass, nor make either lewd
   or racial comments on any channel, nor to any other player.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     121
Keywords deadly sign~
Type     furniture~
Short    the sign of the deadly path~
Long     A large sign floating alongside the pathway demands your attention.~
Flags    dark~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    sign~
ExDesc       The Gods hope your stay on the game will be a pleasant one.  However,
to insure the enjoyment of others, there are two major rules which you
should be made aware of:
 
1) Thou shalt neither attack nor steal from a player who has chosen
   Non-Pkill Status.  Thou shalt only attack other pkillers within
   five levels of your own.
2) Thou shalt not spam, use profanity, harass, nor make either lewd
   or racial comments on any channel, nor to any other player.

Also, often review HELP RULES PKILL for rule changes that affect you.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     123
Keywords sack burlap~
Type     container~
Short    a large burlap sack~
Long     A sack made from a brown burlap lies here.~
WFlags   take hold~
Values   75 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    sack burlap~
ExDesc       To use this sack, type:  examine sack   OR   exam sack
To get items from this sack, type:  get <item name> sack
To put an item in this sack, type:  put <item name> sack
 
You can also use get all or put all.  See help container for more information.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     124
Keywords stairs~
Type     trash~
Short    a staircase~
Long     A spiral staircase leads down from center of the room.~
Flags    prototype~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    stairs staircase~
ExDesc       These staircase looks sturdy.  It descends to another room from the
center of the room.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     125
Keywords menacing granitic blade~
Type     weapon~
Short    a menacing granitic blade~
Long     A blade of metallic stone protrudes from a leather-wrapped hilt.~
Flags    dark magic~
WFlags   take wield~
Values   12 1 6 11 0 0
Stats    2 0 0 0 0
Affect       -1 -1 2 18 0
Affect       -1 -1 1 19 0
#EXDESC
ExDescKey    menacing granitic blade~
ExDesc       Of rough but usable quality, it should serve you well in your early years.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     126
Keywords adamantium mace~
Type     weapon~
Short    a gleaming adamantium mace~
Long     A mace of brightly gleaming metal hums with inner power.~
Flags    hum magic bless metal~
WFlags   take wield~
Values   12 0 0 7 0 0
Stats    2 0 0 0 0
Affect       -1 -1 2 18 0
Affect       -1 -1 1 19 0
#EXDESC
ExDescKey    adamantium mace~
ExDesc       Forged of a gleaming metal, this mace will serve you well in your early years.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     127
Keywords heavy broadsword broad sword~
Type     weapon~
Short    a heavy, iron-forged broadsword~
Long     Light glints from the blade of a heavy, iron-forged broadsword.~
Flags    magic metal~
WFlags   take wield~
Values   12 0 0 3 0 0
Stats    2 0 0 0 0
Affect       -1 -1 2 18 0
Affect       -1 -1 1 19 0
#EXDESC
ExDescKey    broadsword broad sword~
ExDesc       Made of a heavy iron alloy, it will serve you well in your early years.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     190
Keywords prog cont~
Type     container~
Short    a prog container~
Long     A container set up with the progs to make useless for non-guilded players.~
Flags    prototype~
WFlags   take hold~
Values   300 0 0 0 0 0
Stats    1 0 0 0 0
#ENDOBJECT

#OBJECT
Vnum     191
Keywords fountain~
Type     fountain~
Short    a fountain~
Long     A fountain of fresh water sits in the corner.~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#ENDOBJECT

#OBJECT
Vnum     192
Keywords plaque~
Type     furniture~
Short    a plaque~
Long     To the right of the path, a plaque is imbedded in a boulder.  (look plaque)~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    boulder~
ExDesc       This is an average boulder, grey and craggy.  It appears to be here
merely to frame the plaque.  Type LOOK PLAQUE to read what is says.
~
#ENDEXDESC

#EXDESC
ExDescKey    golden plaque~
ExDesc       This is a small area, about 25 rooms total.  If you are new to this type of
game, you should explore these rooms, look at all you you can and collect
your equipment.  You will not be authorized until you pull the rope in the
final room.
 
You can NOT die nor level in this area.  This gives you the chance to test
your character, view your stats, review your SLIST, and read our help files.
 
Here your eyes are your best weapon.  Look at everything!  You can look at
items found in the room descriptions to see additional information and
instructions.  Also be sure to look at actual items and mobiles <creatures
of the game world> that are found in the rooms.
 
Should you have any questions or comments, please feel free to talk to an
Immortal over newbiechat or via tells.  Type WHO IMM to see which Immortals
are currently available.  Also, type HELP NEWBIE for detailed help created
for players new to the game.

Happy Adventuring! -- The Administrators
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     193
Keywords golden plaque~
Type     furniture~
Short    a plaque~
Long     Attached to the banister is an inscribed golden plaque.~
Flags    prototype~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    golden plaque banister~
ExDesc       Welcome, Adventurers, to the Spectral Gates...
All new characters enter the game in this area prior to venturing to
the Darkhaven Academy and Realms beyond.  This is a small area, about
25 rooms total.  Your task is to explore these rooms, look at all you
you can and collect your equipment.  You will not be authorized until
you pull the rope in the final room. 
 
You can NOT die nor level in this area.  This gives you the chance to
test your character, view your stats, review your SLIST and read our
help files.
 
You can look at items found in the room descriptions to see additional
information.  For example, "look tapestries" will tell you things about
this room and the game not obvious at first glance.  Also be sure to
look at actual items and mobiles <creatures in the game> that are in
the room.  Here your eyes are your best weapon.
 
Happy Adventuring! -- Brittany Seccunda-DethBlayde, Godmother
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     194
Keywords thedoor~
Type     furniture~
Short    the opening~
Long     An opening in the tree, leading back to the path.~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#ENDOBJECT

#OBJECT
Vnum     196
Keywords spiral staircase~
Type     furniture~
Short    a staircase~
Long     A spiral staircase leads down from center of the room, with an inscribed

golden plaque attached to its banister.  [look plaque]~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    staircase spiral~
ExDesc       This staircase is done up completely in wrought iron.  Strange mythical
creatures are depicted along the railings as the stairway spirals down
through the floor.  To descend, just type DOWN or D.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     197
Keywords thingy~
Type     furniture~
Short    the gate~
Long     A large, golden gate gives passage into the game.~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#ENDOBJECT

#OBJECT
Vnum     198
Keywords cabin~
Type     furniture~
Short    the cabin~
Long     A cabin of logs has been built into the hillside south of here.~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    cabin~
ExDesc       Disappearing into the hillside, only the very front of the cabin can be
seen.  The building looks old, yet well kept, and the door stands ajar.
~
#ENDEXDESC

#ENDOBJECT

#OBJECT
Vnum     199
Keywords tree~
Type     furniture~
Short    the tree~
Long     Towering beside the pathway is an enormous tree, a large opening at its base.~
Values   0 0 0 0 0 0
Stats    1 0 0 0 0
#EXDESC
ExDescKey    tree opening~
ExDesc       A truly ancient tree, with a man-made opening large enough for you to enter.
HINT:  Type 'opening' or 'leave opening' to enter and leave the tree.
~
#ENDEXDESC

#ENDOBJECT

#ROOM
Vnum     100
Name     Ominous Tapestries~
Sector   inside~
Flags    nomob indoors safe norecall nosummon noastral~
Desc     You appear somehow within an arcane chamber.  Huge hanging tapestries,
each intricately detailed in vibrant colors, cover the featureless walls
of your octagonal surroundings.  Wide square bands of white marble serve
to blur the sharp edges of the walls, giving the room an almost circular
appearance.  Gazing about your surroundings, your eye is drawn upward by
a much larger round tapestry hanging directly overhead, and you struggle
with your balance as you examine the fabric's images.  The growing sense
of wonder which pulls at you is tinged with a dose of trepidation.  The
views about you, while wondrous, are nevertheless quite ominous; these
images are of a truly darkened land.
~
#EXIT
Direction down~
ToRoom    101
Desc      You see a spiral staircase leading to a reception room.
~
#ENDEXIT

Reset M 0 101 1 100
Reset O 0 124 1 100
#EXDESC
ExDescKey    tapestry~
ExDesc       As you peer closer at this tapestry, you find it is not a tapestry at all,
but a window which looks out upon an strange and darkened land.  You begin
to examine each separately, enjoying the view each affords.  Nine separate
windows are here, one above you and one in each of the eight directions.
LOOK North, East, South, West, Northeast, Southeast, Southwest, Northwest.
Perhaps you should have a brief peek at this new world.
~
#ENDEXDESC

#EXDESC
ExDescKey    northwest nw~
ExDesc       To the north and west appears to be a valley and forest, probably home
to the Elven population by the small structures there.  You can also
make out the beginning of an ocean and a few ships at port.
~
#ENDEXDESC

#EXDESC
ExDescKey    southwest sw~
ExDesc       The forest changes in this direction, the trees create a canopy over
quite a large area.  South of the forest is a huge swamp area.
~
#ENDEXDESC

#EXDESC
ExDescKey    south s~
ExDesc       You can see a large, busy dump out this direction, and a large opening to
a subterranean area there.  You can see a dark path leading south from this
dump.  Altogether, it does not look like the most appealing of places for
you to visit.
~
#ENDEXDESC

#EXDESC
ExDescKey    east e~
ExDesc       This side looks very busy.  There is a large crossroads leading in many
directions.  You see a large grove and castle far to the east.  West of
the crossroads is a city gate.  You can see blackened mountains, an open
abandoned looking town and caves that appear to be made for dwarves.
~
#ENDEXDESC

#EXDESC
ExDescKey    up u~
ExDesc       Far above you can make out several few dragons flying though the grey
skies, and even a few Harpies are out stretching their wings.  Most
seem to not see this building, nor you watching their passage.
~
#ENDEXDESC

#EXDESC
ExDescKey    tapestrie~
ExDesc       As you peer closer at this tapestry, you find it is not a tapestry at all,
but a window which looks out upon an strange and darkened land.  You begin
to examine each separately, enjoying the view each affords.  Nine separate
windows are here, one above you and one in each of the eight directions.
LOOK North, East, South, West, Northeast, Southeast, Southwest, Northwest.
Perhaps you should have a brief peek at this new world.
~
#ENDEXDESC

#EXDESC
ExDescKey    tapestries~
ExDesc       As you peer closer at this tapestry, you find it is not a tapestry at all,
but a window which looks out upon an strange and darkened land.  You begin
to examine each separately, enjoying the view each affords.  Nine separate
windows are here, one above you and one in each of the eight directions.
LOOK North, East, South, West, Northeast, Southeast, Southwest, Northwest.
Perhaps you should have a brief peek at this new world.
~
#ENDEXDESC

#EXDESC
ExDescKey    north n~
ExDesc       Outside you see a large field leading to the base of a large mountain,
above which floats a radiant city dominated by a palatial structure.
~
#ENDEXDESC

#EXDESC
ExDescKey    northwest ne~
ExDesc       Far to the north and east you see a large city surrounded by verdant
fields.  In the east of this city rises a huge citadel of gold.
~
#ENDEXDESC

#EXDESC
ExDescKey    west w~
ExDesc       You can make out a guarded city gate in this direction.  North of the
gate is a small forest, inhabited by many small pixies.  Farther west
and north is a large town.  Straight west is a huge tree which seems
to stop the path at its base.
~
#ENDEXDESC

#EXDESC
ExDescKey    southeast se~
ExDesc       In this direction you see what appears to be a boathouse on a river.
The river seems to head from east to west, emptying to a large ocean.
~
#ENDEXDESC

#EXDESC
ExDescKey    southwest sw~
ExDesc       The forest changes in this direction, the trees create a canopy over
quite a large area. South of the forest is a huge swamp area.
~
#ENDEXDESC

#EXDESC
ExDescKey    northeast~
ExDesc       Far to the north and east, you see a large city surrounded by verdant
fields. To the east of the city is a huge golden castle.
~
#ENDEXDESC

#EXDESC
ExDescKey    'tapestry~
#ENDEXDESC

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   if isimmort($n)
or race($n) == halfling
else
  mpforce 0.$n vis
endif
~
#ENDPROG

#MUDPROG
Progtype  rand_prog~
Arglist   20~
Comlist   mpecho A large shadow passes through the room as sounds cascade from above.
mpechoat $r You look up and see the tapestry overhead...
mpforce $r look up
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     101
Name     An Unsettling Reception~
Sector   inside~
Flags    nomob indoors safe norecall nosummon noastral~
Desc     Muted sounds and the scent of cedar assail your senses as you enter this
stately chamber.  Thick carpeting competes with leathered chairs in a bid
for your comfort, and a huge painting of a barren desert occupies the wall
behind a large desk of oak.  Four small luminescent globes float aimlessly
about the room, and you notice that though seemingly free floating these
globes always manage to remain equidistant from one another.  The huge
bust of a fantastical lizard has been set upon the wall, an amazing
hunting trophy of some sort.
~
#EXIT
Direction north~
ToRoom    102
Desc      Before you is a path past a giant tree.
~
#ENDEXIT

#EXIT
Direction up~
ToRoom    100
Desc      Up a spiral staircase is a observation tower, overlooking the Realms.
~
#ENDEXIT

Reset M 0 100 1 101
  Reset E 1 100 1 0
#EXDESC
ExDescKey    cedar~
ExDesc       The walls here are made from the finest cedar, giving the room a luxurious
look and a fantastic smell.  Someone went to great expense and searching to
find these strips of perfectly matched wood.
~
#ENDEXDESC

#EXDESC
ExDescKey    carpet carpeting~
ExDesc       This carpet appears to be made from a cashmere wool laid atop some sort of
thick padding.  Your feet sink at least an inch into the flooring.
~
#ENDEXDESC

#EXDESC
ExDescKey    painting~
ExDesc       Done on a stretched scroll in vivid, detailed colors, it looks almost as if
you could step into the painting (though it is a barren desert and does not
appear to be somewhere you would want to visit).
~
#ENDEXDESC

#EXDESC
ExDescKey    wall walls~
ExDesc       These walls are made from a fine sample of cedar, decorative enough to
require little in the way of painting and other decoration.  One painting
adorns the wall behind the beautiful oak desk:  a painting of a barren
desert, hauntingly vivid, almost as if you could step directly into it.
~
#ENDEXDESC

#EXDESC
ExDescKey    globes globe~
ExDesc       Each globe winks with an internal light, though not a flame or any sort of
thing you can define by past experiences.  They dangle in mid air with no
means of support.  Magic comes to mind, but you shrug it off.
~
#ENDEXDESC

#EXDESC
ExDescKey    bust trophy lizard~
ExDesc       Your mind clicks!  You know what this is, though your mind takes a moment
to digest the information.  You are staring at a bust of a dragon, closer
to such a thing than you have ever been before.  Its eyes seem to glare
menacingly at you, but a shiver courses the length of your spine as a
sigh of relief passes your lips.  It is thankfully dead, and you are safe.
~
#ENDEXDESC

#EXDESC
ExDescKey    chair chairs leather~
ExDesc       Each of these chairs are made from the finest of leathers, polished to a
high sheen and dyed a midnight black.  This is by far the best craftsmanship
you have ever seen, and as you approach you can smell the rich scent given
off by fresh leather.
~
#ENDEXDESC

#EXDESC
ExDescKey    oak desk~
ExDesc       This desk is made from a carved oak wood, stained to a dark brown finish.
The crafting is done so that the legs appear to be clawed feet on the end
of slightly bent legs.  The craftwork is excellent.
~
#ENDEXDESC

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   if isimmort($n)
or race($n) == halfling
else
mpforce 0.$n vis
endif
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     102
Name     The Path of Knowledge~
Sector   inside~
Flags    nomob indoors safe norecall nosummon noastral~
Desc     You travel upon an open pathway beyond the confines of the reception area
to the south, heading toward your future.  After a time, you see a stocky
man leaning casually against a large tree to the side of road.  Your path
leads on to the north, or you can return to the structure to the south.
~
#EXIT
Direction north~
ToRoom    103
Desc      The path divides north of here.
~
#ENDEXIT

#EXIT
Direction south~
ToRoom    101
Desc      To the south is the reception area.
~
#ENDEXIT

#EXIT
Direction somewhere~
ToRoom    104
Keywords  opening~
Flags     can_enter can_leave auto~
#ENDEXIT

Reset M 0 102 1 102
Reset O 0 199 1 102
#EXDESC
ExDescKey    tree~
ExDesc       This tree is huge! On the southern face you see a large opening, it
appears big enough for you to enter. You wonder what is inside.
HINT: Type OPENING or LEAVE OPENING to enter and leave the tree.
~
#ENDEXDESC

#EXDESC
ExDescKey    somewhere~
ExDesc       Somewhere is an exittype that you use for non-direction exits.
They are entered using a "codeword", usually preceeded by
leave, climb, or enter. This somewhere is used by typing 
either LEAVE OPENING or OPENING alone.
~
#ENDEXDESC

#ENDROOM

#ROOM
Vnum     103
Name     A Path Divided~
Sector   inside~
Flags    nomob indoors norecall nosummon noastral~
Desc     Do you wish to spend your time watching over your shoulder, hiding in the
shadows, killing and being killed by other players?
 
Pkill -- that is the decision you must make here.  If you so choose to walk
the path of shadows, killing and stealing from other players, you should be
aware that you may never again walk the path of light.  Once deadly, always
deadly.  As a deadly character you will be the potential victim all other
pkillers within FIVE levels (higher or lower) of your own.  You do not have
to decide to become pkill at this moment; you may embark upon that path at
any time in the future by requesting Immortal assistance, or by joining a
clan.  Both 'HELP RULES PKILL' and 'HELP RULES ASSIST' contain information
important to pkill players.  The gods forbid peaceful players to give much
in the way of assistance to pkillers.
 
- If you have read this and would like to walk the path of the deadly, type:
"say I choose to walk on the shadowy path of the pkillers."

- If you have read this and would like to remain on the peaceful path, type:
"say I choose to walk on the lighted path of the peaceful."
~
#EXIT
Direction south~
ToRoom    102
Desc      Behind you is a path by the leading back to the reception area.
~
#ENDEXIT

Reset O 0 192 1 103
#MUDPROG
Progtype  speech_prog~
Arglist   p shadowy path~
Comlist   if level($n) == 1
mpforce 0.$n config -nice
mpforce 0.$n config -flee
mppkset $n yes
mpechoat $n You are now a Deadly Character... you will be listed under
mpechoat $n 'who deadly' once you reach level 5 and 18 years of age.
mpechoat $n You will now be transferred to your chosen path...
mpechoaround $n $n choses the path of shadows, disappearing into the darkness.
mptransfer 0.$n 107
mpat 107 mpforce 0.$n look
mpat 107 mpforce 0.$n config +autoloot
mpat 107 mpforce 0.$n config +autogold
else
mpechoat $n You have already chosen your path...
mptransfer $n 21001
endif
~
#ENDPROG

#MUDPROG
Progtype  speech_prog~
Arglist   p lighted path~
Comlist   if level($n) == 1
mppkset $n no
mpechoat $n You have chosen the way of light, and are not a deadly character.
mpechoat $n You will now be transferred to your chosen path.
mpechoaround $n $n has chosen the path of light, and disappears!
mptransfer 0.$n 105
mpat 105 mpforce 0.$n look
mpat 105 mpforce 0.$n config +autoloot
mpat 105 mpforce 0.$n config +autogold
else
mpechoat $n You have already chosen your path...
mptransfer $n 21001
endif
~
#ENDPROG

#MUDPROG
Progtype  rand_prog~
Arglist   33~
Comlist   mpechoat $r Your eyes are drawn to the boulder, and you LOOK PLAQUE.
mpforce $r look plaque
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     104
Name     Inside the Tree~
Sector   city~
Flags    nomob indoors norecall nosummon noastral~
Desc     This is a small house, sparsely decorated.  It looks as if the dwarf who
resides here prefers to spend all his time outside rather then indoors.
The table is covered with a few dishes and the bed is unmade, though the
rest of the place is relatively clean.  Against the north wall you see a
large chest.  Upon closer inspection you see the chest is open, and you
wonder what it might contain.  The opening is back to the south.
~
#EXIT
Direction somewhere~
ToRoom    102
Keywords  opening~
Flags     isdoor can_enter can_leave auto~
#ENDEXIT

Reset O 0 194 1 104
Reset O 0 104 1 104
  Reset P 0 105 1 104
Reset D 0 104 10 0
#EXDESC
ExDescKey    table~
ExDesc       A small wooden table, only large enough to seat two.  There are a few
plates and food pieces left here, and the only thing of note is that
the dinnerware is made of wood.
~
#ENDEXDESC

#EXDESC
ExDescKey    dishes~
ExDesc       These dishes are made from a rough-finished pottery.  They look only
finished by fire, cooked to a brittle hardness.  They were definitely
made for use rather than appearance.
~
#ENDEXDESC

#EXDESC
ExDescKey    bed~
ExDesc       Shorter than one that many would find comfortable, it is made with a
wooden frame and a mattress of feathers in a cotten fabric.  The
blankets are woolen and look warm, if not decorative.
~
#ENDEXDESC

#EXDESC
ExDescKey    opening~
ExDesc       The opening leaves out the south of this room.  To use this type of exit,
type LEAVE OPENING or OPENING alone.  This is the way to return to the
path to the new world.
~
#ENDEXDESC

#ENDROOM

#ROOM
Vnum     105
Name     The Path of the Peaceful~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     The path here is bright, glowing.  The trees on the sides are full and
green, the grass thick and flowing, and you realize how lush this path
is and how full of life it feels.  A large sign floats before a tree
on the eastern side of the path.
~
#EXIT
Direction north~
ToRoom    106
Desc      Before you is an adventure, are you ready to embrace it?
~
#ENDEXIT

Reset O 0 120 1 105
#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   mpechoat $n Your eyes read over the sign as you pass it by.  It reads:
mpforce 0.$n look sign
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     106
Name     A Lighted Path~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     The path continues here, the trees growing fuller and the pathway lighter.
You notice a parting ahead, towards the north.  Beyond this parting lies
an open field, a joining of the two paths at which you see a small round
piece of metal.  You can pick it up as you head north.
~
#EXIT
Direction north~
ToRoom    109
Desc      Before you is a large, open field.
~
#ENDEXIT

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   c refresh $n
c refresh $n
mpoload 106 1
drop ring
mpechoat $n You bend down and pick up the piece of metal from the ground.
mpforce 0.$n get lightring
mpechoaround $n $n reaches down and picks something up.
mpforce 0.$n look ring
mpapply $n
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     107
Name     The Path of the Deadly~
Sector   forest~
Flags    dark nomob norecall nosummon noastral~
Desc     The path here is dark, shadowy.  The trees alongside look skeletal, the
grass lays a dull brown, and you realize how lonely this path may prove
to be.  A large sign floats before the trees on the eastern pathside.
~
#EXIT
Direction north~
ToRoom    108
Desc      Before you is an adventure, are you ready to embrace it?
~
#ENDEXIT

Reset O 0 121 1 107
#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   mpechoat $n Your eyes read over the sign as you pass it by. It reads:
mpforce 0.$n look sign
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     108
Name     A Shadowy Path~
Sector   forest~
Flags    dark nomob norecall nosummon noastral~
Desc     Your travels continue, the trees growing thinner and the pathway darker.
You notice a parting ahead, towards the north.  Beyond the parting lies
an open field, a joining of the two paths, and at this joining you see
a small round piece of metal.  You can pick it up as you head north.
~
#EXIT
Direction north~
ToRoom    109
Desc      Before you is a large, open field.
~
#ENDEXIT

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   c refresh $n
c refresh $n
mpoload 107 1
drop ring
mpechoat $n You bend down and pick up the piece of metal from the ground.
mpforce 0.$n get shadowring
mpechoaround $n $n reaches down and picks something up.
mpforce 0.$n look ring
mpapply $n
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     109
Name     Through the Field~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     You find yourself in a grassy valley.  Mountains enclose you on all sides,
and a forest lies ahead to the northwest.  Miniature daisies and clover are
strewn about the field like a fine dusting of snow.  As you scan about your
surroundings, you stop with a start.  The path you have just traversed has
disappeared, leaving behind no trace of its remains.  The trees are thick
and impenetrable, removing your choices on ways to proceed.  There is no
turning back; the only direction is forward, further into this new world.
~
#EXIT
Direction northwest~
ToRoom    110
Desc      Leading away from this field is a path through a circle of trees.
~
#ENDEXIT

Reset M 0 103 1 109
#MUDPROG
Progtype  entry_prog~
Arglist   100~
Comlist   if mobinroom(103) < 1
mpmload 103
endif
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     110
Name     A Ring of Trees~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     You have entered a shadowy path, surrounded with trees.  This mystical
cirle engulfs you with a strange, almost magical feeling.  This clearing
leaves you with a sense of safety, a sense of well being.
~
#EXIT
Direction west~
ToRoom    111
Desc      You see the path continues through the trees.
~
#ENDEXIT

#EXIT
Direction southeast~
ToRoom    109
Desc      Behind you is a field of daisies and clovers.
~
#ENDEXIT

#EXDESC
ExDescKey    trees tree~
ExDesc       These trees seem almost to be live, thinking beings, exuding an aura of 
darkness and magic.  Not that they will harm you, but you sense that you 
would not wish to attempt to chop down any found here.
~
#ENDEXDESC

#MUDPROG
Progtype  entry_prog~
Arglist   100~
Comlist   c heal $n
c refresh $n
c refresh $n
~
#ENDPROG

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   mpforce 0.$n vis
mpoload 109 1
mpat 111 drop feather
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     111
Name     Seeing the Forest through the Trees~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     The forest opens up a bit here, without actually giving of itself to field
or stream.  It is as if the forest were allowing you access to see the true
beauty that lies within.  Everything you had missed before, all the sights
and sounds and scents, all come to you now.  Your eyes alight upon a flock
of small purple birds which flutter and flow about the area.  A feathery,
light moss grows on the western side of the trees, giving off a faint,
musty smell, and small beetles work industriously in the moss, moulding it
into a haven from predators.  A gentle splashing sound causes your eyes to
drift northward, to a small pond.
~
#EXIT
Direction north~
ToRoom    112
Desc      To the north the pathway passes a small pond.
~
#ENDEXIT

#EXIT
Direction east~
ToRoom    110
Desc      To the east is a tight circle of trees.
~
#ENDEXIT

#EXDESC
ExDescKey    flock bird birds~
ExDesc       High above the trees are a group of birds, enjoying the gift of flight.
These birds look like those you have experienced before, yet their coloring
is shockingly different, with feathers of a vivid purple, making the seem
afire when hit directly by the sunlight.
~
#ENDEXDESC

#EXDESC
ExDescKey    moss~
ExDesc       This moss is the most brilliant green you can recall ever seeing.  It is
thick and grows heavily on the west side of almost every tree trunk within
sight.  As you peer closer you can see each is like a small city to the
bugs of the forest which live and hide here to flee the predators.
~
#ENDEXDESC

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   mpoload 110 1
mpat 112 drop bucket
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     112
Name     Beside an Azure Pond~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     The forest has grudgingly given way here to a small pond of azure waters,
shallow and with a crystal clarity which captures your attention.  Lithe
lapis-toned fish swim about, nibbling at the various plants growing among
the depths of the pond.
~
#EXIT
Direction south~
ToRoom    111
Desc      The path goes back through the trees to the south.
~
#ENDEXIT

#EXIT
Direction west~
ToRoom    113
Desc      The base of a hill meets the trail to the west.
~
#ENDEXIT

Reset O 0 111 1 112
#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   mpforce 0.$n get bucket
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     113
Name     A Pathway of Strewn Rock~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     The trees fall away here from the hill's edge, and jumbled rocks lie about
like forgotten toys left behind after a day's play.  Sharp edges and jagged
lines define the pathway as it wends its way upward.
~
#EXIT
Direction east~
ToRoom    112
Desc      East you can travel along the shore of the pond.
~
#ENDEXIT

#EXIT
Direction up~
ToRoom    114
Desc      The trail wends its way up the hillside.
~
#ENDEXIT

#EXDESC
ExDescKey    rock rocks~
ExDesc       The rocks here are of a slate grey or smoky black.  Neither decorative
nor purposeful, they appear to have been ripped from far below the
surface of the hill.
~
#ENDEXDESC

#ENDROOM

#ROOM
Vnum     114
Name     Upon a Desolate Hillside~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     Shattered and blasted stone litters the hillside here.  First glance gives
you the impression that this is the result of volcanic activity, but this
view is rapidly displaced as you gaze upon the ravaged bones strewn along
a section of the hill; a headless skeletal remains of a primitive beast.
The majesty of this huge, winged creature has not been diminished by its
boney state.  As you study the remains, you are amazed by the sheer size
of this beast, as he must have struck true fear into the hearts of those
who slew him.  Charred and shattered, hundreds of smaller bones splay
across the hillside, tangible proof that this great beast fought
fiercely before succumbing to its own death.
~
#EXIT
Direction east~
ToRoom    115
Desc      You can see a large plateau in this direction.
~
#ENDEXIT

#EXIT
Direction down~
ToRoom    113
Desc      The path descends the hill to the south.
~
#ENDEXIT

#EXDESC
ExDescKey    stone stones~
ExDesc       These stones are less sharp and jagged than those lower on the hill, but
definitely more ravaged.  It appears as if they have been applied to very
high temperatures that smoothed and blackened their exteriors.
~
#ENDEXDESC

#EXDESC
ExDescKey    bone bones~
ExDesc       The bones here fit together to paint a vivid picture of a large battle -
man against beast.  The men took losses of their own before the beast was
defeated.  The bones of this creature appear to stretch a good twenty men
long, from neck-top to the tip of its tail, and you can imagine the size
of the missing skull.  Your mind wanders back to the reception area, to
the bust on the wall...
~
#ENDEXDESC

#ENDROOM

#ROOM
Vnum     115
Name     Cresting the Hill~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     Burnt and dessicated ground gives way, grudgingly, to a grassy plateau.
Low hanging clouds obscure your view of the surrounding countryside, with
the only visible landmark being a tall tower to the southwest, piercing
through the clouds and disappearing skyward.  Pathways lead off down the
hillside to the east and west, and deep, intermittent rumblings come from
nearby.  Glancing southward you see the likely source - the dark opening
of a cave mouth.
~
#EXIT
Direction east~
ToRoom    117
Desc      The greener grass on the other side of the hill.
~
#ENDEXIT

#EXIT
Direction south~
ToRoom    116
Desc      You stand before the darkened mouth of a cave.
~
#ENDEXIT

#EXIT
Direction west~
ToRoom    114
Desc      Back down the desolate hillside.
~
#ENDEXIT

#MUDPROG
Progtype  rand_prog~
Arglist   50~
Comlist   mpecho A deep, growling rumble echoes from inside the cave to the south.
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     116
Name     Inside the Cavern~
Sector   inside~
Flags    indoors norecall nosummon noastral~
Desc     Large and darkly shadowed, the uneven form of the walls makes it difficult
to determine the extent of this cave.  Strange shards litter the floor of
the cavern, and fresh marks gouged into the ground in and around these
shards help you to determine just what they are:  remnants of a recently
hatched egg of such enormous proportions the leathery pieces fill you with
a growing dread and caution.  Looking about with care, you easily discover
the skeletal remains of many humanoid creatures.  This barely has time to
register before a sharp rumbling causes you to look up suddenly. 
~
#EXIT
Direction north~
ToRoom    115
Desc      North is the path to your future.
~
#ENDEXIT

Reset M 0 104 1 116
#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   c refresh $n
c refresh $n
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     117
Name     Descending the Hillside~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     Green and lush, the grass grows freely along the pathway here as this side
of the hill was somehow spared the ravages of the battle which took place
on its southern front.  A feeling of safety comes over you as you wend
your way along, as though you yourself had avoided some great danger. 
~
#EXIT
Direction north~
ToRoom    118
Desc      North is the entrance to a valley.
~
#ENDEXIT

#EXIT
Direction west~
ToRoom    115
Desc      You see a plateau on the hill.
~
#ENDEXIT

#ENDROOM

#ROOM
Vnum     118
Name     Edge of a Verdant Valley~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     Here the hillside fades gently into a valley of verdant grass, filled with
flowering thistle.  The soft sound of the wind through the grass, the scent
of flowers on the air, all combine to give a feeling of accomplishment and
expectation... as though you are quickly approaching some indefinable goal.
~
#EXIT
Direction east~
ToRoom    119
Desc      You see a path through the valley and a small wooden cabin.
~
#ENDEXIT

#EXIT
Direction south~
ToRoom    117
Desc      A path leading up the side of a hill.
~
#ENDEXIT

#MUDPROG
Progtype  entry_prog~
Arglist   100~
Comlist   if isimmort($n)
or race($n) == Halfling
else
  mpforce 0.$n vis
endif
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     119
Name     A Valley Pass~
Sector   forest~
Flags    nomob norecall nosummon noastral~
Desc     Tall, sheer mountains hug this small valley like overprotective parents
keeping the ravages of the world at bay, and lending a peaceful feel to
this place.  The very air seems to hold itself in hushed reverence. 
Glancing to the south, you see a small wooden cabin built into the face
of the hill.
~
#EXIT
Direction south~
ToRoom    120
Desc      A door to into a cabin
~
Keywords  door~
Flags     isdoor closed~
#ENDEXIT

#EXIT
Direction west~
ToRoom    118
Desc      Back towards the valley entrance.
~
#ENDEXIT

#EXIT
Direction northeast~
ToRoom    121
Desc      The path ends before a large gate
~
#ENDEXIT

Reset M 0 105 1 119
Reset O 0 198 1 119
#EXDESC
ExDescKey    cabin~
ExDesc       This cabin looks like it melds with the surrounding hillside, a
pleasant mix of wood and stone making this attractive home. The
door is closed, but unlocked. You can OPEN DOOR and go inside.
~
#ENDEXDESC

#EXDESC
ExDescKey    door~
ExDesc       This door is made of a solid wood. It looks more to keep the weather
at bay than to prevent entry, the door is closed but unlocked. You
may OPEN DOOR to enter.
~
#ENDEXDESC

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   if isimmort($n)
or race($n) == Halfling
else
  mpforce 0.$n vis
endif
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     120
Name     Inside a Small Cabin~
Sector   city~
Flags    nomob indoors safe norecall nosummon noastral~
Desc     Various scents assail you as you enter the cabin.  The smell of cooked meats
is overlaid with the scents of a crackling fire and the gentle tang of pine.
Simple, well-made furniture adorns this well kept home.  As your eyes move
about, enjoying the rustic comfort of your surroundings, the gentle sound
of a rocking chair intrudes upon your reverie.
~
#EXIT
Direction north~
ToRoom    119
Desc      Back to the valley pathway
~
Keywords  door~
Flags     isdoor closed~
#ENDEXIT

#EXIT
Direction down~
ToRoom    123
Key       114
Keywords  trap trapdoor~
Flags     isdoor closed locked secret pickproof nopassdoor~
#ENDEXIT

Reset M 0 106 1 120
  Reset E 1 119 1 5
Reset D 0 120 5 2
#EXDESC
ExDescKey    rocking chair~
ExDesc       This rocking chair is made of a supple wood, bent and twisted rather than
carved.  It is finished with a light brown stain, highlighting the natural
grain in the wood.  The chair is placed on top of a hand braided rug, done
in an intricate design.  The south corner of the rug appears to be caught
on something.
~
#ENDEXDESC

#EXDESC
ExDescKey    rug~
ExDesc       This rug is done in a design that appears to be a six-sided star weaved in
and out of a series of circles.  It is like a large Gods Eye light catcher,
the design pulls you deep into its pattern.  The decoration is so intense
you almost miss a crack pinning a small portion of the southern corner.
~
#ENDEXDESC

#EXDESC
ExDescKey    crack corner~
ExDesc       The corner of the rug is caught slightly caught what at first appears to
be two floorboards.  Upon closer review, you see the crack is actually the
frame to some type of trap door in the flooring.  As you pull back the rug,
you notice a large keyhole.  This is very interesting indeed!
~
#ENDEXDESC

#EXDESC
ExDescKey    floorboards floorboard~
ExDesc       As you examine them, you notice a crack which has snagged part of the rug.
~
#ENDEXDESC

#MUDPROG
Progtype  rand_prog~
Arglist   25~
Comlist   mpecho The wooden chair creaks as it rocks back and forth over the floorboards.
~
#ENDPROG

#MUDPROG
Progtype  leave_prog~
Arglist   100~
Comlist   if race($n) == Halfling
mpforce q emote leans out the door...
mpforce q give key $n
mpforce q give armour $n
mpforce q mpechoat $n Good luck in your adventures, $n...
close door
else
endif
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     121
Name     Before the Passage~
Sector   forest~
Flags    safe norecall nosummon noastral~
Desc     The dense mountains, almost impassable to begin, here bring the pathway to
a sheer end.  Tall bluffs rising to the east abruptly halt at a large gate,
gold in color.  The size of the gate and its obvious magical bestowments
give it the power to bar further passage into the world beyond until
unlocked by those who find its key.
~
#EXIT
Direction east~
ToRoom    122
Key       114
Keywords  gate~
Flags     isdoor closed locked~
#ENDEXIT

#EXIT
Direction southwest~
ToRoom    119
#ENDEXIT

Reset D 0 121 1 2
Reset O 0 197 1 121
#EXDESC
ExDescKey    gate~
ExDesc       It would clearly take a large key to unlock this gate.  Maybe the resident
of the cabin southwest of here has it.
~
#ENDEXDESC

#MUDPROG
Progtype  rand_prog~
Arglist   40~
Comlist   mpechoat $r Your eyes are drawn to a scribed sign on the gate, and LOOK GATE.
mpforce $r look gate
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     122
Name     A View of the World~
Sector   forest~
Flags    indoors safe norecall nosummon noastral~
Desc     You find yourself standing atop a wide mesa offering a spectacular view of
the world below.   Expectation permeates the air, and you feel compelled
to do something, as though some force awaits your actions.
~
#EXIT
Direction west~
ToRoom    121
Key       114
Keywords  gate~
Flags     isdoor closed locked~
#ENDEXIT

Reset O 0 115 1 122
Reset D 0 122 3 2
#MUDPROG
Progtype  act_prog~
Arglist   p pulls a rope.~
Comlist   mpechoat $n High overhead you hear a large bell thunder in response.
mpechoaround $n High overhead, a large bell thunders in response.
mpechoat $n If nothing happens, type LOOK ROPE, and pull again in 30 seconds.
mpapplyb $n
push rope
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     123
Name     A Musty Cellar~
Sector   inside~
Flags    indoors safe norecall nosummon noastral~
Desc     Dank musty air fills this room.  Fingers of darkness in the form of
several begrimed cobwebs cling to your face.  Every movement seems
to kick up long settled dust, making it nearly unbearable down here.
The creaking from above is magnified, changed, and lends a haunting
feel to this place.  Deep in the shadows is a small table, it holds
only one item:  a partially burned wooden box.  The trapdoor is up.
~
#EXIT
Direction up~
ToRoom    120
Key       114
Keywords  trap trapdoor~
Flags     isdoor closed~
#ENDEXIT

Reset D 0 123 4 1
#MUDPROG
Progtype  entry_prog~
Arglist   100~
Comlist   mpechoat $n You hear a creaking sound as the trapdoor closes above you.
mpat Quercusi mpforce Quercusi close trapdoor
~
#ENDPROG

#ENDROOM

#ROOM
Vnum     199
Name     Floating in a void~
Sector   city~
Flags    nomob~
#ENDROOM

#ENDAREA
