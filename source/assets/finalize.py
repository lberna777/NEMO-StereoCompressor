#!/usr/bin/env python3
"""Genera gli asset finali con alpha, ritagliati al contenuto."""
import numpy as np
from PIL import Image
from scipy import ndimage
from dechecker import key

def autocrop(rgba, pad=6):
    a = np.asarray(rgba)
    ys, xs = np.where(a[..., 3] > 16)
    y0, y1 = ys.min(), ys.max(); x0, x1 = xs.min(), xs.max()
    y0 = max(0, y0 - pad); x0 = max(0, x0 - pad)
    y1 = min(a.shape[0] - 1, y1 + pad); x1 = min(a.shape[1] - 1, x1 + pad)
    return rgba.crop((x0, y0, x1 + 1, y1 + 1))

# ── KNOB: scontorna + ritaglia quadrato centrato sul knob ──
knob = key("src_knob.jpg", 150, 45, min_area=2000)
knob = autocrop(knob, pad=4)
# rendi quadrato (centro = centro bbox) per rotazione pulita
w, h = knob.size
s = max(w, h)
sq = Image.new("RGBA", (s, s), (0, 0, 0, 0))
sq.paste(knob, ((s - w) // 2, (s - h) // 2), knob)
sq.save("nemo_knob.png")
print("nemo_knob.png", sq.size)

# ── FADER CAP ──
fader = key("src_fader.jpg", 150, 45, min_area=2000)
fader = autocrop(fader, pad=4)
fader.save("nemo_fader.png")
print("nemo_fader.png", fader.size)

# ── PHASE: scontorna, isola le levette, estrai stato OFF (giu') e ON (su) ──
ph = key("src_phase.jpg", 150, 45, min_area=1500)
a = np.asarray(ph)
mask = a[..., 3] > 16
lbl, n = ndimage.label(mask)
comps = []
for i in range(1, n + 1):
    ys, xs = np.where(lbl == i)
    comps.append((i, ys.min(), ys.max(), xs.min(), xs.max(), len(ys)))
# tieni i 4 piu' grandi
comps.sort(key=lambda c: -c[5])
comps = comps[:4]
# ordina per riga (y) poi colonna (x): top-left, top-right, ...
comps.sort(key=lambda c: (c[1] // 50, c[3]))
def crop_comp(c, pad=6):
    _, y0, y1, x0, x1, _ = c
    return ph.crop((max(0, x0 - pad), max(0, y0 - pad), x1 + pad, y1 + pad))
# top-left = OFF (levetta inclinata giu'), top-right = ON (levetta su)
crop_comp(comps[0]).save("nemo_phase_off.png")
crop_comp(comps[1]).save("nemo_phase_on.png")
print("nemo_phase_off.png", "nemo_phase_on.png", "ok")
