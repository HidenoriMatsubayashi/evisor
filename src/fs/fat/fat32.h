#ifndef EVISOR_FS_FAT_FAT32_H_
#define EVISOR_FS_FAT_FAT32_H_

#include <cstdbool>
#include <cstddef>
#include <cstdint>

// clang-format off
#define MAX_FAT_VOLUME_SIZE        4
// clang-format on

namespace evisor {

// BIOS Parameter Block (BPB) for FAT32
typedef struct {
  uint8_t BS_JmpBoot[3];
  uint8_t BS_OEMName[8];
  uint16_t BPB_BytsPerSec;
  uint8_t BPB_SecPerClus;
  uint16_t BPB_RsvdSecCnt;
  uint8_t BPB_NumFATs;
  uint16_t BPB_RootEntCnt;
  uint16_t BPB_TotSec16;
  uint8_t BPB_Media;
  uint16_t BPB_FATSz16;
  uint16_t BPB_SecPerTrk;
  uint16_t BPB_NumHeads;
  uint32_t BPB_HiddSec;
  uint32_t BPB_TotSec32;
  uint32_t BPB_FATSz32;
  uint16_t BPB_ExtFlags;
  uint16_t BPB_FSVer;
  uint32_t BPB_RootClus;
  uint16_t BPB_FSInfo;
  uint16_t BPB_BkBootSec;
  uint8_t BPB_Reserved[12];
  uint8_t BS_DrvNum;
  uint8_t BS_Reserved;
  uint8_t BS_BootSig;
  uint32_t BS_VolID;
  uint8_t BS_VolLab[11];
  uint8_t BS_FilSysType[8];
  uint8_t BS_BootCode32[420];
  uint16_t BS_BootSign;
} __attribute__((__packed__)) fat32_bpb_t;

struct ClusterLinkedListNode {
  uint32_t val;
  ClusterLinkedListNode* prev;
  ClusterLinkedListNode* next;
};

typedef struct {
  struct fat32_fat* fat;
  uint8_t attr;
  uint32_t size;
  ClusterLinkedListNode* cluster_root;
} fat32_file_t;

typedef struct fat32_fat {
  fat32_bpb_t bpb;
  fat32_file_t root;
  uint32_t first_lba;
  uint32_t fat_first_sector;
  uint32_t fat_sectors;
  uint32_t rde_first_sector;
  uint32_t rde_sectors;
  uint32_t data_first_sector;
  uint32_t data_sectors;
  bool valid;
} fat32_fat_t;

class Fat32Fs {
 public:
  Fat32Fs() = default;
  ~Fat32Fs() = default;

  bool Init();
  bool Open(fat32_file_t* result, const char* filename);
  int Read(fat32_file_t* file, void* buf, uint32_t offset, size_t len);
  int GetFileSize(fat32_file_t* file);
  int IsDirectory(fat32_file_t* file);

 private:
  // File System Infomation
  typedef struct {
    bool initialized;
    fat32_fat_t volume[MAX_FAT_VOLUME_SIZE];
  } fat32_fs_t;

  // Master Boot Record (MBR)
  typedef struct {
    uint8_t MBR_bootcode[446];
    struct {
      uint8_t PT_BootID;
      uint8_t PT_StartChs[3];
      uint8_t PT_System;
      uint8_t PT_EndChs[3];
      uint32_t PT_LbaOfs;
      uint32_t PT_LbaSize;
    } __attribute__((__packed__)) MBR_Partition[4];
    uint16_t MBR_Sig;
  } __attribute__((__packed__)) mbr_t;

  // Directory Entry
  typedef struct {
    uint8_t DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
  } __attribute__((__packed__)) fat32_dir_t;

  // Long File Name (LFN) Entry
  typedef struct {
    uint8_t LDIR_Ord;
    uint8_t LDIR_Name1[10];
    uint8_t LDIR_Attr;
    uint8_t LDIR_Type;
    uint8_t LDIR_Chksum;
    uint8_t LDIR_Name2[12];
    uint16_t LDIR_FstClusLO;
    uint8_t LDIR_Name3[4];
  } __attribute__((__packed__)) fat32_lfn_t;

  uint8_t* ReadSector(uint32_t lba, uint32_t sector_num);
  bool CheckBpb(fat32_bpb_t* bpb);

  inline void InitFile(fat32_file_t* file, fat32_fat_t* fat, uint8_t attr,
                       uint32_t size, uint32_t cluster);

  char* GetLfn(fat32_dir_t* entry, size_t offset, fat32_dir_t* prev_entry);
  uint8_t CreateLfnSum(fat32_dir_t* entry);
  char* GetSfn(fat32_dir_t* entry);

  uint32_t CalcCluster(fat32_fat_t* fat, ClusterLinkedListNode* cluster_list,
                       uint32_t offset);
  uint32_t CalcSector(fat32_fat_t* fat, uint32_t cluster, size_t offset);
  inline bool IsValidCluster(uint32_t cluster);
  int CalcNextSector(fat32_fat_t* fat, uint32_t cur_sector,
                     ClusterLinkedListNode* cluster_list);
  uint32_t CalcNextCluster(fat32_fat_t* fat,
                           ClusterLinkedListNode* cluster_list);

  fat32_fs_t fs_;
};

}  // namespace evisor

#endif  // EVISOR_FS_FAT_FAT32_H_
