#include "mymalloc.h"

#include <stdio.h>
#include <unistd.h>

static Block *last_block = NULL; // son tahsis edilen blokdur
Block *Find_f_blok(size_t data_size);
Block *init_mymalloc(void *start_adres, size_t size);
void add_to_free_list(Block *block);

void add_to_free_list(Block *block)
{
    ListType listtype2 = getlisttype();
    if (listtype2 == ADDR_ORDERED_LIST)
    {
        Block *current_block = free_list;
        Block *prev_block = NULL;

        // Yerleştirilecek konumu bul
        while (current_block != NULL && current_block < block)
        {
            prev_block = current_block;
            current_block = current_block->next;
        }

        // Bloğu doğru konuma yerleştir
        block->next = current_block;
        block->prev = prev_block;
        if (prev_block != NULL)
        {
            prev_block->next = block;
        }
        else
        {
            free_list = block;
        }
        if (current_block != NULL)
        {
            current_block->prev = block;
        }
    }
    else if (listtype2 == UNORDERED_LIST)
    {
        // Sadece bloğu boş olarak işaretle ve listenin başına ekle
        block->next = free_list;
        block->prev = NULL;
        if (free_list != NULL)
        {
            free_list->prev = block;
        }
        free_list = block;
    }
}


Block *Find_f_blok(size_t data_size)
{
    Strategy strategy2 = getstrategy(); 
    Block *empty_block = NULL;

    if (strategy2 == FIRST_FIT)
    {
        Block *current_block = free_list;
        while (current_block != NULL)
        {
            if (current_block->info.isfree && current_block->info.size >= data_size)
            {
                empty_block = current_block;
                break;
            }
            current_block = current_block->next;
        }
    }
    else if (strategy2== NEXT_FIT)
    {
        static Block *last_checked_block = NULL; // Son kontrol edilen blok
        Block *current_block = (last_checked_block != NULL) ? last_checked_block->next : free_list; // Eğer son kontrol edilen blok varsa onun bir sonraki bloktan başla, yoksa serbest blok listesinin başından başla
        
        // Serbest blokları döngü içinde gezerek uygun bloğu bul
        while (current_block != NULL)
        {
            if (current_block->info.isfree && current_block->info.size >= data_size)
            {
                empty_block = current_block;
                last_checked_block = current_block; // Son kontrol edilen bloğu güncelle
                break;
            }
            current_block = current_block->next;
        }

        // Serbest blokların sonuna gelindiğinde, liste başına dön
        if (empty_block == NULL)
        {
            current_block = free_list;
            while (current_block != last_checked_block)
            {
                if (current_block->info.isfree && current_block->info.size >= data_size)
                {
                    empty_block = current_block;
                    last_checked_block = current_block; // Son kontrol edilen bloğu güncelle
                    break;
                }
                current_block = current_block->next;
            }
        }
    }
    else if (strategy2 == BEST_FIT)
    {
        Block *best_fit_block = NULL;
        size_t min_size_diff = UINT64_MAX; // Minimum boyut farkı
        Block *current_block = free_list;

        while (current_block != NULL)
        {
            if (current_block->info.isfree && current_block->info.size >= data_size)
            {
                size_t size_diff = current_block->info.size - data_size;
                if (size_diff < min_size_diff)
                {
                    min_size_diff = size_diff;
                    best_fit_block = current_block;
                }
            }
            current_block = current_block->next;
        }

        empty_block = best_fit_block;
    }
    else if (strategy2 == WORST_FIT)
    {
        Block *worst_fit_block = NULL;
        size_t max_size_diff = 0; // Maksimum boyut farkı
        Block *current_block = free_list;

        while (current_block != NULL)
        {
            if (current_block->info.isfree && current_block->info.size >= data_size)
            {
                size_t size_diff = current_block->info.size - data_size;
                if (size_diff > max_size_diff)
                {
                    max_size_diff = size_diff;
                    worst_fit_block = current_block;
                }
            }
            current_block = current_block->next;
        }

        empty_block = worst_fit_block;
    }
    else
    {
        return NULL;
    }

    return empty_block;
}
//sbrk ile yeni alan ayırma durumlarında kullanılan fonksiyon
Block *init_mymalloc(void *start_adres, size_t size)
{
    int data_s = size - sizeof(Block) - sizeof(Tag);
    if (data_s < 0)
        return NULL;

    Block *b = (Block *)start_adres;
    b->info.isfree = 1;
    b->info.size = data_s;
    b->next = NULL;
    b->prev = NULL;
    Tag *btag = (Tag *)((char *)start_adres + sizeof(Block) + data_s);
    btag->isfree = 1;
    btag->size = data_s;

    return b;
}

void *mymalloc(size_t size)
{
    /* İstenilen alanı 16'nın katı yap */
    size = numberof16blocks(size);

    // İlk çağrı olduğu için alan ayır ve blok haline getir
    static int first = 1;
    if (first == 1)
    {
        heap_start = sbrk(0);
        void *result = sbrk(HEAP_SIZE);
        if (result == (void *)-1)
        {
            perror("sbrk error: not available memory");
            return NULL;
        }
        Block *b = init_mymalloc(heap_start, HEAP_SIZE);
        heap_end = heap_start + HEAP_SIZE;
        // Şimdi bloğu istenilen alana göre böl ve datayı return et
        Block *bnew = split_block(b, size);
        first = 0;
        return bnew->data;
    }

    // Stratejiye göre boş bir blok bul
    Block *b2 = Find_f_blok(size);
    if (b2 == NULL)
    {
        // Tekrar alan ayır
        void *start_addr = sbrk(0);
        void *result = sbrk(HEAP_SIZE);
        if (result == (void *)-1)
        {
            perror("sbrk error: not available memory");
            return NULL;
        }
        Block *b3 = init_mymalloc(start_addr, HEAP_SIZE);
        Block *b4 = split_block(b3, size);
        // İkinci bloğu bir büyük blok yap ve bu bloğu böldükten sonra heap endi 
        //güncelle
        heap_end = heap_start + HEAP_SIZE;
        return b4->data;
    }

    // Bulunan blok bölünme için gereken şartları sağlıyorsa böl, yoksa bulunan bloğun datasını döndür
    size_t blok2_size = b2->info.size - sizeof(Block *) - sizeof(Tag);
    blok2_size = 16; // 16'nın katı yap;
    if (blok2_size >= 16)
    {
        Block *cur_b = split_block(b2, size);
        return cur_b->data;
    }
    else
    {
        return b2->data;
    }
    return b2->data;
}

void myfree(void *p)
{
    Block *current_block = (Block *)((char *)p - sizeof(Block));
    current_block->info.isfree = 1;
    Tag *endtag = (Tag *)((char *)p + current_block->info.size);
    endtag->isfree = 1;

    Block *prev = prev_block_in_addr(current_block);
    Block *next = next_block_in_addr(current_block);
    int flag = 0 , flag2 = 0;
    
    if (prev != NULL && prev->info.isfree == 1)
    {
        current_block = left_coalesce(current_block);
        flag =1 ;
    }
    else if (next != NULL && next->info.isfree == 1)
    {
        flag2 = 1;
        current_block = right_coalesce(current_block);
    }
    if(flag == 1 || flag2 == 1){
        add_to_free_list(current_block);
    }

    //free listte ekleme yapılması
    if (listtype == ADDR_ORDERED_LIST)
    {
        // Komşu boş blokların işaretçilerini ayarla
        if (current_block->prev != NULL)
        {
            current_block->prev->next = current_block->next;
        }
        else
        {
            free_list = current_block->next;
        }
        if (current_block->next != NULL)
        {
            current_block->next->prev = current_block->prev;
        }
    }
    else if (listtype == UNORDERED_LIST)
    {
        // Sadece bloğu boş olarak işaretle
        current_block->next = free_list;
        current_block->prev = NULL;
        if (free_list != NULL)
        {
            free_list->prev = current_block;
        }
        free_list = current_block;
    }

    // tek bir blok haline getir
}

Block *split_block(Block *b, size_t size)
{

    size_t remaining_s = b->info.size - size - sizeof(Block) - sizeof(Tag);
    b->info.size = size;
    b->info.isfree = 0;
    Tag *btag = (Tag *)((char *)b + sizeof(Block) + size);
    btag->size = size;
    btag->isfree = 0;
    last_block = b;

    Block *new_b = (Block *)((char *)b + sizeof(Block) + size + sizeof(Tag));
    new_b->info.size = remaining_s;
    new_b->info.isfree = 1;
    Tag *rtag = (Tag *)((char *)new_b + remaining_s);
    rtag->isfree = 1;
    rtag->size = remaining_s;
    add_to_free_list(new_b);

    return b;
}

Block *left_coalesce(Block *b)
{

    // Sol komşu bloğun adresini hesapla
    Block *left = prev_block_in_freelist(b);

    // Eğer sol komşu blok boşsa birleştirme yap
    if (left != NULL && left->info.isfree == 1)
    {
        // Sol bloğun boyutunu güncelle
        left->info.size += sizeof(Block) + b->info.size + sizeof(Tag);
        // Sol bloğun son etiketini güncelle
        Tag *left_tag = (Tag *)((char *)left + left->info.size - sizeof(Tag));
        left_tag->size = left->info.size;
        left_tag->isfree = 1;

        // Birleştirme sonucu oluşan bloğu serbest blok listesine ekle
        add_to_free_list(left);

        // Birleştirme sonucu oluşan bloğun adresini döndür
        return left;
    }

    // Birleştirme yapılamadıysa gelen bloğu serbest blok listesine ekle
    add_to_free_list(b);
    return b;
}

Block *right_coalesce(Block *b)
{
// Sağ komşu bloğun adresini hesapla
    Block *right = next_block_in_freelist(b);

    // Eğer sağ komşu blok boşsa birleştirme yap
    if (right != NULL && right->info.isfree == 1)
    {
        // Gelen bloğun boyutunu güncelle
        b->info.size += sizeof(Block) + right->info.size + sizeof(Tag);
        // Gelen bloğun son etiketini güncelle
        Tag *btag = (Tag *)((char *)b + sizeof(Block) + b->info.size);
        btag->size = b->info.size;
        btag->isfree = 1;

        // Sağ bloğun serbest blok listesinden çıkar
        if (right->next != NULL)
        {
            right->next->prev = right->prev;
        }
        if (right->prev != NULL)
        {
            right->prev->next = right->next;
        }
        else
        {
            free_list = right->next;
        }

        // Birleştirme sonucu oluşan bloğun adresini döndür
        return b;
    }

    // Birleştirme yapılamadıysa gelen bloğu serbest blok listesine ekle
    add_to_free_list(b);
    return b;
}

Block *next_block_in_freelist(Block *b)
{
    Block *next = b->next;
    return next;
}

Block *prev_block_in_freelist(Block *b)
{
    Block *prev = b->prev;
    return prev;
}

Block *prev_block_in_addr(Block *b)
{
    Tag *left = (Tag *)((char *)b - sizeof(Tag));
    size_t left_s = left->size;
    Block *prev = (Block *)((char *)b - left_s - sizeof(Block) - sizeof(Tag));
    return prev;
}

Block *next_block_in_addr(Block *b)
{
    Block *next = (Block *)((char *)b +sizeof(Block) + sizeof(Tag) + b->info.size);
    return next;
}
// bloğu 16 ın katı büyüklüğü haline getir
uint64_t numberof16blocks(size_t size_inbytes)
{
    size_inbytes = (size_inbytes + 15) & ~15;
    return size_inbytes;
}

void printheap()
{
    Block *iter2 = heap_start;
    while (iter2 != NULL && iter2 < heap_end)
    {
        printf("------------------\n");
        printf("size: %ld\nfree: %d\n", iter2->info.size, iter2->info.isfree);
        iter2 = next_block_in_addr(iter2);
    }
}

ListType getlisttype()
{
    return listtype;
}

int setlisttype(ListType listtype2)
{
    listtype = listtype2;
    return 0;
}

Strategy getstrategy()
{
    return strategy;
}

int setstrategy(Strategy strategy2)
{
    strategy =strategy2;
    return 0;
}
