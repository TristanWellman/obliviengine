-- Moves the cube object in a circular motion.
local x, y, z = OE.getObjectPosition("Cube")
local tick = OE.getTick()
OE.setObjectPosition("Cube", x, y+math.cos(tick), z+math.sin(tick))
OE.rotateObject("Cube", 45)
