#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#define PAGE_LENGTH 4096
#define MIN_BLOCK_SIZE 128

struct header 
{
    char is_allocated;
    size_t size;
    struct header *next;
};


int FOIS = 0;

void *my_get_page(size_t size)
{
    ++FOIS;

    size_t factor = 0;
    size_t real_size;

    if (size < PAGE_LENGTH)
        size = PAGE_LENGTH;

    if (size >= PAGE_LENGTH)
    {
        factor = (size + PAGE_LENGTH - 1)/PAGE_LENGTH;
    }

    real_size = factor * PAGE_LENGTH;

    void *addr = mmap(NULL, real_size, PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (addr == MAP_FAILED)
        return NULL;
    
    struct header *head = (struct header *) addr;
    head->is_allocated = 0;
    head->size = real_size;
    head->next = NULL;
    
    return addr;
}

static struct header *list;

size_t get_cptd_size(size_t size)
{
    size_t cptd_size = 2;

    if (size < MIN_BLOCK_SIZE)
    {
        size = MIN_BLOCK_SIZE; 
        return size;
    }

    while (cptd_size <= size)
    {
        cptd_size *= 2; 
    }

    return cptd_size;
}

size_t save;
void *split(struct header *min, size_t cptd_size)
{
    size_t current_size = min->size;
    size_t splitted_size = current_size / 2;
    
    if (min->size == cptd_size)
    { 
        save = list->size;
        return min;
    }

    if ((current_size > cptd_size) && (splitted_size >= cptd_size))
    {
        struct header *phalf = (struct header *)(((char *)min) + splitted_size);
        phalf->is_allocated = 0;
        phalf->size = splitted_size;
        min->size = splitted_size;
        phalf->next = min->next;
        min->next = phalf;
        split(min, cptd_size);
    }

    save = list->size;
    return min;
}

    __attribute__((visibility("default")))
void *malloc(size_t __attribute__((unused)) size)
{
    struct header *p = list;
    struct header *min = NULL;
    
    if (size <= 0)
        return NULL;
    
    size_t cptd_size = get_cptd_size(size + sizeof(struct header));

    if (list == NULL)
    {
        list = (struct header *) my_get_page(cptd_size);
        save = list->size;
    }
    
    list->size = save;
    size_t smallest_size = 29981920;

    for (p = list; p != NULL; p = p->next)
    {
        if ((p->size >= cptd_size) && (p->size < smallest_size) &&
                (p->is_allocated == 0))
        {
            smallest_size = p->size;
            min = p;
        }
    }

    if (min)
    {
        struct header *slot = split(min, cptd_size);
        slot->is_allocated = 1;
        return (void *)(slot + 1);
    }

    struct header *c = (struct header *) my_get_page(cptd_size);
    
    if (c > list)
    {
        for (p = list; (p->next != NULL); p = p->next)
        {
            if (p->next > c)
            {
                c->next = p->next;
                p->next = c;
                break;
            }
            if (p->next == NULL)
            {
                p->next = c;
                c->next = NULL;
            }
        }
    }
    
    else
    {
        c->next = list;
        list = c;
        save = list->size;
    }

    void *new = malloc(size);
    
    return new;
}

void recurse_free_right (struct header *hd)
{
    if (hd->next != NULL)
    {
        if ((hd->next->size == hd->size) && (hd->next->is_allocated == 0))
        {
            hd->next = hd->next->next;
            hd->size = hd->size * 2;
            recurse_free_right(hd);
        }
    }
}

void recurse_free_left (struct header *hd)
{
    struct header *p = list;
    struct header *previous = list;

    while (p != hd)
    {
        previous = p;
        p = p->next;
        if (p == NULL)
            return;
    }

    if ((previous->size == p->size) && (p->is_allocated == 0))
    {
        previous->next = p->next;
        previous->size = previous->size * 2;
        
        if (previous == list)
            return;
        recurse_free_left(previous);
    }
}

    __attribute__((visibility("default")))
void free(void __attribute__((unused)) *ptr)
{
    if (ptr == NULL)
        return;
    
    struct header *hd = ((struct header *)ptr) - 1;

    if (hd < list)
    {
        return;
    }

    hd->is_allocated = 0; 
    recurse_free_right(hd);
    recurse_free_left(hd);
}

   __attribute__((visibility("default")))
void *realloc(void __attribute__((unused)) *ptr,
        size_t __attribute__((unused)) size)
{
    if (ptr == NULL)
        return malloc(size);     
    
    if (size == 0)
    {
        free(ptr);
    }

    struct header *new = malloc(size);
    struct header *hd_ptr = (struct header *)(((char *)ptr) - sizeof(struct
    header));
    size_t former_size = hd_ptr->size;
    size_t smallest_size = size < former_size ? size : former_size;
    void *r = memcpy(new, ptr, smallest_size);
    free(ptr);

    return r;
}

    __attribute__((visibility("default")))
void *calloc(size_t __attribute__((unused)) nmemb,
        size_t __attribute__((unused)) size)
{

    if ((nmemb == 0) || (size == 0))
    {
        return NULL;
    }

    void *ptr = malloc(size * nmemb);  
     
    memset(ptr, 0, size*nmemb);
    
    return ptr;
}
