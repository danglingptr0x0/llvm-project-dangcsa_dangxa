import sys

path = sys.argv[1] if len(sys.argv) > 1 else "test.hex"
limit = int(sys.argv[2]) if len(sys.argv) > 2 else 0

out = []
idx = 0

with open(path, "r") as fh:
    for line in fh:
        text = line.strip()
        if not text:
            continue
        if limit and idx >= limit:
            break
        word = int(text, 16)
        for i in range(8):
            out.append("0x%02x" % ((word >> (i * 8)) & 0xFF))
        idx += 1

sys.stdout.write(" ".join(out) + "\n")
