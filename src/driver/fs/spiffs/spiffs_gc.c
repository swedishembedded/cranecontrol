#include "spiffs.h"
#include "spiffs_nucleus.h"

// Erases a logical block and updates the erase counter.
// If cache is enabled, all pages that might be cached in this block
// is dropped.
static s32_t spiffs_gc_erase_block(spiffs *fs, spiffs_block_ix bix)
{
	s32_t res;
	u32_t addr = SPIFFS_BLOCK_TO_PADDR(fs, bix);
	s32_t size = SPIFFS_CFG_LOG_BLOCK_SZ(fs);

	SPIFFS_GC_DBG("gc: erase block %i\n", bix);

	// here we ignore res, just try erasing the block
	while (size > 0) {
		SPIFFS_GC_DBG("gc: erase %08x:%08x\n", addr, SPIFFS_CFG_PHYS_ERASE_SZ(fs));
		(void)fs->cfg.hal_erase_f(addr, SPIFFS_CFG_PHYS_ERASE_SZ(fs));
		addr += SPIFFS_CFG_PHYS_ERASE_SZ(fs);
		size -= SPIFFS_CFG_PHYS_ERASE_SZ(fs);
	}
	fs->free_blocks++;

	// register erase count for this block
	res = _spiffs_wr(fs, SPIFFS_OP_C_WRTHRU | SPIFFS_OP_T_OBJ_LU2, 0,
			 SPIFFS_ERASE_COUNT_PADDR(fs, bix), sizeof(spiffs_obj_id),
			 (u8_t *)&fs->max_erase_count);
	SPIFFS_CHECK_RES(res);

	fs->max_erase_count++;
	if (fs->max_erase_count == SPIFFS_OBJ_ID_IX_FLAG) {
		fs->max_erase_count = 0;
	}

#if SPIFFS_CACHE
	{
		int i;
		for (i = 0; i < SPIFFS_PAGES_PER_BLOCK(fs); i++) {
			spiffs_cache_drop_page(fs, SPIFFS_PAGE_FOR_BLOCK(fs, bix) + i);
		}
	}
#endif
	return res;
}

// Searches for blocks where all entries are deleted - if one is found,
// the block is erased. Compared to the non-quick gc, the quick one ensures
// that no updates are needed on existing objects on pages that are erased.
s32_t spiffs_gc_quick(spiffs *fs)
{
	s32_t res = SPIFFS_OK;
	u32_t blocks = fs->block_count;
	spiffs_block_ix cur_block = 0;
	u32_t cur_block_addr = 0;
	int cur_entry = 0;
	spiffs_obj_id *obj_lu_buf = (spiffs_obj_id *)fs->lu_work;

	SPIFFS_GC_DBG("gc_quick: running\n", cur_block);
#if SPIFFS_GC_STATS
	fs->stats_gc_runs++;
#endif

	u32_t entries_per_page = (SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(spiffs_obj_id));

	// find fully deleted blocks
	// check each block
	while (res == SPIFFS_OK && blocks--) {
		u16_t deleted_pages_in_block = 0;

		int obj_lookup_page = 0;
		// check each object lookup page
		while (res == SPIFFS_OK && obj_lookup_page < SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
			int entry_offset = obj_lookup_page * entries_per_page;
			res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_READ, 0,
					 cur_block_addr + SPIFFS_PAGE_TO_PADDR(fs, obj_lookup_page),
					 SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->lu_work);
			// check each entry
			while (res == SPIFFS_OK && cur_entry - entry_offset < entries_per_page &&
			       cur_entry <
				       SPIFFS_PAGES_PER_BLOCK(fs) - SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
				spiffs_obj_id obj_id = obj_lu_buf[cur_entry - entry_offset];
				if (obj_id == SPIFFS_OBJ_ID_DELETED) {
					deleted_pages_in_block++;
				} else if (obj_id == SPIFFS_OBJ_ID_FREE) {
					// kill scan, go for next block
					obj_lookup_page = SPIFFS_OBJ_LOOKUP_PAGES(fs);
					break;
				} else {
					// kill scan, go for next block
					obj_lookup_page = SPIFFS_OBJ_LOOKUP_PAGES(fs);
					break;
				}
				cur_entry++;
			} // per entry
			obj_lookup_page++;
		} // per object lookup page

		if (res == SPIFFS_OK &&
		    deleted_pages_in_block ==
			    SPIFFS_PAGES_PER_BLOCK(fs) - SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
			// found a fully deleted block
			fs->stats_p_deleted -= deleted_pages_in_block;
			res = spiffs_gc_erase_block(fs, cur_block);
			return res;
		}

		cur_entry = 0;
		cur_block++;
		cur_block_addr += SPIFFS_CFG_LOG_BLOCK_SZ(fs);
	} // per block

	return res;
}

// Checks if garbaga collecting is necessary. If so a candidate block is found,
// cleansed and erased
s32_t spiffs_gc_check(spiffs *fs, u32_t len)
{
	s32_t res;
	u32_t free_pages =
		(SPIFFS_PAGES_PER_BLOCK(fs) - SPIFFS_OBJ_LOOKUP_PAGES(fs)) * fs->block_count -
		fs->stats_p_allocated - fs->stats_p_deleted;
	int tries = 0;

	if (fs->free_blocks > 3 && len < free_pages * SPIFFS_DATA_PAGE_SIZE(fs)) {
		return SPIFFS_OK;
	}

	//printf("gcing started  %i dirty, blocks %i free, want %i bytes\n", fs->stats_p_allocated + fs->stats_p_deleted, fs->free_blocks, len);

	do {
		SPIFFS_GC_DBG(
			"\ngc_check #%i: run gc free_blocks:%i pfree:%i pallo:%i pdele:%i [%i] len:%i of %i\n",
			tries, fs->free_blocks, free_pages, fs->stats_p_allocated,
			fs->stats_p_deleted,
			(free_pages + fs->stats_p_allocated + fs->stats_p_deleted), len,
			free_pages * SPIFFS_DATA_PAGE_SIZE(fs));

		spiffs_block_ix *cands;
		int count;
		spiffs_block_ix cand;
		res = spiffs_gc_find_candidate(fs, &cands, &count);
		SPIFFS_CHECK_RES(res);
		if (count == 0) {
			SPIFFS_GC_DBG("gc_check: no candidates, return\n");
			return res;
		}
#if SPIFFS_GC_STATS
		fs->stats_gc_runs++;
#endif
		cand = cands[0];
		fs->cleaning = 1;
		//printf("gcing: cleaning block %i\n", cand);
		res = spiffs_gc_clean(fs, cand);
		fs->cleaning = 0;
		if (res < 0) {
			SPIFFS_GC_DBG("gc_check: cleaning block %i, result %i\n", cand, res);
		} else {
			SPIFFS_GC_DBG("gc_check: cleaning block %i, result %i\n", cand, res);
		}
		SPIFFS_CHECK_RES(res);

		res = spiffs_gc_erase_page_stats(fs, cand);
		SPIFFS_CHECK_RES(res);

		res = spiffs_gc_erase_block(fs, cand);
		SPIFFS_CHECK_RES(res);

		free_pages = (SPIFFS_PAGES_PER_BLOCK(fs) - SPIFFS_OBJ_LOOKUP_PAGES(fs)) *
				     fs->block_count -
			     fs->stats_p_allocated - fs->stats_p_deleted;

	} while (++tries < SPIFFS_GC_MAX_RUNS &&
		 (fs->free_blocks <= 2 || len > free_pages * SPIFFS_DATA_PAGE_SIZE(fs)));
	SPIFFS_GC_DBG("gc_check: finished\n");

	//printf("gcing finished %i dirty, blocks %i free, %i pages free, %i tries, res %i\n",
	//    fs->stats_p_allocated + fs->stats_p_deleted,
	//    fs->free_blocks, free_pages, tries, res);

	return res;
}

// Updates page statistics for a block that is about to be erased
s32_t spiffs_gc_erase_page_stats(spiffs *fs, spiffs_block_ix bix)
{
	s32_t res = SPIFFS_OK;
	int obj_lookup_page = 0;
	u32_t entries_per_page = (SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(spiffs_obj_id));
	spiffs_obj_id *obj_lu_buf = (spiffs_obj_id *)fs->lu_work;
	int cur_entry = 0;
	u32_t dele = 0;
	u32_t allo = 0;

	// check each object lookup page
	while (res == SPIFFS_OK && obj_lookup_page < SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
		int entry_offset = obj_lookup_page * entries_per_page;
		res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_READ, 0,
				 bix * SPIFFS_CFG_LOG_BLOCK_SZ(fs) +
					 SPIFFS_PAGE_TO_PADDR(fs, obj_lookup_page),
				 SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->lu_work);
		// check each entry
		while (res == SPIFFS_OK && cur_entry - entry_offset < entries_per_page &&
		       cur_entry < SPIFFS_PAGES_PER_BLOCK(fs) - SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
			spiffs_obj_id obj_id = obj_lu_buf[cur_entry - entry_offset];
			if (obj_id == SPIFFS_OBJ_ID_FREE) {
			} else if (obj_id == SPIFFS_OBJ_ID_DELETED) {
				dele++;
			} else {
				allo++;
			}
			cur_entry++;
		} // per entry
		obj_lookup_page++;
	} // per object lookup page
	SPIFFS_GC_DBG("gc_check: wipe pallo:%i pdele:%i\n", allo, dele);
	fs->stats_p_allocated -= allo;
	fs->stats_p_deleted -= dele;
	return res;
}

// Finds block candidates to erase
s32_t spiffs_gc_find_candidate(spiffs *fs, spiffs_block_ix **block_candidates, int *candidate_count)
{
	s32_t res = SPIFFS_OK;
	u32_t blocks = fs->block_count;
	spiffs_block_ix cur_block = 0;
	u32_t cur_block_addr = 0;
	spiffs_obj_id *obj_lu_buf = (spiffs_obj_id *)fs->lu_work;
	int cur_entry = 0;

	// using fs->work area as sorted candidate memory, (spiffs_block_ix)cand_bix/(s32_t)score
	int max_candidates = MIN(fs->block_count, (SPIFFS_CFG_LOG_PAGE_SZ(fs) -
						   8) / (sizeof(spiffs_block_ix) + sizeof(s32_t)));
	*candidate_count = 0;
	memset(fs->work, 0xff, SPIFFS_CFG_LOG_PAGE_SZ(fs));

	// divide up work area into block indices and scores
	// todo alignment?
	spiffs_block_ix *cand_blocks = (spiffs_block_ix *)fs->work;
	s32_t *cand_scores = (s32_t *)(fs->work + max_candidates * sizeof(spiffs_block_ix));

	*block_candidates = cand_blocks;

	u32_t entries_per_page = (SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(spiffs_obj_id));

	// check each block
	while (res == SPIFFS_OK && blocks--) {
		u16_t deleted_pages_in_block = 0;
		u16_t used_pages_in_block = 0;

		int obj_lookup_page = 0;
		// check each object lookup page
		while (res == SPIFFS_OK && obj_lookup_page < SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
			int entry_offset = obj_lookup_page * entries_per_page;
			res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_READ, 0,
					 cur_block_addr + SPIFFS_PAGE_TO_PADDR(fs, obj_lookup_page),
					 SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->lu_work);
			// check each entry
			while (res == SPIFFS_OK && cur_entry - entry_offset < entries_per_page &&
			       cur_entry <
				       SPIFFS_PAGES_PER_BLOCK(fs) - SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
				spiffs_obj_id obj_id = obj_lu_buf[cur_entry - entry_offset];
				if (obj_id == SPIFFS_OBJ_ID_FREE) {
					// when a free entry is encountered, scan logic ensures that all following entries are free also
					break;
				} else if (obj_id == SPIFFS_OBJ_ID_DELETED) {
					deleted_pages_in_block++;
				} else {
					used_pages_in_block++;
				}
				cur_entry++;
			} // per entry
			obj_lookup_page++;
		} // per object lookup page

		// calculate score and insert into candidate table
		// stoneage sort, but probably not so many blocks
		if (res == SPIFFS_OK && deleted_pages_in_block > 0) {
			// read erase count
			spiffs_obj_id erase_count;
			res = _spiffs_rd(fs, SPIFFS_OP_C_READ | SPIFFS_OP_T_OBJ_LU2, 0,
					 SPIFFS_ERASE_COUNT_PADDR(fs, cur_block),
					 sizeof(spiffs_obj_id), (u8_t *)&erase_count);
			SPIFFS_CHECK_RES(res);

			spiffs_obj_id erase_age;
			if (fs->max_erase_count > erase_count) {
				erase_age = fs->max_erase_count - erase_count;
			} else {
				erase_age =
					SPIFFS_OBJ_ID_FREE - (erase_count - fs->max_erase_count);
			}

			s32_t score = deleted_pages_in_block * SPIFFS_GC_HEUR_W_DELET +
				      used_pages_in_block * SPIFFS_GC_HEUR_W_USED +
				      erase_age * SPIFFS_GC_HEUR_W_ERASE_AGE;
			int cand_ix = 0;
			SPIFFS_GC_DBG("gc_check: bix:%i del:%i use:%i score:%i\n", cur_block,
				      deleted_pages_in_block, used_pages_in_block, score);
			while (cand_ix < max_candidates) {
				if (cand_blocks[cand_ix] == (spiffs_block_ix)-1) {
					cand_blocks[cand_ix] = cur_block;
					cand_scores[cand_ix] = score;
					break;
				} else if (cand_scores[cand_ix] < score) {
					int reorder_cand_ix = max_candidates - 2;
					while (reorder_cand_ix >= cand_ix) {
						cand_blocks[reorder_cand_ix + 1] =
							cand_blocks[reorder_cand_ix];
						cand_scores[reorder_cand_ix + 1] =
							cand_scores[reorder_cand_ix];
						reorder_cand_ix--;
					}
					cand_blocks[cand_ix] = cur_block;
					cand_scores[cand_ix] = score;
					break;
				}
				cand_ix++;
			}
			(*candidate_count)++;
		}

		cur_entry = 0;
		cur_block++;
		cur_block_addr += SPIFFS_CFG_LOG_BLOCK_SZ(fs);
	} // per block

	return res;
}

typedef enum { FIND_OBJ_DATA, MOVE_OBJ_DATA, MOVE_OBJ_IX, FINISHED } spiffs_gc_clean_state;

typedef struct {
	spiffs_gc_clean_state state;
	spiffs_obj_id cur_obj_id;
	spiffs_span_ix cur_objix_spix;
	spiffs_page_ix cur_objix_pix;
	int stored_scan_entry_index;
	u8_t obj_id_found;
} spiffs_gc;

// Empties given block by moving all data into free pages of another block
// Strategy:
//   loop:
//   scan object lookup for object data pages
//   for first found id, check spix and load corresponding object index page to memory
//   push object scan lookup entry index
//     rescan object lookup, find data pages with same id and referenced by same object index
//     move data page, update object index in memory
//     when reached end of lookup, store updated object index
//   pop object scan lookup entry index
//   repeat loop until end of object lookup
//   scan object lookup again for remaining object index pages, move to new page in other block
//
s32_t spiffs_gc_clean(spiffs *fs, spiffs_block_ix bix)
{
	s32_t res = SPIFFS_OK;
	u32_t entries_per_page = (SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(spiffs_obj_id));
	int cur_entry = 0;
	spiffs_obj_id *obj_lu_buf = (spiffs_obj_id *)fs->lu_work;
	spiffs_gc gc;
	spiffs_page_ix cur_pix = 0;
	spiffs_page_object_ix_header *objix_hdr = (spiffs_page_object_ix_header *)fs->work;
	spiffs_page_object_ix *objix = (spiffs_page_object_ix *)fs->work;

	SPIFFS_GC_DBG("gc_clean: cleaning block %i\n", bix);

	memset(&gc, 0, sizeof(spiffs_gc));
	gc.state = FIND_OBJ_DATA;

	if (fs->free_cursor_block_ix == bix) {
		// move free cursor to next block, cannot use free pages from the block we want to clean
		fs->free_cursor_block_ix = (bix + 1) % fs->block_count;
		fs->free_cursor_obj_lu_entry = 0;
		SPIFFS_GC_DBG("gc_clean: move free cursor to block %i\n", fs->free_cursor_block_ix);
	}

	while (res == SPIFFS_OK && gc.state != FINISHED) {
		SPIFFS_GC_DBG("gc_clean: state = %i entry:%i\n", gc.state, cur_entry);
		gc.obj_id_found = 0;

		// scan through lookup pages
		int obj_lookup_page = cur_entry / entries_per_page;
		u8_t scan = 1;
		// check each object lookup page
		while (scan && res == SPIFFS_OK && obj_lookup_page < SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
			int entry_offset = obj_lookup_page * entries_per_page;
			res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_READ, 0,
					 bix * SPIFFS_CFG_LOG_BLOCK_SZ(fs) +
						 SPIFFS_PAGE_TO_PADDR(fs, obj_lookup_page),
					 SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->lu_work);
			// check each entry
			while (scan && res == SPIFFS_OK &&
			       cur_entry - entry_offset < entries_per_page &&
			       cur_entry <
				       SPIFFS_PAGES_PER_BLOCK(fs) - SPIFFS_OBJ_LOOKUP_PAGES(fs)) {
				spiffs_obj_id obj_id = obj_lu_buf[cur_entry - entry_offset];
				cur_pix = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, bix, cur_entry);

				// act upon object id depending on gc state
				switch (gc.state) {
				case FIND_OBJ_DATA:
					if (obj_id != SPIFFS_OBJ_ID_DELETED &&
					    obj_id != SPIFFS_OBJ_ID_FREE &&
					    ((obj_id & SPIFFS_OBJ_ID_IX_FLAG) == 0)) {
						SPIFFS_GC_DBG(
							"gc_clean: FIND_DATA state:%i - found obj id %04x\n",
							gc.state, obj_id);
						gc.obj_id_found = 1;
						gc.cur_obj_id = obj_id;
						scan = 0;
					}
					break;
				case MOVE_OBJ_DATA:
					if (obj_id == gc.cur_obj_id) {
						spiffs_page_header p_hdr;
						res = _spiffs_rd(
							fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
							0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix),
							sizeof(spiffs_page_header), (u8_t *)&p_hdr);
						SPIFFS_CHECK_RES(res);
						SPIFFS_GC_DBG(
							"gc_clean: MOVE_DATA found data page %04x:%04x @ %04x\n",
							gc.cur_obj_id, p_hdr.span_ix, cur_pix);
						if (SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs,
										p_hdr.span_ix) !=
						    gc.cur_objix_spix) {
							SPIFFS_GC_DBG(
								"gc_clean: MOVE_DATA no objix spix match, take in another run\n");
						} else {
							spiffs_page_ix new_data_pix;
							if (p_hdr.flags & SPIFFS_PH_FLAG_DELET) {
								// move page
								res = spiffs_page_move(
									fs, 0, 0, obj_id, &p_hdr,
									cur_pix, &new_data_pix);
								SPIFFS_GC_DBG(
									"gc_clean: MOVE_DATA move objix %04x:%04x page %04x to %04x\n",
									gc.cur_obj_id,
									p_hdr.span_ix, cur_pix,
									new_data_pix);
								SPIFFS_CHECK_RES(res);
								// move wipes obj_lu, reload it
								res = _spiffs_rd(
									fs,
									SPIFFS_OP_T_OBJ_LU |
										SPIFFS_OP_C_READ,
									0,
									bix * SPIFFS_CFG_LOG_BLOCK_SZ(
										      fs) +
										SPIFFS_PAGE_TO_PADDR(
											fs,
											obj_lookup_page),
									SPIFFS_CFG_LOG_PAGE_SZ(fs),
									fs->lu_work);
								SPIFFS_CHECK_RES(res);
							} else {
								// page is deleted but not deleted in lookup, scrap it
								SPIFFS_GC_DBG(
									"gc_clean: MOVE_DATA wipe objix %04x:%04x page %04x\n",
									obj_id, p_hdr.span_ix,
									cur_pix);
								res = spiffs_page_delete(fs,
											 cur_pix);
								SPIFFS_CHECK_RES(res);
								new_data_pix = SPIFFS_OBJ_ID_FREE;
							}
							// update memory representation of object index page with new data page
							if (gc.cur_objix_spix == 0) {
								// update object index header page
								((spiffs_page_ix
									  *)((void *)objix_hdr +
									     sizeof(spiffs_page_object_ix_header)))
									[p_hdr.span_ix] =
										new_data_pix;
								SPIFFS_GC_DBG(
									"gc_clean: MOVE_DATA wrote page %04x to objix_hdr entry %02x in mem\n",
									new_data_pix,
									SPIFFS_OBJ_IX_ENTRY(
										fs, p_hdr.span_ix));
							} else {
								// update object index page
								((spiffs_page_ix
									  *)((void *)objix +
									     sizeof(spiffs_page_object_ix)))
									[SPIFFS_OBJ_IX_ENTRY(
										fs, p_hdr.span_ix)] =
										new_data_pix;
								SPIFFS_GC_DBG(
									"gc_clean: MOVE_DATA wrote page %04x to objix entry %02x in mem\n",
									new_data_pix,
									SPIFFS_OBJ_IX_ENTRY(
										fs, p_hdr.span_ix));
							}
						}
					}
					break;
				case MOVE_OBJ_IX:
					if (obj_id != SPIFFS_OBJ_ID_DELETED &&
					    obj_id != SPIFFS_OBJ_ID_FREE &&
					    (obj_id & SPIFFS_OBJ_ID_IX_FLAG)) {
						// found an index object id
						spiffs_page_header p_hdr;
						spiffs_page_ix new_pix;
						// load header
						res = _spiffs_rd(
							fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
							0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix),
							sizeof(spiffs_page_header), (u8_t *)&p_hdr);
						SPIFFS_CHECK_RES(res);
						if (p_hdr.flags & SPIFFS_PH_FLAG_DELET) {
							// move page
							res = spiffs_page_move(fs, 0, 0, obj_id,
									       &p_hdr, cur_pix,
									       &new_pix);
							SPIFFS_GC_DBG(
								"gc_clean: MOVE_OBJIX move objix %04x:%04x page %04x to %04x\n",
								obj_id, p_hdr.span_ix, cur_pix,
								new_pix);
							SPIFFS_CHECK_RES(res);
							spiffs_cb_object_event(
								fs, 0, SPIFFS_EV_IX_UPD, obj_id,
								p_hdr.span_ix, new_pix, 0);
							// move wipes obj_lu, reload it
							res = _spiffs_rd(
								fs,
								SPIFFS_OP_T_OBJ_LU |
									SPIFFS_OP_C_READ,
								0,
								bix * SPIFFS_CFG_LOG_BLOCK_SZ(fs) +
									SPIFFS_PAGE_TO_PADDR(
										fs,
										obj_lookup_page),
								SPIFFS_CFG_LOG_PAGE_SZ(fs),
								fs->lu_work);
							SPIFFS_CHECK_RES(res);
						} else {
							// page is deleted but not deleted in lookup, scrap it
							SPIFFS_GC_DBG(
								"gc_clean: MOVE_OBJIX wipe objix %04x:%04x page %04x\n",
								obj_id, p_hdr.span_ix, cur_pix);
							res = spiffs_page_delete(fs, cur_pix);
							if (res == SPIFFS_OK) {
								spiffs_cb_object_event(
									fs, 0, SPIFFS_EV_IX_DEL,
									obj_id, p_hdr.span_ix,
									cur_pix, 0);
							}
						}
						SPIFFS_CHECK_RES(res);
					}
					break;
				default:
					scan = 0;
					break;
				}
				cur_entry++;
			} // per entry
			obj_lookup_page++;
		} // per object lookup page

		if (res != SPIFFS_OK)
			break;

		// state finalization and switch
		switch (gc.state) {
		case FIND_OBJ_DATA:
			if (gc.obj_id_found) {
				// find out corresponding obj ix page and load it to memory
				spiffs_page_header p_hdr;
				spiffs_page_ix objix_pix;
				gc.stored_scan_entry_index = cur_entry;
				cur_entry = 0;
				gc.state = MOVE_OBJ_DATA;
				res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ, 0,
						 SPIFFS_PAGE_TO_PADDR(fs, cur_pix),
						 sizeof(spiffs_page_header), (u8_t *)&p_hdr);
				SPIFFS_CHECK_RES(res);
				gc.cur_objix_spix = SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs, p_hdr.span_ix);
				SPIFFS_GC_DBG("gc_clean: FIND_DATA find objix span_ix:%04x\n",
					      gc.cur_objix_spix);
				res = spiffs_obj_lu_find_id_and_span(
					fs, gc.cur_obj_id | SPIFFS_OBJ_ID_IX_FLAG,
					gc.cur_objix_spix, 0, &objix_pix);
				SPIFFS_CHECK_RES(res);
				SPIFFS_GC_DBG(
					"gc_clean: FIND_DATA found object index at page %04x\n",
					objix_pix);
				res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ, 0,
						 SPIFFS_PAGE_TO_PADDR(fs, objix_pix),
						 SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
				SPIFFS_CHECK_RES(res);
				SPIFFS_VALIDATE_OBJIX(objix->p_hdr,
						      gc.cur_obj_id | SPIFFS_OBJ_ID_IX_FLAG,
						      gc.cur_objix_spix);
				gc.cur_objix_pix = objix_pix;
			} else {
				gc.state = MOVE_OBJ_IX;
				cur_entry = 0; // restart entry scan index
			}
			break;
		case MOVE_OBJ_DATA: {
			// store modified objix (hdr) page
			spiffs_page_ix new_objix_pix;
			gc.state = FIND_OBJ_DATA;
			cur_entry = gc.stored_scan_entry_index;
			if (gc.cur_objix_spix == 0) {
				// store object index header page
				res = spiffs_object_update_index_hdr(
					fs, 0, gc.cur_obj_id | SPIFFS_OBJ_ID_IX_FLAG,
					gc.cur_objix_pix, fs->work, 0, 0, &new_objix_pix);
				SPIFFS_GC_DBG(
					"gc_clean: MOVE_DATA store modified objix_hdr page, %04x:%04x\n",
					new_objix_pix, 0);
				SPIFFS_CHECK_RES(res);
			} else {
				// store object index page
				spiffs_page_ix new_objix_pix;
				res = spiffs_page_move(fs, 0, fs->work,
						       gc.cur_obj_id | SPIFFS_OBJ_ID_IX_FLAG, 0,
						       gc.cur_objix_pix, &new_objix_pix);
				SPIFFS_GC_DBG(
					"gc_clean: MOVE_DATA store modified objix page, %04x:%04x\n",
					new_objix_pix, objix->p_hdr.span_ix);
				SPIFFS_CHECK_RES(res);
				spiffs_cb_object_event(fs, 0, SPIFFS_EV_IX_UPD, gc.cur_obj_id,
						       objix->p_hdr.span_ix, new_objix_pix, 0);
			}
		} break;
		case MOVE_OBJ_IX:
			gc.state = FINISHED;
			break;
		default:
			cur_entry = 0;
			break;
		}
		SPIFFS_GC_DBG("gc_clean: state-> %i\n", gc.state);
	} // while state != FINISHED

	return res;
}
