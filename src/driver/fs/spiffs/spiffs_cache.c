/*
 * spiffs_cache.c
 *
 *  Created on: Jun 23, 2013
 *      Author: petera
 */

#include "spiffs.h"
#include "spiffs_nucleus.h"

#if SPIFFS_CACHE

// returns cached page for give page index, or null if no such cached page
static spiffs_cache_page *spiffs_cache_page_get(spiffs *fs, spiffs_page_ix pix)
{
	spiffs_cache *cache = spiffs_get_cache(fs);
	if ((cache->cpage_use_map & cache->cpage_use_mask) == 0)
		return 0;
	int i;
	for (i = 0; i < cache->cpage_count; i++) {
		spiffs_cache_page *cp = spiffs_get_cache_page_hdr(fs, cache, i);
		if ((cache->cpage_use_map & (1 << i)) &&
		    (cp->flags & SPIFFS_CACHE_FLAG_TYPE_WR) == 0 && cp->pix == pix) {
			SPIFFS_CACHE_DBG("CACHE_GET: have cache page %i for %04x\n", i, pix);
			cp->last_access = cache->last_access;
			return cp;
		}
	}
	//SPIFFS_CACHE_DBG("CACHE_GET: no cache for %04x\n", pix);
	return 0;
}

// frees cached page
static s32_t spiffs_cache_page_free(spiffs *fs, int ix, u8_t write_back)
{
	s32_t res = SPIFFS_OK;
	spiffs_cache *cache = spiffs_get_cache(fs);
	spiffs_cache_page *cp = spiffs_get_cache_page_hdr(fs, cache, ix);
	if (cache->cpage_use_map & (1 << ix)) {
		if (write_back && (cp->flags & SPIFFS_CACHE_FLAG_TYPE_WR) == 0 &&
		    (cp->flags & SPIFFS_CACHE_FLAG_DIRTY)) {
			u8_t *mem = spiffs_get_cache_page(fs, cache, ix);
			res = fs->cfg.hal_write_f(SPIFFS_PAGE_TO_PADDR(fs, cp->pix),
						  SPIFFS_CFG_LOG_PAGE_SZ(fs), mem);
		}

		cp->flags = 0;
		cache->cpage_use_map &= ~(1 << ix);

		if (cp->flags & SPIFFS_CACHE_FLAG_TYPE_WR) {
			SPIFFS_CACHE_DBG("CACHE_FREE: free cache page %i objid %04x\n", ix,
					 cp->obj_id);
		} else {
			SPIFFS_CACHE_DBG("CACHE_FREE: free cache page %i pix %04x\n", ix, cp->pix);
		}
	}

	return res;
}

// removes the oldest accessed cached page
static s32_t spiffs_cache_page_remove_oldest(spiffs *fs, u8_t flag_mask, u8_t flags)
{
	s32_t res = SPIFFS_OK;
	spiffs_cache *cache = spiffs_get_cache(fs);

	if ((cache->cpage_use_map & cache->cpage_use_mask) != cache->cpage_use_mask) {
		// at least one free cpage
		return SPIFFS_OK;
	}

	// all busy, scan thru all to find the cpage which has oldest access
	int i;
	int cand_ix = -1;
	u32_t oldest_val = 0;
	for (i = 0; i < cache->cpage_count; i++) {
		spiffs_cache_page *cp = spiffs_get_cache_page_hdr(fs, cache, i);
		if ((cache->last_access - cp->last_access) > oldest_val &&
		    (cp->flags & flag_mask) == flags) {
			oldest_val = cache->last_access - cp->last_access;
			cand_ix = i;
		}
	}

	if (cand_ix >= 0) {
		res = spiffs_cache_page_free(fs, cand_ix, 1);
	}

	return res;
}

// allocates a new cached page and returns it, or null if all cache pages are busy
static spiffs_cache_page *spiffs_cache_page_allocate(spiffs *fs)
{
	spiffs_cache *cache = spiffs_get_cache(fs);
	if (cache->cpage_use_map == 0xffffffff) {
		// out of cache memory
		return 0;
	}
	int i;
	for (i = 0; i < cache->cpage_count; i++) {
		if ((cache->cpage_use_map & (1 << i)) == 0) {
			spiffs_cache_page *cp = spiffs_get_cache_page_hdr(fs, cache, i);
			cache->cpage_use_map |= (1 << i);
			cp->last_access = cache->last_access;
			SPIFFS_CACHE_DBG("CACHE_ALLO: allocated cache page %i\n", i);
			return cp;
		}
	}
	// out of cache entries
	return 0;
}

// drops the cache page for give page index
void spiffs_cache_drop_page(spiffs *fs, spiffs_page_ix pix)
{
	spiffs_cache_page *cp = spiffs_cache_page_get(fs, pix);
	if (cp) {
		spiffs_cache_page_free(fs, cp->ix, 0);
	}
}

// ------------------------------

// reads from spi flash or the cache
s32_t spiffs_phys_rd(spiffs *fs, u8_t op, spiffs_file fh, u32_t addr, u32_t len, u8_t *dst)
{
	s32_t res = SPIFFS_OK;
	spiffs_cache *cache = spiffs_get_cache(fs);
	spiffs_cache_page *cp = spiffs_cache_page_get(fs, SPIFFS_PADDR_TO_PAGE(fs, addr));
	cache->last_access++;
	if (cp) {
#if SPIFFS_CACHE_STATS
		fs->cache_hits++;
#endif
		cp->last_access = cache->last_access;
	} else {
		if ((op & SPIFFS_OP_TYPE_MASK) == SPIFFS_OP_T_OBJ_LU2) {
			// for second layer lookup functions, we do not cache in order to prevent shredding
			return fs->cfg.hal_read_f(addr, len, dst);
		}
#if SPIFFS_CACHE_STATS
		fs->cache_misses++;
#endif
		res = spiffs_cache_page_remove_oldest(fs, SPIFFS_CACHE_FLAG_TYPE_WR, 0);
		cp = spiffs_cache_page_allocate(fs);
		if (cp) {
			cp->flags = SPIFFS_CACHE_FLAG_WRTHRU;
			cp->pix = SPIFFS_PADDR_TO_PAGE(fs, addr);
		}

		s32_t res2 = fs->cfg.hal_read_f(addr - SPIFFS_PADDR_TO_PAGE_OFFSET(fs, addr),
						SPIFFS_CFG_LOG_PAGE_SZ(fs),
						spiffs_get_cache_page(fs, cache, cp->ix));
		if (res2 != SPIFFS_OK) {
			res = res2;
		}
	}
	u8_t *mem = spiffs_get_cache_page(fs, cache, cp->ix);
	memcpy(dst, &mem[SPIFFS_PADDR_TO_PAGE_OFFSET(fs, addr)], len);
	return res;
}

// writes to spi flash and/or the cache
s32_t spiffs_phys_wr(spiffs *fs, u8_t op, spiffs_file fh, u32_t addr, u32_t len, u8_t *src)
{
	spiffs_page_ix pix = SPIFFS_PADDR_TO_PAGE(fs, addr);
	spiffs_cache *cache = spiffs_get_cache(fs);
	spiffs_cache_page *cp = spiffs_cache_page_get(fs, pix);

	if (cp && (op & SPIFFS_OP_COM_MASK) != SPIFFS_OP_C_WRTHRU) {
		// have a cache page
		// copy in data to cache page

		if ((op & SPIFFS_OP_COM_MASK) == SPIFFS_OP_C_DELE &&
		    (op & SPIFFS_OP_TYPE_MASK) != SPIFFS_OP_T_OBJ_LU) {
			// page is being deleted, wipe from cache - unless it is a lookup page
			spiffs_cache_page_free(fs, cp->ix, 0);
			return fs->cfg.hal_write_f(addr, len, src);
		}

		u8_t *mem = spiffs_get_cache_page(fs, cache, cp->ix);
		memcpy(&mem[SPIFFS_PADDR_TO_PAGE_OFFSET(fs, addr)], src, len);

		cache->last_access++;
		cp->last_access = cache->last_access;

		if (cp->flags && SPIFFS_CACHE_FLAG_WRTHRU) {
			// page is being updated, no write-cache, just pass thru
			return fs->cfg.hal_write_f(addr, len, src);
		} else {
			return SPIFFS_OK;
		}
	} else {
		// no cache page, no write cache - just write thru
		return fs->cfg.hal_write_f(addr, len, src);
	}
}

#if SPIFFS_CACHE_WR
// returns the cache page that this fd refers, or null if no cache page
spiffs_cache_page *spiffs_cache_page_get_by_fd(spiffs *fs, spiffs_fd *fd)
{
	spiffs_cache *cache = spiffs_get_cache(fs);

	if ((cache->cpage_use_map & cache->cpage_use_mask) == 0) {
		// all cpages free, no cpage cannot be assigned to obj_id
		return 0;
	}

	int i;
	for (i = 0; i < cache->cpage_count; i++) {
		spiffs_cache_page *cp = spiffs_get_cache_page_hdr(fs, cache, i);
		if ((cache->cpage_use_map & (1 << i)) && (cp->flags & SPIFFS_CACHE_FLAG_TYPE_WR) &&
		    cp->obj_id == fd->obj_id) {
			return cp;
		}
	}

	return 0;
}

// allocates a new cache page and refers this to given fd - flushes an old cache
// page if all cache is busy
spiffs_cache_page *spiffs_cache_page_allocate_by_fd(spiffs *fs, spiffs_fd *fd)
{
	// before this function is called, it is ensured that there is no already existing
	// cache page with same object id
	spiffs_cache_page_remove_oldest(fs, SPIFFS_CACHE_FLAG_TYPE_WR, 0);
	spiffs_cache_page *cp = spiffs_cache_page_allocate(fs);
	if (cp == 0) {
		// could not get cache page
		return 0;
	}

	cp->flags = SPIFFS_CACHE_FLAG_TYPE_WR;
	cp->obj_id = fd->obj_id;
	fd->cache_page = cp;
	return cp;
}

// unrefers all fds that this cache page refers to and releases the cache page
void spiffs_cache_fd_release(spiffs *fs, spiffs_cache_page *cp)
{
	if (cp == 0)
		return;
	int i;
	spiffs_fd *fds = (spiffs_fd *)fs->fd_space;
	for (i = 0; i < fs->fd_count; i++) {
		spiffs_fd *cur_fd = &fds[i];
		if (cur_fd->file_nbr != 0 && cur_fd->cache_page == cp) {
			cur_fd->cache_page = 0;
		}
	}
	spiffs_cache_page_free(fs, cp->ix, 0);

	cp->obj_id = 0;
}

#endif

// initializes the cache
void spiffs_cache_init(spiffs *fs)
{
	if (fs->cache == 0)
		return;
	u32_t sz = fs->cache_size;
	u32_t cache_mask = 0;
	int i;
	int cache_entries = (sz - sizeof(spiffs_cache)) / (SPIFFS_CACHE_PAGE_SIZE(fs));
	if (cache_entries <= 0)
		return;

	for (i = 0; i < cache_entries; i++) {
		cache_mask <<= 1;
		cache_mask |= 1;
	}

	spiffs_cache cache;
	memset(&cache, 0, sizeof(spiffs_cache));
	cache.cpage_count = cache_entries;
	cache.cpages = (u8_t *)(fs->cache + sizeof(spiffs_cache));

	cache.cpage_use_map = (u32_t)0xffffffffUL;
	cache.cpage_use_mask = cache_mask;
	memcpy(fs->cache, &cache, sizeof(spiffs_cache));

	spiffs_cache *c = spiffs_get_cache(fs);

	memset(c->cpages, 0, c->cpage_count * SPIFFS_CACHE_PAGE_SIZE(fs));

	c->cpage_use_map &= ~(c->cpage_use_mask);
	for (i = 0; i < cache.cpage_count; i++) {
		spiffs_get_cache_page_hdr(fs, c, i)->ix = i;
	}
}

#endif // SPIFFS_CACHE
