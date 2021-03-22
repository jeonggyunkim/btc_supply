import hashlib

def VarInt(f):
	ret = b''
	data = f.read(1)
	ret += data
	first = int.from_bytes(data, "little")
	if first < 0xfd:
		return (first, ret)
	elif first == 0xfd:
		data = f.read(2)
	elif first == 0xfe:
		data = f.read(4)
	else:
		data = f.read(8)
	ret += data
	return (int.from_bytes(data, "little"), ret)

def reverse_hash(h):
	ret = ''
	for i in range(len(h) - 2, -1, -2):
		ret += h[i]
		ret += h[i+1]
	return ret

f = open("blocks/0000000000000000000000000000000000000000000000000000000000000000", "rb");

while True:
	header = f.read(80)
	print(hashlib.sha256(header).hexdigest())
	print(list(hashlib.sha256(header).digest()))
	header_hash = hashlib.sha256(hashlib.sha256(header).digest()).hexdigest()
	print(reverse_hash(header_hash))

	break

	tx_count, data = VarInt(f)
	print("Tx number:", tx_count)

	for _ in range(tx_count):
		tx = b''
		data = f.read(4)
		tx += data
		tx_version = int.from_bytes(data, "little")
#	flag = int.from_bytes(f.read(2), "little")

		in_count, data = VarInt(f)
		tx += data
		
		print("Input:", in_count)
		for i in range(in_count):
			prev_txhash = f.read(32)
			tx += prev_txhash
			print("prev_txhash:", reverse_hash(prev_txhash.hex()))

			data = f.read(4)
			tx += data
			prev_txout_index = int.from_bytes(data, "little")
			print("prev_txout_index:", prev_txout_index)

			script_length, data = VarInt(f)
			tx += data

			script = f.read(script_length)
			tx += script

			data = f.read(4)
			tx += data
			seq_no = int.from_bytes(data, "little")

		out_count, data = VarInt(f)
		tx += data
		print("Output:", out_count)
		for o in range(out_count):
			data = f.read(8)
			tx += data
			value = int.from_bytes(data, "little")
			print("value:", value)

			script_length, data = VarInt(f)
			tx += data

			script = f.read(script_length)
			tx += script

		data = f.read(4)
		tx += data
		lock_time = int.from_bytes(data, "little")
		print("Lock time:", lock_time)

		print("Tx hash:", reverse_hash(hashlib.sha256(hashlib.sha256(tx).digest()).hexdigest()))
	print()

f.close()
