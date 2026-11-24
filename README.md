# Image Steganography in C

A command-line tool written in C that hides a secret text file inside a BMP image and extracts it again. The image looks the same to the human eye.

## How it works

A BMP pixel is stored as bytes, and each byte is 8 bits. This project changes only the **last bit (LSB)** of each byte, which changes a color by at most 1 out of 255, so the difference is invisible.

- **Encoding:** the secret's bits are written into the last bits of the image bytes.
- **Decoding:** the last bits are read back and the secret file is rebuilt.

## Features

- Encode a small text file into a 24-bit BMP image
- Decode the hidden file back, with its original extension
- Capacity check: refuses a secret that is too large for the image
- Magic string check: detects images with no hidden data
- Optional output file names

## Project structure

```
.
├── test_encode.c   # main(): reads the command line, calls encode or decode
├── encode.c / .h   # encoding functions
├── decode.c / .h   # decoding functions
├── common.h        # MAGIC_STRING ("#*")
└── types.h         # Status, OperationType, uint
```

## Build

```bash
gcc *.c
```

## Usage

**Encode**
```bash
./a.out -e beautiful.bmp secret.txt            # output: default.bmp
./a.out -e beautiful.bmp secret.txt stego.bmp  # custom output name
```

**Decode**
```bash
./a.out -d default.bmp                         # output: output.txt
./a.out -d default.bmp mysecret                # output: mysecret.txt
```

**Check that it worked**
```bash
diff secret.txt output.txt     # no output means the files are identical
```

## Data layout inside the image

The data is stored right after the 54-byte BMP header, in this order:

| Part | Size in image bytes | Purpose |
|---|---|---|
| BMP header | 54 (copied unchanged) | keeps the file a valid image |
| Magic string `#*` | 16 | marks that a secret exists |
| Extension size | 32 | length of the extension (e.g. 4 for `.txt`) |
| Extension | extension length x 8 | e.g. `.txt` |
| Secret file size | 32 | how many bytes to read |
| Secret data | file size x 8 | the actual secret |
| Remaining pixels | rest of the image | copied unchanged |

One bit needs one image byte, so one character needs 8 bytes and one 32-bit number needs 32 bytes.

## Encoding steps

1. Validate command-line arguments
2. Open the source image, secret file and output image
3. Check that the image has enough capacity
4. Copy the 54-byte BMP header
5. Encode the magic string
6. Encode the extension size and the extension
7. Encode the secret file size
8. Encode the secret data
9. Copy the remaining image data and close the files

## Decoding steps

1. Validate command-line arguments
2. Open the stego image and skip the 54-byte BMP header
3. Decode and verify the magic string
4. Decode the extension size and the extension
5. Create the output file using the decoded extension
6. Decode the secret file size
7. Decode the secret data and write it to the output file

## Capacity formula

```
image bytes >= 16 + 32 + (extension length x 8) + 32 + (secret size x 8)
```
