#include "fat32.h"

#include <algorithm>

#include "common/cstring.h"
#include "common/logger.h"
#include "drivers/mmc/mmc.h"
#include "mm/new.h"
#include "mm/pgtable_stage1.h"
#include "mm/uncached/kmm_uncached_malloc.h"
#include "platforms/timer.h"

namespace evisor {

namespace {
constexpr uint32_t kBlockSize = 512;
constexpr uint32_t kFat32MaxFileNameSize = 255;

constexpr uint16_t kBootSignature = 0xAA55;

constexpr uint8_t kAttrReadOnly = 0x01;
constexpr uint8_t kAttrHidden = 0x02;
constexpr uint8_t kAttrSystem = 0x04;
constexpr uint8_t kAttrVolumeId = 0x08;
constexpr uint8_t kAttrLongName = 0x0f;
constexpr uint8_t kAttrDirectory = 0x10;
constexpr uint8_t kAttrArchive = 0x20;

constexpr uint8_t kDirNameEmptyEntry = 0x00;
constexpr uint8_t kDirNameKanjiEntry = 0x05;
constexpr uint8_t kDirNameRemovedEntry = 0xE5;

constexpr uint8_t kLfnLastLongEntry = 0x40;

constexpr uint32_t kUnusedCluster = 0;
constexpr uint32_t kReservedCluster = 1;
constexpr uint32_t kBadCluster = 0x0FFFFFF7;
}  // namespace

bool Fat32Fs::Init() {
  fs_.initialized = false;

#ifndef BOARD_IS_QEMU
  if (!Mmc::Get().Open()) {
    PANIC("Failed to open mmc device");
    return false;
  }
#endif  // BOARD_IS_QEMU

  uint8_t* buf = ReadSector(0, 1);
  {
    mbr_t* mbr = (mbr_t*)buf;

    // Check boot signature.
    if (mbr->MBR_Sig != kBootSignature) {
      LOG_ERROR("Invalid MBR_Sig (%x)", mbr->MBR_Sig);
      kmm_uncached_free(buf);
      return false;
    }

    for (int i = 0; i < MAX_FAT_VOLUME_SIZE; i++) {
      fs_.volume[i].valid = true;

      // Support FAT32X (LBA) only.
      if (mbr->MBR_Partition[i].PT_System != 0xc) {
        // LOG_TRACE("Not supported filesystem (%x)",
        //           mbr->MBR_Partition[i].PT_System);
        fs_.volume[i].valid = false;
      } else {
        fs_.volume[i].first_lba = mbr->MBR_Partition[0].PT_LbaOfs;
      }
    }
    kmm_uncached_free(buf);
  }

  for (int i = 0; i < MAX_FAT_VOLUME_SIZE; i++) {
    fat32_fat_t* cur = &fs_.volume[i];

    if (!cur->valid) {
      continue;
    }

    fat32_bpb_t* bpb = NULL;
    {
      buf = ReadSector(cur->first_lba, 1);
      cur->bpb = *(fat32_bpb_t*)buf;
      bpb = &(cur->bpb);
      kmm_uncached_free(buf);
    }

    if (!CheckBpb(bpb)) {
      cur->valid = false;
      continue;
    }

    // Calculate areas in partition
    {
      cur->fat_first_sector = bpb->BPB_RsvdSecCnt;
      cur->fat_sectors = bpb->BPB_FATSz32 * bpb->BPB_NumFATs;

      cur->rde_first_sector = cur->fat_first_sector + cur->fat_sectors;
      cur->rde_sectors = (sizeof(fat32_dir_t) * bpb->BPB_RootEntCnt +
                          bpb->BPB_BytsPerSec - 1) /
                         bpb->BPB_BytsPerSec;

      cur->data_first_sector = cur->rde_first_sector + cur->rde_sectors;
      cur->data_sectors = bpb->BPB_TotSec32 - cur->data_first_sector;
    }

    // Check FAT32 volume
    uint32_t clusters = cur->data_sectors / bpb->BPB_SecPerClus;
    if (clusters < 65526) {
      LOG_WARN("FAT32 clusters must be more than 65526");
      cur->valid = false;
      continue;
    }

    // Create root directory file for the user handle
    InitFile(&cur->root, cur, kAttrDirectory, 0, cur->bpb.BPB_RootClus);

#ifdef ENABLE_MMU
    {
      char volume_label[12];
      memcpy(volume_label, cur->bpb.BS_VolLab, sizeof(cur->bpb.BS_VolLab));
      volume_label[11] = '\0';
      LOG_TRACE("Found active volume: %s", volume_label);
    }
#endif
  }

  fs_.initialized = true;
  return true;
}

bool Fat32Fs::Open(fat32_file_t* result, const char* filename) {
  if (!fs_.initialized) {
    LOG_ERROR("Not initialized yet");
    return false;
  }

  // Currently support only the first partition (boot).
  // TODO: implement here
  const fat32_file_t* root = &fs_.volume[0].root;
  if (!(root->attr & kAttrDirectory)) {
    return false;
  }

  uint32_t cluster = root->cluster_root->val;
  auto* cluster_list = root->cluster_root;
  uint32_t sector = CalcSector(root->fat, cluster, 0);
  bool found = false;
  uint8_t* buf = NULL;
  uint8_t* prev_buf = NULL;

  // Traverse directory entries
  while (!found && IsValidCluster(cluster)) {
    buf = ReadSector(root->fat->first_lba + sector, 1);
    for (uint32_t i = 0; i < kBlockSize; i += sizeof(fat32_dir_t)) {
      fat32_dir_t* entry = (fat32_dir_t*)(buf + i);

      if (entry->DIR_Name[0] == kDirNameEmptyEntry) {
        // End of entry
        break;
      }

      if (entry->DIR_Name[0] == kDirNameRemovedEntry) {
        continue;
      }

      // If the entry is part of LFN, skip it since it will be checked after
      // finding SFN
      if (entry->DIR_Attr & kAttrLongName) {
        continue;
      }

      // Try to find LFN
      char* entry_filename = GetLfn(
          entry, i,
          // When crossing two sectors, it is necessary to refer to the previous
          // entry.
          prev_buf
              ? (fat32_dir_t*)(prev_buf + (kBlockSize - sizeof(fat32_dir_t)))
              : NULL);
      if (entry_filename == NULL) {
        entry_filename = GetSfn(entry);
      }

      // LOG_TRACE("Found file: %s", entry_filename);
      if (evisor::strncmp(filename, entry_filename, kFat32MaxFileNameSize) ==
          0) {
        uint32_t entry_cluster =
            ((uint32_t)entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
        if (entry_cluster == 0) {
          // root directory
          entry_cluster = root->fat->bpb.BPB_RootClus;
        }

        InitFile(result, root->fat, entry->DIR_Attr, entry->DIR_FileSize,
                 entry_cluster);
        found = true;
        break;
      }
    }

    if (prev_buf != NULL) {
      kmm_uncached_free(prev_buf);
    }
    prev_buf = buf;
    buf = NULL;

    sector = CalcNextSector(root->fat, sector, cluster_list);
    cluster_list = cluster_list->next;
    cluster = cluster_list->val;
  }

  if (prev_buf != NULL) {
    kmm_uncached_free(prev_buf);
  }
  if (buf != NULL) {
    kmm_uncached_free(buf);
  }

  return found;
}

int Fat32Fs::Read(fat32_file_t* file, void* buf, uint32_t offset, size_t len) {
  if (!fs_.initialized) {
    LOG_ERROR("Not initialized yet");
    return false;
  }

  if (offset + len > file->size) {
    LOG_ERROR("Requested read size is bigger than actual file size");
    return 0;
  }

  fat32_fat_t* fat = file->fat;
  uint32_t tail = std::min(offset + static_cast<uint32_t>(len), file->size);
  uint32_t remains = tail - offset;

  uint32_t cluster = CalcCluster(fat, file->cluster_root, offset);
  uint32_t sector = CalcSector(fat, cluster, offset);
  uint32_t sector_offset = offset % kBlockSize;

  auto* cluster_list = file->cluster_root;
  while (cluster_list->val != cluster) {
    cluster_list = cluster_list->next;
  }

  while (remains > 0 && IsValidCluster(cluster)) {
    uint8_t* read_buf;
    uint32_t copy_len;
    const uint32_t lba = fat->first_lba + sector;
    if (sector_offset > 0 || remains <= kBlockSize ||
        fat->bpb.BPB_SecPerClus == 1) {
      read_buf = ReadSector(lba, 1);
      copy_len = std::min(remains, kBlockSize - sector_offset);
    } else {
      auto read_sectors = static_cast<uint32_t>(
          (static_cast<float>(remains) / kBlockSize) + 0.5);
      uint32_t sector_num =
          std::min(read_sectors, fat->bpb.BPB_SecPerClus - sector);
      read_buf = ReadSector(lba, sector_num);
      copy_len = std::min(remains, kBlockSize * read_sectors);
    }

    memcpy(buf, read_buf + sector_offset, copy_len);
    kmm_uncached_free(read_buf);
    buf = static_cast<uint8_t*>(buf) + copy_len;
    remains -= copy_len;
    sector_offset = 0;

    sector = CalcNextSector(fat, sector, cluster_list);
    cluster_list = cluster_list->next;
  }

  return tail - offset - remains;
}

int Fat32Fs::GetFileSize(fat32_file_t* file) {
  return file->size;
}

int Fat32Fs::IsDirectory(fat32_file_t* file) {
  return (file->attr & kAttrDirectory) != 0;
}

uint8_t* Fat32Fs::ReadSector(uint32_t lba, uint32_t sector_num) {
  auto* buf = static_cast<uint8_t*>(kmm_uncached_malloc(PAGE_SIZE));

  auto& mmc = Mmc::Get();
  mmc.Seek(kBlockSize * lba);
  // auto start = Timer::GetSystemUsec();
  if (mmc.Read(buf, kBlockSize * sector_num) < 0) {
    LOG_ERROR("Failed to emmc_read");
    return NULL;
  }
  // auto end =  Timer::GetSystemUsec();
  // LOG_DEBUG("ReadSector: %d[msec]", (end - start) / 1000);

  return buf;
}

bool Fat32Fs::CheckBpb(fat32_bpb_t* bpb) {
  if (bpb->BPB_BytsPerSec != kBlockSize) {
    LOG_WARN("Unsupprted block size (%d)", bpb->BPB_BytsPerSec);
    return false;
  }

  if (bpb->BS_BootSign != kBootSignature) {
    LOG_ERROR("Invalid BS_BootSign (%x)", bpb->BS_BootSign);
    return false;
  }

  // Check "FAT12   ", "FAT16   ", "FAT     "
  if (!(bpb->BS_FilSysType[0] == 'F' && bpb->BS_FilSysType[1] == 'A' &&
        bpb->BS_FilSysType[2] == 'T')) {
    LOG_WARN("Invalid filesystem type (%c%c%c)", bpb->BS_FilSysType[0],
             bpb->BS_FilSysType[1], bpb->BS_FilSysType[2]);
    return false;
  }

  return true;
}

void Fat32Fs::InitFile(fat32_file_t* file,
                       fat32_fat_t* fat,
                       uint8_t attr,
                       uint32_t size,
                       uint32_t cluster) {
  file->fat = fat;
  file->attr = attr;
  file->size = size;
  file->cluster_root = new ClusterLinkedListNode();
  file->cluster_root->val = cluster;
}

uint32_t Fat32Fs::CalcCluster(fat32_fat_t* fat,
                              ClusterLinkedListNode* cluster_list,
                              uint32_t offset) {
  const fat32_bpb_t* bpb = &(fat->bpb);
  const int max_sector = offset / (bpb->BPB_SecPerClus * bpb->BPB_BytsPerSec);
  uint8_t* buf = nullptr;

  auto cluster = cluster_list->val;
  for (int i = 0; i < max_sector; i++) {
    if (cluster_list->next == nullptr) {
      uint32_t sector =
          fat->fat_first_sector + (cluster * 4 / bpb->BPB_BytsPerSec);
      uint32_t offset = cluster * 4 % bpb->BPB_BytsPerSec;

      if (buf != nullptr) {
        kmm_uncached_free(buf);
        buf = nullptr;
      }
      buf = ReadSector(fat->first_lba + sector, 1);

      cluster = *((uint32_t*)(buf + offset)) & 0x0fffffff;
      if (!IsValidCluster(cluster)) {
        cluster = kBadCluster;
        break;
      }

      cluster_list->next = new ClusterLinkedListNode();
      cluster_list = cluster_list->next;
      cluster_list->val = cluster;
    } else {
      cluster_list = cluster_list->next;
      cluster = cluster_list->val;
    }
  }

  if (buf != nullptr) {
    kmm_uncached_free(buf);
  }

  return cluster;
}

uint8_t Fat32Fs::CreateLfnSum(fat32_dir_t* entry) {
  uint8_t sum = 0;

  for (int i = 0; i < 11; i++) {
    sum = (sum >> 1) + (sum << 7) + entry->DIR_Name[i];
  }

  return sum;
}

char* Fat32Fs::GetSfn(fat32_dir_t* entry) {
  static char result[13];

  // name
  for (int i = 0; i < 8; i++) {
    result[i] = entry->DIR_Name[i];
    if (result[i] == ' ') {
      break;
    }
    if (result[i] == kDirNameKanjiEntry) {
      /*
      If DIR_Name[0] == 0x05, then the actual file name character for this byte
      is 0xE5. 0xE5 is actually a valid KANJI lead byte value for the character
      set used in Japan. The special 0x05 value is used so that this special
      file name case for Japan can be handled properly and not cause FAT file
      system code to think that the entry is free.
      */
      result[i] = 0xe5;
    }
  }

  // extention
  if (entry->DIR_Name[8] != ' ') {
    result[8] = '.';
    for (int i = 8; i < 11; i++) {
      result[i + 1] = entry->DIR_Name[i];
      if (result[i + 1] == ' ') {
        break;
      }
      if (result[i + 1] == kDirNameKanjiEntry) {
        result[i + 1] = 0xe5;
      }
    }
  }
  result[12] = '\0';

  return result;
}

char* Fat32Fs::GetLfn(fat32_dir_t* entry,
                      size_t offset,
                      fat32_dir_t* prev_entry) {
  static char result[256];
  char* p_result = result;

  fat32_lfn_t* lfn = (fat32_lfn_t*)entry;
  uint8_t sum = CreateLfnSum(entry);
  bool is_prev_sector = false;
  int seq = 1;  // sequence number: 1 - 20

  while (true) {
    if (!is_prev_sector && (offset & (kBlockSize - 1)) == 0) {
      lfn = (fat32_lfn_t*)prev_entry;
      is_prev_sector = true;
    } else {
      lfn--;
    }

    if (lfn == NULL || lfn->LDIR_Chksum != sum ||
        (lfn->LDIR_Attr & kAttrLongName) != kAttrLongName ||
        (lfn->LDIR_Ord & 0x1f) != seq++) {
      return NULL;
    }

    // character code: UTF16-LE
    // 1st to 5th characters
    for (int i = 0; i < 10; i += 2) {
      *p_result++ = lfn->LDIR_Name1[i];
    }
    // 6th to 11th characters
    for (int i = 0; i < 12; i += 2) {
      *p_result++ = lfn->LDIR_Name2[i];
    }
    // 12th to 13th characters
    for (int i = 0; i < 4; i += 2) {
      *p_result++ = lfn->LDIR_Name3[i];
    }

    if (lfn->LDIR_Ord & kLfnLastLongEntry) {
      break;
    }

    offset -= sizeof(fat32_dir_t);
  }
  *p_result = '\0';

  return result;
}

int Fat32Fs::CalcNextSector(fat32_fat_t* fat,
                            uint32_t cur_sector,
                            ClusterLinkedListNode* cluster_list) {
  const uint32_t secs_per_clus = fat->bpb.BPB_SecPerClus;
  if (cur_sector % secs_per_clus != secs_per_clus - 1) {
    return cur_sector + 1;
  }

  // Go over a cluster boundary
  auto next_cluster = CalcNextCluster(fat, cluster_list);
  return CalcSector(fat, next_cluster, 0);
}

uint32_t Fat32Fs::CalcNextCluster(fat32_fat_t* fat,
                                  ClusterLinkedListNode* cluster_list) {
  const fat32_bpb_t* bpb = &(fat->bpb);
  uint32_t sector =
      fat->fat_first_sector + (cluster_list->val * 4 / bpb->BPB_BytsPerSec);
  uint32_t offset = cluster_list->val * 4 % bpb->BPB_BytsPerSec;

  uint32_t next_cluster;
  if (cluster_list->next == nullptr) {
    uint8_t* buf = ReadSector(fat->first_lba + sector, 1);
    next_cluster = *((uint32_t*)(buf + offset)) & 0x0fffffff;
    kmm_uncached_free(buf);
    cluster_list->next = new ClusterLinkedListNode();
    cluster_list->next->val = next_cluster;
  } else {
    next_cluster = cluster_list->next->val;
  }

  return next_cluster;
}

uint32_t Fat32Fs::CalcSector(fat32_fat_t* fat,
                             uint32_t cluster,
                             size_t offset) {
  const uint32_t first_sector =
      fat->data_first_sector + (cluster - 2) * fat->bpb.BPB_SecPerClus;
  const uint32_t sector_offset =
      offset % ((uint32_t)fat->bpb.BPB_SecPerClus * kBlockSize) / kBlockSize;

  return first_sector + sector_offset;
}

inline bool Fat32Fs::IsValidCluster(uint32_t cluster) {
  /*
    0x00000000:              Empty cluster
    0x00000001:              Reserved
    0x00000002 - 0x0FFFFFF6: Cluster in use
    0x0FFFFFF7:              Bad cluster
    0x0FFFFFF8 - 0x0FFFFFFF: End of cluster
  */
  return (cluster >= 0x00000002 && cluster <= 0xffffff6);
}

}  // namespace evisor
