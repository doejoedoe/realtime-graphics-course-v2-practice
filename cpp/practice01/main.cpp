#include <wgpu_app.hpp>
#include <file_utils.hpp>

#include <webgpu.h>

#include <exception>
#include <filesystem>
#include <iostream>

static std::filesystem::path const projectRoot = PROJECT_ROOT;

int main() try {
    WgpuApp app("Practice01", 1280, 720, false);

    std::string shaderFile = loadFile(projectRoot / "shaders/shader.wgsl");
    WGPUShaderSourceWGSL shaderSource = WGPU_SHADER_SOURCE_WGSL_INIT;
    shaderSource.code = WGPUStringView{shaderFile.c_str(), shaderFile.size()};
    WGPUShaderModuleDescriptor shaderModDesc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    shaderModDesc.nextInChain = &shaderSource.chain;
    WGPUShaderModule shaderMod = wgpuDeviceCreateShaderModule(app.device(), &shaderModDesc);

    WGPUVertexState vertState = WGPU_VERTEX_STATE_INIT;
    vertState.module = shaderMod;
    vertState.entryPoint = WGPUStringView{"vertexMain", WGPU_STRLEN};

    WGPUColorTargetState colorState = WGPU_COLOR_TARGET_STATE_INIT;
    colorState.format = app.surfaceFormat();
    colorState.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragState = WGPU_FRAGMENT_STATE_INIT;
    fragState.module = shaderMod;
    fragState.entryPoint = WGPUStringView{"fragmentMain", WGPU_STRLEN};
    fragState.targetCount = 1;
    fragState.targets = &colorState;

    WGPURenderPipelineDescriptor pipeDesc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    pipeDesc.vertex = vertState;
    pipeDesc.fragment = &fragState;
    pipeDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(app.device(), &pipeDesc);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                app.resize(event.window.data1, event.window.data2);
                break;
            }
        }

        std::optional<WGPUSurfaceTexture> surfaceTexture = app.beginFrame();
        if (!surfaceTexture)
            continue;

        WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture->texture, nullptr);

        // Frame rendering code goes here
        WGPUCommandEncoderDescriptor encDesc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
        WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder(app.device(), &encDesc);
        
        WGPURenderPassColorAttachment passAtt = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
        passAtt.view = targetView;
        passAtt.loadOp = WGPULoadOp_Clear;
        passAtt.storeOp = WGPUStoreOp_Store;
        passAtt.clearValue = WGPUColor{0.5, 0.6, 0.7, 1.0};

        WGPURenderPassDescriptor passDesc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
        passDesc.colorAttachmentCount = 1;
        passDesc.colorAttachments = &passAtt;

        WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(enc, &passDesc);
        wgpuRenderPassEncoderSetPipeline(pass, pipeline);
        wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);
        wgpuRenderPassEncoderEnd(pass);

        WGPUCommandBufferDescriptor bufDesc = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
        WGPUCommandBuffer buf = wgpuCommandEncoderFinish(enc, &bufDesc);

        wgpuQueueSubmit(app.queue(), 1, &buf);
        
        wgpuRenderPassEncoderRelease(pass);
        wgpuCommandBufferRelease(buf);
        wgpuCommandEncoderRelease(enc);

        wgpuTextureViewRelease(targetView);

        wgpuSurfacePresent(app.surface());
        wgpuTextureRelease(surfaceTexture->texture);
    }
    wgpuShaderModuleRelease(shaderMod);
    wgpuRenderPipelineRelease(pipeline);
} catch (const std::exception & e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
