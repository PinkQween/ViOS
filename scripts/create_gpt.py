#!/usr/bin/env python3
"""Create GPT partition table on disk image"""
import struct
import os
import sys
import binascii
import uuid

def crc32(data):
    return binascii.crc32(data) & 0xffffffff

def create_gpt(img_path):
    sector_size = 512
    img_size = os.path.getsize(img_path)
    total_sectors = img_size // sector_size
    
    with open(img_path, 'r+b') as f:
        # Write protective MBR (sector 0)
        mbr = bytearray(512)
        mbr[446:446+16] = struct.pack('<B3sB3sLL',
            0x00,           # Status
            b'\x00\x02\x00', # CHS first
            0xEE,           # Type (GPT protective)
            b'\xFF\xFF\xFF', # CHS last
            1,              # LBA first sector
            total_sectors - 1  # LBA number of sectors
        )
        mbr[510:512] = b'\x55\xAA'
        f.write(mbr)

        # Create partition entries
        partition_entries = bytearray(128 * 128)

        # Partition 1: EFI System Partition
        partition_entries[0:16] = bytes.fromhex('28732ac11ff8d211ba4b00a0c93ec93b')
        partition_entries[16:32] = uuid.uuid4().bytes
        partition_entries[32:40] = struct.pack('<Q', 2048)  # First LBA
        partition_entries[40:48] = struct.pack('<Q', 2048 + 716800 - 1)  # Last LBA
        partition_entries[48:56] = struct.pack('<Q', 0)  # Attributes
        partition_entries[56:56+72] = 'ABC'.encode('utf-16le').ljust(72, b'\x00')

        # Partition 2: Data partition
        offset = 128
        part2_first_lba = 718848
        part2_last_lba = total_sectors - 34
        partition_entries[offset:offset+16] = bytes.fromhex('28732ac11ff8d211ba4b00a0c93ec93b')
        partition_entries[offset+16:offset+32] = uuid.uuid4().bytes
        partition_entries[offset+32:offset+40] = struct.pack('<Q', part2_first_lba)
        partition_entries[offset+40:offset+48] = struct.pack('<Q', part2_last_lba)
        partition_entries[offset+48:offset+56] = struct.pack('<Q', 0)
        partition_entries[offset+56:offset+56+72] = 'ViOS'.encode('utf-16le').ljust(72, b'\x00')

        partition_array_crc = crc32(partition_entries)

        # Write GPT header (sector 1)
        f.seek(512)
        gpt_header = bytearray(512)
        gpt_header[0:8] = b'EFI PART'
        gpt_header[8:12] = struct.pack('<I', 0x00010000)
        gpt_header[12:16] = struct.pack('<I', 92)
        gpt_header[20:24] = struct.pack('<I', 0)
        gpt_header[24:32] = struct.pack('<Q', 1)
        gpt_header[32:40] = struct.pack('<Q', total_sectors - 1)
        gpt_header[40:48] = struct.pack('<Q', 34)
        gpt_header[48:56] = struct.pack('<Q', total_sectors - 34)
        gpt_header[56:72] = uuid.uuid4().bytes
        gpt_header[72:80] = struct.pack('<Q', 2)
        gpt_header[80:84] = struct.pack('<I', 128)
        gpt_header[84:88] = struct.pack('<I', 128)
        gpt_header[88:92] = struct.pack('<I', partition_array_crc)

        header_crc = crc32(gpt_header[0:92])
        gpt_header[16:20] = struct.pack('<I', header_crc)
        f.write(gpt_header)

        # Write partition entries (starting at sector 2)
        f.seek(1024)
        f.write(partition_entries)

        # Write backup GPT
        backup_entries_lba = total_sectors - 33
        f.seek(backup_entries_lba * 512)
        f.write(partition_entries)

        backup_header = bytearray(512)
        backup_header[0:8] = b'EFI PART'
        backup_header[8:12] = struct.pack('<I', 0x00010000)
        backup_header[12:16] = struct.pack('<I', 92)
        backup_header[20:24] = struct.pack('<I', 0)
        backup_header[24:32] = struct.pack('<Q', total_sectors - 1)
        backup_header[32:40] = struct.pack('<Q', 1)
        backup_header[40:48] = struct.pack('<Q', 34)
        backup_header[48:56] = struct.pack('<Q', total_sectors - 34)
        backup_header[56:72] = gpt_header[56:72]
        backup_header[72:80] = struct.pack('<Q', backup_entries_lba)
        backup_header[80:84] = struct.pack('<I', 128)
        backup_header[84:88] = struct.pack('<I', 128)
        backup_header[88:92] = struct.pack('<I', partition_array_crc)

        backup_header_crc = crc32(backup_header[0:92])
        backup_header[16:20] = struct.pack('<I', backup_header_crc)

        f.seek((total_sectors - 1) * 512)
        f.write(backup_header)

    print(f"GPT structure created on {img_path}")

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <disk_image>")
        sys.exit(1)
    
    create_gpt(sys.argv[1])
