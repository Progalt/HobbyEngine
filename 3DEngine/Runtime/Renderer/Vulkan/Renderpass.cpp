#include "Renderpass.h"
#include <vector>
#include <array>

namespace vk
{
    void Renderpass::create(VkDevice device, VkFormat format, std::vector<AttachmentDesc> descs)
    {
        std::vector<VkAttachmentDescription> attachmentDescs;
        std::vector<VkAttachmentReference> attachmentRef;

        bool hasDepth = false;
        VkAttachmentReference depthRef;



        int i = 0;
        for (auto& x : descs)
        {


            VkAttachmentDescription attachment{};
            attachment.format = x.format;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = (x.load) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment.storeOp = (x.store) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = x.initialLayout;
            attachment.finalLayout = x.finalLayout;
            attachmentDescs.push_back(attachment);


            VkAttachmentReference ref{};
            ref.attachment = i;
            ref.layout = (x.depth) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            if (x.depth == false)
            {
                m_NumTargets++;
                attachmentRef.push_back(ref);
            }
            else
            {
                hasDepth = true;
                depthRef = ref;
            }

            i++;
        }

        /* Only 1 subpass. Subpasses are ignored by the abstraction */

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        if (attachmentRef.size() > 0)
        {
            subpass.colorAttachmentCount = attachmentRef.size();
            subpass.pColorAttachments = attachmentRef.data();
        }

        if (hasDepth)
            subpass.pDepthStencilAttachment = &depthRef;

        if (hasDepth)
            this->hasDepth = true;

        //VkSubpassDependency dependency{};

        std::array<VkSubpassDependency, 2> dependencies;


        VkRenderPassCreateInfo renderPassInfo{};
        if (attachmentRef.size() > 0)
        {
            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            renderPassInfo.dependencyCount = 2;
            renderPassInfo.pDependencies = dependencies.data();
        }
        else if (hasDepth)
        {

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            renderPassInfo.dependencyCount = 2;
            renderPassInfo.pDependencies = dependencies.data();
        }

        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachmentDescs.size();
        renderPassInfo.pAttachments = attachmentDescs.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;


        //renderPassInfo.dependencyCount = 1;
        //renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_Renderpass) != VK_SUCCESS)
        {
            throw std::exception("Failed to create renderpass");
        }

        m_Device = device;

        m_FinalLayout = (vk::ImageLayout)descs[0].finalLayout;
    }



    void Renderpass::Destroy()
    {
        for (auto& fb : m_Framebuffers)
        {
            vkDestroyFramebuffer(m_Device, fb, nullptr);
        }

        vkDestroyRenderPass(m_Device, m_Renderpass, nullptr);
    }
}