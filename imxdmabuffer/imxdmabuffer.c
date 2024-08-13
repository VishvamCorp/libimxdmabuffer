#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <imxdmabuffer_config.h>
#include "imxdmabuffer.h"
#include "imxdmabuffer_priv.h"

#ifdef IMXDMABUFFER_DMA_HEAP_ALLOCATOR_ENABLED
#include "imxdmabuffer_dma_heap_allocator.h"
#endif

#ifdef IMXDMABUFFER_ION_ALLOCATOR_ENABLED
#include "imxdmabuffer_ion_allocator.h"
#endif

#ifdef IMXDMABUFFER_DWL_ALLOCATOR_ENABLED
#include "imxdmabuffer_dwl_allocator.h"
#endif

#ifdef IMXDMABUFFER_IPU_ALLOCATOR_ENABLED
#include "imxdmabuffer_ipu_allocator.h"
#endif

#ifdef IMXDMABUFFER_G2D_ALLOCATOR_ENABLED
#include "imxdmabuffer_g2d_allocator.h"
#endif

#ifdef IMXDMABUFFER_PXP_ALLOCATOR_ENABLED
#include "imxdmabuffer_pxp_allocator.h"
#endif

static struct ImxAllocatorStats _imx_dma_buffer_allocator_get_stats(ImxDmaBufferAllocator * allocator);

ImxDmaBufferAllocator* imx_dma_buffer_allocator_new(ImxDmaBufferType_t type, int *error)
{
	ImxDmaBufferAllocator* allocator = NULL;

    switch (type) {
        case IMX_DMA_BUFFER_TYPE_DEFAULT:
#ifdef IMXDMABUFFER_DMA_HEAP_ALLOCATOR_ENABLED
        case IMX_DMA_BUFFER_TYPE_DMA_HEAP:
            allocator = imx_dma_buffer_dma_heap_allocator_new(
                -1,
                IMX_DMA_BUFFER_DMA_HEAP_ALLOCATOR_DEFAULT_HEAP_FLAGS,
                IMX_DMA_BUFFER_DMA_HEAP_ALLOCATOR_DEFAULT_FD_FLAGS,
                error
            );
            break;
#endif
#ifdef IMXDMABUFFER_ION_ALLOCATOR_ENABLED
        case IMX_DMA_BUFFER_TYPE_ION:
            allocator = imx_dma_buffer_ion_allocator_new(
                IMX_DMA_BUFFER_ION_ALLOCATOR_DEFAULT_ION_FD,
                IMX_DMA_BUFFER_ION_ALLOCATOR_DEFAULT_HEAP_ID_MASK,
                IMX_DMA_BUFFER_ION_ALLOCATOR_DEFAULT_HEAP_FLAGS,
                error
            );
            break;
#endif
#ifdef IMXDMABUFFER_DWL_ALLOCATOR_ENABLED
        case IMX_DMA_BUFFER_TYPE_DWL:
            allocator = imx_dma_buffer_dwl_allocator_new(error);
            break;
#endif
#ifdef IMXDMABUFFER_IPU_ALLOCATOR_ENABLED
        case IMX_DMA_BUFFER_TYPE_IPU:
        	allocator = imx_dma_buffer_ipu_allocator_new(IMX_DMA_BUFFER_IPU_ALLOCATOR_DEFAULT_IPU_FD, error);
            break;
#endif
#ifdef IMXDMABUFFER_G2D_ALLOCATOR_ENABLED
        case IMX_DMA_BUFFER_TYPE_G2D:
        	allocator = imx_dma_buffer_g2d_allocator_new();
            break;
#endif
#ifdef IMXDMABUFFER_PXP_ALLOCATOR_ENABLED
        case IMX_DMA_BUFFER_TYPE_PXP:
        	allocator = imx_dma_buffer_pxp_allocator_new(IMX_DMA_BUFFER_PXP_ALLOCATOR_DEFAULT_PXP_FD, error);
            break;
#endif

        default:
            break;
    };

#ifdef IMXDMABUFFER_ALLOC_STATS_ENABLED
    if (allocator)
	{
        allocator->stat = calloc(1, sizeof(struct ImxAllocatorStats));
        allocator->get_stats = _imx_dma_buffer_allocator_get_stats;
    }
#endif

	return allocator;
}


void imx_dma_buffer_allocator_destroy(ImxDmaBufferAllocator *allocator)
{
	assert(allocator != NULL);
	assert(allocator->destroy != NULL);

#ifdef IMXDMABUFFER_ALLOC_STATS_ENABLED
    if (allocator->stat != NULL)
	{
		free(allocator->stat);
		allocator->stat = NULL;
		allocator->get_stats = NULL;
	}
#endif

	allocator->destroy(allocator);
}

void imx_dma_buffer_allocator_free(ImxDmaBufferAllocator *allocator)
{
	if (allocator == NULL)
	{
        return;
    }

	if (allocator->stat != NULL)
	{
		free(allocator->stat);
		allocator->stat = NULL;
	}

	free(allocator);
}


ImxDmaBuffer* imx_dma_buffer_allocate(ImxDmaBufferAllocator *allocator, size_t size, size_t alignment, int *error)
{
	assert(allocator != NULL);
	assert(allocator->allocate != NULL);
	assert(size >= 1);
	return allocator->allocate(allocator, size, alignment, error);
}


void imx_dma_buffer_deallocate(ImxDmaBuffer *buffer)
{
	assert(buffer != NULL);
	assert(buffer->allocator != NULL);
	assert(buffer->allocator->deallocate != NULL);
	buffer->allocator->deallocate(buffer->allocator, buffer);
}


uint8_t* imx_dma_buffer_map(ImxDmaBuffer *buffer, unsigned int flags, int *error)
{
	assert(buffer != NULL);
	assert(buffer->allocator != NULL);
	assert(buffer->allocator->map != NULL);
	return buffer->allocator->map(buffer->allocator, buffer, flags, error);
}


void imx_dma_buffer_unmap(ImxDmaBuffer *buffer)
{
	assert(buffer != NULL);
	assert(buffer->allocator != NULL);
	assert(buffer->allocator->unmap != NULL);
	buffer->allocator->unmap(buffer->allocator, buffer);
}


void imx_dma_buffer_start_sync_session(ImxDmaBuffer *buffer)
{
	assert(buffer != NULL);
	assert(buffer->allocator != NULL);
	assert(buffer->allocator->start_sync_session != NULL);
	buffer->allocator->start_sync_session(buffer->allocator, buffer);
}


void imx_dma_buffer_stop_sync_session(ImxDmaBuffer *buffer)
{
	assert(buffer != NULL);
	assert(buffer->allocator != NULL);
	assert(buffer->allocator->stop_sync_session != NULL);
	buffer->allocator->stop_sync_session(buffer->allocator, buffer);
}


imx_physical_address_t imx_dma_buffer_get_physical_address(ImxDmaBuffer *buffer)
{
	assert(buffer != NULL);
	assert(buffer->allocator != NULL);
	assert(buffer->allocator->get_physical_address != NULL);
	return buffer->allocator->get_physical_address(buffer->allocator, buffer);
}


int imx_dma_buffer_get_fd(ImxDmaBuffer *buffer)
{
	assert(buffer != NULL);
	assert(buffer->allocator != NULL);
	return (buffer->allocator->get_fd != NULL) ? buffer->allocator->get_fd(buffer->allocator, buffer) : -1;
}


size_t imx_dma_buffer_get_size(ImxDmaBuffer *buffer)
{
	assert(buffer != NULL);
	assert(buffer->allocator != NULL);
	return buffer->allocator->get_size(buffer->allocator, buffer);
}


void imx_increment_alloc_stats(ImxDmaBufferAllocator * allocator, size_t size)
{
	if (allocator == NULL || size == 0)
        return;


	if (allocator->stat != NULL)
	{
		struct ImxAllocatorStats *stat = allocator->stat;

        stat->total_allocated += size;

        if (stat->total_allocated > stat->total_freed)
		{
            stat->current_usage = stat->total_allocated - stat->total_freed;
		} else {
			stat->current_usage = 0;
		}

		stat->alloc_cnt++;

		if (stat->current_usage > stat->peak_usage)
		{
			stat->peak_usage = stat->current_usage;
		}
	}
}


void imx_decrement_alloc_stats(ImxDmaBufferAllocator * allocator, size_t size)
{
	if (allocator == NULL || size == 0)
		return;

	if (allocator->stat != NULL)
	{
	    struct ImxAllocatorStats *stat = allocator->stat;

        stat->total_freed += size;

		if (stat->total_allocated > stat->total_freed)
		{
			stat->current_usage = stat->total_allocated - stat->total_freed;
		} else {
			stat->current_usage = 0;
		}

		stat->dealloc_cnt++;
	}
}


static ImxDmaBuffer* wrapped_dma_buffer_allocator_allocate(ImxDmaBufferAllocator *allocator, size_t size, size_t alignment, int *error)
{
	/* This allocator is used for wrapping existing DMA memory. Therefore,
	 * it doesn't actually allocate anything. This also means that the
	 * NULL return value does not actually indicate an error. This
	 * inconsistency is okay, since the allocator will never be accessible
	 * from the outside. */
	IMX_DMA_BUFFER_UNUSED_PARAM(allocator);
	IMX_DMA_BUFFER_UNUSED_PARAM(size);
	IMX_DMA_BUFFER_UNUSED_PARAM(alignment);
	IMX_DMA_BUFFER_UNUSED_PARAM(error);
	return NULL;
}


static void wrapped_dma_buffer_allocator_deallocate(ImxDmaBufferAllocator *allocator, ImxDmaBuffer *buffer)
{
	IMX_DMA_BUFFER_UNUSED_PARAM(allocator);
	IMX_DMA_BUFFER_UNUSED_PARAM(buffer);
}


static uint8_t* wrapped_dma_buffer_allocator_map(ImxDmaBufferAllocator *allocator, ImxDmaBuffer *buffer, unsigned int flags, int *error)
{
	IMX_DMA_BUFFER_UNUSED_PARAM(allocator);
	ImxWrappedDmaBuffer *wrapped_buf = (ImxWrappedDmaBuffer *)(buffer);
	return (wrapped_buf->map != NULL) ? wrapped_buf->map(wrapped_buf, flags, error) : NULL;
}


static void wrapped_dma_buffer_allocator_unmap(ImxDmaBufferAllocator *allocator, ImxDmaBuffer *buffer)
{
	IMX_DMA_BUFFER_UNUSED_PARAM(allocator);
	ImxWrappedDmaBuffer *wrapped_buf = (ImxWrappedDmaBuffer *)(buffer);
	if (wrapped_buf->unmap != NULL)
		wrapped_buf->unmap(wrapped_buf);
}


static imx_physical_address_t wrapped_dma_buffer_allocator_get_physical_address(ImxDmaBufferAllocator *allocator, ImxDmaBuffer *buffer)
{
	IMX_DMA_BUFFER_UNUSED_PARAM(allocator);
	return ((ImxWrappedDmaBuffer *)(buffer))->physical_address;
}


static int wrapped_dma_buffer_allocator_get_fd(ImxDmaBufferAllocator *allocator, ImxDmaBuffer *buffer)
{
	IMX_DMA_BUFFER_UNUSED_PARAM(allocator);
	return ((ImxWrappedDmaBuffer *)(buffer))->fd;
}


static size_t wrapped_dma_buffer_allocator_get_size(ImxDmaBufferAllocator *allocator, ImxDmaBuffer *buffer)
{
	IMX_DMA_BUFFER_UNUSED_PARAM(allocator);
	return ((ImxWrappedDmaBuffer *)(buffer))->size;
}


static ImxDmaBufferAllocator wrapped_dma_buffer_allocator =
{
	NULL, /* the wrapped allocator is static and internal, so a destroy() function makes no sense */
	wrapped_dma_buffer_allocator_allocate,
	wrapped_dma_buffer_allocator_deallocate,
	wrapped_dma_buffer_allocator_map,
	wrapped_dma_buffer_allocator_unmap,
	imx_dma_buffer_noop_start_sync_session_func,
	imx_dma_buffer_noop_stop_sync_session_func,
	wrapped_dma_buffer_allocator_get_physical_address,
	wrapped_dma_buffer_allocator_get_fd,
	wrapped_dma_buffer_allocator_get_size,
    NULL, /* no stats for wrapped allocator */
    NULL, /* no stats for wrapped allocator */
	{ 0, }
};


void imx_dma_buffer_init_wrapped_buffer(ImxWrappedDmaBuffer *buffer)
{
	memset(buffer, 0, sizeof(ImxWrappedDmaBuffer));
	buffer->parent.allocator = &wrapped_dma_buffer_allocator;
}

static struct ImxAllocatorStats _imx_dma_buffer_allocator_get_stats(ImxDmaBufferAllocator * allocator)
{
	struct ImxAllocatorStats stats = {0};

    if (allocator != NULL && allocator->stat != NULL)
	{
        memcpy(&stats, allocator->stat, sizeof(struct ImxAllocatorStats));
    }

    return stats;
}
