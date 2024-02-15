#!/usr/bin/env python
import sys
import os
import subprocess

from fontTools.ttLib import TTFont

TEXTS_DIR = "texts"
IMAGES_DIR = "images"

TTF_PATH = sys.argv[1]
FONT_SIZE = sys.argv[2]
TTF_NAME, TTF_EXT = os.path.splitext(os.path.basename(TTF_PATH))

ttf = TTFont(TTF_PATH, 0, verbose=0, allowVID=0, ignoreDecompileErrors=True, fontNumber=-1)

for d in [TEXTS_DIR, IMAGES_DIR]:
    if not os.path.isdir(d):
        os.mkdir(d)

for x in ttf["cmap"].tables:
    for y in x.cmap.items():
        char_unicode = unichr(y[0])
        char_utf8 = char_unicode.encode('utf_8')
        char_name = y[1]
        f = open(os.path.join(TEXTS_DIR, char_name + '.txt'), 'w')
        f.write(char_utf8)
        f.close()
ttf.close()

files = os.listdir(TEXTS_DIR)
for filename in files:
    name, ext = os.path.splitext(filename)
    input_txt = TEXTS_DIR + "/" + filename
    output_png = IMAGES_DIR + "/" + TTF_NAME + "_" + name + "_" + FONT_SIZE + ".png"
    subprocess.call(["convert", "-font", TTF_PATH, "-pointsize", FONT_SIZE, "-background", "rgba(0,0,0,0)", "label:@" + input_txt, output_png])

print("finished")
