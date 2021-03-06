#include "common/include/inttypes.h"
#include "common/include/devtree.h"
#include "loader/include/lib.h"
#include "loader/include/devtree.h"


/*
 * #address-cells and #size-cells
 */
static void get_num_cells(struct devtree_node *node,
    int *num_addr_cells, int *num_size_cells)
{
    if (num_addr_cells) {
        struct devtree_prop *prop = devtree_find_prop(node, "#address-cells");
        if (prop) {
            u32 data = swap_big_endian32(*(u32 *)devtree_get_prop_data(prop));
            *num_addr_cells = (int)data;
        }
    }

    if (num_size_cells) {
        struct devtree_prop *prop = devtree_find_prop(node, "#size-cells");
        if (prop) {
            u32 data = swap_big_endian32(*(u32 *)devtree_get_prop_data(prop));
            *num_size_cells = (int)data;
        }
    }
}

void devtree_get_reg_num_cells(struct devtree_node *node,
    int *num_addr_cells, int *num_size_cells)
{
    // Default
    if (num_addr_cells) *num_addr_cells = 1;
    if (num_size_cells) *num_size_cells = 1;

    // Look at parent
    node = devtree_get_parent_node(node);
    if (node) {
        get_num_cells(node, num_addr_cells, num_size_cells);
    }
}

void devtree_get_ranges_num_cells(struct devtree_node *node,
    int *num_child_addr_cells, int *num_parent_addr_cells, int *num_size_cells)
{
    // Default
    if (num_child_addr_cells) *num_child_addr_cells = 1;
    if (num_parent_addr_cells) *num_parent_addr_cells = 1;
    if (num_size_cells) *num_size_cells = 1;

    // Current node to get # child addr cells and # size cells
    get_num_cells(node, num_child_addr_cells, num_size_cells);

    // Parent node to get # parent addr cells
    node = devtree_get_parent_node(node);
    if (node) {
        get_num_cells(node, num_parent_addr_cells, NULL);
    }
}


/*
 * Reg and Ranges
 *  int return value
 *      -1 - Unable to parse
 *       0 - Already the last range pair
 *      >0 - Next reg/range pair index
 */
static inline u64 parse_cell_num(void *data, int num_cells)
{
    switch (num_cells) {
    case 0: return 0;
    case 1: return swap_big_endian32(*(u32 *)data);
    case 2:
    default:
        panic_not(num_cells == 2,
            "Unable to handle cells > 2, num_cells: %d\n", num_cells);
        return swap_big_endian64(*(u64 *)(data + num_cells - 2));
    }

    return 0;
}

static int get_ranges(struct devtree_node *node, int idx,
    u64 *child_addr, u64 *parent_addr, u64 *size)
{
    struct devtree_prop *prop = devtree_find_prop(node, "ranges");
    if (!prop) {
        return -1;
    }

    int num_parent_cells = 1, num_child_cells = 1, num_size_cells = 1;
    devtree_get_ranges_num_cells(node, &num_parent_cells, &num_child_cells,
        &num_size_cells);

    int pair_size = (num_parent_cells + num_child_cells + num_size_cells) << 2;
    if (pair_size > prop->len) {
        return -1;
    }

    int count;  // = prop->len / pair_size;
    div_u32((u32)prop->len, (u32)pair_size, (u32 *)&count, NULL);
    if (idx >= count) {
        return -1;
    }

    void *data = devtree_get_prop_data(prop);
    data += pair_size * idx;
    if (child_addr) {
        *child_addr = parse_cell_num(data, num_child_cells);
    }

    data += num_child_cells << 2;
    if (parent_addr) {
        *parent_addr = parse_cell_num(data, num_parent_cells);
    }

    data += num_parent_cells << 2;
    if (size) {
        *size = parse_cell_num(data, num_size_cells);
    }

    return count - idx > 1 ? idx + 1 : 0;
}

u64 devtree_translate_addr(struct devtree_node *node, u64 addr)
{
    // Keep tracing up
    for (node = devtree_get_parent_node(node); node;
        node = devtree_get_parent_node(node)
    ) {
        u64 parent = 0, child = 0, size = 0;
        int range_idx = 0;
        do {
            range_idx = get_ranges(node, range_idx, &child, &parent, &size);
            if (range_idx >= 0 && addr >= child && addr < child + size) {
                addr = parent + addr - child;
                break;
            }
        } while (range_idx > 0);
    };

    return addr;
}

int devtree_get_addr(struct devtree_node *node, int idx, const char *name,
    u64 *addr, u64 *size)
{
    struct devtree_prop *prop = devtree_find_prop(node, name);
    if (!prop) {
        return -1;
    }

    int num_addr_cells = 1, num_size_cells = 1;
    devtree_get_reg_num_cells(node, &num_addr_cells, &num_size_cells);

    int pair_size = (num_addr_cells + num_size_cells) << 2;
    if (!pair_size || pair_size > prop->len) {
        return -1;
    }

    int count = 0;  //prop->len / pair_size;
    div_u32((u32)prop->len, (u32)pair_size, (u32 *)&count, NULL);
    if (idx >= count) {
        return -1;
    }

    void *data = devtree_get_prop_data(prop);

    data += pair_size * idx;
    if (addr) {
        *addr = parse_cell_num(data, num_addr_cells);
    }

    data += num_addr_cells << 2;
    if (size) {
        *size = parse_cell_num(data, num_size_cells);
    }

    return count - idx > 1 ? idx + 1 : 0;
}

int devtree_get_reg(struct devtree_node *node, int idx, u64 *addr, u64 *size)
{
    return devtree_get_addr(node, idx, "reg", addr, size);
}

int devtree_get_translated_addr(struct devtree_node *node, int idx,
    const char *name, u64 *addr, u64 *size)
{
    u64 val_addr = 0, val_size = 0;
    int next_idx = devtree_get_addr(node, idx, name, &val_addr, &val_size);
    if (next_idx < 0) {
        return -1;
    }

    val_addr = devtree_translate_addr(node, val_addr);
    if (addr) *addr = val_addr;
    if (size) *size = val_size;

    return next_idx;
}

int devtree_get_translated_reg(struct devtree_node *node, int idx,
    u64 *addr, u64 *size)
{
    return devtree_get_translated_addr(node, idx, "reg", addr, size);
}


/*
 * Compatible
 *  int return value
 *      -1 - Unable to parse
 *       0 - Already the last range pair
 *      >0 - Next reg/range pair index
 */
int devtree_get_compatible(struct devtree_node *node, int idx, char **name)
{
    struct devtree_prop *prop = devtree_find_prop(node, "compatible");
    if (!prop) {
        return -1;
    }

    // Skip
    char *data = devtree_get_prop_data(prop);
    size_t used_len = 0;
    for (int i = 0; i < idx; i++) {
        size_t l = strlen(data) + 1;
        data += l;
        used_len += l;
        if (used_len >= prop->len) {
            return -1;
        }
    }

    // Found the requested idx
    if (name) {
        *name = data;
    }

    // See if there's any more
    used_len += strlen(data) + 1;
    return used_len >= prop->len ? 0 : idx + 1;
}
