///////////////////////////////////////////////////////////////////////////////
//
/// \file       filter_decoder.c
/// \brief      Filter ID mapping to filter-specific functions
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////

#include "filter_decoder.h"
#include "filter_common.h"
#include "lzma_decoder.h"
#include "lzma2_decoder.h"


typedef struct {
	/// Filter ID
	lzma_vli id;

	/// Initializes the filter encoder and calls lzma_next_filter_init()
	/// for filters + 1.
	lzma_init_function init;

	/// Calculates memory usage of the encoder. If the options are
	/// invalid, UINT64_MAX is returned.
	uint64_t (*memusage)(const void *options);

	/// Decodes Filter Properties.
	///
	/// \return     - LZMA_OK: Properties decoded successfully.
	///             - LZMA_OPTIONS_ERROR: Unsupported properties
	///             - LZMA_MEM_ERROR: Memory allocation failed.
	lzma_ret (*props_decode)(
			void **options, const lzma_allocator *allocator,
			const uint8_t *props, size_t props_size);

} lzma_filter_decoder;


static const lzma_filter_decoder decoders[] = {
	{
		.id = LZMA_FILTER_LZMA1,
		.init = &lzma_lzma_decoder_init,
		.memusage = &lzma_lzma_decoder_memusage,
		.props_decode = &lzma_lzma_props_decode,
	},
	{
		.id = LZMA_FILTER_LZMA2,
		.init = &lzma_lzma2_decoder_init,
		.memusage = &lzma_lzma2_decoder_memusage,
		.props_decode = &lzma_lzma2_props_decode,
	},
};


static const lzma_filter_decoder *
decoder_find(lzma_vli id)
{
	for (size_t i = 0; i < ARRAY_SIZE(decoders); ++i)
		if (decoders[i].id == id)
			return decoders + i;

	return NULL;
}


extern lzma_ret
lzma_raw_decoder_init(lzma_next_coder *next, const lzma_allocator *allocator,
		const lzma_filter *options)
{
	return lzma_raw_coder_init(next, allocator,
			options, (lzma_filter_find)(&decoder_find), false);
}


extern LZMA_API(lzma_ret)
lzma_raw_decoder(lzma_stream *strm, const lzma_filter *options)
{
	lzma_next_strm_init(lzma_raw_decoder_init, strm, options);

	strm->internal->supported_actions[LZMA_RUN] = true;
	strm->internal->supported_actions[LZMA_FINISH] = true;

	return LZMA_OK;
}


extern LZMA_API(lzma_ret)
lzma_properties_decode(lzma_filter *filter, const lzma_allocator *allocator,
		const uint8_t *props, size_t props_size)
{
	// Make it always NULL so that the caller can always safely free() it.
	filter->options = NULL;

	const lzma_filter_decoder *const fd = decoder_find(filter->id);
	if (fd == NULL)
		return LZMA_OPTIONS_ERROR;

	if (fd->props_decode == NULL)
		return props_size == 0 ? LZMA_OK : LZMA_OPTIONS_ERROR;

	return fd->props_decode(
			&filter->options, allocator, props, props_size);
}
