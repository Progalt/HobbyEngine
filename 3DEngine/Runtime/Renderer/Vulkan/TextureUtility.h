#pragma once


#include <vulkan/vulkan.h>
#include "Texture.h"

static VkImageAspectFlags get_aspect_mask(const vk::Texture* texture, const bool only_depth = false, const bool only_stencil = false)
{
    VkImageAspectFlags aspect_mask = 0;

    if (texture->IsColourFormat())
    {
        aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    else
    {
        if (texture->IsDepthFormat() && !only_stencil)
        {
            aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        if (texture->IsStencilFormat() && !only_depth)
        {
            aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    return aspect_mask;
}



static VkPipelineStageFlags GetAccessMaskFromLayout(const VkImageLayout layout, const bool is_destination_mask)
{
    VkPipelineStageFlags accessMask = 0;

    switch (layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        accessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        accessMask = VK_ACCESS_MEMORY_READ_BIT;
        break;

        // Transfer
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        accessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        accessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

        // Color attachments
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        accessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

        // Depth attachments
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
        accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        break;

        // Shader reads
    case VK_IMAGE_LAYOUT_GENERAL:
        accessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        accessMask = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        accessMask = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        accessMask = VK_ACCESS_SHADER_READ_BIT;
        break;

    default:
        break;
    }

    return accessMask;
}

static VkPipelineStageFlags AccessFlagsToPipelineStage(VkAccessFlags access_flags)
{
    VkPipelineStageFlags stages = 0;
    uint32_t gfxStages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    while (access_flags != 0)
    {
        VkAccessFlagBits accessFlag = static_cast<VkAccessFlagBits>(access_flags & (~(access_flags - 1)));
        access_flags &= ~accessFlag;

        switch (accessFlag)
        {
        case VK_ACCESS_INDIRECT_COMMAND_READ_BIT:
            stages |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
            break;

        case VK_ACCESS_INDEX_READ_BIT:
            stages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            break;

        case VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT:
            stages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            break;

        case VK_ACCESS_UNIFORM_READ_BIT:
            stages |= gfxStages | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;

        case VK_ACCESS_INPUT_ATTACHMENT_READ_BIT:
            stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;

            // Shader
        case VK_ACCESS_SHADER_READ_BIT:
            stages |= gfxStages | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;

        case VK_ACCESS_SHADER_WRITE_BIT:
            stages |= gfxStages | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;

            // Color attachments
        case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT:
            stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;

        case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:
            stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;

            // Depth stencil attachments
        case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
            stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;

        case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
            stages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;

            // Transfer
        case VK_ACCESS_TRANSFER_READ_BIT:
            stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;

        case VK_ACCESS_TRANSFER_WRITE_BIT:
            stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;

            // Host
        case VK_ACCESS_HOST_READ_BIT:
            stages |= VK_PIPELINE_STAGE_HOST_BIT;
            break;

        case VK_ACCESS_HOST_WRITE_BIT:
            stages |= VK_PIPELINE_STAGE_HOST_BIT;
            break;
        }
    }
    return stages;
}


inline void SetLayout(VkCommandBuffer cmdBuffer, VkImage image, const VkImageAspectFlags aspect_mask, const uint32_t mip_index, const uint32_t mip_range, const uint32_t array_length, const VkImageLayout layout_old, const VkImageLayout layout_new)
{
    VkImageMemoryBarrier imgBarrier = {};
    imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgBarrier.pNext = nullptr;
    imgBarrier.oldLayout = static_cast<VkImageLayout>(layout_old);
    imgBarrier.newLayout = static_cast<VkImageLayout>(layout_new);
    imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBarrier.image = image;
    imgBarrier.subresourceRange.aspectMask = aspect_mask;
    imgBarrier.subresourceRange.baseMipLevel = mip_index;
    imgBarrier.subresourceRange.levelCount = mip_range;
    imgBarrier.subresourceRange.baseArrayLayer = 0;
    imgBarrier.subresourceRange.layerCount = array_length;
    imgBarrier.srcAccessMask = GetAccessMaskFromLayout(imgBarrier.oldLayout, false);
    imgBarrier.dstAccessMask = GetAccessMaskFromLayout(imgBarrier.newLayout, true);

    VkPipelineStageFlags sourceStageMask = 0;
    {
        if (imgBarrier.oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            sourceStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        else if (imgBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
        {
            sourceStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
        else
        {
            sourceStageMask = AccessFlagsToPipelineStage(imgBarrier.srcAccessMask);
        }
    }

    VkPipelineStageFlags destStageMask = 0;
    {
        if (imgBarrier.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            destStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
        else
        {
            destStageMask = AccessFlagsToPipelineStage(imgBarrier.dstAccessMask);
        }
    }

    vkCmdPipelineBarrier
    (
       cmdBuffer, 
        sourceStageMask,                       
        destStageMask,                  
        0,                                        
        0,                                        
        nullptr,                                  
        0,                                       
        nullptr,                                 
        1,                                        
        &imgBarrier                            
    );
}

static void SetLayout(VkCommandBuffer cmd_buffer, vk::Texture* texture, const uint32_t mip_start, const uint32_t mip_range, const uint32_t array_length, const VkImageLayout layout_old, const VkImageLayout layout_new)
{
    SetLayout(cmd_buffer, texture->GetImage(), get_aspect_mask(texture), mip_start, mip_range, array_length, layout_old, layout_new);
}