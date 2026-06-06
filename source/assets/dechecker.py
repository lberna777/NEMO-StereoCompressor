#!/usr/bin/env python3
"""Rimuove la scacchiera (ex-trasparenza appiattita da WhatsApp) da un cutout.
Strategia: flood-fill dai bordi sui pixel 'background-like' (grigi e chiari).
Il centro chiaro di un oggetto resta opaco perche' racchiuso da bordi scuri."""
import sys
import numpy as np
from PIL import Image
from scipy import ndimage

def key(path, lum_t, sat_t, feather=1.0, min_area=400):
    im = Image.open(path).convert("RGB")
    a = np.asarray(im).astype(np.float32)
    R, G, B = a[..., 0], a[..., 1], a[..., 2]
    lum = 0.299 * R + 0.587 * G + 0.114 * B
    sat = a.max(2) - a.min(2)               # proxy saturazione
    fillable = (lum > lum_t) & (sat < sat_t)

    # componenti connesse dei pixel 'background-like'
    lbl, n = ndimage.label(fillable)
    border = set(np.unique(np.concatenate([
        lbl[0, :], lbl[-1, :], lbl[:, 0], lbl[:, -1]])))
    border.discard(0)
    bg = np.isin(lbl, list(border))

    alpha = np.where(bg, 0.0, 255.0).astype(np.float32)
    # erosione 1px per togliere la frangia mista metallo/scacchiera
    opaque = alpha > 127
    opaque = ndimage.binary_erosion(opaque, iterations=1)
    # despeckle: elimina i frammenti opachi piccoli (residui di scacchiera)
    ol, on = ndimage.label(opaque)
    if on > 0:
        sizes = ndimage.sum(np.ones_like(ol), ol, range(1, on + 1))
        keep = {i + 1 for i, s in enumerate(sizes) if s >= min_area}
        opaque = np.isin(ol, list(keep))
    alpha = np.where(opaque, 255.0, 0.0)
    # feather morbido per anti-alias
    if feather > 0:
        alpha = ndimage.gaussian_filter(alpha, feather)

    out = np.dstack([a, alpha]).astype(np.uint8)
    return Image.fromarray(out, "RGBA")

def preview(rgba, bg=(255, 0, 255)):
    base = Image.new("RGB", rgba.size, bg)
    base.paste(rgba, (0, 0), rgba)
    return base

if __name__ == "__main__":
    src = sys.argv[1]
    lum_t = float(sys.argv[2]); sat_t = float(sys.argv[3])
    out = sys.argv[4]
    img = key(src, lum_t, sat_t)
    img.save(out)
    preview(img).save(out.replace(".png", "_prev.png"))
    print("saved", out, img.size)
