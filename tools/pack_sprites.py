"""Pack the two original magenta-background art sheets into a game atlas."""
from PIL import Image, ImageDraw
import sys, colorsys

def extract(path, cuts):
    im=Image.open(path).convert("RGBA")
    out=[]
    for a,b in zip(cuts,cuts[1:]):
        s=im.crop((round(a*im.width),0,round(b*im.width),im.height))
        p=s.load()
        for y in range(s.height):
            for x in range(s.width):
                r,g,b,_=p[x,y]
                if min(r,b)>g+42 and r>122 and b>125 and abs(r-b)<94:
                    p[x,y]=(0,0,0,0)
        s=s.crop(s.getbbox());s.thumbnail((244,244),Image.Resampling.LANCZOS);out.append(s)
    return out
if __name__=="__main__":
    sprites=extract(sys.argv[1],[0,.194,.400,.599,.800,1])+extract(sys.argv[2],[0,.148,.350,.503,.658,.813,1])
    red=sprites[6].copy()
    px=red.load()
    for y in range(red.height):
        for x in range(red.width):
            r,g,b,a=px[x,y]
            if a and b>r+9 and b>g-9:
                px[x,y]=(min(255,int(b*1.06)),int(g*.63+r*.16),int(r*.65+b*.14),a)
    sprites.append(red)
    # Enemy uniforms keep the silhouette, but their cloth and crystal colorways differ.
    for i,original in enumerate(sprites[:10]):
        alt=original.copy();px=alt.load()
        for y in range(alt.height):
            for x in range(alt.width):
                r,g,b,a=px[x,y]
                if not a: continue
                h,s,v=colorsys.rgb_to_hsv(r/255,g/255,b/255)
                if i==0: change=s>.4 and (h<.075 or h>.9)
                elif i in (1,2,4): change=s>.25 and .18<h<.91
                else: change=s>.31 and .035<h<.18
                if change:
                    rr,gg,bb=colorsys.hsv_to_rgb(.96 if i!=0 else .60,max(.58,s),v)
                    px[x,y]=(int(rr*255),int(gg*255),int(bb*255),a)
        sprites.append(alt)
    atlas=Image.new("RGBA",(256*6,256*4))
    for i,s in enumerate(sprites):
        atlas.alpha_composite(s,((i%6)*256+(256-s.width)//2,(i//6)*256+248-s.height))
    atlas.save("assets/units.png",optimize=True)
    ico=Image.new("RGBA",(256,256),(0,0,0,0));d=ImageDraw.Draw(ico)
    d.rounded_rectangle((4,4,251,251),radius=40,fill="#132a2c",outline="#d6b578",width=9)
    d.polygon([(46,192),(132,47),(208,190),(156,161),(128,108),(91,170)],fill="#ebcd8f")
    d.polygon([(60,208),(127,159),(193,205),(142,193),(121,180)],fill="#52b8ad")
    ico.save("assets/rift.ico",sizes=[(16,16),(24,24),(32,32),(48,48),(64,64),(128,128),(256,256)])
