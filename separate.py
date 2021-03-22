import os
import hashlib

def reverse_hash(h):
	ret = ''
	for i in range(len(h) - 2, -1, -2):
		ret += h[i]
		ret += h[i+1]
	return ret

def log(string):
	f = open("separate.log", "at")
	print(string, file = f)
	f.close()

files = os.listdir('blocks_raw')

for name in files:
	f = open("blocks_raw/{}".format(name), "rb")

	cnt = 0
	while True:
		magic = int.from_bytes(f.read(4), "little")
		if magic != 0xd9b4bef9:
			break

		block_size = int.from_bytes(f.read(4), "little")

		block = f.read(block_size)
		prev_hash = reverse_hash(block[4:36].hex())

		ff = open("blocks/" + prev_hash, "wb")
		ff.write(block)
		ff.close()
	f.close()

	os.system("rm blocks_raw/{}".format(name))
	log("Done: {}".format(name))
