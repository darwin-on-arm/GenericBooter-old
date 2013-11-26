/*
 * Copyright 2013, winocm. <winocm@icloud.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 *   If you are going to use this software in any form that does not involve
 *   releasing the source to this project or improving it, let me know beforehand.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * XML DeviceTree code originally by Christina Brooks.
 * Copyright (c) 2012, 2013 - Christina Brooks.
 */

#include "genboot.h"
#include "helper.h"

#define KERNEL_VMADDR   0x80001000

extern void *__end;
memory_region_t kernel_region;
uint32_t kernel_entrypoint;     /* PA of entrypoint */

#define vtophys(x, virt, phys)		\
	(((uintptr_t)x - (uintptr_t)virt) + (uintptr_t)phys)

#define phystov(x, virt, phys) 		\
	(((uintptr_t)x - (uintptr_t)phys) +(uintptr_t)virt)

/* 
 * 'memory-map' range types
 *
 * kBootDriverTypeKEXT are for in-memory kexts that are
 * picked up by kxld once the kernel is up and running.
 *
 * If a range named 'RAMDisk' is present, the BSD layer will
 * root from that ramdisk instead of finding a root by matching
 * IOKit stuff.
 */
enum {
    kBootDriverTypeInvalid = 0,
    kBootDriverTypeKEXT = 1,
    kBootDriverTypeMKEXT = 2
};

/*
 * Apple's device tree is weird. Objects parents are referenced
 * by the 'AAPL,phandle' property. 
 */

static void PopulateDeviceTreeNode(TagPtr tag, Node * node);
static void WalkDeviceTreeNodeChildren(TagPtr tag, Node * parent);

/* Array with either ints or strings to DT data. */
static void *ArrayToDeviceTreeData(TagPtr tag, uint32_t * len)
{
    TagPtr next;
    unsigned cnt = 0;
    uint8_t *buf;

    if (!tag)
        panic("no tag");

    if (!tag->tag)
        panic("no tag");

    if (!(tag->type == kTagTypeArray))
        panic("tag is not array");

    next = tag->tag;

    /* Count up first */
    while (next) {
        if (next->type == kTagTypeInteger) {
            cnt += sizeof(uint32_t);
        } else if (next->type == kTagTypeString) {
            cnt += strlen(next->string) + 1;
        } else {
            panic("unknown type");
        }
        next = next->tagNext;
    }

    buf = malloc(cnt);
    next = tag->tag;
    cnt = 0;

    /* Populate buffer */
    while (next) {
        if (next->type == kTagTypeInteger) {
            uint32_t *buf32 = (uint32_t *) & buf[cnt];
            *buf32 = (uint32_t) next->string;
            cnt += sizeof(uint32_t);
        } else if (next->type == kTagTypeString) {
            size_t len = strlen(next->string) + 1;
            bcopy(next->string, &buf[cnt], len);
            cnt += len;
        }
        next = next->tagNext;
    }

    *len = cnt;
    return (void *)buf;
}

/* Integer to DT data */
static void *IntegerToDeviceTreeData(unsigned long value, uint32_t * len)
{
    uint32_t *buf = malloc(sizeof(uint32_t));
    *buf = value;
    *len = sizeof(uint32_t);
    return (void *)buf;
}

/* String to DT data */
static void *StringToDeviceTreeData(const char *value, uint32_t * len)
{
    size_t slen = strlen(value) + 1;
    void *buf = malloc(slen);
    bcopy((char *)value, buf, slen);

    if (len != NULL) {
        *len = slen;
    }

    return (void *)buf;
}

static void WalkDeviceTreeNodeChildren(TagPtr tag, Node * parent)
{
    TagPtr next;

    if (!tag)
        panic("no tag");

    if (!tag->tag)
        panic("no tag");

    if (!(tag->type == kTagTypeArray))
        panic("tag is not array");

    next = tag->tag;

    while (next) {
        Node *new_node;

        new_node = DT__AddChild(parent, NULL);
        PopulateDeviceTreeNode(next, new_node);

        next = next->tagNext;
    }
}

#define CopyKey(next) ((char*)StringToDeviceTreeData(next->string, NULL))

static void PopulateDeviceTreeNode(TagPtr tag, Node * node)
{
    TagPtr next;
    uint32_t plen;
    void *pval;

    if (!(tag->type == kTagTypeDict))
        panic("tag is not dict");

    next = tag->tag;

    while (next) {
        /* next->tag has the value of the key */
        if (next->tag) {
            if (next->tag->type == kTagTypeArray) {
                if (next->string && next->string[0] == '@') {
                    /* @children */
                    WalkDeviceTreeNodeChildren(next->tag, node);
                } else {
                    /* Property array */
                    pval = ArrayToDeviceTreeData(next->tag, &plen);
                    DT__AddProperty(node, CopyKey(next), plen, pval);
                }
            } else if (next->tag->type == kTagTypeInteger) {
                pval = IntegerToDeviceTreeData((uint32_t)
                                               next->tag->string, &plen);
                DT__AddProperty(node, CopyKey(next), plen, pval);
            } else if (next->tag->type == kTagTypeString) {
                pval = StringToDeviceTreeData(next->tag->string, &plen);
                DT__AddProperty(node, CopyKey(next), plen, pval);
            }
        }

        next = next->tagNext;
    }
}

/**
 * get_image3
 *
 * This is bad for many reasons.
 */
void *get_image3(uint32_t type)
{
    Image3RootHeader *img3 = (Image3RootHeader *) & __end;
    while (img3 != NULL) {
        if (img3->header.magic != 'Img3')
            return NULL;
        if (img3->shshExtension.imageType == type)
            return (img3);
        img3 = (Image3RootHeader *) ((uint8_t *) (img3) + img3->header.size);
    }
    return img3;
}

/**
 * prepare_kernel
 *
 * Prepare and load the Kernel from a concatenated image3 object.
 */
int prepare_kernel(void)
{
    void *kernelData, *kernelImage;
    uint32_t kernelSize;
    loader_context_t ctx;
    uintptr_t kernelBase;

    kernelImage = get_image3(kImage3TypeKernel);
    assert(kernelImage != NULL);

    image3_get_tag_data(kernelImage, kImage3TagData, &kernelData, &kernelSize);

    assert((kernelData != NULL) && (kernelSize > 0));

    assert(!macho_initialize(&ctx, kernelData));
    assert(!macho_set_vm_bias(&ctx, KERNEL_VMADDR));
    assert((kernelSize = macho_get_vmsize(&ctx)) > 0);

    kernelBase =
        (uintptr_t) memory_region_reserve(&kernel_region, kernelSize, 0);
    printf("Loading kernel at 0x%08x\n", kernelBase, kernelSize);

    assert(!macho_file_map(&ctx, kernelBase));
    assert(!macho_get_entrypoint(&ctx, &kernel_entrypoint));

    kernel_entrypoint =
        vtophys(kernel_entrypoint, KERNEL_VMADDR, kernel_region.base);

    return true;
}

/**
 * Prepare boot-args.
 */
static int prepare_boot_args(void)
{
    gBootArgs.Revision = kBootArgsRevision;
    gBootArgs.Version = kBootArgsVersion2;
    gBootArgs.virtBase = 0x80000000;
    return true;
}

/**
 * AllocateMemoryRange
 *
 * Originally from boot, allocates a memory range into the DeviceTee.
 */
int AllocateMemoryRange(Node * memory_map, char *rangeName, long start,
                        long length, long type)
{
    char *nameBuf;
    uint32_t *buffer;

    nameBuf = malloc(strlen(rangeName) + 1);
    if (nameBuf == 0)
        return false;
    strcpy(nameBuf, rangeName);

    buffer = malloc(2 * sizeof(uint32_t));
    if (buffer == 0)
        return false;

    buffer[0] = start;
    buffer[1] = length;

    DT__AddProperty(memory_map, nameBuf, 2 * sizeof(uint32_t), (char *)buffer);

    return true;
}


/**
 * CreateDeviceTreeNode
 *
 * Puts a node into the DeviceTree.
 */
int CreateDeviceTreeNode(Node * node, char *rangeName, void* datap, int size)
{
    char *nameBuf;

    nameBuf = malloc(strlen(rangeName) + 1);
    if (nameBuf == 0)
        return false;
    strcpy(nameBuf, rangeName);

    DT__AddProperty(node, nameBuf, size, (char *)datap);

    return true;
}


/**
 * CreateMemoryMapNode
 *
 * Create the memory map node used to enter the kernel ranges into.
 */
Node *gChosen;

Node *CreateMemoryMapNode(void)
{
    Node *root = DT__RootNode();
    Node *chosen;
    Node *memory_map;

    /* /chosen/memory-map */
    chosen = DT__AddChild(root, "chosen");
    memory_map = DT__AddChild(chosen, "memory-map");

    gChosen = chosen;

    return memory_map;
}

/**
 * prepare_devicetree
 *
 * Prepare and flatten the devicetree.
 */
int prepare_devicetree(void)
{
    void *deviceTreeImage, *deviceTreeData;
    uint32_t deviceTreeSize, length, pos = 0;
    Node *root;
    TagPtr tag;
    char *xmlRepresentation;

    deviceTreeImage = get_image3(kImage3TypeXmlDeviceTree);
    assert(deviceTreeImage != NULL);
    image3_get_tag_data(deviceTreeImage, kImage3TagData, &deviceTreeData,
                        &deviceTreeSize);

    /* Create root of DT */
    DT__Initialize();
    root = DT__RootNode();

    xmlRepresentation = (char *)deviceTreeData;

    /* Enter everything into the DeviceTree. (not mine) */
    assert(root);

    while (1) {
        length = XMLParseNextTag(xmlRepresentation + pos, &tag);
        if (length == -1)
            break;
        pos += length;
        if (!tag)
            continue;
        if (tag->type == kTagTypeDict) {
            PopulateDeviceTreeNode(tag, root);
            XMLFreeTag(tag);
            return true;
        }
        XMLFreeTag(tag);
    }

    return false;
}

/**
 * enter additional data into the DeviceTree
 */
int prepare_devicetree_stage2(void)
{
    void *deviceTreeP;
    uint32_t deviceTreeLength;
    Node *memory_map = CreateMemoryMapNode();
    assert(memory_map);

    /* Insert the cool iBoot-like stuff. */
    uint32_t one = 1;
    uint64_t ecid = 0xBEEFBEEFBEEFBEEF;

    assert(gChosen);
    CreateDeviceTreeNode(gChosen, "firmware-version", "iBoot-1234.5.6~93", sizeof("iBoot-1234.5.6~93"));
    CreateDeviceTreeNode(gChosen, "debug-enabled", &one, sizeof(uint32_t));
    CreateDeviceTreeNode(gChosen, "secure-boot", &one, sizeof(uint32_t));    

    CreateDeviceTreeNode(gChosen, "die-id", &ecid, sizeof(uint64_t));    
    CreateDeviceTreeNode(gChosen, "unique-chip-id", &ecid, sizeof(uint64_t));    

    CreateDeviceTreeNode(DT__RootNode(), "serial-number", "SOMESRNLNMBR", sizeof("SOMESRNLNMBR"));

    /* Verify we have a ramdisk. */
    if (ramdisk_base) {
        void *reloc_ramdisk_base =
            (void *)memory_region_reserve(&kernel_region, ramdisk_size,
                                          4096);
        printf
            ("creating ramdisk at 0x%x of size 0x%x, from image at 0x%x\n",
             reloc_ramdisk_base, ramdisk_size, ramdisk_base);
        bcopy((void *)ramdisk_base, reloc_ramdisk_base, ramdisk_size);
        AllocateMemoryRange(memory_map, "RAMDisk",
                            (uint32_t) reloc_ramdisk_base, ramdisk_size,
                            kBootDriverTypeInvalid);
    }

    /* Flatten the finalized device-tree image. */
    DT__FlattenDeviceTree(NULL, &deviceTreeLength);

    /* Allocate memory for it. */
    deviceTreeP = memory_region_reserve(&kernel_region, deviceTreeLength, 0);

    /* Flatten. */
    DT__FlattenDeviceTree((void **)&deviceTreeP, &deviceTreeLength);

    /* Enter into Boot-Args */
    gBootArgs.deviceTreeLength = deviceTreeLength;
    gBootArgs.deviceTreeP =
        (void *)phystov(deviceTreeP, gBootArgs.virtBase, gBootArgs.physBase);
    printf("creating device tree at 0x%x of size 0x%x\n", deviceTreeP,
           deviceTreeLength);

    return true;
}

/**
 * prepare_finalized_boot_args
 *
 * Prepare the final boot-args.
 */
boot_args *prepare_finalized_boot_args(void)
{
    boot_args *bootArgs;

    gBootArgs.topOfKernelData = align_up(kernel_region.pos, 0x100000);

    printf("gBootArgs.commandLine = [%s]\n", gBootArgs.commandLine);

    /* Allocate kernel memory for this. */
    bootArgs =
        (boot_args *) memory_region_reserve(&kernel_region,
                                            sizeof(boot_args), 1024);
    bcopy(&gBootArgs, bootArgs, sizeof(boot_args));

    return bootArgs;
}

/**
 * start_darwin
 *
 * Start the Darwin kernel.
 */
void start_darwin(void)
{
    boot_args *args;
    kernel_start *start_routine;

    /* 
     * Initialize the kernel memory region. 
     *
     * The kernel is always mapped to eDRAM + PAGE_SIZE. This is done
     * for sleep support. (eventually.)
     */
    kernel_region.pos = kernel_region.base = (gBootArgs.physBase + 0x1000);

    /* XXX: Zero out beginning of RAM. */
    printf("preparing system...\n");
    bzero((void *)gBootArgs.physBase, (32 * 1024 * 1024));

    /* Initialize boot-args. */
    assert(prepare_boot_args());

    /* Map kernel. */
    assert(prepare_kernel());

    /* Map device tree. */
    assert(prepare_devicetree());

    printf("Enter ramdisk into /chosen/memory-map and flatten.\n");
    /* Enter ramdisk into /chosen/memory-map and flatten. */
    assert(prepare_devicetree_stage2());

    printf("Copy boot-args over to kernel region...\n");
    /* Copy boot-args over to kernel region. */
    assert((args = prepare_finalized_boot_args()) != NULL);

    printf("Fire the kernel!\n");
    /* Jump to the kernel. */
    start_routine = (kernel_start *) kernel_entrypoint;
    printf
        ("kernel prepped at 0x%08x, transferring control out of booter now!\n",
         start_routine);

    /* _locore_jump_to */
    _locore_jump_to(start_routine, (void *)args);

    panic("Should never return from here.\n");
    return;
}
