
#include <ngx_config.h>
#include <ngx_core.h>


/* STUB: page size */
#define NGX_RADIX_TREE_POOL_SIZE  4096


static void *ngx_radix_alloc(ngx_radix_tree_t *tree, size_t size);


ngx_radix_tree_t *ngx_radix_tree_create(ngx_pool_t *pool)
{
    ngx_radix_tree_t  *tree;

    if (!(tree = ngx_palloc(pool, sizeof(ngx_radix_tree_t)))) {
        return NULL;
    }

    tree->root = NULL;
    tree->pool = pool;
    tree->free = NULL;
    tree->size = 0;

    return tree;
}


ngx_int_t ngx_radix32tree_insert(ngx_radix_tree_t *tree,
                                 uint32_t key, uint32_t mask, uintptr_t value)
{
    uint32_t           bit;
    ngx_radix_node_t  *node, *new;

    bit = 0x80000000;
    node = tree->root;

    while (node && (bit & mask)) {
        if (key & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;
    }

    if (node) {
        if (node->value) {
            return NGX_BUSY;
        }

        node->value = value;
        return NGX_OK;
    }

    while (bit & mask) {
        if (!(new = ngx_radix_alloc(tree, sizeof(ngx_radix_node_t)))) {
            return NGX_ERROR;
        }

        new->value = value;

        if (key & bit) {
            node->right = new;

        } else {
            node->left = new;
        }

        bit >>= 1;
        new = node;
    }

    return NGX_OK;
}


void ngx_radix32tree_delete(ngx_radix_tree_t *tree, uint32_t key, uint32_t mask)
{
    uint32_t           bit;
    ngx_radix_node_t  *node;

    bit = 0x80000000;
    node = tree->root;

    while (node && (bit & mask)) {
        if (key & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;
    }

    if (node) {
        node->value = (uintptr_t) 0;
    }
}


uintptr_t ngx_radix32tree_find(ngx_radix_tree_t *tree, uint32_t key)
{
    uint32_t           bit;
    uintptr_t          value;
    ngx_radix_node_t  *node;

    bit = 0x80000000;
    value = NULL;
    node = tree->root;

    while (node) {
        if (node->value) {
            value = node->value;
        }

        if (key & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;
    }

    return value;
}


static void *ngx_radix_alloc(ngx_radix_tree_t *tree, size_t size)
{
    char  *p;

    if (tree->size < size) {
        if (!(tree->free = ngx_palloc(tree->pool, NGX_RADIX_TREE_POOL_SIZE))) {
            return NULL;
        }

        tree->size = NGX_RADIX_TREE_POOL_SIZE;
    }

    p = tree->free;
    tree->free += size;
    tree->size -= size;

    return p;
}
