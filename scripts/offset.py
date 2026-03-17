import numpy as np
from PIL import Image
import os

# ===== CONFIG =====
input_file = "input.tiff"
output_dir = "out"

offsets = [-3, -2, -1, 0, 1, 2, 3]
gray_method = "weighted"  # "weighted", "median", "green"
# ==================

os.makedirs(output_dir, exist_ok=True)

img = Image.open(input_file)
img = np.array(img)

assert img.ndim == 3 and img.shape[2] == 3, "Image must be RGB"

R = img[:, :, 0]
G = img[:, :, 1]
B = img[:, :, 2]


def shift_channel(channel, offset):
    """Shift horizontally with edge clamp"""
    if offset == 0:
        return channel

    result = np.empty_like(channel)

    if offset > 0:
        result[:, offset:] = channel[:, :-offset]
        result[:, :offset] = channel[:, :1]
    else:
        offset = -offset
        result[:, :-offset] = channel[:, offset:]
        result[:, -offset:] = channel[:, -1:]

    return result


def to_gray(r, g, b):
    if gray_method == "weighted":
        return (0.3 * r + 0.59 * g + 0.11 * b).astype(r.dtype)
    elif gray_method == "median":
        return np.median(np.stack([r, g, b]), axis=0).astype(r.dtype)
    elif gray_method == "green":
        return g
    else:
        raise ValueError("Unknown gray method")


print("Generating test TIFFs...")

# Fix green as reference (faster search)
for r_off in offsets:
    for b_off in offsets:
        g_off = 0

        Rs = shift_channel(R, r_off)
        Gs = shift_channel(G, g_off)
        Bs = shift_channel(B, b_off)

        gray = to_gray(Rs, Gs, Bs)

        out = Image.fromarray(gray)

        filename = f"{output_dir}/gray_r{r_off}_g{g_off}_b{b_off}.tif"
        out.save(filename)

print("Done.")