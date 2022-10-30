
import io
import sys
import os
import zlib

def dumpfile(buff, outpath, length, block_size):
	blocks = (length + (block_size - 1)) // block_size
	block_sizes = []
	for i in range(blocks):
		block_sizes.append(int.from_bytes(buff.read(4), byteorder="little"))
	with open(outpath, "wb") as wf:
		for i in range(blocks):
			wf.write(zlib.decompress(buff.read(block_sizes[i])))

def dumppath(buff, outpath):
	os.makedirs(name=outpath, exist_ok=True)
	num_entries = int.from_bytes(buff.read(4), byteorder="little")
	for i in range(num_entries):
		fn = buff.read(36).rstrip(b"\x00").decode("ASCII")

		should_output = True
		if ("/" in fn) or ("\\" in fn) or (":" in fn):
			should_output = False
		if fn == "." or fn == "..":
			should_output = False

		offset_of_data = int.from_bytes(buff.read(4), byteorder="little")
		decompressed_size = int.from_bytes(buff.read(4), byteorder="little")
		block_size = int.from_bytes(buff.read(4), byteorder="little")

		if should_output:
			cur_pos = buff.tell()
			buff.seek(offset_of_data)
			if block_size != 0:
				dumpfile(buff, outpath + fn, decompressed_size, block_size)
			else:
				dumppath(buff, outpath + fn + "/")
			buff.seek(cur_pos)

def dumppsf2(infile, outpath):
	if outpath == "":
		outpath = "."

	with open(infile, "rb") as f:
		hdr = f.read(4)
		if hdr != b"PSF\x02":
			raise Exception("Not a PSF2 file!")
		bufsize = int.from_bytes(f.read(4), byteorder="little")
		unused1 = f.read(8)
		dumppath(io.BytesIO(f.read(bufsize)), outpath + "/")

if __name__ == "__main__":
	infile = ""
	outpath = "."
	if len(sys.argv) > 1:
		infile = sys.argv[1]
	else:
		raise Exception("Please specify input file")
	if len(sys.argv) > 2:
		outpath = sys.argv[2]
	dumppsf2(infile, outpath)
