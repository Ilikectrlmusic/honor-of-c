"""Replace just Dan Sha in the shared atlases (portrait, scene, 6x3 poses).
Usage: python tools/pack_dansha.py portrait.png scene.png poses.png
Optional authoring dependencies: Pillow and jpegtran. Building needs neither.
"""
import argparse
from pathlib import Path
import re
import subprocess
from statistics import median
from PIL import Image, ImageOps
from pack_motion import ROOT, CELL, cells, enemy, root_fraction


def pack(portrait, scene, poses):
    assets = ROOT / 'assets'
    work = ROOT / 'build' / 'dansha-packing'
    work.mkdir(parents=True, exist_ok=True)
    fg = ImageOps.fit(Image.open(portrait).convert('RGBA'), (400,664),
                      method=Image.Resampling.LANCZOS, centering=(.5,0))
    bg = ImageOps.fit(Image.open(scene).convert('RGB'), fg.size, method=Image.Resampling.LANCZOS)
    cards = bg.copy()
    cards.paste(fg, mask=fg.getchannel('A'))
    for filename, im in [('roster-cards.jpg',cards),('roster-domains.jpg',bg)]:
        target = assets/filename
        with Image.open(target) as old:
            im.save(work/filename, qtables=old.quantization, subsampling=2)
        # Copy quantized JPEG blocks, leaving unaffected heroes un-recompressed.
        out = work/('out-'+filename)
        subprocess.run(['jpegtran','-drop','+800+0',str(work/filename),
                        '-outfile',str(out),str(target)],check=True)
        target.write_bytes(out.read_bytes())
    dst = assets/'roster-busts.png'
    with Image.open(dst) as old: busts=old.copy()
    busts.paste(fg,(800,0)); busts.save(dst)
    frames = cells(Image.open(poses).convert('RGBA'),6,3)
    idle=median(f.height for f in frames[6:9])
    scale=min(124/idle,(CELL-6)/max(max(f.size) for f in frames))
    dst=assets/'motion-3.png'
    with Image.open(dst) as old: atlas=old.copy()
    top=12*CELL
    atlas.paste((0,0,0,0),(0,top,atlas.width,top+6*CELL))
    pivots=[]
    for i,f in enumerate(frames):
        root=root_fraction(f)
        f=f.resize(tuple(max(1,round(v*scale)) for v in f.size),Image.Resampling.LANCZOS)
        left=(CELL-f.width)//2
        pivots.append(round(left+f.width*root))
        x=i%6*CELL+left;y=top+(i//6+1)*CELL-3-f.height
        atlas.paste(f,(x,y));atlas.paste(enemy(f,3),(x,y+3*CELL))
    atlas.save(dst)
    header=ROOT/'src/roster_motion.h';lines=header.read_text().splitlines()
    begin=next(i for i,s in enumerate(lines) if 'extraPivot' in s)+1
    lines[begin+2]='    {'+', '.join(map(str,pivots))+'},'
    end=next(i for i,s in enumerate(lines) if 'extraHeight' in s)
    values=re.search(r'\{(.*?)\}',lines[end]).group(1).split(', ')
    values[2]=str(round(idle*scale))
    lines[end]=re.sub(r'\{.*?\}','{'+', '.join(values)+'}',lines[end])
    header.write_text('\n'.join(lines)+'\n',encoding='ascii')

if __name__=='__main__':
    parser=argparse.ArgumentParser()
    for key in ('portrait','scene','poses'):parser.add_argument(key,type=Path)
    pack(**vars(parser.parse_args()))
