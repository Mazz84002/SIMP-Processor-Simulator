import numpy as np
import matplotlib.pyplot as plt
import argparse

def visualize(monitor_text):
    f = open(monitor_text)
    pixels = f.readlines()
    pixels = [p.rstrip('\n') for p in pixels]
    f.close()
    
    while len(pixels) < 256*256:
        pixels.append("00\n")

    img = np.char.replace(np.array(pixels).reshape((256, 256)), 'FF', '255')
    img = np.char.replace(img, 'ff', '255').astype(int)

    plt.imshow(img)
    plt.show()
    plt.savefig('square.png')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--monitor_file", "-f")

    args = parser.parse_args()
    visualize(args.monitor_file)