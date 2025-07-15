#!/usr/bin/env python3
"""
FAT32 Filesystem Initialization Script for ViOS
This script creates a proper FAT32 filesystem structure according to the VBR fix checklist:
1. VBR sector (sector 0 of partition)
2. FSInfo sector (sector 1 after VBR)
3. Backup VBR (sector 6 after VBR)
4. Reserved sectors cleared (sectors 2-5 and 7-31)
5. FAT1 and FAT2 tables initialized
6. Root directory cluster allocated & empty or with kernel file
"""

import struct
import os
import sys

class FAT32Initializer:
    def verify_vbr_written(self, partition_offset, vbr_sector):
        """Verify that the VBR sector in the disk image matches the assembled vbr.bin"""
        with open(self.image_path, 'rb') as f:
            f.seek(partition_offset)
            disk_vbr = f.read(512)
        if disk_vbr != vbr_sector:
            print("WARNING: VBR sector in disk image does not match assembled vbr.bin!")
        else:
            print("✓ Verified VBR sector matches assembled vbr.bin")
    def __init__(self, image_path, partition_start_lba=2048):
        self.image_path = image_path
        self.partition_start_lba = partition_start_lba
        self.bytes_per_sector = 512
        self.sectors_per_cluster = 8
        self.reserved_sectors = 32
        self.num_fats = 2
        # Calculate proper sectors per FAT for FAT32
        # Total sectors: 262144, Reserved: 32, Data clusters need > 65525 for FAT32
        self.sectors_per_fat = 512  # Increased to ensure FAT32 detection
        self.root_cluster = 2
        self.fsinfo_sector = 1
        self.backup_boot_sector = 6
        
        # Calculate some derived values
        self.fat_start_sector = self.reserved_sectors
        self.fat2_start_sector = self.fat_start_sector + self.sectors_per_fat
        self.data_start_sector = self.fat_start_sector + (self.num_fats * self.sectors_per_fat)
        self.cluster_size = self.sectors_per_cluster * self.bytes_per_sector
        
    def load_vbr_bin(self, path):
        """Load assembled VBR binary from file"""
        with open(path, 'rb') as f:
            data = f.read()
        if len(data) != 512:
            raise ValueError(f"VBR binary {path} is not 512 bytes!")
        return data
    
    def create_fsinfo_sector(self):
        """Create the FSInfo sector"""
        fsinfo = bytearray(512)
        
        # Lead signature
        fsinfo[0:4] = b'RRaA'
        
        # Reserved area (484 bytes of zeros already there)
        
        # Struct signature
        fsinfo[484:488] = b'rrAa'
        
        # Free cluster count (0xFFFFFFFF = unknown)
        struct.pack_into('<L', fsinfo, 488, 0xFFFFFFFF)
        
        # Next free cluster (0xFFFFFFFF = unknown)
        struct.pack_into('<L', fsinfo, 492, 0xFFFFFFFF)
        
        # Reserved (12 bytes of zeros already there)
        
        # Trail signature
        fsinfo[508:512] = b'\x00\x00\x55\xAA'
        
        return bytes(fsinfo)
    
    def create_fat_table(self):
        """Create a FAT table"""
        fat = bytearray(self.sectors_per_fat * self.bytes_per_sector)
        
        # FAT32 entries are 32-bit (4 bytes each)
        # First two entries are reserved
        struct.pack_into('<L', fat, 0, 0x0FFFFFF8)   # Media descriptor + end of chain
        struct.pack_into('<L', fat, 4, 0x0FFFFFFF)   # End of chain
        
        # Root directory cluster (cluster 2) - mark as end of chain
        struct.pack_into('<L', fat, 8, 0x0FFFFFFF)   # End of chain for root cluster
        
        # All other entries are 0 (free clusters)
        
        return bytes(fat)
    
    def create_root_directory(self):
        """Create an empty root directory cluster"""
        root_dir = bytearray(self.cluster_size)
        
        # Create volume label entry
        volume_entry = bytearray(32)
        volume_entry[0:11] = b'VIOS FAT32 '  # Volume label
        volume_entry[11] = 0x08              # Volume label attribute
        
        root_dir[0:32] = volume_entry
        
        # Rest of the cluster is empty (zeros)
        
        return bytes(root_dir)
    
    def initialize_filesystem(self):
        """Initialize the complete FAT32 filesystem, using assembled VBR binary"""
        print(f"Initializing FAT32 filesystem in {self.image_path}")
        vbr_bin_path = os.path.join(os.path.dirname(__file__), '../bin/vbr.bin')
        vbr_sector = self.load_vbr_bin(vbr_bin_path)
        
        # Open the disk image
        with open(self.image_path, 'r+b') as f:
            # Calculate partition offset
            partition_offset = self.partition_start_lba * self.bytes_per_sector

            # 1. Write VBR sector (sector 0 of partition)
            f.seek(partition_offset)
            f.write(vbr_sector)
            print("✓ VBR sector written (from assembled vbr.bin)")
            # Verify VBR sector
            self.verify_vbr_written(partition_offset, vbr_sector)

            # 2. Write FSInfo sector (sector 1 after VBR)
            f.seek(partition_offset + self.fsinfo_sector * self.bytes_per_sector)
            f.write(self.create_fsinfo_sector())
            print("✓ FSInfo sector written")

            # 3. Clear reserved sectors 2-5 and 7-31
            empty_sector = b'\x00' * self.bytes_per_sector
            for sector in range(2, 6):  # sectors 2-5
                f.seek(partition_offset + sector * self.bytes_per_sector)
                f.write(empty_sector)
            for sector in range(7, self.reserved_sectors):  # sectors 7-31
                f.seek(partition_offset + sector * self.bytes_per_sector)
                f.write(empty_sector)
            print("✓ Reserved sectors cleared")

            # 4. Write backup VBR (sector 6 after VBR)
            f.seek(partition_offset + self.backup_boot_sector * self.bytes_per_sector)
            f.write(vbr_sector)
            print("✓ Backup VBR written (from assembled vbr.bin)")

            # 5. Write FAT1 and FAT2 tables
            fat_table = self.create_fat_table()

            # Write FAT1
            f.seek(partition_offset + self.fat_start_sector * self.bytes_per_sector)
            f.write(fat_table)
            print("✓ FAT1 table written")

            # Write FAT2
            f.seek(partition_offset + self.fat2_start_sector * self.bytes_per_sector)
            f.write(fat_table)
            print("✓ FAT2 table written")

            # 6. Write root directory cluster
            root_dir = self.create_root_directory()
            root_cluster_lba = self.data_start_sector + (self.root_cluster - 2) * self.sectors_per_cluster
            f.seek(partition_offset + root_cluster_lba * self.bytes_per_sector)
            f.write(root_dir)
            print("✓ Root directory cluster written")

            # Sync to disk
            f.flush()
            os.fsync(f.fileno())
        
        print("✓ FAT32 filesystem initialization complete!")
        print(f"  - Partition starts at LBA {self.partition_start_lba}")
        print(f"  - Reserved sectors: {self.reserved_sectors}")
        print(f"  - FAT1 starts at sector {self.fat_start_sector}")
        print(f"  - FAT2 starts at sector {self.fat2_start_sector}")
        print(f"  - Data starts at sector {self.data_start_sector}")
        print(f"  - Root cluster: {self.root_cluster}")

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 fat32_init.py <disk_image_path>")
        sys.exit(1)
    
    image_path = sys.argv[1]
    
    if not os.path.exists(image_path):
        print(f"Error: Disk image {image_path} does not exist")
        sys.exit(1)
    
    initializer = FAT32Initializer(image_path)
    initializer.initialize_filesystem()

if __name__ == "__main__":
    main()
