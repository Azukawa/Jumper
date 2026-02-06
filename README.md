# Jumper
2d platformer using SMB3-like integer physics

Playing lots of Super Mario Bros 3 got me marveling how excellent platformer mechanics and gamefeel can be achieved using basic fundamental methods.
I seek to recreate smooth, fast and responsive gameplay feel using only integers in the gamelogic.

Player can now jump around and pick up and throw a spear that has its own physics.

Responsive key inputs in a fixed timestep can be tricky!
Keyinputs are polled at much faster rate than new gameplay ticks are calculated.
Keyinputs arriving in between ticks need to be stored to wait for a new tick and then get used or "eaten" by a new tick.

Left and Right arrows  - Move left and right.
Up Arrow or Z          -  Jump
SPACE or X             -  Pick up and drop Spear. Hold SPACE or X to Throw spear. When moving fast enough, spear will get stuck on walls.



