import math
import sys

if len(sys.argv) < 2:
    print("No args provided")
    sys.exit(1)

pi = 3.1415
fov = pi / 3
sw = 1920

rayAngleStep = fov / sw

rayAngleInitial = float(sys.argv[1])
#rayAngleFinal = rayAngleInitial + (rayAngleStep * 1920)
rayAngleFinal = rayAngleInitial + fov
rayAngle = rayAngleInitial

angles = []

for i in range(sw):
	angles.append(rayAngle)
	rayAngle -= rayAngleStep

splitAngle = -1

quadrant = rayAngleInitial / (pi / 2)

if quadrant < 0:
	quadrant += (2*pi)
quadrant = math.floor(quadrant)

print(quadrant)
if quadrant == 1 or quadrant == 0:
    quadrantSplit = (pi - rayAngleInitial) / rayAngleStep
elif quadrant == 2:
    quadrantSplit = (3*pi/2 - rayAngleInitial) / rayAngleStep
elif quadrant == 3:
    quadrantSplit = (2 * pi - rayAngleInitial) / rayAngleStep
elif quadrant == 4:
    quadrantSplit = (2 * pi + pi/2 - rayAngleInitial) / rayAngleStep
else:
	print("Error!")
	quit()

if quadrantSplit < sw:
	splitAngle = angles[math.ceil(quadrantSplit)]

print(f"\nQ1:{pi/2}, Q2:{pi}, Q3:{3*pi/2}, Q4:{2*pi}")
print(f"rayAngleInitial: {rayAngleInitial}\nrayAngleStep: {rayAngleStep}, fov: {fov}, sw: {sw}")
print(f"quadrant: {quadrant}\nquadrantSplit: {quadrantSplit}")
print(f"rayAngleFinal: {rayAngleFinal}, splitAngle: {splitAngle} @ {math.ceil(quadrantSplit)}")
