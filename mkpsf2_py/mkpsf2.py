
import sys
import os
import zlib

# see if we can overwrite the given filename
def overwrite_check(filename):
	try:
		with open(filename, "rb") as f:
			read_bytes = f.read(4)
			if len(read_bytes) != 4:
				print("can't verify whether %s is overwritable; quitting" % filename)
			if read_bytes != b"PSF\x02":
				print("%s exists and is not PSF2; will not overwrite" % filename)
				return -1
	except:
		pass
	return 0

def put32lsb(offs, p, value):
	p[offs + 0] = (value >>  0) & 0xFF
	p[offs + 1] = (value >>  8) & 0xFF
	p[offs + 2] = (value >> 16) & 0xFF
	p[offs + 3] = (value >> 24) & 0xFF

def get32lsb(offs, p):
	return (
		((((p[offs + 0])) & 0xFF) <<  0) |
		((((p[offs + 1])) & 0xFF) <<  8) |
		((((p[offs + 2])) & 0xFF) << 16) |
		((((p[offs + 3])) & 0xFF) << 24)
	)

reserved_buffer = bytearray()

def buildfile(filename, length, block_size):

	print("[%d] file: %s" % (len(reserved_buffer), filename))

	blocks = (length + (block_size - 1)) // block_size
	base = len(reserved_buffer)
	reserved_buffer.extend(b"\x00" * (4 * blocks))
	with open(filename, "rb") as f:
		for b in range(blocks):
			l = length - (b * block_size);
			if l > block_size:
				l = block_size
			block_buffer = f.read(l)
			if len(block_buffer) != l:
				print("error reading position %d" % (b*block_size))
				return -1
			block_zbuffer = b""
			try:
				block_zbuffer = zlib.compress(block_buffer, level=9)
			except:
				print("error compressing position %d" % (b*block_size))
				return -1
			put32lsb(base + 4 * b, reserved_buffer, len(block_zbuffer))
			reserved_buffer.extend(block_zbuffer)
	return 0

def builddir(dirname, block_size):
	base = len(reserved_buffer)
	n_entries = 0

	print("[%d] entering %s..." % (len(reserved_buffer), dirname))

	lastpath = None

	try:
		lastpath = os.getcwd()
	except:
		print("unable to save previous dir; giving up")
		return -1

	# try:
	os.chdir(dirname)
	# except:
	# 	print("unable to switch to directory; giving up")
	# 	return -1

	# allocate space for the number of entries
	reserved_buffer.extend(b"\x00" * (4))

	# pass A: build the directory list
	with os.scandir(".") as it:
		for ent in it:
			if ent.name == ".":
				continue
			if ent.name == "..":
				continue
			if len(ent.name) > 36:
				print("filename '%s' is too long" % ent.name)
				return -1
			ofs = len(reserved_buffer)
			name_encoded = ent.name.encode("ASCII")
			name_encoded += b"\x00" * (36 - len(name_encoded))
			reserved_buffer.extend(name_encoded)
			reserved_buffer.extend(b"\x00" * (12))
			if ent.is_dir():
				put32lsb(ofs + 36, reserved_buffer, 1)
			else:
				l = ent.stat().st_size
				put32lsb(ofs + 40, reserved_buffer, l)
				put32lsb(ofs + 44, reserved_buffer, block_size)
			n_entries += 1
	put32lsb(base, reserved_buffer, n_entries)

	# pass B: include files and traverse subdirectories
	for n in range(n_entries):
		ofs = base + 4 + 48 * n
		name_encoded = reserved_buffer[ofs:ofs + 36]
		name_encoded = name_encoded.rstrip(b"\x00")
		name = name_encoded.decode("ASCII")
		is_sub = get32lsb(ofs + 36, reserved_buffer) != 0
		put32lsb(ofs + 36, reserved_buffer, len(reserved_buffer))
		if is_sub:
			# subdirectory
			# recursively traverse
			if builddir(name, block_size) < 0:
				return -1
		else:
			# file
			l = get32lsb(ofs + 40, reserved_buffer)
			if l != 0:
				if buildfile(name, l, block_size) < 0:
					return -1

	try:
		os.chdir(lastpath)
	except:
		print("unable to switch to previous directory; giving up")
		return -1

	print("[%d] exiting %s (%d %s)" % (len(reserved_buffer), dirname, n_entries, "entry" if n_entries == 1 else "entries"))

	return 0

def psf2create(psf2filename, dirname):

	if overwrite_check(psf2filename) < 0:
		return -1
	if builddir(dirname, 32768) < 0:
		return -1
	print("[%d] finished" % len(reserved_buffer))
	print("creating %s..." % psf2filename)
	with open(psf2filename, "wb") as f:
		r = len(reserved_buffer)
		f.write(b"PSF\x02")
		# Size of reserved area
		f.write(bytes(((r >>  0) & 0xFF, )))
		f.write(bytes(((r >>  8) & 0xFF, )))
		f.write(bytes(((r >> 16) & 0xFF, )))
		f.write(bytes(((r >> 24) & 0xFF, )))
		# Compressed program length
		f.write(b"\x00" * 4)
		# Compressed program CRC32
		f.write(b"\x00" * 4)
		f.write(reserved_buffer)
	print("done")
	return 0

def main(argv):
	if len(argv) < 3:
		print("usage: %s psf2file directory", argv[0])
		return 1
	print(argv[1], argv[2])
	r = psf2create(argv[1], argv[2])
	if r != 0:
		return r
	return 0

if __name__ == "__main__":
	main(sys.argv)








































