#include "usrlib.h"

void *memset(void *destination, int32_t c, uint64_t length)
{
	uint8_t chr = (uint8_t)c;
	char   *dst = (char *)destination;

	while (length--)
		dst[length] = chr;

	return destination;
}

void *memcpy(void *destination, const void *source, uint64_t length)
{
	uint64_t       i = 0;
	uint8_t       *d = (uint8_t *)destination;
	const uint8_t *s = (const uint8_t *)source;

	// Copy byte by byte until aligned to 8 bytes
	while (i < length && ((uint64_t)(d + i) % sizeof(uint64_t) != 0 ||
	                      (uint64_t)(s + i) % sizeof(uint64_t) != 0)) {
		d[i] = s[i];
		i++;
	}

	// Copy 8 bytes at a time when aligned
	uint64_t       *d64           = (uint64_t *)(d + i);
	const uint64_t *s64           = (const uint64_t *)(s + i);
	uint64_t        remaining     = length - i;
	uint64_t        aligned_count = remaining / sizeof(uint64_t);

	for (uint64_t j = 0; j < aligned_count; j++) {
		d64[j] = s64[j];
	}

	i += aligned_count * sizeof(uint64_t);

	// Copy remaining bytes
	while (i < length) {
		d[i] = s[i];
		i++;
	}

	return destination;
}

void *memset64(void *destination, uint64_t pattern, uint64_t length)
{
	uint8_t *d = (uint8_t *)destination;
	uint64_t i = 0;

	// Write bytes until destination is 8-byte aligned or until no bytes left
	while (i < length && ((uint64_t)(d + i) % sizeof(uint64_t) != 0)) {
		d[i++] = (uint8_t)pattern; // Use LSB of pattern for tail bytes
	}

	// Now destination is aligned or no more bytes left
	uint64_t remaining = length - i;
	uint64_t words     = remaining / sizeof(uint64_t);

	uint64_t *d64 = (uint64_t *)(d + i);
	for (uint64_t j = 0; j < words; j++) {
		d64[j] = pattern;
	}

	i += words * sizeof(uint64_t);

	// Trailing bytes
	while (i < length) {
		d[i++] = (uint8_t)pattern;
	}

	return destination;
}