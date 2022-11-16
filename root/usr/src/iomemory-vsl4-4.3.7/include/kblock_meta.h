/*
  This "meta" file is supposed to abstract things that change in kblock.c
  and should provide simplified defines that are named after their function.
  Both aimed at making the code cleaner, more readable and perhaps more
  maintainable. Blocks should be isolated and only cover one item.
 */
#ifndef __FIO_KBLOCK_META_H__
#define __FIO_KBLOCK_META_H__

#include <linux/version.h>

#if KFIOC_X_LINUX_HAS_PART_STAT_H
#include <linux/part_stat.h>
#endif /* KFIOC_X_LINUX_HAS_PART_STAT_H */


#if KFIOC_X_BLK_ALLOC_DISK_EXISTS
  #define BLK_ALLOC_QUEUE dp->gd->queue;
  #define BLK_ALLOC_DISK blk_alloc_disk
#else /* KFIOC_X_BLK_ALLOC_DISK_EXISTS */
  #if KFIOC_X_HAS_MAKE_REQUEST_FN != 1
    #define BLK_ALLOC_QUEUE blk_alloc_queue(node);
  #endif
  #define BLK_ALLOC_DISK alloc_disk
#endif


#if KFIOC_X_BIO_HAS_BI_BDEV
  #define BIO_DISK bi_bdev->bd_disk
#else /* KFIOC_X_BIO_HAS_BI_BDEV */
  #define BIO_DISK bi_disk
#endif /* KFIOC_X_BIO_HAS_BI_BDEV */


#if KFIOC_X_HAS_MAKE_REQUEST_FN
  static unsigned int kfio_make_request(struct request_queue *queue, struct bio *bio);
  #define KFIO_SUBMIT_BIO_RC return FIO_MFN_RET;
  #define BLK_QUEUE_SPLIT blk_queue_split(queue, &bio);

  #if KFIOC_X_BLK_ALLOC_QUEUE_NODE_EXISTS
    #define BLK_ALLOC_QUEUE blk_alloc_queue_node(GFP_NOIO, node);
  #elif KFIOC_X_BLK_ALLOC_QUEUE_EXISTS
    #define BLK_ALLOC_QUEUE blk_alloc_queue(GFP_NOIO);
  #else
    #define BLK_ALLOC_QUEUE blk_alloc_queue(kfio_make_request, node);
  #endif /* KFIOC_X_BLK_ALLOC_QUEUE_NODE_EXISTS */

#else /* KFIOC_X_HAS_MAKE_REQUEST_FN */
  #if KFIOC_X_SUBMIT_BIO_RETURNS_BLK_QC_T
    #define KFIO_SUBMIT_BIO blk_qc_t kfio_submit_bio(struct bio *bio)
    #define KFIO_SUBMIT_BIO_RC return FIO_MFN_RET;
  #else
    #define KFIO_SUBMIT_BIO void kfio_submit_bio(struct bio *bio)
    #define KFIO_SUBMIT_BIO_RC
  #endif
  KFIO_SUBMIT_BIO;
  #define BLK_QUEUE_SPLIT blk_queue_split(&bio);
#endif /* KFIOC_X_HAS_MAKE_REQUEST_FN */

// should check for hd_struct vs gendisk
#if KFIOC_X_GENHD_PART0_IS_A_POINTER
  #define GD_PART0 disk->gd->part0
  #define GET_BDEV disk->gd->part0
#else /* KFIOC_X_GENHD_PART0_IS_A_POINTER */
  #define GD_PART0 &disk->gd->part0
  #define GET_BDEV bdget_disk(disk->gd, 0);
#endif /* KFIOC_X_GENHD_PART0_IS_A_POINTER */


#if KFIOC_X_VOID_ADD_DISK
#define ADD_DISK add_disk(disk->gd);
#else
#define ADD_DISK if (add_disk(disk->gd)) { infprint("Error while adding disk!"); }
#endif /* KFIOC_X_VOID_ADD_DISK */

#if KFIOC_X_DISK_HAS_OPEN_MUTEX
#define SHUTDOWN_MUTEX &disk->gd->open_mutex
#else
#define SHUTDOWN_MUTEX &linux_bdev->bd_mutex
#endif /* KFIOC_X_DISK_HAS_OPEN_MUTEX */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0)
// 5.17 moved GD to explicitly do this by default
#define KFIO_DISABLE_GENHD_FL_EXT_DEVT 1
#endif

// while (atomic_read(&linux_bdev->bd_openers) > 0 && linux_bdev->bd_disk == disk->gd)
// 5.19 changed bd_openers from int to atomic_t in kblock.c
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0)
#define BD_OPENERS atomic_read(&linux_bdev->bd_openers)
#define SET_QUEUE_FLAG_DISCARD
#else
#define BD_OPENERS linux_bdev->bd_openers
// https://lore.kernel.org/linux-btrfs/20220409045043.23593-25-hch@lst.de/
#define SET_QUEUE_FLAG_DISCARD blk_queue_flag_set(QUEUE_FLAG_DISCARD, rq);
#endif

/*
/home/funs/Documents/Projects/fusion/iomemory-vsl4/root/usr/src/iomemory-vsl4-4.3.7/kcpu.c:214:5: warning: ISO C90 forbids variable length array ‘node_hist’ [-Wvla]
  214 |     uint32_t node_hist[nodes_possible], node_map[nodes_possible];
      |     ^~~~~~~~
/home/funs/Documents/Projects/fusion/iomemory-vsl4/root/usr/src/iomemory-vsl4-4.3.7/kcpu.c:214:5: warning: ISO C90 forbids variable length array ‘node_map’ [-Wvla]
???
 UBSAN: array-index-out-of-bounds in /home/funs/Documents/Projects/fusion/iomemory-vsl4/root/usr/src/iomemory-vsl4-4.3.7/kcpu.c:257:29
Nov 14 19:51:13 cipher kernel: [ 1229.568587] index 1 is out of range for type 'uint32_t [*]'
Nov 14 19:51:13 cipher kernel: [ 1229.568590] CPU: 15 PID: 10419 Comm: insmod Tainted: P           OE     5.19.0-23-generic #24-Ubuntu
Nov 14 19:51:13 cipher kernel: [ 1229.568593] Hardware name: Gigabyte Technology Co., Ltd. X570 AORUS PRO WIFI/X570 AORUS PRO WIFI, BIOS F36c 05/12/2022
[snip]
Nov 14 19:51:13 cipher kernel: [ 1229.568628]  kfio_map_cpus_to_read_queues+0x1ed/0x690 [iomemory_vsl4]
Nov 14 19:51:21 cipher kernel: [ 1237.539908] Spurious interrupt (vector 0xef) on CPU#0. Acked

184 times on two card loading, equates to 184 / 8 = 23 (my core count)...
254 (0xfe) - Local APIC error interrupt (generated when the local APIC detects an erroneous condition)


*/

#endif /* __FIO_KBLOCK_META_H__ */
