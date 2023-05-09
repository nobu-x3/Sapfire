#include <stdio.h>
#include <string.h>

#include "core/logger.h"
#include "core/sfstring.h"
#include "platform/platform.h"
#include "sfmemory.h"

struct mem_stats {
		u64 total;
		u64 tagged[MEMORY_TAG_MAX];
};

static const char *tagged_strings[MEMORY_TAG_MAX] = {
	"UNKNOWN:    ", "LIN_ALLOC:  ", "GAME:       ",
	"VECTOR:     ", "RENDERER:   ", "STRING:     "};
static struct mem_stats stats;

void memory_initialize() {
		platform_set_memory(&stats, 0, sizeof(stats));
		SF_INFO("Memory subsystem initialized successfully.");
}

void memory_shutdown() { SF_DEBUG(get_mem_usage_str()); }

void *sfalloc(u64 size, memory_tag tag) {
		if (tag == MEMORY_TAG_UNKNOWN) {
				SF_WARNING("sfalloc called with MEMORY_TAG_UNKNOWN");
		}
		stats.total += size;
		stats.tagged[tag] += size;
		void *block = platform_allocate(size, FALSE);
		platform_set_memory(block, 0, size);
		return block;
}

void sffree(void *block, u64 size, memory_tag tag) {
		if (tag == MEMORY_TAG_UNKNOWN) {
				SF_WARNING("sffree called with MEMORY_TAG_UNKNOWN");
		}
		stats.total -= size;
		stats.tagged[tag] -= size;
		platform_free(block, FALSE);
		block = SF_NULL;
}

void *sfmemcpy(void *dest, const void *src, u64 size) {
		return platform_copy_memory(dest, src, size);
}

void *sfmemset(void *dest, i32 val, u64 size) {
		return platform_set_memory(dest, val, size);
}

char *get_mem_usage_str() {
		const u64 gib = 1024 * 1024 * 1024;
		const u64 mib = 1024 * 1024;
		const u64 kib = 1024;

		char buffer[8000] = "Tagged memory usage:\n";
		u64 offset = sfstrlen(buffer);
		for (u16 i = 0; i < MEMORY_TAG_MAX; ++i) {
				char unit[4] = "XiB";
				float amount = 1.0f;
				if (stats.tagged[i] >= gib) {
						unit[0] = 'G';
						amount = stats.tagged[i] / (float)gib;
				} else if (stats.tagged[i] >= mib) {
						unit[0] = 'M';
						amount = stats.tagged[i] / (float)mib;
				} else if (stats.tagged[i] > kib) {
						unit[0] = 'K';
						amount = stats.tagged[i] / (float)kib;
				} else {
						unit[0] = 'B';
						unit[1] = 0;
						amount = (float)stats.tagged[i];
				}
				offset += snprintf(buffer + offset, 8000, "  %s: %.2f%s\n",
								   tagged_strings[i], amount, unit);
		}
		char *out_string = sfstrdup(buffer);
		return out_string;
}