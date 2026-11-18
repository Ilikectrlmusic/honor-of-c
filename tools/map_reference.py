"""Draw the exact design reference for the hand-painted arena (Pillow, optional)."""
from PIL import Image, ImageDraw
import random, math
random.seed(319)
im=Image.new("RGB",(1500,1100),"#47614e")
d=ImageDraw.Draw(im)
top=[(270,1910),(340,1540),(342,1080),(375,610),(535,390),(935,310),(1530,275),(2160,275),(2500,288),(2730,290)]
mid=[(270,1910),(625,1665),(990,1455),(1260,1285),(1500,1100),(1740,915),(2010,745),(2375,535),(2730,290)]
low=[(3000-x,2200-y) for x,y in reversed(top)]
river=[(-100,-40),(375,195),(690,455),(1010,780),(1480,1100),(1860,1400),(2200,1650),(2530,1930),(3080,2240)]
def line(points,color,width):
    ps=[(x//2,y//2) for x,y in points]; r=width//4
    d.line(ps,color,width//2,joint="curve")
    for x,y in ps: d.ellipse((x-r,y-r,x+r,y+r),fill=color)
def disc(x,y,r,fill):
    d.ellipse(((x-r)//2,(y-r)//2,(x+r)//2,(y+r)//2),fill=fill)
for i in range(7800):
    x,y=random.randrange(3000),random.randrange(2200); r=random.randrange(3,65)
    disc(x,y,r,random.choice(["#4a6451","#4e6852","#526d56","#435e4b","#49654d"]))
line(river,"#769682",226);line(river,"#3a7979",204); line(river,"#377677",149)
for p in [[(368,880),(800,1020),(1100,680),(1470,310)],[(720,1610),(800,1020),(1150,1290)],[(1510,1900),(1920,1560),(2160,1180),(2600,1340)],[(2170,605),(2160,1180),(1810,960)]]:
    line(p,"#69745a",76)
for p in (top,mid,low):
    line(p,"#596953",178);line(p,"#99a289",149);line(p,"#a7ad95",123)
for x,y in [(270,1910),(2730,290)]:
    disc(x,y,222,"#667b71");disc(x,y,200,"#adb5a2");disc(x,y,164,"#8f9e91");disc(x,y,145,"#acb3a2")
for x,y in [(800,1020),(1100,680),(1920,1560),(2160,1180)]: disc(x,y,90,"#849071")
im.save("tools/map-reference.png")
